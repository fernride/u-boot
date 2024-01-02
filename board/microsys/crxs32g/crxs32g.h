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

#ifndef CRXS32G_H
#define CRXS32G_H

#include <config.h>
#include <linux/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SERDES_M2,
    SERDES_2G5
} serdes_t;

extern serdes_t get_serdes_sel(void);
extern int set_serdes_sel(const serdes_t serdes_mode);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CRXS32G_H */

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
