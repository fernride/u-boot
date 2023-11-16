// SPDX-License-Identifier: GPL-2.0+
/*
 * Marvell PHY drivers
 *
 * Copyright 2010-2011 Freescale Semiconductor, Inc.
 * author Andy Fleming
 */
#include <common.h>
#include <errno.h>
#include <phy.h>

#define PHY_AUTONEGOTIATE_TIMEOUT 5000

#define MII_MARVELL_PHY_PAGE		22

/* 88E1011 PHY Status Register */
#define MIIM_88E1xxx_PHY_STATUS		0x11
#define MIIM_88E1xxx_PHYSTAT_SPEED	0xc000
#define MIIM_88E1xxx_PHYSTAT_GBIT	0x8000
#define MIIM_88E1xxx_PHYSTAT_100	0x4000
#define MIIM_88E1xxx_PHYSTAT_DUPLEX	0x2000
#define MIIM_88E1xxx_PHYSTAT_SPDDONE	0x0800
#define MIIM_88E1xxx_PHYSTAT_LINK	0x0400

#define MIIM_88E1xxx_PHY_SCR		0x10
#define MIIM_88E1xxx_PHY_MDI_X_AUTO	0x0060

/* 88E1111 PHY LED Control Register */
#define MIIM_88E1111_PHY_LED_CONTROL	24
#define MIIM_88E1111_PHY_LED_DIRECT	0x4100
#define MIIM_88E1111_PHY_LED_COMBINE	0x411C

/* 88E1111 Extended PHY Specific Control Register */
#define MIIM_88E1111_PHY_EXT_CR		0x14
#define MIIM_88E1111_RX_DELAY		0x80
#define MIIM_88E1111_TX_DELAY		0x2

/* 88E1111 Extended PHY Specific Status Register */
#define MIIM_88E1111_PHY_EXT_SR		0x1b
#define MIIM_88E1111_HWCFG_MODE_MASK		0xf
#define MIIM_88E1111_HWCFG_MODE_COPPER_RGMII	0xb
#define MIIM_88E1111_HWCFG_MODE_FIBER_RGMII	0x3
#define MIIM_88E1111_HWCFG_MODE_SGMII_NO_CLK	0x4
#define MIIM_88E1111_HWCFG_MODE_COPPER_RTBI	0x9
#define MIIM_88E1111_HWCFG_FIBER_COPPER_AUTO	0x8000
#define MIIM_88E1111_HWCFG_FIBER_COPPER_RES	0x2000

#define MIIM_88E1111_COPPER		0
#define MIIM_88E1111_FIBER		1

/* 88E1118 PHY defines */
#define MIIM_88E1118_PHY_PAGE		22
#define MIIM_88E1118_PHY_LED_PAGE	3

/* 88E1121 PHY LED Control Register */
#define MIIM_88E1121_PHY_LED_CTRL	16
#define MIIM_88E1121_PHY_LED_PAGE	3
#define MIIM_88E1121_PHY_LED_DEF	0x0030

/* 88E1121 PHY IRQ Enable/Status Register */
#define MIIM_88E1121_PHY_IRQ_EN		18
#define MIIM_88E1121_PHY_IRQ_STATUS	19

#define MIIM_88E1121_PHY_PAGE		22

/* 88E1145 Extended PHY Specific Control Register */
#define MIIM_88E1145_PHY_EXT_CR 20
#define MIIM_M88E1145_RGMII_RX_DELAY	0x0080
#define MIIM_M88E1145_RGMII_TX_DELAY	0x0002

#define MIIM_88E1145_PHY_LED_CONTROL	24
#define MIIM_88E1145_PHY_LED_DIRECT	0x4100

#define MIIM_88E1145_PHY_PAGE	29
#define MIIM_88E1145_PHY_CAL_OV 30

#define MIIM_88E1149_PHY_PAGE	29

/* 88E1310 PHY defines */
#define MIIM_88E1310_PHY_LED_CTRL	16
#define MIIM_88E1310_PHY_IRQ_EN		18
#define MIIM_88E1310_PHY_RGMII_CTRL	21
#define MIIM_88E1310_PHY_PAGE		22

/* 88E151x PHY defines */
/* Page 2 registers */
#define MIIM_88E151x_PHY_MSCR		21
#define MIIM_88E151x_RGMII_RX_DELAY	BIT(5)
#define MIIM_88E151x_RGMII_TX_DELAY	BIT(4)
#define MIIM_88E151x_RGMII_RXTX_DELAY	(BIT(5) | BIT(4))
/* Page 3 registers */
#define MIIM_88E151x_LED_FUNC_CTRL	16
#define MIIM_88E151x_LED_FLD_SZ		4
#define MIIM_88E151x_LED0_OFFS		(0 * MIIM_88E151x_LED_FLD_SZ)
#define MIIM_88E151x_LED1_OFFS		(1 * MIIM_88E151x_LED_FLD_SZ)
#define MIIM_88E151x_LED0_ACT		3
#define MIIM_88E151x_LED1_100_1000_LINK	6
#define MIIM_88E151x_LED_TIMER_CTRL	18
#define MIIM_88E151x_INT_EN_OFFS	7
/* Page 18 registers */
#define MIIM_88E151x_GENERAL_CTRL	20
#define MIIM_88E151x_MODE_SGMII		1
#define MIIM_88E151x_RESET_OFFS		15

static int m88e1xxx_phy_extread(struct phy_device *phydev, int addr,
				int devaddr, int regnum)
{
	int oldpage = phy_read(phydev, MDIO_DEVAD_NONE, MII_MARVELL_PHY_PAGE);
	int val;

	phy_write(phydev, MDIO_DEVAD_NONE, MII_MARVELL_PHY_PAGE, devaddr);
	val = phy_read(phydev, MDIO_DEVAD_NONE, regnum);
	phy_write(phydev, MDIO_DEVAD_NONE, MII_MARVELL_PHY_PAGE, oldpage);

	return val;
}

static int m88e1xxx_phy_extwrite(struct phy_device *phydev, int addr,
				 int devaddr, int regnum, u16 val)
{
	int oldpage = phy_read(phydev, MDIO_DEVAD_NONE, MII_MARVELL_PHY_PAGE);

	phy_write(phydev, MDIO_DEVAD_NONE, MII_MARVELL_PHY_PAGE, devaddr);
	phy_write(phydev, MDIO_DEVAD_NONE, regnum, val);
	phy_write(phydev, MDIO_DEVAD_NONE, MII_MARVELL_PHY_PAGE, oldpage);

	return 0;
}

/* Marvell 88E1011S */
static int m88e1011s_config(struct phy_device *phydev)
{
	/* Reset and configure the PHY */
	phy_write(phydev, MDIO_DEVAD_NONE, MII_BMCR, BMCR_RESET);

	phy_write(phydev, MDIO_DEVAD_NONE, 0x1d, 0x1f);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1e, 0x200c);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1d, 0x5);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1e, 0);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1e, 0x100);

	phy_write(phydev, MDIO_DEVAD_NONE, MII_BMCR, BMCR_RESET);

	genphy_config_aneg(phydev);

	return 0;
}

/* Parse the 88E1011's status register for speed and duplex
 * information
 */
