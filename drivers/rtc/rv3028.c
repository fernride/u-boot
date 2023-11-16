/* SPDX-License-Identifier:    GPL-2.0+ */
/*
 * Copyright (C) 2023 MicroSys Electronics GmbH
 * Author: Kay Potthoff <kay.potthoff@microsys.de>
 *
 */

/*!
 * \addtogroup <group> <title>
 * @{
 *
 * \file
 * <description>
 */

#include <common.h>
#include <command.h>
#include <dm.h>
#include <eeprom.h>
#include <i2c.h>
#include <rtc.h>
#include <dm/device_compat.h>

#define RV3028_SEC_REG		0x0
#define RV3028_MIN_REG		0x1

#define RV3028_HOURS_REG	0x2
#define RV3028_HOURS_PM		BIT(5)

#define RV3028_WDAY_REG		0x3
#define RV3028_DATE_REG		0x4
#define RV3028_MONTH_REG	0x5
#define RV3028_YEAR_REG		0x6

#define RV3028_CTRL2_REG	0x10
#define RV3028_CTRL2_12_24	BIT(1)
#define RV3028_CTRL2_RESET	BIT(0)

#define RV3028_STATUS_REG	0x0e
#define RV3028_STATUS_PORF      BIT(0)

#define RV3038_DATE_REGS_LEN 7

static int rv3028_rtc_read8(struct udevice *dev, unsigned int reg)
{
	u8 data;
	int ret;

	ret = dm_i2c_read(dev, reg, &data, sizeof(data));

	return ret < 0 ? ret : data;
}

static int rv3028_rtc_write8(struct udevice *dev, unsigned int reg, int val)
{
	u8 data = val;

	return dm_i2c_write(dev, reg, &data, 1);
}

static int rv3028_get_control2(struct udevice *dev)
{
	return rv3028_rtc_read8(dev, RV3028_CTRL2_REG);
}

static int rv3028_set_control2(struct udevice *dev, int val)
{
	return rv3028_rtc_write8(dev, RV3028_CTRL2_REG, val);
}

static int rv3028_get_status(struct udevice *dev)
{
	return rv3028_rtc_read8(dev, RV3028_STATUS_REG);
}

static int rv3028_set_status(struct udevice *dev, int val)
{
	return rv3028_rtc_write8(dev, RV3028_STATUS_REG, val);
}

static int rv3028_rtc_get(struct udevice *dev, struct rtc_time *tm)
{
	u8 regs[RV3038_DATE_REGS_LEN];
	int ret;

	ret = dm_i2c_read(dev, 0, regs, sizeof(regs));
	if (ret < 0) {
		printf("%s: error reading RTC: %x\n", __func__, ret);
		return -EIO;
	}

	tm->tm_sec = bcd2bin(regs[RV3028_SEC_REG]);
	tm->tm_min = bcd2bin(regs[RV3028_MIN_REG]);

	{
		const int status = rv3028_get_control2(dev);
		if (status < 0)
			return -EIO;

		const u8 hours = regs[RV3028_HOURS_REG];

		if (status & RV3028_CTRL2_12_24) {
			tm->tm_hour = bcd2bin(hours & 0x1f);
			if (hours & RV3028_HOURS_PM)
				tm->tm_hour += 12;
		} else {
			tm->tm_hour = bcd2bin(hours & 0x3f);
		}
	}

	tm->tm_mday = bcd2bin(regs[RV3028_DATE_REG]);
	tm->tm_mon = bcd2bin(regs[RV3028_MONTH_REG]);
	tm->tm_year = bcd2bin(regs[RV3028_YEAR_REG]) + 2000;
	tm->tm_wday = bcd2bin(regs[RV3028_WDAY_REG]) - 1;

	tm->tm_yday = 0;
	tm->tm_isdst = 0;

	debug("%s: %4d-%02d-%02d (wday=%d) %2d:%02d:%02d\n",
	      __func__, tm->tm_year, tm->tm_mon, tm->tm_mday,
	      tm->tm_wday, tm->tm_hour, tm->tm_min, tm->tm_sec);

	return 0;
}

static int rv3028_rtc_set(struct udevice *dev, const struct rtc_time *tm)
{
	u8 regs[RV3038_DATE_REGS_LEN];
	int ret, status;

	debug("%s: %4d-%02d-%02d (wday=%d( %2d:%02d:%02d\n",
	      __func__, tm->tm_year, tm->tm_mon, tm->tm_mday,
	      tm->tm_wday, tm->tm_hour, tm->tm_min, tm->tm_sec);

	if (tm->tm_year < 2000) {
		printf("%s: year %d (before 2000) not supported\n",
		       __func__, tm->tm_year);
		return -EINVAL;
	}

	regs[RV3028_SEC_REG] = bin2bcd(tm->tm_sec);
	regs[RV3028_MIN_REG] = bin2bcd(tm->tm_min);
	regs[RV3028_HOURS_REG] = bin2bcd(tm->tm_hour);
	regs[RV3028_WDAY_REG] = bin2bcd(tm->tm_wday + 1) & 0x7;
	regs[RV3028_DATE_REG] = bin2bcd(tm->tm_mday);
	regs[RV3028_MONTH_REG] = bin2bcd(tm->tm_mon);
	regs[RV3028_YEAR_REG] = bin2bcd(tm->tm_year - 2000);

	ret = dm_i2c_write(dev, 0, regs, sizeof(regs));
	if (ret < 0)
		return ret;

	status = rv3028_get_status(dev);
	status &= ~RV3028_STATUS_PORF;
	rv3028_set_status(dev, status);

	return ret;
}

static int rv3028_rtc_reset(struct udevice *dev)
{
	int ctrl2 = rv3028_get_control2(dev);

	if (ctrl2 < 0)
		return ctrl2;

	ctrl2 |= RV3028_CTRL2_RESET;

	return rv3028_set_control2(dev, ctrl2);
}

static int rv3028_probe(struct udevice *dev)
{
	i2c_set_chip_flags(dev, DM_I2C_CHIP_RD_ADDRESS |
				DM_I2C_CHIP_WR_ADDRESS);

	return 0;
}

static const struct rtc_ops rv3028_rtc_ops = {
	.get = rv3028_rtc_get,
	.set = rv3028_rtc_set,
	.read8 = rv3028_rtc_read8,
	.write8 = rv3028_rtc_write8,
	.reset = rv3028_rtc_reset,
};

static const struct udevice_id rv3028_rtc_ids[] = {
	{ .compatible = "mc,rv3028" },
	{ .compatible = "mc,rv3028c7" },
	{ }
};

U_BOOT_DRIVER(rtc_rv3028) = {
	.name	= "rtc-rv3028",
	.id	= UCLASS_RTC,
	.probe	= rv3028_probe,
	.of_match = rv3028_rtc_ids,
	.ops	= &rv3028_rtc_ops,
};

/*!@}*/
