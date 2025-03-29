/*
 * tusb320.c- Sigmastar
 *
 * Copyright (c) [2020~2023] SigmaStar Technology.
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

#include <linux/bitfield.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/regmap.h>
#include <linux/mfd/syscon.h>
#include <linux/usb/role.h>
#include <linux/usb/typec.h>
#include <linux/usb/typec_altmode.h>
#include <linux/of_gpio.h>
#include <linux/gpio.h>
#include <linux/extcon-provider.h>

#define TUSB320_REG8                     0x08
#define TUSB320_REG8_ACCESSORY_CONNECTED GENMASK(3, 1)

#define TUSB320_REG9                  0x09
#define TUSB320_REG9_ATTACHED_STATE   GENMASK(7, 6)
#define TUSB320_REG9_CABLE_DIR        BIT(5)
#define TUSB320_REG9_INTERRUPT_STATUS BIT(4)

#define TUSB320_REGA                0x0A
#define TUSB320_REGA_MODE_SELECT    GENMASK(5, 4)
#define TUSB320_REGA_I2C_SOFT_RESET BIT(3)
#define TUSB320_REGA_SOURCE_PREF    GENMASK(2, 1)
#define TUSB320_REGA_DISABLE_TERM   BIT(0)

#define TUSB320_REG45 0x45
#define TUSB320_REGA0 0xA0

#define TUSB320_SWITCH_REG          0x1F
#define TUSB320_SWITCH_PORT_MASK    GENMASK(1, 0)
#define TUSB320_SWITCH_PORT_NOMARL  FIELD_PREP(TUSB320_SWITCH_PORT_MASK, 0)
#define TUSB320_SWITCH_PORT_REVERSE FIELD_PREP(TUSB320_SWITCH_PORT_MASK, 3)

enum tusb320_accessory
{
    TUSB320_ACCESSORY_NONE = 0,
    /* 0b001 ~ 0b011 are reserved */
    TUSB320_ACCESSORY_AUDIO            = 4,
    TUSB320_ACCESSORY_AUDIO_CHARGETHRU = 5,
    TUSB320_ACCESSORY_DEBUG_DFP        = 6,
    TUSB320_ACCESSORY_DEBUG_UFP        = 7,
};

static const char *const tusb320_accessories[] = {
    [TUSB320_ACCESSORY_NONE]             = "none",
    [TUSB320_ACCESSORY_AUDIO]            = "audio",
    [TUSB320_ACCESSORY_AUDIO_CHARGETHRU] = "audio with charge thru",
    [TUSB320_ACCESSORY_DEBUG_DFP]        = "debug while DFP",
    [TUSB320_ACCESSORY_DEBUG_UFP]        = "debug while UFP",
};

enum tusb320_attached_state
{
    TUSB320_ATTACHED_STATE_NONE = 0,
    TUSB320_ATTACHED_STATE_DFP  = 1,
    TUSB320_ATTACHED_STATE_UFP  = 2,
    TUSB320_ATTACHED_STATE_ACC  = 3,
};

static const char *const tusb320_attached_states[] = {
    [TUSB320_ATTACHED_STATE_NONE] = "not attached",
    [TUSB320_ATTACHED_STATE_DFP]  = "downstream facing port",
    [TUSB320_ATTACHED_STATE_UFP]  = "upstream facing port",
    [TUSB320_ATTACHED_STATE_ACC]  = "accessory",
};

enum tusb320_cable_dir
{
    TUSB320_CABLE_DIR_CC1 = 0,
    TUSB320_CABLE_DIR_CC2 = 1,
};

static const char *const tusb320_cable_directions[] = {
    [TUSB320_CABLE_DIR_CC1] = "CC1",
    [TUSB320_CABLE_DIR_CC2] = "CC2",
};

enum tusb320_mode
{
    TUSB320_MODE_PORT = 0,
    TUSB320_MODE_UFP  = 1,
    TUSB320_MODE_DFP  = 2,
    TUSB320_MODE_DRP  = 3,
};

static const char *const tusb320_mode_selects[] = {
    [TUSB320_MODE_PORT] = "select by Port Pin",
    [TUSB320_MODE_UFP]  = "UFP",
    [TUSB320_MODE_DFP]  = "DFP",
    [TUSB320_MODE_DRP]  = "DRP",
};