static int m88e1xxx_parse_status(struct phy_device *phydev)
{
	unsigned int speed;
	unsigned int mii_reg;

	mii_reg = phy_read(phydev, MDIO_DEVAD_NONE, MIIM_88E1xxx_PHY_STATUS);

	if ((mii_reg & MIIM_88E1xxx_PHYSTAT_LINK) &&
	    !(mii_reg & MIIM_88E1xxx_PHYSTAT_SPDDONE)) {
		int i = 0;

		puts("Waiting for PHY realtime link");
		while (!(mii_reg & MIIM_88E1xxx_PHYSTAT_SPDDONE)) {
			/* Timeout reached ? */
			if (i > PHY_AUTONEGOTIATE_TIMEOUT) {
				puts(" TIMEOUT !\n");
				phydev->link = 0;
				return -ETIMEDOUT;
			}

			if ((i++ % 1000) == 0)
				putc('.');
			udelay(1000);
			mii_reg = phy_read(phydev, MDIO_DEVAD_NONE,
					   MIIM_88E1xxx_PHY_STATUS);
		}
		puts(" done\n");
		mdelay(500);	/* another 500 ms (results in faster booting) */
	} else {
		if (mii_reg & MIIM_88E1xxx_PHYSTAT_LINK)
			phydev->link = 1;
		else
			phydev->link = 0;
	}

	if (mii_reg & MIIM_88E1xxx_PHYSTAT_DUPLEX)
		phydev->duplex = DUPLEX_FULL;
	else
		phydev->duplex = DUPLEX_HALF;

	speed = mii_reg & MIIM_88E1xxx_PHYSTAT_SPEED;

	switch (speed) {
	case MIIM_88E1xxx_PHYSTAT_GBIT:
		phydev->speed = SPEED_1000;
		break;
	case MIIM_88E1xxx_PHYSTAT_100:
		phydev->speed = SPEED_100;
		break;
	default:
		phydev->speed = SPEED_10;
		break;
	}

	return 0;
}

static int m88e1011s_startup(struct phy_device *phydev)
{
	int ret;

	ret = genphy_update_link(phydev);
	if (ret)
		return ret;

	return m88e1xxx_parse_status(phydev);
}

/* Marvell 88E1111S */
static int m88e1111s_config(struct phy_device *phydev)
{
	int reg;

	if (phy_interface_is_rgmii(phydev)) {
		reg = phy_read(phydev,
			       MDIO_DEVAD_NONE, MIIM_88E1111_PHY_EXT_CR);
		if ((phydev->interface == PHY_INTERFACE_MODE_RGMII) ||
		    (phydev->interface == PHY_INTERFACE_MODE_RGMII_ID)) {
			reg |= (MIIM_88E1111_RX_DELAY | MIIM_88E1111_TX_DELAY);
		} else if (phydev->interface == PHY_INTERFACE_MODE_RGMII_RXID) {
			reg &= ~MIIM_88E1111_TX_DELAY;
			reg |= MIIM_88E1111_RX_DELAY;
		} else if (phydev->interface == PHY_INTERFACE_MODE_RGMII_TXID) {
			reg &= ~MIIM_88E1111_RX_DELAY;
			reg |= MIIM_88E1111_TX_DELAY;
		}

		phy_write(phydev,
			  MDIO_DEVAD_NONE, MIIM_88E1111_PHY_EXT_CR, reg);

		reg = phy_read(phydev,
			       MDIO_DEVAD_NONE, MIIM_88E1111_PHY_EXT_SR);

		reg &= ~(MIIM_88E1111_HWCFG_MODE_MASK);

		if (reg & MIIM_88E1111_HWCFG_FIBER_COPPER_RES)
			reg |= MIIM_88E1111_HWCFG_MODE_FIBER_RGMII;
		else
			reg |= MIIM_88E1111_HWCFG_MODE_COPPER_RGMII;

		phy_write(phydev,
			  MDIO_DEVAD_NONE, MIIM_88E1111_PHY_EXT_SR, reg);
	}

	if (phydev->interface == PHY_INTERFACE_MODE_SGMII) {
		reg = phy_read(phydev,
			       MDIO_DEVAD_NONE, MIIM_88E1111_PHY_EXT_SR);

		reg &= ~(MIIM_88E1111_HWCFG_MODE_MASK);
		reg |= MIIM_88E1111_HWCFG_MODE_SGMII_NO_CLK;
		reg |= MIIM_88E1111_HWCFG_FIBER_COPPER_AUTO;

		phy_write(phydev, MDIO_DEVAD_NONE,
			  MIIM_88E1111_PHY_EXT_SR, reg);
	}

	if (phydev->interface == PHY_INTERFACE_MODE_RTBI) {
		reg = phy_read(phydev,
			       MDIO_DEVAD_NONE, MIIM_88E1111_PHY_EXT_CR);
		reg |= (MIIM_88E1111_RX_DELAY | MIIM_88E1111_TX_DELAY);
		phy_write(phydev,
			  MDIO_DEVAD_NONE, MIIM_88E1111_PHY_EXT_CR, reg);

		reg = phy_read(phydev, MDIO_DEVAD_NONE,
			       MIIM_88E1111_PHY_EXT_SR);
		reg &= ~(MIIM_88E1111_HWCFG_MODE_MASK |
			MIIM_88E1111_HWCFG_FIBER_COPPER_RES);
		reg |= 0x7 | MIIM_88E1111_HWCFG_FIBER_COPPER_AUTO;
		phy_write(phydev, MDIO_DEVAD_NONE,
			  MIIM_88E1111_PHY_EXT_SR, reg);

		/* soft reset */
		phy_reset(phydev);

		reg = phy_read(phydev, MDIO_DEVAD_NONE,
			       MIIM_88E1111_PHY_EXT_SR);
		reg &= ~(MIIM_88E1111_HWCFG_MODE_MASK |
			 MIIM_88E1111_HWCFG_FIBER_COPPER_RES);
		reg |= MIIM_88E1111_HWCFG_MODE_COPPER_RTBI |
			MIIM_88E1111_HWCFG_FIBER_COPPER_AUTO;
		phy_write(phydev, MDIO_DEVAD_NONE,
			  MIIM_88E1111_PHY_EXT_SR, reg);
	}

	/* soft reset */
	phy_reset(phydev);

	genphy_config_aneg(phydev);
	genphy_restart_aneg(phydev);

	return 0;
}

/**
 * m88e151x_phy_writebits - write bits to a register
 */
void m88e151x_phy_writebits(struct phy_device *phydev,
			    u8 reg_num, u16 offset, u16 len, u16 data)
{
	u16 reg, mask;

	if ((len + offset) >= 16)
		mask = 0 - (1 << offset);
	else
		mask = (1 << (len + offset)) - (1 << offset);

	reg = phy_read(phydev, MDIO_DEVAD_NONE, reg_num);

	reg &= ~mask;
	reg |= data << offset;

	phy_write(phydev, MDIO_DEVAD_NONE, reg_num, reg);
}

