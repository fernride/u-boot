# SPDX-License-Identifier:	GPL-2.0+
#
# Copyright (C) 2023 MicroSys Electronics GmbH
#

SRC = $(realpath $(srctree))

FSL_COMMON_PATH = $(SRC)/board/freescale/common
NXP_BOARD_PATH  = $(SRC)/board/nxp

obj-y += s32g274ar3sbc2.o
obj-y += ../mpxs32g/mpxs32g.o
obj-y += ../crxs32g/crxs32g.o
obj-y += ../mpxs32g274ar3/mpxs32g274ar3.o
obj-y += ../../../board/nxp/s32-cc/board_common.o
obj-$(CONFIG_ID_EEPROM) += ../../../board/freescale/common/sys_eeprom.o

ccflags-y += -I$(SRC)/board/microsys/mpxs32g -I$(SRC)/board/microsys/crxs32g
ccflags-y += -I$(SRC)/board/microsys/mpxs32g274ar3
ccflags-y += -I$(FSL_COMMON_PATH) -I$(NXP_BOARD_PATH)/s32-cc/include
ccflags-y += -I$(SRC)/drivers/net -I$(SRC)/drivers/net/pfeng/hw_abi
