/* -*-C-*- */
/* SPDX-License-Identifier:    GPL-2.0+ */
/*
 * Copyright (C) 2020-2022 MicroSys Electronics GmbH
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

#include "mpxs32g274ar3.h"
#include "mpxs32g.h"

#include <i2c.h>
#include <s32-cc/serdes_hwconfig.h>
#include <asm/gpio.h>
#include <dm/uclass.h>
#include <hwconfig.h>
#include <net.h>

void setup_iomux_uart(void)
{
#if (CONFIG_FSL_LINFLEX_MODULE == 0)

	/* Muxing for linflex0 */
//	setup_iomux_uart0();

#elif (CONFIG_FSL_LINFLEX_MODULE == 1)
	/* Muxing for linflex1 */

	/* set PC08 - MSCR[40] - for UART1 TXD */
	writel(SIUL2_MSCR_S32G_G1_PORT_CTRL_UART1_TXD,
			SIUL2_0_MSCRn(SIUL2_PC08_MSCR_S32_G1_UART1));

	/* set PC04 - MSCR[36] - for UART1 RXD */
	writel(SIUL2_MSCR_S32G_G1_PORT_CTRL_UART_RXD,
			SIUL2_0_MSCRn(SIUL2_PC04_MSCR_S32_G1_UART1));

	/* set PC04 - MSCR[736]/IMCR[224] - for UART1 RXD */
	writel(SIUL2_IMCR_S32G_G1_UART1_RXD_to_pad,
			SIUL2_1_IMCRn(SIUL2_PC04_IMCR_S32_G1_UART1));
#else
#error "Unsupported UART pinmuxing configuration"
#endif
}

int misc_init_f(void)
{
	return 0;
}

static uchar boot_cfg = 0xff;

uint8_t get_mux_sd_sel_bit(void)
{
	return 4;
}

int set_boot_cfg(const uchar reg)
{
	struct udevice *dev = NULL;

	if (i2c_get_chip_for_busnum(0, RCW_EEPROM_ADDR, 1, &dev)==0) {
		dm_i2c_write(dev, 0x10, &reg, 1);
		boot_cfg = reg;
	}

	return 0;
}

uchar get_boot_cfg(const bool verbose)
{
	if (boot_cfg == 0xff) {
		struct udevice *dev = NULL;
		if (i2c_get_chip_for_busnum(0, RCW_EEPROM_ADDR, 1, &dev)==0) {
			dm_i2c_read(dev, 0x10, &boot_cfg, 1);
			if (verbose) {
				printf("CFG EEPROM[%d]\n", 0);
				print_boot_cfg(boot_cfg);
			}
		}
	}

	return boot_cfg;
}

uint8_t get_board_rev(void)
{
	static uchar reg = 0xff;

	if (reg == 0xff) {
		struct udevice *dev = NULL;
		if (i2c_get_chip_for_busnum(0, MCU_I2C_ADDRESS,
				1, &dev) == 0) {

			dm_i2c_set_bus_speed(dev, 100000);

			const int ret = dm_i2c_read(dev, 0x0e, &reg, 1);

			if (ret < 0)
				printf("Error: cannot read MCU %d!\n", ret);
		}
		else
			puts("Error: cannot find MCU!\n");
	}

	return reg;
}

/*!@}*/

/* *INDENT-OFF* */
/******************************************************************************
 * Local Variables:
 * mode: C
 * c-indent-level: 4
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 * kate: space-indent on; indent-width 4; mixedindent off; indent-mode cstyle;
 * vim: set expandtab filetype=c:
 * vi: set et tabstop=4 shiftwidth=4: */
