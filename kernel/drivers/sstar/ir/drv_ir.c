/*
 * drv_ir.c- Sigmastar
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

#include <linux/delay.h>
#include <linux/err.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_device.h>
#include <linux/freezer.h>
#include <media/rc-core.h>
#include <linux/gpio.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <linux/clk-provider.h>
#include <linux/ctype.h>
#include <linux/irqreturn.h>
#include <linux/sched/clock.h>
#include <ms_msys.h>
#include <ms_platform.h>
#ifdef CONFIG_CAM_CLK
#include <drv_camclk_Api.h>
#endif
#include <hal_ir.h>

#define DRV_IR_MAXQUEUE 100
struct ir_key_queue
{
    int front;
    int rear;
    u32 item[DRV_IR_MAXQUEUE];
};

struct ir_rc_attr
{
    u32                  vendor;
    u32                  product;
    u32                  protocol;
    char                 rc_name[16];
    char                 map_name[16];
    char                 input_name[20];
    char                 input_phys[16];
    struct rc_dev *      rcdev;
    struct rc_map_list   map_list;
    struct rc_map_table *map_table;
};

struct ir_hw_decoder
{
    u32  irq;
    u32  rc_irq;
    u32  group;
    u32  pre_key;
    u32  decode_mode;
    char ir_irqname[4];
    char rc_irqname[4];

    int                      workrun;
    struct completion        key_done;
    struct work_struct       key_dispatch_wk;
    struct workqueue_struct *key_dispatch_wq;

    struct ir_key_queue queue;
    struct semaphore    queue_lock;

    struct device *    sysdev;
    struct timer_list  timer;
    struct ir_rc_attr  rc_attr;
    struct hal_ir_dev *hal_ir_dev;

#ifdef CONFIG_CAM_CLK
    void *camclk;
#else
    struct clk *clk;
#endif
};

static void sstar_ir_key_enqueue(struct ir_hw_decoder *ir_dev, u32 data)
{
    if (down_trylock(&ir_dev->queue_lock))
    {
        return;
    }
    if (ir_dev->queue.front == ((ir_dev->queue.rear + 1) % DRV_IR_MAXQUEUE))
    {
        ir_dbg("the data queue of ir%u is full\n", ir_dev->group);
    }
    else
    {
        ir_dev->queue.item[ir_dev->queue.rear] = data;
        ir_dev->queue.rear                     = (ir_dev->queue.rear + 1) % DRV_IR_MAXQUEUE;
    }
    up(&ir_dev->queue_lock);
}

static u32 sstar_ir_key_dequeue(struct ir_hw_decoder *ir_dev)
{
    u32 data = 0xFFFF;

    down(&ir_dev->queue_lock);
    if (ir_dev->queue.front == ir_dev->queue.rear)
    {
        ir_dbg("the data queue of ir%u is empty\n", ir_dev->group);
    }
    else
    {
        data                = ir_dev->queue.item[ir_dev->queue.front];
        ir_dev->queue.front = (ir_dev->queue.front + 1) % DRV_IR_MAXQUEUE;
    }
    up(&ir_dev->queue_lock);

    return data;
}

static void sstar_ir_set_int_enable(struct ir_hw_decoder *ir_dev, u8 enable)
{
    if (enable)
    {
        enable_irq(ir_dev->irq);
        enable_irq(ir_dev->rc_irq);
    }
    else
    {
        disable_irq(ir_dev->irq);
        disable_irq(ir_dev->rc_irq);
    }
}

static void sstar_ir_key_dispatch_thread(struct work_struct *work)
{
    int                   ret         = 0;
    u8                    flag        = 0;
    u32                   current_key = 0;
    struct ir_hw_decoder *ir_dev      = container_of(work, struct ir_hw_decoder, key_dispatch_wk);

    ir_dev->pre_key = 0x7FFF;
    ir_dbg("current thread %d: %s enter\n", current->pid, current->comm);

    while (ir_dev->workrun)
    {
        try_to_freeze();

        if (ir_dev->pre_key == 0x7FFF)
        {
            ret = wait_for_completion_interruptible(&ir_dev->key_done);
        }
        else
        {
            /*
             * Depend on different IR to wait timeout.
             * IR_TYPE_MSTAR_DTV, 150 is better, because ISR need such time to get another ir key.
             *
             * NOTE:
             * Too small, you will find the repeat function in android don't work. (up immediately)
             * It will become down->up->down->down.....(not continue down)
             * In input driver(2.6.35), over REP_DELAY(250 msecs) will auto-repeat, and every REP_PERIOD(33 msecs) will
             * send repeat key. In input driver(3.0.20), over REP_DELAY(500 msecs) will auto-repeat, and every
             * REP_PERIOD(125 msecs) will send repeat key. In android, over DEFAULT_LONG_PRESS_TIMEOUT(500 mesc) will
             * auto-repeat, and every KEY_REPEAT_DELAY(50 mesc) will send repeat key.
             */
            ret = wait_for_completion_interruptible_timeout(&ir_dev->key_done, msecs_to_jiffies(HAL_IR_EVENT_TIMEOUT));
        }

        if (!ir_dev->workrun)
            return;

        if (ret < 0)
        {
            ir_dbg("%d: %s interrupted before wait time becomes zero\n", current->pid, current->comm);
            continue;
        }

        ir_dbg("current thread %d: %s preparing to process queue data\n", current->pid, current->comm);
        current_key = sstar_ir_key_dequeue(ir_dev);
        flag        = current_key & 0x1;
        current_key = current_key >> 0x1;
        if ((ir_dev->pre_key != 0x7FFF) && (current_key == 0x7FFF))
        {
            rc_keyup(ir_dev->rc_attr.rcdev);
        }
        else if ((ir_dev->pre_key != 0x7FFF) && (current_key != 0x7FFF) && (ir_dev->pre_key != current_key))
        {
            rc_keyup(ir_dev->rc_attr.rcdev);
            rc_keydown_notimeout(ir_dev->rc_attr.rcdev, ir_dev->rc_attr.protocol, current_key, 0);
        }
        else if ((ir_dev->pre_key == 0x7FFF) && (current_key != 0x7FFF))
        {
            rc_keydown_notimeout(ir_dev->rc_attr.rcdev, ir_dev->rc_attr.protocol, current_key, 0);
        }
        else if ((ir_dev->pre_key != 0x7FFF) && (current_key != 0x7FFF) && (ir_dev->pre_key == current_key) && !flag)
        {
            rc_keyup(ir_dev->rc_attr.rcdev);
            rc_keydown_notimeout(ir_dev->rc_attr.rcdev, ir_dev->rc_attr.protocol, current_key, 0);
        }

        ir_dev->pre_key = current_key;
    }
}