enum tusb320_source_pref
{
    TUSB320_SOURCE_PREF_DRP     = 0,
    TUSB320_SOURCE_PREF_TRY_SNK = 1,
    /* 0b10 is reserved */
    TUSB320_SOURCE_PREF_TRY_SRC = 3,
};

/*
 * The struct typec_port declaration is copyed from driver/usb/typec/class.c.
 * As it was declared in c file, we can not include it. We found that the
 * declaration has been moved to class.h in later version, but here we can only
 * do it in this way.
 */
struct typec_port
{
    unsigned int  id;
    struct device dev;
    struct ida    mode_ids;

    int                   prefer_role;
    enum typec_data_role  data_role;
    enum typec_role       pwr_role;
    enum typec_role       vconn_role;
    enum typec_pwr_opmode pwr_opmode;
    enum typec_port_type  port_type;
    struct mutex          port_type_lock;

    enum typec_orientation orientation;
    struct typec_switch *  sw;
    struct typec_mux *     mux;

    const struct typec_capability *cap;
    const struct typec_operations *ops;
    ANDROID_KABI_RESERVE(1);
};

struct tusb320
{
    struct device *    dev;
    struct regmap *    regmap;
    struct typec_port *port;
    struct mutex       lock;
    struct extcon_dev *edev;
    int                irq;
    int                gpio_irq;
    int                gpio_vbus_ctl;
    int                gpio_shutdown;
};

static int tusb320_dump_regs(struct tusb320 *tusb, char *label)
{
    unsigned int val;
    int          i, ret;

    // mutex_lock(&tusb->lock);

    for (i = TUSB320_REG8; i < 11; i++)
    {
        ret = regmap_read(tusb->regmap, i, &val);
        if (ret)
            goto done;

        dev_info(tusb->dev, "%s Reg[%02x]=0x%02x\n", label, i, val);
    }

    ret = regmap_read(tusb->regmap, TUSB320_REG45, &val);
    if (ret)
        goto done;

    dev_info(tusb->dev, "%s Reg[%02x]=0x%02x\n", label, TUSB320_REG45, val);

    ret = regmap_read(tusb->regmap, TUSB320_REGA0, &val);
    if (ret)
        goto done;

    dev_info(tusb->dev, "%s Reg[%02x]=0x%02x\n", label, TUSB320_REGA0, val);

done:
    // mutex_unlock(&tusb->lock);

    return ret;
}

static int tusb320_check_device_id(struct tusb320 *tusb)
{
    static const char sig[] = {'\0', 'T', 'U', 'S', 'B', '3', '2', '0'};
    unsigned int      val;
    int               i, ret;

    mutex_lock(&tusb->lock);

    for (i = 0; i < sizeof(sig); i++)
    {
        ret = regmap_read(tusb->regmap, sizeof(sig) - 1 - i, &val);
        if (ret)
            goto done;

        if (val != sig[i])
        {
            dev_err(tusb->dev, "device id mismatch!\n");
            ret = -ENODEV;
            goto done;
        }
    }

done:
    mutex_unlock(&tusb->lock);

    return ret;
}

static int tusb320_reset(struct tusb320 *tusb)
{
    int ret;

    mutex_lock(&tusb->lock);

    /* Disable CC state machine */
    ret = regmap_write_bits(tusb->regmap, TUSB320_REGA, TUSB320_REGA_DISABLE_TERM,
                            FIELD_PREP(TUSB320_REGA_DISABLE_TERM, 1));
    if (ret)
        goto done;

    /* Set to DFP by default, overriding any hardwired PORT setting */
    ret = regmap_write_bits(tusb->regmap, TUSB320_REGA, TUSB320_REGA_MODE_SELECT,
                            FIELD_PREP(TUSB320_REGA_MODE_SELECT, TUSB320_MODE_DFP));
    if (ret)
        goto done;

    /* Wait 5 ms per datasheet specification */
    usleep_range(5000, 10000);

    /* Perform soft reset */
    ret = regmap_write_bits(tusb->regmap, TUSB320_REGA, TUSB320_REGA_I2C_SOFT_RESET,
                            FIELD_PREP(TUSB320_REGA_I2C_SOFT_RESET, 1));
    if (ret)
        goto done;

    /* Wait 95 ms for chip to reset per datasheet specification */
    msleep(95);

    /* Clear any old interrupt status bit */
    ret = regmap_write_bits(tusb->regmap, TUSB320_REG9, TUSB320_REG9_INTERRUPT_STATUS,
                            FIELD_PREP(TUSB320_REG9_INTERRUPT_STATUS, 1));
    if (ret)
        goto done;

    /* Re-enable CC state machine */
    ret = regmap_write_bits(tusb->regmap, TUSB320_REGA, TUSB320_REGA_DISABLE_TERM,
                            FIELD_PREP(TUSB320_REGA_DISABLE_TERM, 0));
    dev_info(tusb->dev, "soft reset done.\n");
    tusb320_dump_regs(tusb, "soft-reset");
    if (ret)
        goto done;

done:
    mutex_unlock(&tusb->lock);

    return ret;
}

