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

#include "mpxs32g.h"
#include <common.h>
#include <i2c.h>
#include <s32-cc/serdes_hwconfig.h>
#include "pfeng/pfeng.h"

__weak uint8_t get_mux_sd_sel_bit(void)
{
	return 3;
}

__weak uint8_t get_clk_cfg_bit_from_serdes_id(const unsigned int id)
{
	return id;
}

bool s32_serdes_is_external_clk_in_hwconfig(unsigned int id)
{
	return true;
}

#ifndef CONFIG_MICROSYS_CRX_NONE
void check_kconfig(const serdes_t serdes_mode)
{
	switch (serdes_mode) {
	case SERDES_2G5:
#if CONFIG_IS_ENABLED(TARGET_S32G274AR2SBC2)
		env_set("kconfig", "#conf-s32g274ar2sbc2_2g5");
#elif CONFIG_IS_ENABLED(TARGET_S32G274AR2SBC3)
		env_set("kconfig", "#conf-s32g274ar2sbc3_2g5");
#elif CONFIG_IS_ENABLED(TARGET_S32G274AR3SBC2)
		env_set("kconfig", "#conf-s32g274ar3sbc2_2g5");
#elif CONFIG_IS_ENABLED(TARGET_S32G274AR3SBC3)
		env_set("kconfig", "#conf-s32g274ar3sbc3_2g5");
#elif CONFIG_IS_ENABLED(TARGET_S32G274AR5SBC3)
		env_set("kconfig", "#conf-s32g274ar5sbc3_2g5");
#elif CONFIG_IS_ENABLED(TARGET_S32G399AR3SBC2)
		env_set("kconfig", "#conf-s32g399ar3sbc2_2g5");
#elif CONFIG_IS_ENABLED(TARGET_S32G399AR3SBC3)
		env_set("kconfig", "#conf-s32g399ar3sbc3_2g5");
#endif
		env_set("sja1110_cfg", "sja1110.firmware_name=sja1110_uc_2g5.bin");
break;
	default:
#if CONFIG_IS_ENABLED(TARGET_S32G274AR2SBC2)
		env_set("kconfig", "#conf-s32g274ar2sbc2_m2");
#elif CONFIG_IS_ENABLED(TARGET_S32G274AR2SBC3)
		env_set("kconfig", "#conf-s32g274ar2sbc3_m2");
#elif CONFIG_IS_ENABLED(TARGET_S32G274AR3SBC2)
		env_set("kconfig", "#conf-s32g274ar3sbc2_m2");
#elif CONFIG_IS_ENABLED(TARGET_S32G274AR3SBC3)
		env_set("kconfig", "#conf-s32g274ar3sbc3_m2");
#elif CONFIG_IS_ENABLED(TARGET_S32G274AR5SBC3)
		env_set("kconfig", "#conf-s32g274ar5sbc3_m2");
#elif CONFIG_IS_ENABLED(TARGET_S32G399AR3SBC2)
		env_set("kconfig", "#conf-s32g399ar3sbc2_m2");
#elif CONFIG_IS_ENABLED(TARGET_S32G399AR3SBC3)
		env_set("kconfig", "#conf-s32g399ar3sbc3_m2");
#endif
		env_set("sja1110_cfg", "sja1110.firmware_name=sja1110_uc_m2.bin");
		break;
	}
}
#endif

int fix_pfe_enetaddr(void)
{
	int pfe_index;
	uchar ea[ARP_HLEN];

	for (pfe_index = 0; pfe_index < PFENG_EMACS_COUNT; pfe_index++) {
		if (eth_env_get_enetaddr_by_index("eth", pfe_index+1, ea)) {
			eth_env_set_enetaddr_by_index("pfe", pfe_index, ea);
		}
	}

	return 0;
}

void print_boot_cfg(const uint8_t boot_cfg)
{
	printf("  PCIe0/SGMII CLK:  %dMHz\n",
			boot_cfg&BIT(get_clk_cfg_bit_from_serdes_id(0))
			? 100 : 125);
	printf("  PCIe1/SGMII CLK:  %dMHz\n",
			boot_cfg&BIT(get_clk_cfg_bit_from_serdes_id(1))
			? 100 : 125);
	printf("  SEL SDHC:         %s\n",
			boot_cfg&BIT(get_mux_sd_sel_bit())
			? "eMMC" : "SDHC");
}

__weak int set_boot_cfg(const uchar cfg)
{
	return 0;
}

__weak uchar get_boot_cfg(const bool verbose)
{
	return 0x0b;
}

__weak uint8_t get_board_rev(void)
{
	return 0;
}