static void sstar_ir_event_handle(struct timer_list *t)
{
    struct ir_raw_event   event;
    struct ir_hw_decoder *ir_dev = from_timer(ir_dev, t, timer);

    memset(&event, 0, sizeof(struct ir_raw_event));
    event.duration = 0x3FFF;
    ir_raw_event_store(ir_dev->rc_attr.rcdev, &event);
    ir_raw_event_handle(ir_dev->rc_attr.rcdev);
}

static irqreturn_t sstar_ir_int_handler(int irq, void *dev_id)
{
    u8                    flag;
    u32                   value;
    struct ir_hw_decoder *ir_dev = (struct ir_hw_decoder *)dev_id;

    ir_dbg("interrupt belonging to ir%u is triggered\n", ir_dev->group);
    value = hal_ir_get_key(ir_dev->hal_ir_dev);
    flag  = ir_dev->hal_ir_dev->decode_info->flag;
    if (value)
    {
        sstar_ir_key_enqueue(ir_dev, value << 0x1 | flag);
        complete(&ir_dev->key_done);
    }
    else if (ir_dev->hal_ir_dev->decode_mode == HAL_IR_SW_MODE)
    {
        u32                 i;
        struct ir_raw_event event;

        memset(&event, 0, sizeof(struct ir_raw_event));
        for (i = 0; i < ir_dev->hal_ir_dev->sw_shot_total; i++)
        {
            event.pulse    = ~(ir_dev->hal_ir_dev->sw_shot_type[i]);
            event.duration = ir_dev->hal_ir_dev->sw_shot_count[i] / HAL_IR_GETCNT(1);
            ir_raw_event_store(ir_dev->rc_attr.rcdev, &event);
        }
        ir_raw_event_handle(ir_dev->rc_attr.rcdev);
        mod_timer(&ir_dev->timer, jiffies + msecs_to_jiffies(3));
    }

    return IRQ_HANDLED;
}