static int m88e151x_config(struct phy_device *phydev)
{
	u16 reg;

	/*
	 * As per Marvell Release Notes - Alaska 88E1510/88E1518/88E1512
	 * /88E1514 Rev A0, Errata Section 3.1
	 */

	/* EEE initialization */
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E1118_PHY_PAGE, 0x00ff);
	phy_write(phydev, MDIO_DEVAD_NONE, 17, 0x214B);
	phy_write(phydev, MDIO_DEVAD_NONE, 16, 0x2144);
	phy_write(phydev, MDIO_DEVAD_NONE, 17, 0x0C28);
	phy_write(phydev, MDIO_DEVAD_NONE, 16, 0x2146);
	phy_write(phydev, MDIO_DEVAD_NONE, 17, 0xB233);
	phy_write(phydev, MDIO_DEVAD_NONE, 16, 0x214D);
	phy_write(phydev, MDIO_DEVAD_NONE, 17, 0xCC0C);
	phy_write(phydev, MDIO_DEVAD_NONE, 16, 0x2159);
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E1118_PHY_PAGE, 0x0000);

	/* SGMII-to-Copper mode initialization */
	if (phydev->interface == PHY_INTERFACE_MODE_SGMII) {
		/* Select page 18 */
		phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E1118_PHY_PAGE, 18);

		/* In reg 20, write MODE[2:0] = 0x1 (SGMII to Copper) */
		m88e151x_phy_writebits(phydev, MIIM_88E151x_GENERAL_CTRL,
				       0, 3, MIIM_88E151x_MODE_SGMII);

		/* PHY reset is necessary after changing MODE[2:0] */
		m88e151x_phy_writebits(phydev, MIIM_88E151x_GENERAL_CTRL,
				       MIIM_88E151x_RESET_OFFS, 1, 1);

		/* Reset page selection */
		phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E1118_PHY_PAGE, 0);

		udelay(100);
	}

	if (phydev->interface == PHY_INTERFACE_MODE_SGMII) {
		reg = phy_read(phydev, MDIO_DEVAD_NONE,
			       MIIM_88E1111_PHY_EXT_SR);

		reg &= ~(MIIM_88E1111_HWCFG_MODE_MASK);
		reg |= MIIM_88E1111_HWCFG_MODE_SGMII_NO_CLK;
		reg |= MIIM_88E1111_HWCFG_FIBER_COPPER_AUTO;

		phy_write(phydev, MDIO_DEVAD_NONE,
			  MIIM_88E1111_PHY_EXT_SR, reg);
	}

	if (phy_interface_is_rgmii(phydev)) {
		phy_write(phydev, MDIO_DEVAD_NONE, MII_MARVELL_PHY_PAGE, 2);

		reg = phy_read(phydev, MDIO_DEVAD_NONE, MIIM_88E151x_PHY_MSCR);
		reg &= ~MIIM_88E151x_RGMII_RXTX_DELAY;
		if (phydev->interface == PHY_INTERFACE_MODE_RGMII ||
		    phydev->interface == PHY_INTERFACE_MODE_RGMII_ID)
			reg |= MIIM_88E151x_RGMII_RXTX_DELAY;
		else if (phydev->interface == PHY_INTERFACE_MODE_RGMII_RXID)
			reg |= MIIM_88E151x_RGMII_RX_DELAY;
		else if (phydev->interface == PHY_INTERFACE_MODE_RGMII_TXID)
			reg |= MIIM_88E151x_RGMII_TX_DELAY;
		phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E151x_PHY_MSCR, reg);

		phy_write(phydev, MDIO_DEVAD_NONE, MII_MARVELL_PHY_PAGE, 0);
	}

	// Switch to LED page:
	phy_write(phydev, MDIO_DEVAD_NONE, MII_MARVELL_PHY_PAGE, 3);

	reg = phy_read(phydev, MDIO_DEVAD_NONE, 16);

	// LED[0]: blink
	reg &= ~(0xf<<0);
	reg |= (0b0001<<0);

	// LED[1]: link
	reg &= ~(0xf<<4);
	reg |= (0b0110 << 4);

	// LED[2]: High-Z
	reg &= ~(0xf << 8);
	reg |= 0b1010 << 8;

	phy_write(phydev, MDIO_DEVAD_NONE, 16, reg);

	reg = phy_read(phydev, MDIO_DEVAD_NONE, 17);

	{
		int i;
		for (i=0; i<3; i++)
			reg &= ~(0x3<<2*i);

		// LED[2]: On - low, Off - tristate
		reg &= ~(0x03 << 4);
		reg |= 0b10 << 4;
	}

	phy_write(phydev, MDIO_DEVAD_NONE, 17, reg);

	/*
	 * Take care that INTn is disabled:
	 */
	reg = phy_read(phydev, MDIO_DEVAD_NONE, 18);
	if (reg & BIT(7)) {
		reg &= ~BIT(7); // disable interrupt
		reg |= BIT(11); // set interrupt polarity to low
		phy_write(phydev, MDIO_DEVAD_NONE, 18, reg);
	}

	/*
	 * Summary for pin LED[2]/INTn
	 * ===========================
	 *
	 * 1. LED[2] Control is set to Force Hi-Z
	 * 2. LED[2] Polarity is set to 'On - drive LED[2] low,
	 *    Off - tristate LED[2]'
	 * 3. Interrupt is disabled.
	 */

	// Switch back to copper page:
	phy_write(phydev, MDIO_DEVAD_NONE, MII_MARVELL_PHY_PAGE, 0);
	phy_write(phydev, MDIO_DEVAD_NONE, 18, 0); // disable all IRQs

	/* soft reset */
	phy_reset(phydev);

	genphy_config_aneg(phydev);
	genphy_restart_aneg(phydev);

	return 0;
}

/* Marvell 88E1118 */
static int m88e1118_config(struct phy_device *phydev)
{
	/* Change Page Number */
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E1118_PHY_PAGE, 0x0002);
	/* Delay RGMII TX and RX */
	phy_write(phydev, MDIO_DEVAD_NONE, 0x15, 0x1070);
	/* Change Page Number */
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E1118_PHY_PAGE, 0x0003);
	/* Adjust LED control */
	phy_write(phydev, MDIO_DEVAD_NONE, 0x10, 0x021e);
	/* Change Page Number */
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E1118_PHY_PAGE, 0x0000);

	return genphy_config_aneg(phydev);
}

static int m88e1118_startup(struct phy_device *phydev)
{
	int ret;

	/* Change Page Number */
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E1118_PHY_PAGE, 0x0000);

	ret = genphy_update_link(phydev);
	if (ret)
		return ret;

	return m88e1xxx_parse_status(phydev);
}

/* Marvell 88E1121R */
static int m88e1121_config(struct phy_device *phydev)
{
	int pg;

	/* Configure the PHY */
	genphy_config_aneg(phydev);

	/* Switch the page to access the led register */
	pg = phy_read(phydev, MDIO_DEVAD_NONE, MIIM_88E1121_PHY_PAGE);
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E1121_PHY_PAGE,
		  MIIM_88E1121_PHY_LED_PAGE);
	/* Configure leds */
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E1121_PHY_LED_CTRL,
		  MIIM_88E1121_PHY_LED_DEF);
	/* Restore the page pointer */
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E1121_PHY_PAGE, pg);

	/* Disable IRQs and de-assert interrupt */
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E1121_PHY_IRQ_EN, 0);
	phy_read(phydev, MDIO_DEVAD_NONE, MIIM_88E1121_PHY_IRQ_STATUS);

	return 0;
}

/* Marvell 88E1145 */
static int m88e1145_config(struct phy_device *phydev)
{
	int reg;

	/* Errata E0, E1 */
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E1145_PHY_PAGE, 0x001b);
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E1145_PHY_CAL_OV, 0x418f);
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E1145_PHY_PAGE, 0x0016);
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E1145_PHY_CAL_OV, 0xa2da);

	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E1xxx_PHY_SCR,
		  MIIM_88E1xxx_PHY_MDI_X_AUTO);

	reg = phy_read(phydev, MDIO_DEVAD_NONE, MIIM_88E1145_PHY_EXT_CR);
	if (phydev->interface == PHY_INTERFACE_MODE_RGMII_ID)
		reg |= MIIM_M88E1145_RGMII_RX_DELAY |
			MIIM_M88E1145_RGMII_TX_DELAY;
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E1145_PHY_EXT_CR, reg);

	genphy_config_aneg(phydev);

	/* soft reset */
	reg = phy_read(phydev, MDIO_DEVAD_NONE, MII_BMCR);
	reg |= BMCR_RESET;
	phy_write(phydev, MDIO_DEVAD_NONE, MII_BMCR, reg);

	return 0;
}