enum s32g_boot_media get_boot_media()
{
	enum s32g_boot_media media = S32G_BOOT_SD;
	printf("get_boot_media(): boot target selection (eeprom @ 0x50) is not supported\n");
	// uint8_t boot_cfg0;

	// struct udevice *dev = NULL;
	// if (i2c_get_chip_for_busnum(0, RCW_EEPROM_ADDR, 1, &dev)==0) {
	// 	if (dm_i2c_read(dev, 0x0, &boot_cfg0, 1)==0) {
	// 		media = (enum s32g_boot_media) ((boot_cfg0 >> 5) & 0x7);
	// 	}
	// }

	return media;
}

bool s32_serdes_is_hwconfig_instance_enabled(unsigned int id)
{
	return true;
}

unsigned long s32_serdes_get_clock_fmhz_from_hwconfig(unsigned int id)
{
	unsigned long fmhz = MHZ_100;

	uchar reg = get_boot_cfg(false);

	fmhz = (reg & BIT(get_clk_cfg_bit_from_serdes_id(id)))
			? MHZ_100 : MHZ_125;

	if (id == 0) {

		const unsigned long current_fmhz = fmhz;

		if (fmhz != MHZ_100) {
			fmhz = MHZ_100;
			reg |= BIT(get_clk_cfg_bit_from_serdes_id(id));
			set_boot_cfg(reg);
			printf("SerDes%d clocking has changed from %dMHz to %dMHz!\n",
					id,
					current_fmhz==MHZ_100 ? 100:125,
						fmhz==MHZ_100 ? 100:125);
			puts("Performing necessary reset ...\n");
			do_reset(NULL, 0, 0, NULL);
		}
	}

	printf("PCIe%d clock %dMHz\n", id, fmhz==MHZ_100 ? 100 : 125);

	return fmhz;
}

#ifndef CONFIG_MICROSYS_CRX_NONE
enum serdes_mode s32_serdes_get_serdes_mode_from_hwconfig(unsigned int id)
{
	enum serdes_mode serdes_m = SERDES_MODE_PCIE_XPCS0;
	serdes_t serdes_sel = SERDES_M2;

	unsigned long fmhz = MHZ_100;
	bool changed = false;

	debug("[%s: id = %d]\n", __func__, id);

	if (id == 0) {
		debug("[%s: serdes_m = %d]\n", __func__, SERDES_MODE_PCIE_XPCS0);
		return SERDES_MODE_PCIE_XPCS0;
	}

	uchar reg = get_boot_cfg(true);

	fmhz = (reg & BIT(get_clk_cfg_bit_from_serdes_id(id)))
			? MHZ_100 : MHZ_125;

	size_t subarg_len = 0;
	char *option_str = s32_serdes_get_serdes_hwconfig_subarg(id, "mode",
			&subarg_len);

	if (!strncmp(option_str, "pcie&xpcs0", subarg_len)) {
		serdes_m = SERDES_MODE_PCIE_XPCS0;
		serdes_sel = SERDES_M2;
	}

	if (!strncmp(option_str, "xpcs0", subarg_len)
			|| !strncmp(option_str, "xpcs0&xpcs1", subarg_len)) {
		serdes_m = SERDES_MODE_XPCS0_XPCS1;
		serdes_sel = SERDES_2G5;
	}

	if ((serdes_sel == SERDES_M2)
			&& (fmhz == MHZ_125)) {
		// change to 100MHz:
		reg |= BIT(get_clk_cfg_bit_from_serdes_id(id));
		changed = true;
	}

	if ((serdes_sel == SERDES_2G5)
			&& (fmhz == MHZ_100)) {
		// change to 125MHz:
		reg &= ~BIT(get_clk_cfg_bit_from_serdes_id(id));
		changed = true;
	}

	check_kconfig(serdes_sel);
	set_serdes_sel(serdes_sel);

	if (changed) {
		set_boot_cfg(reg);
		printf("SerDes%d clocking has changed from %dMHz to %dMHz!\n",
				id,
				fmhz==MHZ_100 ? 100:125,
				fmhz==MHZ_100 ? 125:100);
		puts("Performing necessary reset ...\n");
		do_reset(NULL, 0, 0, NULL);
	}

	debug("[%s: serdes_m = %d]\n", __func__, serdes_m);

	return serdes_m;
}
#endif

int board_early_init_r(void)
{
	const enum s32g_boot_media media = get_boot_media();

	printf("Board: Rev. %d\n", get_board_rev());

	switch (media) {
	case S32G_BOOT_QSPI:
		puts("Boot:  QSPI\n");
		break;
	case S32G_BOOT_SD:
		puts("Boot:  SD\n");
		break;
	case S32G_BOOT_EMMC:
		puts("Boot:  eMMC\n");
		break;
	}

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