static ssize_t sstar_ir_decode_mode_store(struct device *dev, struct device_attribute *attr, const char *buf,
                                          size_t size)
{
    int                   ret;
    u32                   decode_mode;
    u32                   customer_cdoe;
    struct ir_hw_decoder *ir_dev = (struct ir_hw_decoder *)dev_get_drvdata(dev);

    ret = sscanf(buf, "%u %u", &decode_mode, &customer_cdoe);
    if (ret == 1)
    {
        ir_dev->hal_ir_dev->decode_mode = decode_mode;
        hal_ir_config(ir_dev->hal_ir_dev);
    }
    else if (ret == 2)
    {
        ir_dev->hal_ir_dev->decode_mode   = decode_mode;
        ir_dev->hal_ir_dev->full_ccode[0] = (customer_cdoe >> 8) & 0xFF;
        ir_dev->hal_ir_dev->full_ccode[1] = customer_cdoe & 0xFF;
        hal_ir_config(ir_dev->hal_ir_dev);
    }
    else
    {
        ir_err("correct format 1: echo [decode_mode] > decode_mode\n");
        ir_err("correct format 2: echo [decode_mode] [customer_cdoe] > decode_mode\n");
        return -EINVAL;
    }

    return size;
}

static ssize_t sstar_ir_decode_mode_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *                str    = buf;
    char *                end    = buf + PAGE_SIZE;
    struct ir_hw_decoder *ir_dev = (struct ir_hw_decoder *)dev_get_drvdata(dev);

    str += scnprintf(str, end - str, "/**\n");
    str += scnprintf(str, end - str, " *if decode_mode = 1, decoding by FULL\n");
    str += scnprintf(str, end - str, " *if decode_mode = 2, decoding by RAW\n");
    str += scnprintf(str, end - str, " *if decode_mode = 3, decoding by RC5\n");
    str += scnprintf(str, end - str, " *if decode_mode = 4, decoding by SW\n");
    str += scnprintf(str, end - str, " */\n");
    str += scnprintf(str, end - str, "decode_mode = %u\n ", ir_dev->hal_ir_dev->decode_mode);

    return (str - buf);
}
DEVICE_ATTR(decode_mode, 0644, sstar_ir_decode_mode_show, sstar_ir_decode_mode_store);