static int m88e1145_startup(struct phy_device *phydev)
{
	int ret;

	ret = genphy_update_link(phydev);
	if (ret)
		return ret;

	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E1145_PHY_LED_CONTROL,
		  MIIM_88E1145_PHY_LED_DIRECT);
	return m88e1xxx_parse_status(phydev);
}

/* Marvell 88E1149S */
static int m88e1149_config(struct phy_device *phydev)
{
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E1149_PHY_PAGE, 0x1f);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1e, 0x200c);
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E1149_PHY_PAGE, 0x5);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1e, 0x0);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1e, 0x100);

	genphy_config_aneg(phydev);

	phy_reset(phydev);

	return 0;
}

/* Marvell 88E1310 */
static int m88e1310_config(struct phy_device *phydev)
{
	u16 reg;

	/* LED link and activity */
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E1310_PHY_PAGE, 0x0003);
	reg = phy_read(phydev, MDIO_DEVAD_NONE, MIIM_88E1310_PHY_LED_CTRL);
	reg = (reg & ~0xf) | 0x1;
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E1310_PHY_LED_CTRL, reg);

	/* Set LED2/INT to INT mode, low active */
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E1310_PHY_PAGE, 0x0003);
	reg = phy_read(phydev, MDIO_DEVAD_NONE, MIIM_88E1310_PHY_IRQ_EN);
	reg = (reg & 0x77ff) | 0x0880;
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E1310_PHY_IRQ_EN, reg);

	/* Set RGMII delay */
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E1310_PHY_PAGE, 0x0002);
	reg = phy_read(phydev, MDIO_DEVAD_NONE, MIIM_88E1310_PHY_RGMII_CTRL);
	reg |= 0x0030;
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E1310_PHY_RGMII_CTRL, reg);

	/* Ensure to return to page 0 */
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E1310_PHY_PAGE, 0x0000);

	return genphy_config_aneg(phydev);
}

static int m88e1680_config(struct phy_device *phydev)
{
	/*
	 * As per Marvell Release Notes - Alaska V 88E1680 Rev A2
	 * Errata Section 4.1
	 */
	u16 reg;
	int res;

	/* Matrix LED mode (not neede if single LED mode is used */
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E1118_PHY_PAGE, 0x0004);
	reg = phy_read(phydev, MDIO_DEVAD_NONE, 27);
	reg |= (1 << 5);
	phy_write(phydev, MDIO_DEVAD_NONE, 27, reg);

	/* QSGMII TX amplitude change */
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E1118_PHY_PAGE, 0x00fd);
	phy_write(phydev, MDIO_DEVAD_NONE,  8, 0x0b53);
	phy_write(phydev, MDIO_DEVAD_NONE,  7, 0x200d);
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E1118_PHY_PAGE, 0x0000);

	/* EEE initialization */
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E1118_PHY_PAGE, 0x00ff);
	phy_write(phydev, MDIO_DEVAD_NONE, 17, 0xb030);
	phy_write(phydev, MDIO_DEVAD_NONE, 16, 0x215c);
	phy_write(phydev, MDIO_DEVAD_NONE, 22, 0x00fc);
	phy_write(phydev, MDIO_DEVAD_NONE, 24, 0x888c);
	phy_write(phydev, MDIO_DEVAD_NONE, 25, 0x888c);
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E1118_PHY_PAGE, 0x0000);
	phy_write(phydev, MDIO_DEVAD_NONE,  0, 0x9140);

	res = genphy_config_aneg(phydev);
	if (res < 0)
		return res;

	/* soft reset */
	reg = phy_read(phydev, MDIO_DEVAD_NONE, MII_BMCR);
	reg |= BMCR_RESET;
	phy_write(phydev, MDIO_DEVAD_NONE, MII_BMCR, reg);

	return 0;
}

static inline int m88e1548p_set_page(struct phy_device *phydev, uint16_t page)
{
	phy_write(phydev, MDIO_DEVAD_NONE, 22, page&0xff);
	return 0;
}

static int m88e1548p_config(struct phy_device *phydev)
{
	int do_reset = 0;
	uint16_t reg, mode;

	m88e1548p_set_page(phydev, 18);

	mode = phy_read(phydev, MDIO_DEVAD_NONE, 20);

	if (phydev->interface == PHY_INTERFACE_MODE_SGMII && (mode&7) != 0x1) {
		/*
		 * Set MODE[2:0] to SGMII to copper
		 */
	    mode &= ~0x7;
		mode |= 0x1;
		do_reset = 1;
	}

	if (phydev->interface == PHY_INTERFACE_MODE_QSGMII && (mode&7) != 0) {
        mode &= ~0x7;
		do_reset = 1;
	}

	/*
	 * Check if MACsec and PTP is enabled.
	 * Note: Per CONFIG[3] PTP_EN has been set, but PTP has to be
	 * disabled for our configuration. Otherwise the PHY won't work as expected.
	 */
	reg = phy_read(phydev, MDIO_DEVAD_NONE, 27);
	if (reg & BIT(13)) {
		reg &= ~BIT(13); // disable MACsec and PTP
		phy_write(phydev, MDIO_DEVAD_NONE, 27, reg);
	}

	if (do_reset) {
		phy_write(phydev, MDIO_DEVAD_NONE, 20, mode);
		mode |= (1<<15); // do a reset of pages 6 and 18
		phy_write(phydev, MDIO_DEVAD_NONE, 20, mode);
		udelay(200);
	}

	m88e1548p_set_page(phydev, 3);
	reg = phy_read(phydev, MDIO_DEVAD_NONE, 16);

	// LED[1]:
	reg &= ~(0xf<<4);

	// LED[0]
	reg &= ~0xf;
	reg |= 1;

	phy_write(phydev, MDIO_DEVAD_NONE, 16, reg);

#ifdef CONFIG_CARRIER_CRX07
	reg = phy_read(phydev, MDIO_DEVAD_NONE, 17);
	{
		int i;
		for (i=0; i<4; i++) {
			reg &= ~(0x3<<2*i);
			//reg |= (0x1<<2*i);
		}
	}
	phy_write(phydev, MDIO_DEVAD_NONE, 17, reg);
#endif

    if (phydev->interface == PHY_INTERFACE_MODE_QSGMII && (mode&7) == 0) {
        m88e1548p_set_page(phydev, 4);
        reg = phy_read(phydev, MDIO_DEVAD_NONE, 0);
        reg |= BIT(12); // enables auto-neg on SerDes in QSGMII mode
        phy_write(phydev, MDIO_DEVAD_NONE, 0, reg);
    }

	m88e1548p_set_page(phydev, 0);

	genphy_config(phydev);

	genphy_config_aneg(phydev);

	phy_reset(phydev);

	genphy_restart_aneg(phydev);

	return 0;
}

