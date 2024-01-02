/* -*-C-*- */
/* SPDX-License-Identifier:    GPL-2.0+ */
/*
 * Copyright (C) 2020 MicroSys Electronics GmbH
 * Author: Kay Potthoff <kay.potthoff@microsys.de>
 *
 */

/*!
 * \addtogroup U-Boot FXL6408 GPIO Expander
 * @{
 *
 * \file
 * U-Boot GPIO driver for Fairchild's FXL6408 I2C GPIO expander.
 */

#include <common.h>
#include <dm.h>
#include <i2c.h>
#include <asm/gpio.h>

#define GPIO_COUNT  8

struct fxl6408_gpio_plat {
};

struct fxl6408_gpio_data {
    char *pin_name[GPIO_COUNT];
};

static int fxl6408_gpio_direction_input(struct udevice *dev, unsigned offset)
{
    uchar reg;

    dm_i2c_read(dev, 0x03, &reg, 1);

    reg &= ~BIT(offset);

    dm_i2c_write(dev, 0x03, &reg, 1);

    return 0;
}

static int fxl6408_gpio_direction_output(struct udevice *dev, unsigned offset, int value)
{
    uchar reg;

    dm_i2c_read(dev, 0x03, &reg, 1);

    reg |= BIT(offset);

    dm_i2c_write(dev, 0x03, &reg, 1);

    return 0;
}

static int fxl6408_gpio_get_function(struct udevice *dev, unsigned offset)
{
    uchar reg;

    dm_i2c_read(dev, 0x03, &reg, 1);

    return (reg & BIT(offset)) ? GPIOF_OUTPUT : GPIOF_INPUT;
}

static int fxl6408_gpio_get_value(struct udevice *dev, unsigned offset)
{
    uchar reg;

    if (fxl6408_gpio_get_function(dev, offset) == GPIOF_INPUT) {
        dm_i2c_read(dev, 0x0f, &reg, 1);
    }
    else {
        dm_i2c_read(dev, 0x05, &reg, 1);
    }

    return (reg & BIT(offset)) ? 1 : 0;
}

static int fxl6408_gpio_set_value(struct udevice *dev, unsigned offset, int value)
{
    uchar reg;

    dm_i2c_read(dev, 0x05, &reg, 1);

    if (value)
        reg |= BIT(offset);
    else
        reg &= ~BIT(offset);

    dm_i2c_write(dev, 0x05, &reg, 1);

    return 0;
}

int fxl6408_request(struct udevice *dev, unsigned offset, const char *label)
{
    struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
    struct fxl6408_gpio_data *priv = dev_get_priv(dev);

    if (label)
        uc_priv->name[offset] = strdup(label);
    else if (priv->pin_name[offset])
        uc_priv->name[offset] = priv->pin_name[offset];
    else
        return -EIO;

    return 0;
}

static int fxl6408_gpio_ofdata_to_platdata(struct udevice *dev)
{
    struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
    //struct fxl6408_gpio_plat *pdata = dev_get_platdata(dev);
    struct fxl6408_gpio_data *priv = dev_get_priv(dev);
    ofnode node;
    char name[32];
    const char *pin;

    uc_priv->bank_name = dev_read_string(dev, "gpio-bank-name");

    if (!uc_priv->bank_name) {
        node = dev_ofnode(dev);
        snprintf(name, sizeof(name), "%s-", ofnode_get_name(node));
        uc_priv->bank_name = strdup(name);
    }

    uc_priv->gpio_count = GPIO_COUNT;

    memset(&(priv->pin_name[0]), 0, sizeof(char*)*GPIO_COUNT);

    const int ncount = dev_read_string_count(dev, "pin-names");

    if (ncount > 0) {
        const int max_count = ncount > GPIO_COUNT ? GPIO_COUNT : ncount;
        for (int i=0; i < max_count; i++) {
            dev_read_string_index(dev, "pin-names", i, &pin);
            priv->pin_name[i] = strdup(pin);
        }
    }

    return 0;
}

static int fxl6408_gpio_probe(struct udevice *dev)
{
    uchar reg;
    const uint8_t *v;

    v = dev_read_u8_array_ptr(dev, "direction", 1);
    if (v) {
        dm_i2c_write(dev, 0x03, &v[0], 1); // defines the IO direction
    }

    v = dev_read_u8_array_ptr(dev, "input-default-state", 1);
    if (v) {
        dm_i2c_write(dev, 0x09, &v[0], 1); // defines the expected state
    }

    v = dev_read_u8_array_ptr(dev, "output-default-state", 1);
    if (v) {
        dm_i2c_write(dev, 0x05, &v[0], 1); // defines the default state
    }

    v = dev_read_u8_array_ptr(dev, "pull-config", 2);
    if (v) {
        dm_i2c_write(dev, 0x0d, &v[1], 1); // pull-up/down
        dm_i2c_write(dev, 0x0b, &v[0], 1); // pull-enable
    }

    reg = 0x00;
    dm_i2c_write(dev, 0x07, &reg, 1);  // disable high-z outputs

    reg = 0xff;
    dm_i2c_write(dev, 0x11, &reg, 1);  // mask interrupts
    dm_i2c_read(dev, 0x13, &reg, 1);   // clear interrupts

    return 0;
}

static const struct dm_gpio_ops gpio_fxl6408_ops = {
    .request            = fxl6408_request,
    .direction_input    = fxl6408_gpio_direction_input,
    .direction_output   = fxl6408_gpio_direction_output,
    .get_value          = fxl6408_gpio_get_value,
    .set_value          = fxl6408_gpio_set_value,
    .get_function       = fxl6408_gpio_get_function,
};

static const struct udevice_id fxl6408_gpio_ids[] = {
    { .compatible = "fcs,fxl6408" },
    { /* sentinel */ }
};

U_BOOT_DRIVER(gpio_fxl6408) = {
    .name   = "gpio_fxl6408",
    .id = UCLASS_GPIO,
    .ops    = &gpio_fxl6408_ops,
    .ofdata_to_platdata = fxl6408_gpio_ofdata_to_platdata,
    .platdata_auto_alloc_size = sizeof(struct fxl6408_gpio_plat),
    .of_match = fxl6408_gpio_ids,
    .probe  = fxl6408_gpio_probe,
    .priv_auto_alloc_size = sizeof(struct fxl6408_gpio_data),
};

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
