/*
 * g_sstar_dnl.c - Sigmastar
 *
 * Copyright (c) [2019~2020] SigmaStar Technology.
 *
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License version 2 for more details.
 *
 */
#include <common.h>
#include <log.h>
#include <malloc.h>

#include <mmc.h>
#include <part.h>
#include <usb.h>

#include "g_sstar_dnl.h"
#include <usb_mass_storage.h>
#include <dfu.h>
#include <thor.h>

#include <env_callback.h>

#include "gadget_chips.h"
#include <linux/usb/composite.h>

/* Index of String Descriptor describing this configuration */
#define STRING_USBDOWN    2
#define MAX_STRING_SERIAL 256
/* Number of supported configurations */
#define CONFIGURATION_NUMBER 1

static const char product[] = "UFU";
static char       g_dnl_serial[MAX_STRING_SERIAL];
static const char manufacturer[] = CONFIG_G_DNL_MANUFACTURER;

static void g_dnl_set_serialnumber(char *s)
{
    memset(g_dnl_serial, 0, MAX_STRING_SERIAL);
    strncpy(g_dnl_serial, s, MAX_STRING_SERIAL - 1);
}

static struct usb_device_descriptor device_desc = {
    .bLength         = sizeof device_desc,
    .bDescriptorType = USB_DT_DEVICE,

    .bcdUSB          = __constant_cpu_to_le16(0x0200),
    .bDeviceClass    = USB_CLASS_PER_INTERFACE,
    .bDeviceSubClass = 0, /*0x02:CDC-modem , 0x00:CDC-serial*/

    .idVendor  = __constant_cpu_to_le16(CONFIG_USB_GADGET_VENDOR_NUM),
    .idProduct = __constant_cpu_to_le16(CONFIG_USB_GADGET_PRODUCT_NUM),
    /* .iProduct = DYNAMIC */
    /* .iSerialNumber = DYNAMIC */
    .bNumConfigurations = 1,
};

/*
 * static strings, in UTF-8
 * IDs for those strings are assigned dynamically at g_dnl_bind()
 */
static struct usb_string g_dnl_string_defs[] = {
    {.s = manufacturer}, {.s = product}, {.s = g_dnl_serial}, {} /* end of list */
};

static struct usb_gadget_strings g_dnl_string_tab = {
    .language = 0x0409, /* en-us */
    .strings  = g_dnl_string_defs,
};

static struct usb_gadget_strings *g_dnl_composite_strings[] = {
    &g_dnl_string_tab,
    NULL,
};

static int g_dnl_unbind(struct usb_composite_dev *cdev)
{
    struct usb_gadget *gadget = cdev->gadget;

    debug(
        "%s: calling usb_gadget_disconnect for "
        "controller '%s'\n",
        __func__, gadget->name);
    usb_gadget_disconnect(gadget);

    return 0;
}

static inline struct g_dnl_bind_callback *g_dnl_bind_callback_first(void)
{
    return ll_entry_start(struct g_dnl_bind_callback, g_sstar_dnl_bind_callbacks);
}

static inline struct g_dnl_bind_callback *g_dnl_bind_callback_end(void)
{
    return ll_entry_end(struct g_dnl_bind_callback, g_sstar_dnl_bind_callbacks);
}

static int g_dnl_do_config(struct usb_configuration *c)
{
    const char *                s        = c->cdev->driver->name;
    struct g_dnl_bind_callback *callback = g_dnl_bind_callback_first();

    debug("%s: configuration: 0x%p composite dev: 0x%p\n", __func__, c, c->cdev);

    for (; callback != g_dnl_bind_callback_end(); callback++)
        if (!strcmp(s, callback->usb_function_name))
            return callback->fptr(c);
    return -ENODEV;
}

static int g_dnl_config_register(struct usb_composite_dev *cdev)
{
    struct usb_configuration *config;
    const char *              name = "usb_dnload";

    config = memalign(CONFIG_SYS_CACHELINE_SIZE, sizeof(*config));
    if (!config)
        return -ENOMEM;

    memset(config, 0, sizeof(*config));

    config->label               = name;
    config->bmAttributes        = USB_CONFIG_ATT_ONE | USB_CONFIG_ATT_SELFPOWER;
    config->bConfigurationValue = CONFIGURATION_NUMBER;
    config->iConfiguration      = STRING_USBDOWN;
    config->bind                = g_dnl_do_config;

    return usb_add_config(cdev, config);
}