static int sstar_ir_of_dts(struct platform_device *pdev, struct ir_hw_decoder *ir_dev)
{
    int  i;
    int  retval;
    int  array_size;
    u32 *of_rc_map_table = NULL;

    retval = of_property_read_u32(pdev->dev.of_node, "mode", &ir_dev->decode_mode);
    if (retval < 0)
    {
        ir_dbg("fail to get the mode of ir%u from dts\n", ir_dev->group);
        return retval;
    }

    retval = of_property_read_u32(pdev->dev.of_node, "protocol", &ir_dev->rc_attr.protocol);
    if (retval < 0)
    {
        ir_dbg("fail to get the protocol of ir%u from dts\n", ir_dev->group);
        ir_dev->rc_attr.protocol = RC_PROTO_UNKNOWN;
    }
    ir_dev->rc_attr.map_list.map.rc_proto = ir_dev->rc_attr.protocol;

    retval = of_property_read_u32(pdev->dev.of_node, "vendor", &ir_dev->rc_attr.vendor);
    if (retval < 0)
    {
        ir_dbg("fail to get the vendor of ir%u from dts\n", ir_dev->group);
        ir_dev->rc_attr.vendor = ID_VENDOR;
    }

    retval = of_property_read_u32(pdev->dev.of_node, "product", &ir_dev->rc_attr.product);
    if (retval < 0)
    {
        ir_dbg("fail to get the product of ir%u from dts\n", ir_dev->group);
        ir_dev->rc_attr.product = ID_PRODUCT;
    }

    array_size = of_property_count_elems_of_size(pdev->dev.of_node, "rc-map-table", sizeof(u32));
    if (array_size < 0)
    {
        ir_dbg("fail to get the rc-map-table of ir%u from dts\n", ir_dev->group);
        return array_size;
    }

    of_rc_map_table           = kzalloc(array_size * sizeof(*of_rc_map_table), GFP_KERNEL);
    ir_dev->rc_attr.map_table = devm_kzalloc(&pdev->dev, array_size * sizeof(struct rc_map_table), GFP_KERNEL);
    if (!ir_dev->rc_attr.map_table || !of_rc_map_table)
    {
        ir_err("fait to allocate memory for rc map table\n");
        return -ENOMEM;
    }

    retval = of_property_read_u32_array(pdev->dev.of_node, "rc-map-table", of_rc_map_table, array_size);
    if (retval < 0)
    {
        ir_dbg("fail to get the rc-map-table of ir%u from dts\n", ir_dev->group);
        kfree(of_rc_map_table);
        of_rc_map_table = NULL;
        return retval;
    }
    for (i = 0; i < (array_size / 2); i++)
    {
        ir_dev->rc_attr.map_table[i].scancode = of_rc_map_table[2 * i];
        ir_dev->rc_attr.map_table[i].keycode  = of_rc_map_table[2 * i + 1];
    }
    ir_dev->rc_attr.map_list.map.scan = ir_dev->rc_attr.map_table;
    ir_dev->rc_attr.map_list.map.size = array_size / 2;
    kfree(of_rc_map_table);
    of_rc_map_table = NULL;

    ir_dev->hal_ir_dev->decode_mode   = ir_dev->decode_mode;
    ir_dev->hal_ir_dev->full_ccode[0] = ((ir_dev->rc_attr.map_table[0].scancode) >> 16) & 0xFF;
    ir_dev->hal_ir_dev->full_ccode[1] = ((ir_dev->rc_attr.map_table[0].scancode) >> 8) & 0xFF;

    return 0;
}

static int sstar_ir_rc_open(struct rc_dev *dev)
{
    int                   ret    = 0;
    struct ir_hw_decoder *ir_dev = NULL;

    ir_dev = (struct ir_hw_decoder *)dev->priv;
    if (!ir_dev)
    {
        ir_err("fail to open rc device\n");
        return -EINVAL;
    }

    ret = clk_prepare_enable(ir_dev->clk);
    if (ret)
    {
        ir_err("failed to enable ir clock\n");
        return ret;
    }

    return 0;
}

static void sstar_ir_rc_close(struct rc_dev *dev)
{
    struct ir_hw_decoder *ir_dev = NULL;

    ir_dev = (struct ir_hw_decoder *)dev->priv;
    if (!ir_dev)
    {
        ir_err("fail to close rc device\n");
        return;
    }

    clk_disable_unprepare(ir_dev->clk);
}

