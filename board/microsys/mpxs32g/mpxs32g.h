/* -*-C-*- */
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

#ifndef MPXS32G_H
#define MPXS32G_H

#include <config.h>
#include <linux/types.h>

#ifndef CONFIG_MICROSYS_CRX_NONE
#include "crxs32g.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define RCW_EEPROM_ADDR 0x50

#ifdef CONFIG_MICROSYS_CRX_NONE
/*
 * Just another dirty hack ...
 * Please check drivers/net/phy/dp83867.c
 */
struct dp83867_private {
	u32 rx_id_delay;
	u32 tx_id_delay;
	int fifo_depth;
	int io_impedance;
	bool rxctrl_strap_quirk;
	int port_mirroring;
	bool set_clk_output;
	unsigned int clk_output_sel;
	bool sgmii_ref_clk_en;
	bool sgmii_an_enabled;
};
#endif

enum s32g_boot_media {
	S32G_BOOT_QSPI = 0,
	S32G_BOOT_SD   = 2,
	S32G_BOOT_EMMC = 3
};

#ifndef CONFIG_MICROSYS_CRX_NONE
extern void check_kconfig(const serdes_t serdes_mode);
#endif

extern int fix_pfe_enetaddr(void);

extern int set_boot_cfg(const uchar cfg);
extern uchar get_boot_cfg(const bool verbose);

extern uint8_t get_board_rev(void);

extern void print_boot_cfg(const uint8_t sw);

extern enum s32g_boot_media get_boot_media(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MPXS32G_H */

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