__weak int board_usb_init(int index, enum usb_init_type init)
{
    return 0;
}

__weak int board_usb_cleanup(int index, enum usb_init_type init)
{
    return 0;
}

static int g_sstar_dnl_bind_fixup(struct usb_device_descriptor *dev, const char *name)
{
    put_unaligned(CONFIG_G_DNL_VENDOR_NUM, &dev->idVendor);
    put_unaligned(CONFIG_G_DNL_PRODUCT_NUM, &dev->idProduct);
    return 0;
}

int g_sstar_dnl_board_usb_cable_connected(void)
{
    return -EOPNOTSUPP;
}

static int g_dnl_get_bcd_device_number(struct usb_composite_dev *cdev)
{
    struct usb_gadget *gadget = cdev->gadget;
    int                gcnum;

    gcnum = usb_gadget_controller_number(gadget);
    if (gcnum > 0)
        gcnum += 0x200;

    return gcnum;
}

/**
 * Update internal serial number variable when the "serial#" env var changes.
 *
 * Handle all cases, even when flags == H_PROGRAMMATIC or op == env_op_delete.
 */
static int on_serialno(const char *name, const char *value, enum env_op op, int flags)
{
    g_dnl_set_serialnumber((char *)value);
    return 0;
}
U_BOOT_ENV_CALLBACK(sstar_serialno, on_serialno);

static int g_dnl_bind(struct usb_composite_dev *cdev)
{
    struct usb_gadget *gadget = cdev->gadget;
    int                id, ret;
    int                gcnum;

    debug("%s: gadget: 0x%p cdev: 0x%p\n", __func__, gadget, cdev);

    id = usb_string_id(cdev);

    if (id < 0)
        return id;
    g_dnl_string_defs[0].id   = id;
    device_desc.iManufacturer = id;

    id = usb_string_id(cdev);
    if (id < 0)
        return id;

    g_dnl_string_defs[1].id = id;
    device_desc.iProduct    = id;

    g_sstar_dnl_bind_fixup(&device_desc, cdev->driver->name);

    if (strlen(g_dnl_serial))
    {
        id = usb_string_id(cdev);
        if (id < 0)
            return id;

        g_dnl_string_defs[2].id   = id;
        device_desc.iSerialNumber = id;
    }

    ret = g_dnl_config_register(cdev);
    if (ret)
        goto error;

    gcnum = g_dnl_get_bcd_device_number(cdev);
    if (gcnum >= 0)
        device_desc.bcdDevice = cpu_to_le16(gcnum);
    else
    {
        debug("%s: controller '%s' not recognized\n", __func__, gadget->name);
        device_desc.bcdDevice = __constant_cpu_to_le16(0x9999);
    }

    debug(
        "%s: calling usb_gadget_connect for "
        "controller '%s'\n",
        __func__, gadget->name);
    usb_gadget_connect(gadget);

    return 0;

error:
    g_dnl_unbind(cdev);
    return -ENOMEM;
}

static struct usb_composite_driver g_dnl_driver = {
    .name      = NULL,
    .dev       = &device_desc,
    .strings   = g_dnl_composite_strings,
    .max_speed = USB_SPEED_SUPER,

    .bind   = g_dnl_bind,
    .unbind = g_dnl_unbind,
};

/*
 * NOTICE:
 * Registering via USB function name won't be necessary after rewriting
 * g_dnl to support multiple USB functions.
 */
int g_sstar_dnl_register(const char *name)
{
    int ret;

    debug("%s: g_dnl_driver.name = %s\n", __func__, name);
    g_dnl_driver.name = name;

    ret = usb_composite_register(&g_dnl_driver);
    if (ret)
    {
        printf("%s: failed!, error: %d\n", __func__, ret);
        return ret;
    }
    return 0;
}

void g_sstar_dnl_unregister(void)
{
    usb_composite_unregister(&g_dnl_driver);
}