static int sstar_ir_input_init(struct ir_hw_decoder *ir_dev)
{
    int            retval = 0;
    struct rc_dev *dev;

    dev = rc_allocate_device(RC_DRIVER_IR_RAW);
    if (!dev)
    {
        ir_err("fail to allocate ir%u rc device\n", ir_dev->group);
        return -ENOMEM;
    }

    if ((snprintf(ir_dev->rc_attr.rc_name, sizeof(ir_dev->rc_attr.rc_name), "sstar-ir%u", ir_dev->group) >= 0))
        dev->driver_name = (const char *)(char *)ir_dev->rc_attr.rc_name;
    if ((snprintf(ir_dev->rc_attr.map_name, sizeof(ir_dev->rc_attr.map_name), "sstar-ir%u-map", ir_dev->group) >= 0))
        dev->map_name = (const char *)(char *)ir_dev->rc_attr.map_name;
    if ((snprintf(ir_dev->rc_attr.input_name, sizeof(ir_dev->rc_attr.input_name), "sstar-input-ir%u", ir_dev->group)
         >= 0))
        dev->device_name = (const char *)(char *)ir_dev->rc_attr.input_name;
    if ((snprintf(ir_dev->rc_attr.input_phys, sizeof(ir_dev->rc_attr.input_phys), "/dev/input") >= 0))
        dev->input_phys = ir_dev->rc_attr.input_phys;

    dev->driver_type       = RC_DRIVER_IR_RAW;
    dev->allowed_protocols = RC_PROTO_BIT_RC5 | RC_PROTO_BIT_NEC;
    dev->input_id.bustype  = BUS_VIRTUAL;
    dev->input_id.vendor   = ir_dev->rc_attr.vendor;
    dev->input_id.product  = ir_dev->rc_attr.product;
    dev->open              = sstar_ir_rc_open;
    dev->close             = sstar_ir_rc_close;
    dev->priv              = ir_dev;

    ir_dev->rc_attr.map_list.map.name = (const char *)(char *)ir_dev->rc_attr.map_name;
    rc_map_register(&ir_dev->rc_attr.map_list);

    retval = rc_register_device(dev);
    if (retval != 0)
    {
        rc_free_device(dev);
        ir_err("fail to register rc%u device\n", ir_dev->group);
        return retval;
    }

    // clear_bit(EV_REP, dev->input_dev->evbit);
    ir_dev->rc_attr.rcdev = dev;

    init_completion(&ir_dev->key_done);

    return 0;
}

static int sstar_ir_input_exit(struct ir_hw_decoder *ir_dev)
{
    rc_unregister_device(ir_dev->rc_attr.rcdev);
    rc_map_unregister(&ir_dev->rc_attr.map_list);

    return 0;
}

static int sstar_ir_workqueue_init(struct ir_hw_decoder *ir_dev)
{
    ir_dev->workrun         = 1;
    ir_dev->key_dispatch_wq = create_workqueue("keydispatch_wq");
    INIT_WORK(&ir_dev->key_dispatch_wk, sstar_ir_key_dispatch_thread);
    queue_work(ir_dev->key_dispatch_wq, &ir_dev->key_dispatch_wk);

    return 0;
}

static int sstar_ir_workqueue_exit(struct ir_hw_decoder *ir_dev)
{
    ir_dev->workrun = 0;
    complete(&ir_dev->key_done);
    destroy_workqueue(ir_dev->key_dispatch_wq);

    return 0;
}

static int sstar_ir_irq_release(struct ir_hw_decoder *ir_dev)
{
    free_irq(ir_dev->irq, ir_dev);
    if (ir_dev->group == 0)
        free_irq(ir_dev->rc_irq, ir_dev);

    return 0;
}

static int sstar_ir_irq_request(struct ir_hw_decoder *ir_dev)
{
    int irq_ret    = -1;
    int rc_irq_ret = -1;

    if ((snprintf(ir_dev->ir_irqname, sizeof(ir_dev->ir_irqname), "IR%u", ir_dev->group) >= 0))
    {
        irq_ret = request_irq(ir_dev->irq, sstar_ir_int_handler, IRQF_SHARED, ir_dev->ir_irqname, ir_dev);
    }
    if ((snprintf(ir_dev->rc_irqname, sizeof(ir_dev->rc_irqname), "RC%u", ir_dev->group) >= 0) && ir_dev->group == 0)
    {
        rc_irq_ret = request_irq(ir_dev->rc_irq, sstar_ir_int_handler, IRQF_SHARED, ir_dev->rc_irqname, ir_dev);
    }
    if (irq_ret != 0 && rc_irq_ret != 0)
    {
        ir_err("fail to request the irq of ir%u\n", ir_dev->group);
        sstar_ir_input_exit(ir_dev);
        sstar_ir_workqueue_exit(ir_dev);
        return irq_ret;
    }
    return 0;
}