static int tusb320_handle_sink_source(struct tusb320 *tusb, enum tusb320_attached_state attached_state)
{
    switch (attached_state)
    {
        /* Sink attach detect in DFP mode, turn on vbus. */
        case TUSB320_ATTACHED_STATE_DFP:
            gpio_set_value(tusb->gpio_vbus_ctl, 1);
            break;

        /* No attach or UFP mode, turn off vbus. */
        case TUSB320_ATTACHED_STATE_UFP:
        case TUSB320_ATTACHED_STATE_NONE:
            gpio_set_value(tusb->gpio_vbus_ctl, 0);
            break;

        default:
            return 0;
    }

    return 0;
}

static int tusb320_sync_state(struct tusb320 *tusb)
{
    enum tusb320_attached_state attached_state;
    enum tusb320_cable_dir      cable_dir;
    int                         cable;
    union extcon_property_value prop;
    enum tusb320_accessory      accessory;
    unsigned int                reg8;
    unsigned int                reg9;
    int                         ret;

    ret = regmap_read(tusb->regmap, TUSB320_REG8, &reg8);
    if (ret)
        return ret;

    ret = regmap_read(tusb->regmap, TUSB320_REG9, &reg9);
    if (ret)
        return ret;

    attached_state = FIELD_GET(TUSB320_REG9_ATTACHED_STATE, reg9);
    cable_dir      = FIELD_GET(TUSB320_REG9_CABLE_DIR, reg9);
    accessory      = FIELD_GET(TUSB320_REG8_ACCESSORY_CONNECTED, reg8);

    dev_info(tusb->dev, "attached state: %s, cable direction: %s, accessory: %s\n",
             tusb320_attached_states[attached_state], tusb320_cable_directions[cable_dir],
             tusb320_accessories[accessory]);

    prop.intval = cable_dir;
    if (attached_state != TUSB320_ATTACHED_STATE_NONE)
    {
        if (attached_state == TUSB320_ATTACHED_STATE_UFP)
        {
            cable = EXTCON_USB;
        }
        else if (attached_state == TUSB320_ATTACHED_STATE_DFP)
        {
            cable = EXTCON_USB_HOST;
        }

        extcon_set_state(tusb->edev, cable, true);
        ret = extcon_set_property_sync(tusb->edev, cable, EXTCON_PROP_USB_TYPEC_POLARITY, prop);
    }
    else
    {
        extcon_set_state(tusb->edev, EXTCON_USB, false);
        extcon_set_property_sync(tusb->edev, EXTCON_USB, EXTCON_PROP_USB_TYPEC_POLARITY, prop);
        extcon_set_state(tusb->edev, EXTCON_USB_HOST, false);
        extcon_set_property_sync(tusb->edev, EXTCON_USB_HOST, EXTCON_PROP_USB_TYPEC_POLARITY, prop);
    }

    tusb320_handle_sink_source(tusb, attached_state);
    return ret;
}

static int tusb320_set_source_pref(struct tusb320 *tusb, enum tusb320_source_pref pref)
{
    int ret;

    mutex_lock(&tusb->lock);

    ret = regmap_update_bits(tusb->regmap, TUSB320_REGA, TUSB320_REGA_SOURCE_PREF,
                             FIELD_PREP(TUSB320_REGA_SOURCE_PREF, pref));

    mutex_unlock(&tusb->lock);

    return ret;
}

