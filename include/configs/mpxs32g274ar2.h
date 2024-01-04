/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2020-2022 MicroSys Electronics GmbH
 */

#ifndef __MPXS32G274AR2_H
#define __MPXS32G274AR2_H

#include <configs/s32g2.h>

#undef CONFIG_MMC_HS400_SUPPORT
#undef CONFIG_MMC_HS400_ES_SUPPORT

#ifndef CONFIG_BOARD_EARLY_INIT_R
#define CONFIG_BOARD_EARLY_INIT_R
#endif

#undef CONFIG_LOADADDR
#undef CONFIG_SYS_LOAD_ADDR
#define CONFIG_LOADADDR 0x90000000
#define CONFIG_SYS_LOAD_ADDR CONFIG_LOADADDR

#if CONFIG_IS_ENABLED(USB_EHCI_MX6)
#define CONFIG_MXC_USB_PORTSC        PORT_PTS_ULPI
#endif

#ifndef CONFIG_SYS_I2C_MXC_I2C1
#define CONFIG_SYS_I2C_MXC_I2C1
#endif
#ifndef CONFIG_SYS_I2C_MXC_I2C2
#define CONFIG_SYS_I2C_MXC_I2C2
#endif
#ifndef CONFIG_SYS_I2C_MXC_I2C3
#define CONFIG_SYS_I2C_MXC_I2C3
#endif
#ifndef CONFIG_SYS_I2C_MXC_I2C5
#define CONFIG_SYS_I2C_MXC_I2C5
#endif

/*
 * EEPROM for environment is 54@I2C0 (BR24G128)
 */
// #define CONFIG_SYS_I2C_SPEED 100000
// #define CONFIG_I2C_ENV_EEPROM_BUS             0
// #undef CONFIG_SYS_I2C_EEPROM_ADDR
// #define CONFIG_SYS_I2C_EEPROM_ADDR            0x54
// #undef CONFIG_SYS_I2C_EEPROM_ADDR_LEN
// #define CONFIG_SYS_I2C_EEPROM_ADDR_LEN        2
// #undef CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS
// #define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS 5
// #undef CONFIG_SYS_EEPROM_SIZE
// #define CONFIG_SYS_EEPROM_SIZE                (16*1024)
// #undef CONFIG_ENV_OFFSET
// #define CONFIG_ENV_OFFSET                     0x2000

/*
 * Enables command 'mac':
 */
#define CONFIG_ID_EEPROM
#define CONFIG_SYS_EEPROM_BUS_NUM 0
//#define CONFIG_SYS_I2C_EEPROM_ADDR 0x54
//#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN 2
#define CONFIG_SYS_I2C_EEPROM_NXID
#define CONFIG_SYS_I2C_EEPROM_NXID_MAC 4

#undef CONFIG_ENV_SIZE
#define CONFIG_ENV_SIZE                       0x2000
#ifndef CONFIG_CMD_EEPROM
#define CONFIG_CMD_EEPROM
#endif

/*
 * SJA1110
 * =======
 *
 * CS1: SPI_HAP_SS0: Switch Subsystem
 * CS2: SPI_HAP_SS1: Microcontroller Subsystem
 */
//#define CONFIG_SYS_DSPI_CTAR1 (DSPI_CTAR_TRSZ(0xf) | DSPI_CTAR_CPHA)
//#define CONFIG_SYS_DSPI_CTAR2 (DSPI_CTAR_TRSZ(0xf) | DSPI_CTAR_CPHA)

#define BOOTARGS_SD                                                 \
    "bootargs_sd=console=ttyLF0,115200 "                            \
        " root=/dev/mmcblk0p1 rootwait rw earlycon "                \
        CONFIG_EXTRA_KERNEL_BOOT_ARGS "\0"

#define BOOTFIT_SD \
    "bootfit_sd=setenv bootargs ${bootargs_sd} ${sja1110_cfg}; ext4load mmc ${mmcdev}:1 ${loadaddr} boot/fitImage.itb; bootm ${loadaddr}${kconfig}\0"

#define BOOTIMG_SD \
    "bootimg_sd=setenv bootargs ${bootargs_sd} ${sja1110_cfg}; ext4load mmc ${mmcdev}:1 ${loadaddr} boot/Image; ext4load mmc ${mmcdev}:1 ${fdt_addr} boot/s32g274sbc.dtb; bootm ${loadaddr} - ${fdt_addr}\0"

#undef CONFIG_EXTRA_ENV_SETTINGS

#undef CONFIG_S32CC_HWCONFIG
#if CONFIG_IS_ENABLED(MICROSYS_CRXS32GR2) \
	|| CONFIG_IS_ENABLED(MICROSYS_CRXS32GR3)