u64 sstar_ir_get_sys_time(void)
{
    return sched_clock();
}

static int sstar_ir_probe(struct platform_device *pdev)
{
    int                   retval;
    struct resource *     res;
    struct ir_hw_decoder *ir_dev;
#ifdef CONFIG_CAM_CLK
    u32                  camclk_id = 0;
    CAMCLK_Set_Attribute set_cfg;
#else
    struct clk *ir_clk;
#endif

    ir_dev = devm_kzalloc(&pdev->dev, sizeof(*ir_dev), GFP_KERNEL);
    if (!ir_dev)
    {
        ir_err("fait to allocate memory\n");
        return -ENOMEM;
    }
    else
    {
        ir_dev->hal_ir_dev              = devm_kzalloc(&pdev->dev, sizeof(struct hal_ir_dev), GFP_KERNEL);
        ir_dev->hal_ir_dev->decode_info = devm_kzalloc(&pdev->dev, sizeof(struct hal_ir_key_info), GFP_KERNEL);
    }

    retval = of_property_read_u32(pdev->dev.of_node, "group", &ir_dev->group);
    if (retval < 0)
    {
        ir_err("fail to get group from dts\n");
        return retval;
    }

    ir_dev->irq    = irq_of_parse_and_map(pdev->dev.of_node, 0);
    ir_dev->rc_irq = irq_of_parse_and_map(pdev->dev.of_node, 1);
    if ((!ir_dev->irq) && (!ir_dev->rc_irq))
    {
        ir_err("fail to parse and map an ir%u interrupt into linux virq space\n", ir_dev->group);
        return -ENODEV;
    }

    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if (!res->start)
    {
        ir_err("fail to get a resource for ir%u device\n", ir_dev->group);
        return -EINVAL;
    }
    ir_dev->hal_ir_dev->membase = IO_ADDRESS(res->start);

#ifdef CONFIG_CAM_CLK
    of_property_read_u32_index(pdev->dev.of_node, "camclk", 0, &(camclk_id));
    if (!camclk_id)
    {
        ir_err("fail to get the camclk of ir%u from dts\n", ir_dev->group);
        return -CAMCLK_RET_NOTSUPPORT;
    }
    else
    {
        if (CamClkRegister("IR", camclk_id, &(ir_dev->camclk)) == CAMCLK_RET_OK)
        {
            CAMCLK_SETRATE_ROUND(set_cfg, HAL_IR_XTAL_CLKFREQ);
            CamClkAttrSet(ir_dev->camclk, &set_cfg);
            CamClkSetOnOff(ir_dev->camclk, 1);
        }
    }
#else
    ir_clk = of_clk_get(pdev->dev.of_node, 0);
    if (IS_ERR_OR_NULL(ir_clk))
    {
        ir_err("fail to allocate and link ir%u clk to clk_core\n", ir_dev->group);
        return PTR_ERR(ir_clk);
    }
    else
    {
        retval = clk_set_rate(ir_clk, HAL_IR_XTAL_CLKFREQ);
        if (retval < 0)
        {
            ir_err("fail to specify a new rate for ir%u clock\n", ir_dev->group);
            return retval;
        }
    }
    ir_dev->clk = ir_clk;
#endif

    platform_set_drvdata(pdev, ir_dev);

    ir_dev->sysdev = device_create(msys_get_sysfs_class(), NULL, pdev->dev.devt, NULL, "ir%u", ir_dev->group);
    if (ir_dev->sysdev)
    {
        dev_set_drvdata(ir_dev->sysdev, (void *)ir_dev);
        device_create_file(ir_dev->sysdev, &dev_attr_decode_mode);
    }
    else
    {
        ir_dbg("fail to create sysfs attribute file for ir%u device\n", ir_dev->group);
    }

    retval = sstar_ir_of_dts(pdev, ir_dev);
    if (retval)
    {
        ir_err("fail to get the property of ir%u\n", ir_dev->group);
        return retval;
    }

    retval = sstar_ir_input_init(ir_dev);
    if (retval != 0)
    {
        ir_err("fail to init ir%u input system\n", ir_dev->group);
        return retval;
    }

    sstar_ir_workqueue_init(ir_dev);
    sema_init(&ir_dev->queue_lock, 1);
    hal_ir_init(ir_dev->hal_ir_dev);
    ir_dev->hal_ir_dev->calbak_get_sys_time = sstar_ir_get_sys_time;
    retval                                  = sstar_ir_irq_request(ir_dev);
    if (retval != 0)
    {
        ir_err("fail to request the irq of ir%u\n", ir_dev->group);
        return retval;
    }

    timer_setup(&ir_dev->timer, sstar_ir_event_handle, 0);

    return 0;
}

