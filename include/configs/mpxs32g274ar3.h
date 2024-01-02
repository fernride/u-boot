/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2023 MicroSys Electronics GmbH
 */

#ifndef __MPXS32G274AR3_H
#define __MPXS32G274AR3_H

#include <configs/mpxs32g274ar2.h>

#undef CONFIG_SYS_EEPROM_SIZE
#define CONFIG_SYS_EEPROM_SIZE                (0x2000)

#undef CONFIG_ENV_SIZE
#define CONFIG_ENV_SIZE                       (0x1e00)

#undef CONFIG_ENV_OFFSET
#define CONFIG_ENV_OFFSET                     (0x200)

#endif