#define MRVL_88Q2112_AN_DISABLE   0x0000
#define MRVL_88Q2112_AN_ENABLE    0x1000
#define MRVL_88Q2112_1000BASE_T1  0x0001
#define MRVL_88Q2112_AN_RESTART     0x0200
#define MRVL_88Q2112_A2     0x0003
#define MRVL_88Q2112_A1     0x0002
#define MRVL_88Q2112_A0     0x0001
#define MRVL_88Q2112_Z1     0x0000
#define MRVL_88Q2112_MODE_LEGACY    0x06B0
#define MRVL_88Q2112_MODE_DEFAULT   0x0000
#define MRVL_88Q2112_MODE_ADVERTISE 0x0002
#define MRVL_88Q2112_LINKUP_TIMEOUT     200 /* unit: milliseconds */

static inline int phy_rev(struct phy_device *phydev)
{
    return (phydev->phy_id & 0xf);
}

static bool m88q2112_aneg_enabled(struct phy_device *phydev)
{
    int reg = phy_read(phydev, MDIO_MMD_AN, 0x0200);

    return (0x0 != (reg & MRVL_88Q2112_AN_ENABLE));
}

static int m88q2112_set_aneg(struct phy_device *phydev, const bool do_enable, const bool do_restart)
{
    int reg, reg_o;

    reg = reg_o = phy_read(phydev, MDIO_MMD_AN, 0x0200);

    if (do_enable)
        reg |= MRVL_88Q2112_AN_ENABLE;
    else
        reg &= ~MRVL_88Q2112_AN_ENABLE;

    if (do_restart && do_enable)
        reg |= MRVL_88Q2112_AN_RESTART;

    if (reg != reg_o) {

        phy_write(phydev, MDIO_MMD_AN, 0x0200, reg);

        if (do_restart && do_enable) {
            unsigned int count = 0;
            do {
                reg = phy_read(phydev, MDIO_MMD_AN, 0x0201);
                if ((reg & BIT(5)) == 0)
                    udelay(1000);
            } while (((reg & BIT(5)) == 0) && (count++ < 500));
        }
    }

    return 0;
}

static int m88q2112_get_speed(struct phy_device *phydev)
{
    int reg;

    if (m88q2112_aneg_enabled(phydev)) {
        reg = ((phy_read(phydev, MDIO_MMD_AN, 0x801a) & 0x4000) >> 13) & 0x3;
        if (reg == 2) return SPEED_1000;
        else return SPEED_100;
    }
    else {
        reg = phy_read(phydev, MDIO_MMD_PMAPMD, 0x0834) & 0xf;
        if (reg == 1) return SPEED_1000;
        else return SPEED_100;
    }
}

static int  m88q2112_set_speed(struct phy_device *phydev)
{
    int reg, reg_o;

    reg = reg_o = phy_read(phydev, MDIO_MMD_PMAPMD, 0x0834);

    reg &= 0xFFF0;

    if (phydev->speed == SPEED_1000)
        reg |= 0x0001;

    if (reg != reg_o) {
        phy_write(phydev, MDIO_MMD_PMAPMD, 0x0834, reg);
        udelay(500*1000);
    }

    return 0;
}

static bool m88q2112_is_master(struct phy_device *phydev)
{
    return ((phy_read(phydev, MDIO_MMD_AN, 0x8001) >> 14) & 0x0001) != 0;
}

static void m88q2112_set_master(struct phy_device *phydev, const bool master)
{
    int reg, reg_o;

    reg = reg_o = phy_read(phydev, MDIO_MMD_PMAPMD, 0x0834);

    if (master)
        reg |= 0x4000;
    else
        reg &= 0xBFFF;

    if (reg != reg_o)
        phy_write(phydev, MDIO_MMD_PMAPMD, 0x0834, reg);
}

static bool m88q2112_check_link(struct phy_device *phydev)
{

    volatile int retData1, retData2 = 0;
    unsigned int count = 0;
    bool link = false;

    do {

        phydev->speed = m88q2112_get_speed(phydev);

        if (SPEED_1000 == phydev->speed) {
            phy_read(phydev, MDIO_MMD_PCS, 0x0901);
            retData1 = phy_read(phydev, MDIO_MMD_PCS, 0x0901);
            retData2 = phy_read(phydev, MDIO_MMD_AN, 0x8001);
        }
        else {
            retData1 = phy_read(phydev, MDIO_MMD_PCS, 0x8109);
            retData2 = phy_read(phydev, MDIO_MMD_PCS, 0x8108);
        }

        link = (0x0 != (retData1 & 0x0004)) && (0x0 != (retData2 & 0x3000));

        if (!link)
            udelay(1000); // wait a millisecond

    } while (!link && (count++ < MRVL_88Q2112_LINKUP_TIMEOUT));

    return link;
}

static int m88q2112_set_tx_enable(struct phy_device *phydev, const bool enable)
{
    int reg;

    reg = phy_read(phydev, MDIO_MMD_PMAPMD, 0x0900);

    if (enable)
        reg &= ~BIT(14);
    else
        reg |= BIT(14);

    return phy_write(phydev, MDIO_MMD_PMAPMD, 0x0900, reg);
}

static __attribute__((unused)) int m88q2112_reset_pma(struct phy_device *phydev)
{
    int reg;

    reg = phy_read(phydev, MDIO_MMD_PMAPMD, 0x0900);

    reg |= BIT(15);

    return phy_write(phydev, MDIO_MMD_PMAPMD, 0x0900, reg);
}

