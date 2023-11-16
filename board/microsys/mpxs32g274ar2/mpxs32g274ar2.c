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

#include "mpxs32g274ar2.h"
#include "mpxs32g.h"

#include <i2c.h>
#include <s32-cc/serdes_hwconfig.h>
#include <asm/gpio.h>
#include <dm/uclass.h>
#include <hwconfig.h>
#include <net.h>

#include "pfeng/pfeng.h"

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

static uchar eeprom_dip = 0xff;

int set_boot_cfg(const uchar reg)
{
	struct udevice *dev = NULL;
	if (i2c_get_chip_for_busnum(0, 0x4d, 1, &dev)==0) {
		dm_i2c_write(dev, 0, &reg, 1);
		eeprom_dip = reg;
	}

	return 0;
}

uchar get_boot_cfg(const bool verbose)
{

	if (eeprom_dip == 0xff) {
		struct udevice *dev = NULL;
		if (i2c_get_chip_for_busnum(0, 0x4d, 1, &dev)==0) {
			dm_i2c_read(dev, 0, &eeprom_dip, 1);
			if (verbose) {
				printf("DIP EEPROM[%d]\n", 0);
				print_boot_cfg(eeprom_dip);
			}
		}
	}

	return eeprom_dip;
}

uint8_t get_board_rev(void)
{
	struct udevice *dev = NULL;
	static uchar reg = 0xff;

	if ((reg==0xff)
		&& (i2c_get_chip_for_busnum(0, 0x43, 1, &dev)==0)) {
		if (dm_i2c_read(dev, 0x0f, &reg, 1)==0) {
			reg = (((reg&BIT(7))>>7)
				| ((reg&BIT(6))>>5)
				| ((reg&BIT(5))>>3)) + 1;
		}
	}

	return reg;
}

int misc_init_f(void)
{
	return 0;
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