static int tusb320_set_mode(struct tusb320 *tusb, enum tusb320_mode mode)
{
    int ret;

    mutex_lock(&tusb->lock);

    /* Disable CC state machine */
    ret = regmap_write_bits(tusb->regmap, TUSB320_REGA, TUSB320_REGA_DISABLE_TERM,
                            FIELD_PREP(TUSB320_REGA_DISABLE_TERM, 1));
    if (ret)
        goto done;

    /* Set the desired port mode */
    ret = regmap_write_bits(tusb->regmap, TUSB320_REGA, TUSB320_REGA_MODE_SELECT,
                            FIELD_PREP(TUSB320_REGA_MODE_SELECT, mode));
    if (ret)
        goto done;

    /* Wait 5 ms per datasheet specification */
    usleep_range(5000, 10000);

    /* Re-enable CC state machine */
    ret = regmap_write_bits(tusb->regmap, TUSB320_REGA, TUSB320_REGA_DISABLE_TERM,
                            FIELD_PREP(TUSB320_REGA_DISABLE_TERM, 0));
    if (ret)
        goto done;

    dev_info(tusb->dev, "%s: set mode %s done\n", __func__, tusb320_mode_selects[mode]);
done:
    mutex_unlock(&tusb->lock);

    return ret;
}

static int tusb320_try_role(struct typec_port *port, int role)
{
    struct tusb320 *         tusb = typec_get_drvdata(port);
    enum tusb320_source_pref pref;

    switch (role)
    {
        case TYPEC_NO_PREFERRED_ROLE:
            pref = TUSB320_SOURCE_PREF_DRP;
            break;
        case TYPEC_SINK:
            pref = TUSB320_SOURCE_PREF_TRY_SNK;
            break;
        case TYPEC_SOURCE:
            pref = TUSB320_SOURCE_PREF_TRY_SRC;
            break;
        default:
            dev_warn(tusb->dev, "unknown port role %d\n", role);
            return -EINVAL;
    }

    return tusb320_set_source_pref(tusb, pref);
}

static int tusb320_port_type_set(struct typec_port *port, enum typec_port_type type)
{
    struct tusb320 *  tusb = typec_get_drvdata(port);
    enum tusb320_mode mode;

    switch (type)
    {
        case TYPEC_PORT_SRC:
            mode = TUSB320_MODE_DFP;
            break;
        case TYPEC_PORT_SNK:
            mode = TUSB320_MODE_UFP;
            break;
        case TYPEC_PORT_DRP:
            mode = TUSB320_MODE_DRP;
            break;
        default:
            dev_warn(tusb->dev, "unknown port type %d\n", type);
            return -EINVAL;
    }

    port->port_type = type;
    return tusb320_set_mode(tusb, mode);
}

static int tusb320_gpios_probe(struct tusb320 *tusb)
{
    int                 ret = 0;
    struct device_node *np  = NULL;

    np = of_parse_phandle(tusb->dev->of_node, "vbus_switch_ctrl", 0);
    if (np && of_property_read_bool(np, "gpio_vbus_ctrl"))
    {
        tusb->gpio_vbus_ctl = of_get_named_gpio(np, "gpio_vbus_ctrl", 0);
        if (!gpio_is_valid(tusb->gpio_vbus_ctl))
        {
            dev_err(tusb->dev, "%s: of get gpio_vbus_ctrl %d error\n", __func__, tusb->gpio_vbus_ctl);
            ret = -EINVAL;
            goto err;
        }
        dev_info(tusb->dev, "%s: p1 vbus contorl gpio num=%d\n", __func__, tusb->gpio_vbus_ctl);

        ret = devm_gpio_request(tusb->dev, tusb->gpio_vbus_ctl, "gpio_vbus_ctrl_p1");
        if (ret < 0)
        {
            dev_err(tusb->dev, "%s: gpio_request error, ret=%d, gpio=%d\n", __func__, ret, tusb->gpio_vbus_ctl);
            goto err;
        }

        ret = gpio_direction_output(tusb->gpio_vbus_ctl, 0);
        if (ret)
            goto err;
    }

    if (of_property_read_bool(tusb->dev->of_node, "tusb320,gpio_usb3_cc_enb"))
    {
        tusb->gpio_shutdown = of_get_named_gpio(tusb->dev->of_node, "tusb320,gpio_usb3_cc_enb", 0);
        if (!gpio_is_valid(tusb->gpio_shutdown))
        {
            dev_err(tusb->dev, "%s: of get gpio_usb3_cc_enb %d error\n", __func__, tusb->gpio_shutdown);
            ret = -EINVAL;
            goto err;
        }
        dev_info(tusb->dev, "%s: shutdown contorl gpio num=%d\n", __func__, tusb->gpio_shutdown);

        ret = devm_gpio_request(tusb->dev, tusb->gpio_shutdown, "gpio_tusb320_shutdown");
        if (ret < 0)
        {
            dev_err(tusb->dev, "%s: gpio_request error, ret=%d, gpio=%d\n", __func__, ret, tusb->gpio_shutdown);
            goto err;
        }

        ret = gpio_direction_output(tusb->gpio_shutdown, 0);
    }

err:
    if (np)
        of_node_put(np);
    return ret;
}