static void m88q2112_apply_ge(struct phy_device *phydev, const bool aneg)
{
    m88q2112_set_aneg(phydev, aneg, aneg);

    switch (phy_rev(phydev)) {
    case MRVL_88Q2112_A2:       // fall-through to MRVL_88Q2112_A1 intentional
    case MRVL_88Q2112_A1:       // fall-through to MRVL_88Q2112_A0 intentional
    case MRVL_88Q2112_A0:

        m88q2112_set_tx_enable(phydev, false);
        m88q2112_set_speed(phydev);

        phy_write(phydev, MDIO_MMD_PCS, 0xFFE4, 0x07B5);
        phy_write(phydev, MDIO_MMD_PCS, 0xFFE4, 0x06B6);
        udelay(5*1000);

        phy_write(phydev, MDIO_MMD_PCS, 0xFFDE, 0x402F);
        phy_write(phydev, MDIO_MMD_PCS, 0xFE2A, 0x3C3D);
        phy_write(phydev, MDIO_MMD_PCS, 0xFE34, 0x4040);
        phy_write(phydev, MDIO_MMD_PCS, 0xFE4B, 0x9337);
        phy_write(phydev, MDIO_MMD_PCS, 0xFE2A, 0x3C1D);
        phy_write(phydev, MDIO_MMD_PCS, 0xFE34, 0x0040);
        phy_write(phydev, MDIO_MMD_AN, 0x8032, 0x0064);
        phy_write(phydev, MDIO_MMD_AN, 0x8031, 0x0A01);
        phy_write(phydev, MDIO_MMD_AN, 0x8031, 0x0C01);
        phy_write(phydev, MDIO_MMD_PCS, 0xFE0F, 0x0000);
        phy_write(phydev, MDIO_MMD_PCS, 0x800C, 0x0000);
        phy_write(phydev, MDIO_MMD_PCS, 0x801D, 0x0800);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC00, 0x01C0);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC17, 0x0425);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC94, 0x5470);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC95, 0x0055);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC19, 0x08D8);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC1a, 0x0110);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC1b, 0x0A10);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC3A, 0x2725);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC61, 0x2627);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC3B, 0x1612);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC62, 0x1C12);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC9D, 0x6367);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC9E, 0x8060);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC00, 0x01C8);
        phy_write(phydev, MDIO_MMD_PCS, 0x8000, 0x0000);
        phy_write(phydev, MDIO_MMD_PCS, 0x8016, 0x0011);

        if (MRVL_88Q2112_A0 != phy_rev(phydev))
            phy_write(phydev, MDIO_MMD_PCS, 0xFDA3, 0x1800);

        phy_write(phydev, MDIO_MMD_PCS, 0xFE02, 0x00C0);
        phy_write(phydev, MDIO_MMD_PCS, 0xFFDB, 0x0010);
        phy_write(phydev, MDIO_MMD_PCS, 0xFFF3, 0x0020);
        phy_write(phydev, MDIO_MMD_PCS, 0xFE40, 0x00A6);

        phy_write(phydev, MDIO_MMD_PCS, 0xFE60, 0x0000);
        phy_write(phydev, MDIO_MMD_PCS, 0xFE04, 0x0008);
        phy_write(phydev, MDIO_MMD_PCS, 0xFE2A, 0x3C3D);
        phy_write(phydev, MDIO_MMD_PCS, 0xFE4B, 0x9334);

        phy_write(phydev, MDIO_MMD_PCS, 0xFC10, 0xF600);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC11, 0x073D);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC12, 0x000D);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC13, 0x0010);
        break;

    default:    // Z1 case
        // port init.
        phy_write(phydev, MDIO_MMD_PCS, 0x0000, 0x0000);
        phy_write(phydev, MDIO_MMD_PMAPMD, 0x0900, 0x0000);
        phy_write(phydev, MDIO_MMD_PCS, 0x800d, 0x0000);
        // Link LED
        phy_write(phydev, MDIO_MMD_PCS, 0x8000, 0x0000);
        phy_write(phydev, MDIO_MMD_PCS, 0x8016, 0x0011);
        // restore default from 100M
        phy_write(phydev, MDIO_MMD_PCS, 0xFE05, 0x0000);
        phy_write(phydev, MDIO_MMD_PCS, 0xFE07, 0x6A10);
        phy_write(phydev, MDIO_MMD_PCS, 0xFB95, 0x5720);
        phy_write(phydev, MDIO_MMD_PCS, 0xFE5D, 0x175C);
        phy_write(phydev, MDIO_MMD_PCS, 0x8016, 0x0071);
        //set speed
        m88q2112_set_speed(phydev);
        // Init code
        phy_write(phydev, MDIO_MMD_PCS, 0xFE12, 0x000E);
        phy_write(phydev, MDIO_MMD_PCS, 0xFE05, 0x05AA);
        phy_write(phydev, MDIO_MMD_PCS, 0xFE04, 0x0016);
        phy_write(phydev, MDIO_MMD_PCS, 0xFE07, 0x681F);
        phy_write(phydev, MDIO_MMD_PCS, 0xFE5D, 0x045C);
        phy_write(phydev, MDIO_MMD_PCS, 0xFE7C, 0x001E);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC00, 0x01C0);
        phy_write(phydev, MDIO_MMD_AN, 0x8032, 0x0020);
        phy_write(phydev, MDIO_MMD_AN, 0x8031, 0x0012);
        phy_write(phydev, MDIO_MMD_AN, 0x8031, 0x0A12);
        phy_write(phydev, MDIO_MMD_AN, 0x8032, 0x003C);
        phy_write(phydev, MDIO_MMD_AN, 0x8031, 0x0001);
        phy_write(phydev, MDIO_MMD_AN, 0x8031, 0x0A01);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC10, 0xD870);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC11, 0x1522);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC12, 0x07FA);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC13, 0x010B);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC15, 0x35A4);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC2D, 0x3C34);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC2E, 0x104B);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC2F, 0x1C15);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC30, 0x3C3C);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC31, 0x3C3C);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC3A, 0x2A2A);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC61, 0x2829);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC3B, 0x0E0E);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC62, 0x1C12);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC32, 0x03D2);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC46, 0x0200);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC86, 0x0401);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC4E, 0x1820);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC9C, 0x0101);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC95, 0x007A);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC3E, 0x221F);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC3F, 0x0A08);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC03, 0x020E);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC04, 0x0077);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC03, 0x0210);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC04, 0x0088);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC03, 0x0215);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC04, 0x00AA);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC03, 0x01D5);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC04, 0x00AA);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC03, 0x0216);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC04, 0x00AB);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC03, 0x01D6);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC04, 0x00AB);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC03, 0x0213);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC04, 0x00A0);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC03, 0x01D3);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC04, 0x00A0);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC03, 0x0214);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC04, 0x00AB);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC03, 0x01D4);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC04, 0x00AB);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC03, 0x046B);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC04, 0x00FA);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC03, 0x046C);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC04, 0x01F4);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC03, 0x046E);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC04, 0x01F4);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC03, 0x0455);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC04, 0x0320);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC03, 0x0416);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC04, 0x0323);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC03, 0x0004);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC04, 0x0000);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC03, 0x03CC);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC04, 0x0055);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC03, 0x03CD);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC04, 0x0055);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC03, 0x03CE);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC04, 0x0022);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC03, 0x03CF);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC04, 0x0022);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC03, 0x03D0);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC04, 0x0022);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC03, 0x03D1);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC04, 0x0022);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC03, 0x03E4);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC04, 0x0055);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC03, 0x03E5);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC04, 0x0055);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC03, 0x03E6);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC04, 0x0022);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC03, 0x03E7);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC04, 0x0022);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC03, 0x03E8);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC04, 0x0022);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC03, 0x03E9);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC04, 0x0022);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC03, 0x040C);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC04, 0x0033);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC5D, 0x06BF);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC89, 0x0003);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC5C, 0x007F);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC69, 0x383A);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC6A, 0x383A);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC6B, 0x0082);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC6F, 0x888F);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC70, 0x0D1A);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC71, 0x0505);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC72, 0x090C);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC73, 0x0C0F);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC74, 0x0400);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC75, 0x0103);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC7A, 0x081E);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC8C, 0xBC40);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC8D, 0x9830);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC91, 0x0000);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC63, 0x4440);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC64, 0x3C3F);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC65, 0x783C);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC66, 0x0002);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC7B, 0x7818);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC7C, 0xC440);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC7D, 0x5360);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC5F, 0x4034);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC60, 0x7858);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC7E, 0x003F);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC8E, 0x0003);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC57, 0x1820);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC00, 0x01C8);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC93, 0x141C);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC9B, 0x0091);
        phy_write(phydev, MDIO_MMD_PCS, 0xFC94, 0x6D88);
        phy_write(phydev, MDIO_MMD_PCS, 0xFE4A, 0x5653);
        phy_write(phydev, MDIO_MMD_PCS, 0x0900, 0x8000);
        break;
    }
}