static int sstar_ir_remove(struct platform_device *pdev)
{
    int                   ret = 0;
    struct clk_hw *       irclk_hw;
    struct clk_hw *       ir_parentclk_hw;
    struct ir_hw_decoder *ir_dev = platform_get_drvdata(pdev);

    if (!ir_dev)
    {
        ir_err("fail to get ir driver data\n");
        return -EINVAL;
    }

    sstar_ir_input_exit(ir_dev);
    sstar_ir_workqueue_exit(ir_dev);
    device_unregister(ir_dev->sysdev);

    if (ir_dev->clk)
    {
        irclk_hw        = __clk_get_hw(ir_dev->clk);
        ir_parentclk_hw = clk_hw_get_parent_by_index(irclk_hw, 0);
        if (IS_ERR_OR_NULL(ir_parentclk_hw))
        {
            ir_err("failed to get hw parent clk by index\n");
            return PTR_ERR(ir_parentclk_hw);
        }
        ret = clk_set_parent(ir_dev->clk, ir_parentclk_hw->clk);
        if (ret < 0)
        {
            ir_err("fail to reparent ir%u clock\n, ir_dev->group");
            return ret;
        }
        clk_put(ir_dev->clk);
    }

    sstar_ir_set_int_enable(ir_dev, 0);
    sstar_ir_irq_release(ir_dev);
    del_timer_sync(&ir_dev->timer);

    return 0;
}

#ifdef CONFIG_PM_SLEEP
static int sstar_ir_suspend(struct platform_device *pdev, pm_message_t state)
{
    struct ir_hw_decoder *ir_dev = platform_get_drvdata(pdev);

    if (!ir_dev)
    {
        ir_err("fail to get ir driver data\n");
        return -EINVAL;
    }

    sstar_ir_set_int_enable(ir_dev, 0);

    if (ir_dev->hal_ir_dev->decode_mode == HAL_IR_SW_MODE)
    {
        hal_ir_set_software(ir_dev->hal_ir_dev, 0);
    }

    return 0;
}

static int sstar_ir_resume(struct platform_device *pdev)
{
    struct ir_hw_decoder *ir_dev = platform_get_drvdata(pdev);

    if (!ir_dev)
    {
        ir_err("fail to get ir driver data\n");
        return -EINVAL;
    }

    hal_ir_init(ir_dev->hal_ir_dev);
    sstar_ir_set_int_enable(ir_dev, 1);

    return 0;
}
#endif

static const struct of_device_id sstar_ir_match_table[] = {{.compatible = "sstar,ir"}, {}};

static struct platform_driver sstar_ir_driver = {.probe  = sstar_ir_probe,
                                                 .remove = sstar_ir_remove,
#ifdef CONFIG_PM_SLEEP
                                                 .suspend = sstar_ir_suspend,
                                                 .resume  = sstar_ir_resume,
#endif
                                                 .driver = {
                                                     .name           = "sstar-ir",
                                                     .owner          = THIS_MODULE,
                                                     .of_match_table = sstar_ir_match_table,
                                                 }};

module_platform_driver(sstar_ir_driver);

MODULE_AUTHOR("SSTAR");
MODULE_DESCRIPTION("IR driver");
MODULE_LICENSE("GPL");