static const struct typec_operations tusb320_ops = {
    .try_role      = tusb320_try_role,
    .port_type_set = tusb320_port_type_set,
};

static irqreturn_t tusb320_irq_handler_thread(int irq, void *dev_id)
{
    struct tusb320 *tusb = dev_id;
    unsigned int    reg;
    int             ret;

    mutex_lock(&tusb->lock);

    /* Check interrupt status bit */
    ret = regmap_read(tusb->regmap, TUSB320_REG9, &reg);
    if (ret)
        goto unhandled;

    if (!(reg & TUSB320_REG9_INTERRUPT_STATUS))
        goto unhandled;

    /* Clear interrupt status bit */
    ret = regmap_write(tusb->regmap, TUSB320_REG9, reg);
    if (ret)
        goto unhandled;

    ret = tusb320_sync_state(tusb);
    if (ret)
        dev_err_ratelimited(tusb->dev, "failed to sync state in irq: %d\n", ret);

    mutex_unlock(&tusb->lock);

    return IRQ_HANDLED;

unhandled:
    mutex_unlock(&tusb->lock);

    return IRQ_NONE;
}

static const struct regmap_config tusb320_regmap_config = {
    .reg_bits        = 8,
    .val_bits        = 8,
    .disable_locking = true,
};

/* List of detectable cables */
static const unsigned int tusb320_extcon_cable[] = {
    EXTCON_USB,
    EXTCON_USB_HOST,
    EXTCON_NONE,
};

void tusb320_action_unregister_port(void *data)
{
    struct typec_port *port = data;

    typec_unregister_port(port);
}