static bool m88q2112_apply_mode(struct phy_device *phydev, const int opMode)
{
    bool result = false;
    switch (phy_rev(phydev)) {

    case MRVL_88Q2112_A1:
        phy_write(phydev, MDIO_MMD_PCS, 0xFDB8, 0x0001);    //Set A1 to legacy mode
        phy_write(phydev, MDIO_MMD_PMAPMD, 0x0902, MRVL_88Q2112_MODE_LEGACY | MRVL_88Q2112_MODE_ADVERTISE);
        result = true;
        break;

    case MRVL_88Q2112_A0:   // fall-through to MRVL_88Q2112_Z1 intentional
    case MRVL_88Q2112_Z1:
        phy_write(phydev, MDIO_MMD_PMAPMD, 0x0902, MRVL_88Q2112_MODE_LEGACY | MRVL_88Q2112_MODE_ADVERTISE);
        result = true;
        break;

    case MRVL_88Q2112_A2:
        if (MRVL_88Q2112_MODE_LEGACY == opMode) {
            // Enable 1000 BASE-T1 legacy mode support
            phy_write(phydev, MDIO_MMD_PCS, 0xFDB8, 0x0001);
            phy_write(phydev, MDIO_MMD_PCS, 0xFD3D, 0x0C14);
        }
        else {
            // Set back to default compliant mode setting
            phy_write(phydev, MDIO_MMD_PCS, 0xFDB8, 0x0000);
            phy_write(phydev, MDIO_MMD_PCS, 0xFD3D, 0x0000);
        }
        phy_write(phydev, MDIO_MMD_PMAPMD, 0x0902, opMode | MRVL_88Q2112_MODE_ADVERTISE);
        result = true;
        break;

    default:    // error case - unexpected revision
        break;
    }

    return result;
}

static int  m88q2112_set_low_power_mode(struct phy_device *phydev)
{
    int reg = phy_read(phydev, MDIO_MMD_PMAPMD, 0x0000);

    phy_write(phydev, MDIO_MMD_PMAPMD, 0x0000, reg | 0x0800);

    udelay(10*1000);

    return reg;
}

static int  m88q2112_leave_low_power_mode(struct phy_device *phydev)
{
    int reg = phy_read(phydev, MDIO_MMD_PMAPMD, 0x0000);

    phy_write(phydev, MDIO_MMD_PMAPMD, 0x0000, reg & 0xF7FF);

    udelay(10*1000);

    return reg;
}

void m88q2112_ge_soft_reset(struct phy_device *phydev)
{
    uint16_t regDataAuto = 0;

    if (MRVL_88Q2112_Z1 != phy_rev(phydev)) {    // A2/A1/A0
        if (m88q2112_aneg_enabled(phydev)) {
            phy_write(phydev, 3, 0xFFF3, 0x0024);
        }
        //enable low-power mode
        m88q2112_set_low_power_mode(phydev);

        phy_write(phydev, 3, 0xFFF3, 0x0020);
        phy_write(phydev, 3, 0xFFE4, 0x000C);
        udelay(1000);

        phy_write(phydev, 3, 0xffe4, 0x06B6);

        // disable low-power mode
        m88q2112_leave_low_power_mode(phydev);

        phy_write(phydev, 3, 0xFC47, 0x0030);
        phy_write(phydev, 3, 0xFC47, 0x0031);
        phy_write(phydev, 3, 0xFC47, 0x0030);
        phy_write(phydev, 3, 0xFC47, 0x0000);
        phy_write(phydev, 3, 0xFC47, 0x0001);
        phy_write(phydev, 3, 0xFC47, 0x0000);

        phy_write(phydev, 3, 0x0900, 0x8000);

        m88q2112_set_tx_enable(phydev, true);

        phy_write(phydev, 3, 0xFFE4, 0x000C);
    }
    else {  // Z1 Case
        regDataAuto = phy_read(phydev, 3, 0x0900);
        phy_write(phydev, 3, 0x0900, regDataAuto | 0x8000);
        udelay(5*1000);
    }
}

void m88q2112_apply_fe(struct phy_device *phydev, const int isAneg)
{
    uint16_t regData = 0;

    udelay(1000);
    if (isAneg == MRVL_88Q2112_AN_ENABLE)
        phy_write(phydev, 7, 0x0200, MRVL_88Q2112_AN_ENABLE | MRVL_88Q2112_AN_RESTART);
    else
        phy_write(phydev, 7, 0x0200, MRVL_88Q2112_AN_DISABLE);

    if (MRVL_88Q2112_Z1 != phy_rev(phydev)) {    // A2/A1/A0
        phy_write(phydev, 3, 0xFA07, 0x0202);

        regData = phy_read(phydev, 1, 0x0834);
        regData = regData & 0xFFF0;
        phy_write(phydev, 1, 0x0834, regData);
        udelay(5*1000);

        phy_write(phydev, 3, 0x8000, 0x0000);
        phy_write(phydev, 3, 0x8100, 0x0200);
        phy_write(phydev, 3, 0xFA1E, 0x0002);
        phy_write(phydev, 3, 0xFE5C, 0x2402);
        phy_write(phydev, 3, 0xFA12, 0x001F);
        phy_write(phydev, 3, 0xFA0C, 0x9E05);
        phy_write(phydev, 3, 0xFBDD, 0x6862);
        phy_write(phydev, 3, 0xFBDE, 0x736E);
        phy_write(phydev, 3, 0xFBDF, 0x7F79);
        phy_write(phydev, 3, 0xFBE0, 0x8A85);
        phy_write(phydev, 3, 0xFBE1, 0x9790);
        phy_write(phydev, 3, 0xFBE3, 0xA39D);
        phy_write(phydev, 3, 0xFBE4, 0xB0AA);
        phy_write(phydev, 3, 0xFBE5, 0x00B8);
        phy_write(phydev, 3, 0xFBFD, 0x0D0A);
        phy_write(phydev, 3, 0xFBFE, 0x0906);
        phy_write(phydev, 3, 0x801D, 0x8000);
        phy_write(phydev, 3, 0x8016, 0x0011);
    }
    else {  // Z1 Case
        // port init.
        phy_write(phydev, 3, 0x0000, 0x0000);
        phy_write(phydev, 1, 0x0900, 0x0000);
        phy_write(phydev, 3, 0x800D, 0x0000);
        // Link LED
        phy_write(phydev, 3, 0x8000, 0x0000);
        phy_write(phydev, 3, 0x8016, 0x0011);
        //set speed
        regData = phy_read(phydev, 1, 0x0834);
        regData = regData & 0xFFF0;
        phy_write(phydev, 1, 0x0834, regData);
        udelay(500*1000);
        // Init code
        phy_write(phydev, 3, 0x8000, 0x0000);
        phy_write(phydev, 3, 0xFE05, 0x3DAA);
        phy_write(phydev, 3, 0xFE07, 0x6BFF);
        phy_write(phydev, 3, 0xFB95, 0x52F0);
        phy_write(phydev, 3, 0xFE5D, 0x171C);
        phy_write(phydev, 3, 0x8016, 0x0011);
        phy_write(phydev, 3, 0x0900, 0x8000);
    }
}

void m88q2112_fe_soft_reset(struct phy_device *phydev)
{
    uint16_t regData = 0;
    if (MRVL_88Q2112_Z1 != phy_rev(phydev)) {    // A2/A1/A0
        phy_write(phydev, 3, 0x0900, 0x8000);
        phy_write(phydev, 3, 0xFA07, 0x0200);
    }
    else {    // Z1 Case
        regData = phy_read(phydev, 3, 0x0900);
        phy_write(phydev, 3, 0x0900, regData | 0x8000);
        udelay(5*1000);
    }
}

static void m88q2112_init_fe(struct phy_device *phydev)
{
    m88q2112_apply_fe(phydev, MRVL_88Q2112_AN_DISABLE);
    m88q2112_fe_soft_reset(phydev);
}

static bool m88q2112_init_ge(struct phy_device *phydev)
{
    m88q2112_apply_ge(phydev, false);
    if (!m88q2112_apply_mode(phydev, MRVL_88Q2112_MODE_DEFAULT))
        return false;
    m88q2112_ge_soft_reset(phydev);
    return true;
}