#define CONFIG_S32CC_HWCONFIG \
	"serdes0:mode=pcie&xpcs0,clock=ext,fmhz=100;xpcs0_1:speed=1G;pcie0:mode=rc;serdes1:mode=pcie&xpcs0,clock=ext,fmhz=100;xpcs1_1:speed=1G;pcie1:mode=rc"
#else
#define CONFIG_S32CC_HWCONFIG \
	"serdes0:mode=pcie&xpcs0,clock=ext,fmhz=100;xpcs0_1:speed=1G;pcie0:mode=rc;serdes1:mode=xpcs0,clock=ext,fmhz=125;xpcs1_1:speed=2G5"
#endif

#if CONFIG_IS_ENABLED(MICROSYS_CRXS32GR2) || CONFIG_IS_ENABLED(MICROSYS_CRXS32GR3)
#define PFENG_MODE "enable,sgmii,rgmii,rgmii"
#define PFENG_EMAC "1"
#endif

#undef PFE_EXTRA_ENV_SETTINGS
#define PFE_EXTRA_ENV_SETTINGS \
    "pfeng_mode=" PFENG_MODE "\0" \
    "ethact=eth_pfeng\0" \
    "pfengemac=" PFENG_EMAC "\0"
#define PFE_INIT_CMD "pfeng stop; "

#define PFENG_FLASH_FW_OFFSET 3000000

//#ifdef CONFIG_S32_ATF_BOOT_FLOW
#define FLASH_IMG "flash_img=boot/fip.s32-qspi\0"
//#else
//#define FLASH_IMG "flash_img=boot/u-boot-" CONFIG_SYS_BOARD ".s32-qspi\0"
//#endif

#define FLASH_CMD "flash=ext4load mmc 0:1 ${loadaddr} ${flash_img}; sf probe 6:0; sf update ${loadaddr} 0 ${filesize}\0"

#define FLASH_FW_IMG "flashfw_img=s32g_pfe_class.fw\0"
#define FLASH_FW_CMD "flashfw=ext4load mmc 0:1 ${loadaddr} ${flashfw_img}; sf probe 6:0; sf update ${loadaddr} "__stringify(PFENG_FLASH_FW_OFFSET)" ${filesize}\0"

#define RCWSD   "rcwsd=mw.l ${loadaddr} 000f0140 1; i2c dev 0; i2c write ${loadaddr} 50 0.1 4 -s; i2c mw 4d 0.1 13 1\0"
#define RCWEMMC "rcwemmc=mw.l ${loadaddr} 00070160 1; i2c dev 0; i2c write ${loadaddr} 50 0.1 4 -s; i2c mw 4d 0.1 1b 1\0"
#define RCWQSPI "rcwqspi=mw.l ${loadaddr} 00000100 1; i2c dev 0; i2c write ${loadaddr} 50 0.1 4 -s\0"

#if CONFIG_IS_ENABLED(FSL_PFENG_FW_LOC_QSPI)
#define PFENGFW "pfengfw="__stringify(PFENG_FLASH_FW_OFFSET)"@6:0"
#else
#define PFENGFW "pfengfw=mmc@0:1:s32g_pfe_class.fw"
#endif

#define CONFIG_EXTRA_ENV_SETTINGS                                 \
    XEN_EXTRA_ENV_SETTINGS                                        \
    PFE_EXTRA_ENV_SETTINGS                                        \
    "fdt_addr=" __stringify(S32CC_FDT_ADDR) "\0"                        \
    "mmcdev=" __stringify(CONFIG_SYS_MMC_ENV_DEV) "\0"            \
    "mmcroot=/dev/mmcblk0p1 rootwait rw\0"                        \
    "sja1110_cfg=sja1110.firmware_name=sja1110_uc.bin\0"           \
    BOOTARGS_SD                                                   \
    BOOTFIT_SD \
    BOOTIMG_SD \
    PCIE_EXTRA_ENV_SETTINGS \
    PFENGFW "\0" \
    FLASH_IMG FLASH_CMD \
    FLASH_FW_IMG FLASH_FW_CMD \
    RCWSD RCWEMMC RCWQSPI

#undef CONFIG_BOOTCOMMAND

#if defined(CONFIG_FLASH_BOOT)
#define CONFIG_BOOTCOMMAND \
    PFE_INIT_CMD "run bootcmd_flash"
#elif defined(CONFIG_SD_BOOT)
#define CONFIG_BOOTCOMMAND                                   \
    PFE_INIT_CMD "mmc dev ${mmcdev}; if mmc rescan; then "   \
               "run bootfit_sd; "                            \
       "fi"
#endif

#undef CONFIG_SPI_FLASH_MACRONIX

/*
 * Task #4484:
 * Support of higher baud rate
 * Background is to reduce software update time via serial
 * connection in production.
 */
#define CONFIG_SYS_BAUDRATE_TABLE { 9600, 19200, 38400, 57600, 115200, 230400, 460800 }

#endif