static int tusb320_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    struct tusb320 *        tusb;
    struct typec_capability typec_cap = {};
    int                     ret;

    tusb = devm_kzalloc(&client->dev, sizeof(*tusb), GFP_KERNEL);
    if (!tusb)
        return -ENOMEM;

    tusb->dev = &client->dev;
    mutex_init(&tusb->lock);

    if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C))
    {
        dev_err(tusb->dev, "%s i2c check functionality failed\n", __func__);
        return -ENODEV;
    }

    tusb->regmap = devm_regmap_init_i2c(client, &tusb320_regmap_config);
    if (IS_ERR(tusb->regmap))
    {
        dev_err(tusb->dev, "%s: devm_regmap_init_i2c failed\n", __func__);
        return PTR_ERR(tusb->regmap);
    }

    ret = tusb320_gpios_probe(tusb);
    if (ret)
        return ret;

    ret = tusb320_check_device_id(tusb);
    if (ret)
        return ret;

    ret = tusb320_reset(tusb);
    if (ret)
        return ret;

    /*
     * State might change after reset, so update them.
     */
    tusb320_sync_state(tusb);

    typec_cap.type              = TYPEC_PORT_DRP;
    typec_cap.data              = TYPEC_PORT_DRD;
    typec_cap.revision          = USB_TYPEC_REV_1_1;
    typec_cap.prefer_role       = TYPEC_SOURCE;
    typec_cap.orientation_aware = 1;
    typec_cap.fwnode            = dev_fwnode(tusb->dev);
    typec_cap.driver_data       = tusb;
    typec_cap.ops               = &tusb320_ops;

    tusb->port = typec_register_port(tusb->dev, &typec_cap);
    if (IS_ERR(tusb->port))
    {
        dev_err(tusb->dev, "%s: register typec port failed\n", __func__);
        return PTR_ERR(tusb->port);
    }

    ret = devm_add_action_or_reset(tusb->dev, tusb320_action_unregister_port, tusb->port);
    if (ret)
        return ret;

    tusb->gpio_irq = of_get_named_gpio(tusb->dev->of_node, "tusb320,gpio_usb3_cc_int", 0);
    if (!gpio_is_valid(tusb->gpio_irq))
    {
        dev_err(tusb->dev, "%s: of get gpio_usb3_cc_int %d error\n", __func__, tusb->gpio_irq);
        return -EINVAL;
    }

    ret = devm_gpio_request(tusb->dev, tusb->gpio_irq, "tusb320_int");
    if (ret < 0)
    {
        dev_err(tusb->dev, "%s: gpio_request error, ret=%d, gpio=%d\n", __func__, ret, tusb->gpio_irq);
        return ret;
    }

    ret = gpio_direction_input(tusb->gpio_irq);
    if (ret < 0)
    {
        dev_err(tusb->dev, "%s: gpio_direction_input error, ret=%d, gpio=%d\n", __func__, ret, tusb->gpio_irq);
        return ret;
    }

    tusb->irq = gpio_to_irq(tusb->gpio_irq);
    if (tusb->irq < 0)
    {
        dev_err(tusb->dev, "%s: gpio_to_irq error, irq num=%d\n", __func__, tusb->irq);
        return -EINVAL;
    }

    ret = devm_request_threaded_irq(tusb->dev, tusb->irq, NULL, tusb320_irq_handler_thread,
                                    IRQF_ONESHOT | IRQF_TRIGGER_FALLING, "tusb320", tusb);
    if (ret)
        return ret;
    /* Allocate extcon device */
    tusb->edev = devm_extcon_dev_allocate(tusb->dev, tusb320_extcon_cable);
    if (IS_ERR(tusb->edev))
    {
        dev_err(tusb->dev, "failed to allocate memory for extcon\n");
        return -ENOMEM;
    }

    /* Register extcon device */
    ret = devm_extcon_dev_register(tusb->dev, tusb->edev);
    if (ret)
    {
        dev_err(tusb->dev, "failed to register extcon device\n");
        return ret;
    }

    extcon_set_property_capability(tusb->edev, EXTCON_USB, EXTCON_PROP_USB_VBUS);
    extcon_set_property_capability(tusb->edev, EXTCON_USB, EXTCON_PROP_USB_TYPEC_POLARITY);
    extcon_set_property_capability(tusb->edev, EXTCON_USB_HOST, EXTCON_PROP_USB_VBUS);
    extcon_set_property_capability(tusb->edev, EXTCON_USB_HOST, EXTCON_PROP_USB_TYPEC_POLARITY);

#if IS_ENABLED(CONFIG_USB_DWC3_HOST)
    ret = tusb->port->ops->port_type_set(tusb->port, TYPEC_PORT_SRC);
#endif
#if IS_ENABLED(CONFIG_USB_DWC3_GADGET)
    ret = tusb->port->ops->port_type_set(tusb->port, TYPEC_PORT_SNK);
#endif
    i2c_set_clientdata(client, tusb);

    return ret;
}

static const struct of_device_id tusb320_dt_match[] = {{
                                                           .compatible = "ti,tusb320la",
                                                       },
                                                       {
                                                           .compatible = "ti,tusb320ha",
                                                       },
                                                       {/* sentinel */}};
MODULE_DEVICE_TABLE(of, tusb320_dt_match);
static const struct i2c_device_id tusb320_i2c_id[] = {{"tusb320", 0}, {}};

static struct i2c_driver tusb320_driver = {
    .driver =
        {
            .name           = "tusb320",
            .of_match_table = tusb320_dt_match,
        },
    .probe    = tusb320_probe,
    .id_table = tusb320_i2c_id,
};

module_i2c_driver(tusb320_driver);

MODULE_AUTHOR("Zuhuang Zhang <zuhuang.zhang@sigmastar.com.cn>");
MODULE_DESCRIPTION("TUSB320 USB Type-C connector driver by SigmaStar");
MODULE_LICENSE("GPL v2");