static int m88q2112_setup_speed_during_linkup(struct phy_device *phydev)
{
    if (m88q2112_aneg_enabled(phydev)) return 0;

    if (phydev->speed != m88q2112_get_speed(phydev)) {

        m88q2112_set_low_power_mode(phydev);

        m88q2112_set_speed(phydev);

        if (SPEED_1000 == phydev->speed) {
            phy_write(phydev, MDIO_MMD_PCS, 0xffe4, 0x07B6);
        }

        m88q2112_leave_low_power_mode(phydev);
    }

    //apply the init script according to target speed.
    if (SPEED_1000 == phydev->speed) {
         return m88q2112_init_ge(phydev);
    }
    else {
        m88q2112_init_fe(phydev);
    }

    return 0;
}

int m88q2112_startup(struct phy_device *phydev)
{
    phydev->link = m88q2112_check_link(phydev);

    if (phydev->link)
        m88q2112_setup_speed_during_linkup(phydev);
    else {
        if (phydev->speed == SPEED_1000)
            m88q2112_init_ge(phydev);
        else
            m88q2112_init_fe(phydev);

        phydev->link = m88q2112_check_link(phydev);

    }

    return 0;
}

//static int m88q2112_rgmii_reset(struct phy_device *phydev)
//{
//    int reg = phy_read(phydev, 3, 0x8000);
//
//    reg |= BIT(15);
//
//    return phy_write(phydev, 3, 0x8000, reg);
//}

int m88q2112_config(struct phy_device *phydev)
{
    gen10g_discover_mmds(phydev);

    if (!m88q2112_is_master(phydev))
        m88q2112_set_master(phydev, true);

//    if (phy_interface_is_rgmii(phydev)) {
//        ret = phy_read(phydev, MDIO_MMD_VEND2, 0x8001);
//        if (ret & BIT(14)) {
//            ret &= ~BIT(14);
//            phy_write(phydev, MDIO_MMD_VEND2, 0x8001, ret);
//            m88q2112_rgmii_reset(phydev);
//            udelay(20*1000);
//        }
//    }

    phydev->speed = m88q2112_get_speed(phydev);

    return 0;
}

static struct phy_driver M88E1011S_driver = {
	.name = "Marvell 88E1011S",
	.uid = 0x1410c60,
	.mask = 0xffffff0,
	.features = PHY_GBIT_FEATURES,
	.config = &m88e1011s_config,
	.startup = &m88e1011s_startup,
	.shutdown = &genphy_shutdown,
};

static struct phy_driver M88E1111S_driver = {
	.name = "Marvell 88E1111S",
	.uid = 0x1410cc0,
	.mask = 0xffffff0,
	.features = PHY_GBIT_FEATURES,
	.config = &m88e1111s_config,
	.startup = &m88e1011s_startup,
	.shutdown = &genphy_shutdown,
};

static struct phy_driver M88E1118_driver = {
	.name = "Marvell 88E1118",
	.uid = 0x1410e10,
	.mask = 0xffffff0,
	.features = PHY_GBIT_FEATURES,
	.config = &m88e1118_config,
	.startup = &m88e1118_startup,
	.shutdown = &genphy_shutdown,
};

static struct phy_driver M88E1118R_driver = {
	.name = "Marvell 88E1118R",
	.uid = 0x1410e40,
	.mask = 0xffffff0,
	.features = PHY_GBIT_FEATURES,
	.config = &m88e1118_config,
	.startup = &m88e1118_startup,
	.shutdown = &genphy_shutdown,
};

static struct phy_driver M88E1121R_driver = {
	.name = "Marvell 88E1121R",
	.uid = 0x1410cb0,
	.mask = 0xffffff0,
	.features = PHY_GBIT_FEATURES,
	.config = &m88e1121_config,
	.startup = &genphy_startup,
	.shutdown = &genphy_shutdown,
};

static struct phy_driver M88E1145_driver = {
	.name = "Marvell 88E1145",
	.uid = 0x1410cd0,
	.mask = 0xffffff0,
	.features = PHY_GBIT_FEATURES,
	.config = &m88e1145_config,
	.startup = &m88e1145_startup,
	.shutdown = &genphy_shutdown,
};

static struct phy_driver M88E1149S_driver = {
	.name = "Marvell 88E1149S",
	.uid = 0x1410ca0,
	.mask = 0xffffff0,
	.features = PHY_GBIT_FEATURES,
	.config = &m88e1149_config,
	.startup = &m88e1011s_startup,
	.shutdown = &genphy_shutdown,
};

static struct phy_driver M88E151x_driver = {
	.name = "Marvell 88E151x",
	.uid = 0x1410dd0,
	.mask = 0xffffff0,
	.features = PHY_GBIT_FEATURES,
	.config = &m88e151x_config,
	.startup = &m88e1011s_startup,
	.shutdown = &genphy_shutdown,
	.readext = &m88e1xxx_phy_extread,
	.writeext = &m88e1xxx_phy_extwrite,
};

static struct phy_driver M88E1310_driver = {
	.name = "Marvell 88E1310",
	.uid = 0x01410e90,
	.mask = 0xffffff0,
	.features = PHY_GBIT_FEATURES,
	.config = &m88e1310_config,
	.startup = &m88e1011s_startup,
	.shutdown = &genphy_shutdown,
};

static struct phy_driver M88E1680_driver = {
	.name = "Marvell 88E1680",
	.uid = 0x1410ed0,
	.mask = 0xffffff0,
	.features = PHY_GBIT_FEATURES,
	.config = &m88e1680_config,
	.startup = &genphy_startup,
	.shutdown = &genphy_shutdown,
};

static struct phy_driver M88E1548_driver = {
	.name = "Marvell 88E1548P",
	.uid = 0x1410ec0,
	.mask = 0xffffff0,
	.features = PHY_GBIT_FEATURES | SUPPORTED_MII,
	.config = &m88e1548p_config,
	.startup = &m88e1011s_startup,
	.shutdown = &genphy_shutdown,
};

static struct phy_driver M88X3310_driver = {
	.name = "Marvell 88X3310",
	.uid = 0x002b09a0,
	.mask = 0xffffff0,
	.features = PHY_10G_FEATURES | SUPPORTED_MII,
	.config = &gen10g_config,
	.startup = &gen10g_startup,
	.shutdown = &gen10g_shutdown,
};

static struct phy_driver M88Q2112_driver = {
	.name = "Marvell 88Q2112",
	.uid = 0x002b0980,
	.mask = 0xffffff0,
	.features = PHY_GBIT_FEATURES | SUPPORTED_MII,
	.config = &m88q2112_config,
	.startup = &m88q2112_startup,
	.shutdown = &gen10g_shutdown,
};

int phy_marvell_init(void)
{
	phy_register(&M88E1310_driver);
	phy_register(&M88E1149S_driver);
	phy_register(&M88E1145_driver);
	phy_register(&M88E1121R_driver);
	phy_register(&M88E1118_driver);
	phy_register(&M88E1118R_driver);
	phy_register(&M88E1111S_driver);
	phy_register(&M88E1011S_driver);
	phy_register(&M88E151x_driver);
	phy_register(&M88E1680_driver);
	phy_register(&M88E1548_driver);
	phy_register(&M88X3310_driver);
	phy_register(&M88Q2112_driver);
	return 0;
}
