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

#include "crxs32g.h"
#include <common.h>
#include <i2c.h>
#include <dm/uclass.h>
#include <s32-cc/serdes_hwconfig.h>

#define PFE0_SGMII_XPCS_ID_1G	0
#define PFE0_SGMII_PHY_LANE_1G	1

#define PFE0_SGMII_XPCS_ID_2G5	0
#define PFE0_SGMII_PHY_LANE_2G5	0

#define GMAC0_SGMII_XPCS_ID_1G	0
#define GMAC0_SGMII_PHY_LANE_1G	1

serdes_t get_serdes_sel(void)
{
	// uchar reg = 0xff;

	// struct udevice *dev = NULL;
	// if (i2c_get_chip_for_busnum(1, 0x44, 1, &dev)==0) {
	// 	dm_i2c_read(dev, 0x5, &reg, 1);
	// }

	// return (reg & 1) ? SERDES_M2 : SERDES_2G5;
	return SERDES_2G5;
}

int set_serdes_sel(const serdes_t serdes_mode)
{
	// uchar reg = 0xff;

	// struct udevice *dev = NULL;
	// if (i2c_get_chip_for_busnum(1, 0x44, 1, &dev)==0) {
	// 	dm_i2c_read(dev, 0x5, &reg, 1);
	// 	if (serdes_mode == SERDES_2G5)
	// 		reg &= ~1;
	// 	else
	// 		reg |= 1;
	// 	dm_i2c_write(dev, 0x5, &reg, 1);
	// }

	return 0;
}

int s32_serdes_get_xpcs_speed_from_hwconfig(unsigned int serdes_id,
					    unsigned int phy_lane)
{
	int speed = SPEED_UNKNOWN;

	debug("[%s: serdes%d xpcs%d]\n", __func__, serdes_id, phy_lane);

	if (serdes_id == 0
			&& phy_lane == GMAC0_SGMII_PHY_LANE_1G) {
		speed = SPEED_1000;
	}

	if (serdes_id == 1) {
		const serdes_t serdes_mode = get_serdes_sel();
		debug("[%s: serdes_mode = %d]\n", __func__, serdes_mode);
		switch (serdes_mode) {
		case SERDES_M2:
			if (phy_lane == PFE0_SGMII_PHY_LANE_1G)
				speed = SPEED_1000;
			break;
		case SERDES_2G5:
			if (phy_lane == PFE0_SGMII_PHY_LANE_2G5)
				speed = SPEED_2500;
			break;
		}
	}

	debug("[%s: speed = %d]\n", __func__, speed);

	return speed;
}

enum serdes_xpcs_mode s32_serdes_get_xpcs_cfg_from_hwconfig(
		unsigned int serdes_id,
		unsigned int xpcs_id)
{
	/* Set default mode to invalid to force configuration */
	enum serdes_xpcs_mode xpcs_mode = SGMII_INVALID;

	debug("[%s: serdes%d xpcs%d]\n", __func__, serdes_id, xpcs_id);

	if (serdes_id == 0
			&& xpcs_id == GMAC0_SGMII_XPCS_ID_1G) {
		xpcs_mode = SGMII_XPCS_1G;
	}

	if (serdes_id == 1) {

		s32_serdes_get_serdes_mode_from_hwconfig(serdes_id);

		const serdes_t serdes_mode = get_serdes_sel();
		debug("[%s: serdes_mode = %d]\n", __func__, serdes_mode);

		switch (serdes_mode) {
		case SERDES_M2:
			if (xpcs_id == PFE0_SGMII_XPCS_ID_1G)
				xpcs_mode = SGMII_XPCS_1G;
			break;
		case SERDES_2G5:
			if (xpcs_id == PFE0_SGMII_XPCS_ID_2G5)
				xpcs_mode = SGMII_XPCS_2G5;
			break;
		}
	}

	debug("[%s: xpcs_mode = %d]\n", __func__, xpcs_mode);

	return xpcs_mode;
}

enum pcie_type s32_serdes_get_pcie_type_from_hwconfig(unsigned int id)
{
	const serdes_t serdes_mode = get_serdes_sel();
	enum pcie_type pcietype = PCIE_INVALID;

	switch (serdes_mode) {
	case SERDES_M2:
		pcietype = PCIE_RC;
		break;
	case SERDES_2G5:
		pcietype = PCIE_INVALID;
		break;
	}

	return pcietype;
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
