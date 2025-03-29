/*
 * sstar_lh_rtos_virtio.c- Sigmastar
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
/*
 * sstar_lh_rtos_virtio.c
 */

#include <linux/clk.h>
#include <linux/err.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/notifier.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_irq.h>
#include <linux/platform_device.h>
#include <linux/rpmsg.h>
#include <linux/slab.h>
#include <linux/virtio.h>
#include <linux/virtio_config.h>
#include <linux/virtio_ids.h>
#include <linux/virtio_ring.h>
#include <linux/arm-smccc.h>
#include <linux/smp.h>
#include <linux/delay.h>
#include <linux/version.h>
#include <linux/kthread.h>

#include "ms_platform.h"
#include "registers.h"
#include "drv_dualos.h"
#include "cam_inter_os.h"
#include <asm/smp_plat.h>

#ifdef CONFIG_ARM_GIC_V3
#include <asm/cputype.h>
#include <linux/irqchip/arm-gic-v3.h>
#endif

#define TARGET_BITS_CORE0 (1 << 16)
#define TARGET_BITS_CORE1 (1 << 17)
#define NSATT_BITS_GROUP0 (0 << 15)
#define NSATT_BITS_GROUP1 (1 << 15)
#define SGIINTID_BITS_08  (8)
#define SGIINTID_BITS_09  (9)
#define SGIINTID_BITS_10  (10)
#define SGIINTID_BITS_11  (11)
#define SGIINTID_BITS_15  (15)

typedef struct
{
    unsigned int arg0_l;
    unsigned int arg0_h;
    unsigned int arg1_l;
    unsigned int arg1_h;
    unsigned int arg2_l;
    unsigned int arg2_h;
    unsigned int arg3_l;
    unsigned int arg3_h;
    unsigned int ret_l;
    unsigned int ret_h;
} interos_call_mbox_args_t;

struct sstar_virdev
{
    struct virtio_device  vdev;
    unsigned int          vring[2];
    struct virtqueue *    vq[2];
    int                   base_vq_id;
    int                   num_of_vqs;
    struct notifier_block nb;

    wait_queue_head_t   wq;
    struct task_struct *task;
};

struct sstar_lh_rtos_virtio_vproc
{
    char *       rproc_name;
    struct mutex lock;
    int          vdev_nums;
#define MAX_VDEV_NUMS 1
    struct sstar_virdev ivdev[MAX_VDEV_NUMS];
};

struct sstar_lh_rtos_virtio_mbox
{
    const char *                  name;
    struct blocking_notifier_head notifier;
};

static struct sstar_lh_rtos_virtio_mbox rpmsg_mbox = {
    .name = "rtos",
};

static struct sstar_lh_rtos_virtio_vproc sstar_lh_rtos_virtio_vprocs[] = {
    {
        .rproc_name = "rtos",
    },
};

static u32 in_idx, out_idx;
#if defined(CONFIG_RPMSG_USE_SGI_TO_REPLACE_SMC_ON_GICV3)
static u32 rtos_core_id = 0;
#endif

/*
 * For now, allocate 256 buffers of 512 bytes for each side. each buffer
 * will then have 16B for the msg header and 496B for the payload.
 * This will require a total space of 256KB for the buffers themselves, and
 * 3 pages for every vring (the size of the vring depends on the number of
 * buffers it supports).
 */
#define RPMSG_NUM_BUFS   (512)
#define RPMSG_BUF_SIZE   (512)
#define RPMSG_BUFS_SPACE (RPMSG_NUM_BUFS * RPMSG_BUF_SIZE)

/*
 * The alignment between the consumer and producer parts of the vring.
 * Note: this is part of the "wire" protocol. If you change this, you need
 * to update your BIOS image as well
 */
#define RPMSG_VRING_ALIGN (4096)

/* With 256 buffers, our vring will occupy 3 pages */
#define RPMSG_RING_SIZE ((DIV_ROUND_UP(vring_size(RPMSG_NUM_BUFS / 2, RPMSG_VRING_ALIGN), PAGE_SIZE)) * PAGE_SIZE)

#define to_sstar_virdev(vd)    container_of(vd, struct sstar_virdev, vdev)
#define to_sstar_rpdev(vd, id) container_of(vd, struct sstar_lh_rtos_virtio_vproc, ivdev[id])

struct sstar_lh_rtos_virtio_vq_info
{
    __u16                              num;   /* number of entries in the virtio_ring */
    __u16                              vq_id; /* a globaly unique index of this virtqueue */
    void *                             addr;  /* address where we mapped the virtio ring */
    struct sstar_lh_rtos_virtio_vproc *rpdev;
};

extern void recode_timestamp(int mark, const char *name);

static unsigned long sstar_lh_rtos_virtio_smc(u32 type)
{
    struct arm_smccc_res res;

    arm_smccc_smc(type, 0, 0, 0, 0, 0, 0, 0, &res);
    return res.a0;
}

void sstar_lh_rtos_virtio_reroute_smc(void)
{
    sstar_lh_rtos_virtio_smc(INTEROS_SC_L2R_RPMSG_NOTIFY);
}

void sstar_lh_rtos_virtio_isr(int irq)
{
    struct sstar_lh_rtos_virtio_vproc *vproc = &sstar_lh_rtos_virtio_vprocs[0];

    // printk("sstar_lh_rtos_virtio_isr() get int [%d]\r\n", irq);

    ++in_idx;
    dsb(sy);
    wake_up_all(&vproc->ivdev[0].wq);
}

static u64 sstar_lh_rtos_virtio_get_features(struct virtio_device *vdev)
{
    /* VIRTIO_RPMSG_F_NS has been made private */
    return 1 << 0;
}

static int sstar_lh_rtos_virtio_finalize_features(struct virtio_device *vdev)
{
    /* Give virtio_ring a chance to accept features */
    vring_transport_features(vdev);
    return 0;
}

#ifdef CONFIG_ARM_GIC_V3
#define MPIDR_TO_SGI_AFFINITY(cluster_id, level) \
    (MPIDR_AFFINITY_LEVEL(cluster_id, level) << ICC_SGI1R_AFFINITY_##level##_SHIFT)

static void _arm_gic_reg_write_ICC_SGI1R_u64(u64 val)
{
    asm volatile(" mcrr     p15, 0, %0, %1, c12\n\t" ::"r"((val)&0xFFFFFFFF), "r"((val) >> 32) : "memory", "cc");
}
#else // CONFIG_ARM_GIC_V2
#define GICD_SGIR         0x0F00
#define GICD_BASE         0xF4001000
#define GICD_WRITEL(a, v) (*(volatile unsigned int *)(u32)(GICD_BASE + a) = (v))
#endif

static void sstar_lh_rtos_virtio_send_ipi(int cpu, int int_id)
{
#ifdef CONFIG_ARM_GIC_V3
    u64 cluster_id = 0;
    u16 tgt_list   = ((1 << cpu) & 0xFFFF);
    u64 val        = 0;

    val =
        (MPIDR_TO_SGI_AFFINITY(cluster_id, 3) | MPIDR_TO_SGI_AFFINITY(cluster_id, 2) | int_id << ICC_SGI1R_SGI_ID_SHIFT
         | MPIDR_TO_SGI_AFFINITY(cluster_id, 1) | tgt_list << ICC_SGI1R_TARGET_LIST_SHIFT);

    _arm_gic_reg_write_ICC_SGI1R_u64(val);
#else // CONFIG_ARM_GIC_V2
    GICD_WRITEL(GICD_SGIR, (1 << (cpu + 16)) | (1 << 15) | int_id);
#endif
}

/* kick the remote processor, and let it know which virtqueue to poke at */
static void sstar_lh_rtos_virtio_notify_cpu(int cpu)
{
#if defined(CONFIG_SMP)

#ifdef CONFIG_ARM_GIC_V3
#if defined(CONFIG_RPMSG_USE_SGI_TO_REPLACE_SMC_ON_GICV3)
    sstar_lh_rtos_virtio_send_ipi(cpu, IPI_NR_LINUX_2_RTOS_RPMSG);
#else
    if (cpu == 0x0)
        sstar_lh_rtos_virtio_smc(INTEROS_SC_L2R_RPMSG_NOTIFY);
    else // Linux core0 handle this interrupt and reroute SMC to RTOS
        sstar_lh_rtos_virtio_send_ipi(0x0, IPI_NR_LINUX_2_RTOS_RPMSG);
#endif
#else // CONFIG_ARM_GIC_V2
    if (cpu == 0x0)
        sstar_lh_rtos_virtio_smc(INTEROS_SC_L2R_RPMSG_NOTIFY);
    else // Linux core0 handle this interrupt and reroute SMC to RTOS
        sstar_lh_rtos_virtio_send_ipi(0x0, IPI_NR_LINUX_2_RTOS_RPMSG);
#endif

#elif defined(CONFIG_SS_AMP)

#ifdef CONFIG_ARM_GIC_V3
    sstar_lh_rtos_virtio_send_ipi(0x1, IPI_NR_LINUX_2_RTOS_RPMSG);
#else // CONFIG_ARM_GIC_V2, Linux send SMC to MON FW (secure mode) and trigger sending SGI to RTOS
    struct arm_smccc_res res;
    arm_smccc_smc(INTEROS_SC_L2R_CALL, TARGET_BITS_CORE1, NSATT_BITS_GROUP0, SGIINTID_BITS_15, 0, 0, 0, 0, &res);
#endif

#else // LH

#ifdef CONFIG_ARM_GIC_V3
#if defined(CONFIG_RPMSG_USE_SGI_TO_REPLACE_SMC_ON_GICV3)
    sstar_lh_rtos_virtio_send_ipi(rtos_core_id, IPI_NR_LINUX_2_RTOS_RPMSG);
#else
    sstar_lh_rtos_virtio_smc(INTEROS_SC_L2R_RPMSG_NOTIFY);
#endif
#else // CONFIG_ARM_GIC_V2
    sstar_lh_rtos_virtio_smc(INTEROS_SC_L2R_RPMSG_NOTIFY);
#endif

#endif
}

static bool sstar_lh_rtos_virtio_notify(struct virtqueue *vq)
{
    if (get_rtkinfo() && get_rtkinfo()->has_dead)
        return false;

#if defined(CONFIG_SMP)
#if defined(CONFIG_RPMSG_USE_SGI_TO_REPLACE_SMC_ON_GICV3)
    sstar_lh_rtos_virtio_notify_cpu(rtos_core_id);
#else
    int cpu = get_cpu();
    sstar_lh_rtos_virtio_notify_cpu(cpu);
    put_cpu();
#endif
#elif defined(CONFIG_SS_AMP)
    sstar_lh_rtos_virtio_notify_cpu(1);
#else
    sstar_lh_rtos_virtio_notify_cpu(0);
#endif
    return true;
}

static int sstar_lh_rtos_virtio_callback(struct notifier_block *this, unsigned long index, void *data)
{
    u32                  id = 0;
    struct sstar_virdev *virdev;

    virdev = container_of(this, struct sstar_virdev, nb);

    /*
     * We can't known which virtqueue triggers the interrupt,
     * so let's iterate all the virtqueues.
     */
    id = virdev->base_vq_id;
    for (; id < virdev->num_of_vqs; id++)
        vring_interrupt(id, virdev->vq[id]);

    return NOTIFY_DONE;
}

int sstar_lh_rtos_virtio_register_nb(const char *name, struct notifier_block *nb)
{
    if ((name == NULL) || (nb == NULL))
        return -EINVAL;

    if (!strcmp(rpmsg_mbox.name, name))
        blocking_notifier_chain_register(&(rpmsg_mbox.notifier), nb);
    else
        return -ENOENT;

    return 0;
}

int sstar_lh_rtos_virtio_unregister_nb(const char *name, struct notifier_block *nb)
{
    if ((name == NULL) || (nb == NULL))
        return -EINVAL;

    if (!strcmp(rpmsg_mbox.name, name))
        blocking_notifier_chain_unregister(&(rpmsg_mbox.notifier), nb);
    else
        return -ENOENT;

    return 0;
}

static struct virtqueue *rp_find_vq(struct virtio_device *vdev, unsigned int            index,
                                    void (*callback)(struct virtqueue *vq), const char *name)
{
    struct sstar_virdev *                virdev = to_sstar_virdev(vdev);
    struct sstar_lh_rtos_virtio_vproc *  rpdev  = to_sstar_rpdev(virdev, virdev->base_vq_id / 2);
    struct sstar_lh_rtos_virtio_vq_info *rpvq;
    struct virtqueue *                   vq;
    int                                  err;

    rpvq = kmalloc(sizeof(*rpvq), GFP_KERNEL);
    if (!rpvq)
        return ERR_PTR(-ENOMEM);

    /* ioremap'ing normal memory, so we cast away sparse's complaints */
    rpvq->addr = (__force void *)ioremap_wc(virdev->vring[index], RPMSG_RING_SIZE);
    if (!rpvq->addr)
    {
        err = -ENOMEM;
        goto free_rpvq;
    }

    memset(rpvq->addr, 0, RPMSG_RING_SIZE);

    pr_debug("vring%d: phys 0x%x, virt 0x%p\n", index, virdev->vring[index], rpvq->addr);

    vq = vring_new_virtqueue(index, RPMSG_NUM_BUFS / 2, RPMSG_VRING_ALIGN, vdev, true, false, rpvq->addr,
                             sstar_lh_rtos_virtio_notify, callback, name);
    if (!vq)
    {
        pr_err("vring_new_virtqueue failed\n");
        err = -ENOMEM;
        goto unmap_vring;
    }

    virdev->vq[index] = vq;
    vq->priv          = rpvq;
    /* system-wide unique id for this virtqueue */
    rpvq->vq_id = virdev->base_vq_id + index;
    rpvq->rpdev = rpdev;
    mutex_init(&rpdev->lock);

    return vq;

unmap_vring:
    /* iounmap normal memory, so make sparse happy */
    iounmap((__force void __iomem *)rpvq->addr);
free_rpvq:
    kfree(rpvq);
    return ERR_PTR(err);
}

static void sstar_lh_rtos_virtio_del_vqs(struct virtio_device *vdev)
{
    struct virtqueue *                 vq, *n;
    struct sstar_virdev *              virdev = to_sstar_virdev(vdev);
    struct sstar_lh_rtos_virtio_vproc *rpdev  = to_sstar_rpdev(virdev, virdev->base_vq_id / 2);

    list_for_each_entry_safe(vq, n, &vdev->vqs, list)
    {
        struct sstar_lh_rtos_virtio_vq_info *rpvq = vq->priv;

        iounmap(rpvq->addr);
        vring_del_virtqueue(vq);
        kfree(rpvq);
    }

    if (&virdev->nb)
        sstar_lh_rtos_virtio_unregister_nb((const char *)rpdev->rproc_name, &virdev->nb);
}

static int sstar_lh_rtos_virtio_find_vqs(struct virtio_device *vdev, unsigned int nvqs, struct virtqueue *vqs[],
                                         vq_callback_t *callbacks[], const char *const names[], const bool *ctx,
                                         struct irq_affinity *desc)
{
    struct sstar_virdev *              virdev = to_sstar_virdev(vdev);
    struct sstar_lh_rtos_virtio_vproc *rpdev  = to_sstar_rpdev(virdev, virdev->base_vq_id / 2);
    int                                i, err;

    /* we maintain two virtqueues per remote processor (for RX and TX) */
    if (nvqs != 2)
        return -EINVAL;

    for (i = 0; i < nvqs; ++i)
    {
        vqs[i] = rp_find_vq(vdev, i, callbacks[i], names[i]);
        if (IS_ERR(vqs[i]))
        {
            err = PTR_ERR(vqs[i]);
            goto error;
        }
    }

    virdev->num_of_vqs = nvqs;

    virdev->nb.notifier_call = sstar_lh_rtos_virtio_callback;
    sstar_lh_rtos_virtio_register_nb((const char *)rpdev->rproc_name, &virdev->nb);

    return 0;

error:
    sstar_lh_rtos_virtio_del_vqs(vdev);
    return err;
}

static void sstar_lh_rtos_virtio_reset(struct virtio_device *vdev)
{
    dev_dbg(&vdev->dev, "reset !\n");
}

static u8 sstar_lh_rtos_virtio_get_status(struct virtio_device *vdev)
{
    return 0;
}

static void sstar_lh_rtos_virtio_set_status(struct virtio_device *vdev, u8 status)
{
    dev_dbg(&vdev->dev, "%s new status: %d\n", __func__, status);
}

static void sstar_lh_rtos_virtio_vproc_release(struct device *dev)
{
    /* this handler is provided so driver core doesn't yell at us */
}

static struct virtio_config_ops sstar_lh_rtos_virtio_config_ops = {
    .get_features      = sstar_lh_rtos_virtio_get_features,
    .finalize_features = sstar_lh_rtos_virtio_finalize_features,
    .find_vqs          = sstar_lh_rtos_virtio_find_vqs,
    .del_vqs           = sstar_lh_rtos_virtio_del_vqs,
    .reset             = sstar_lh_rtos_virtio_reset,
    .set_status        = sstar_lh_rtos_virtio_set_status,
    .get_status        = sstar_lh_rtos_virtio_get_status,
};

static const struct of_device_id sstar_lh_rtos_virtio_dt_ids[] = {{
                                                                      .compatible = "sstar,sstar-lh-rtos-virtio",
                                                                  },
                                                                  {/* sentinel */}};
MODULE_DEVICE_TABLE(of, sstar_lh_rtos_virtio_dt_ids);

#define SHARE_SIZE (0x6000)
#ifdef CONFIG_ARM_LPAE
static u64 sstar_lh_rtos_virtio_share_area;
#else
static u32 sstar_lh_rtos_virtio_share_area;
#endif
static int set_vring_phy_buf(struct platform_device *pdev, struct sstar_lh_rtos_virtio_vproc *rpdev, int vdev_nums)
{
    struct resource *res;
    resource_size_t  size;
    unsigned int     start, end;
    int              i, ret = 0;

    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if (!res && sstar_lh_rtos_virtio_share_area != 0x0)
    {
        res = request_mem_region(sstar_lh_rtos_virtio_share_area, SHARE_SIZE, "rpmsg");
    }

    if (res)
    {
        size  = resource_size(res);
        start = res->start;
        end   = res->start + size;
        for (i = 0; i < vdev_nums; i++)
        {
            rpdev->ivdev[i].vring[0] = start;
            rpdev->ivdev[i].vring[1] = start + RPMSG_RING_SIZE;
            start += RPMSG_RING_SIZE * 2;
            if (start > end)
            {
                pr_err("Too small memory size %x!\n", (u32)size);
                ret = -EINVAL;
                break;
            }
        }
    }
    else
    {
        return -ENOMEM;
    }

    return ret;
}

static int rpmsg_irq_handler(void *arg)
{
    u32                                idx;
    struct sstar_lh_rtos_virtio_vproc *vproc = (struct sstar_lh_rtos_virtio_vproc *)arg;

    DEFINE_WAIT(wait);
    while (1)
    {
        if (kthread_should_stop())
            break;

        prepare_to_wait(&vproc->ivdev[0].wq, &wait, TASK_UNINTERRUPTIBLE);
        idx = in_idx;
        if (idx == out_idx)
            schedule();
        finish_wait(&vproc->ivdev[0].wq, &wait);

        while (out_idx != idx)
        {
            blocking_notifier_call_chain(&(rpmsg_mbox.notifier), 0, NULL);
            out_idx++;
        }
    }
    return 0;
}

static int sstar_lh_rtos_virtio_probe(struct platform_device *pdev)
{
    int                 i, j, ret = 0;
    struct device_node *np = pdev->dev.of_node;

    BLOCKING_INIT_NOTIFIER_HEAD(&(rpmsg_mbox.notifier));

    pr_info("RPMSG is ready for cross core communication!\n");

    for (i = 0; i < ARRAY_SIZE(sstar_lh_rtos_virtio_vprocs); i++)
    {
        struct sstar_lh_rtos_virtio_vproc *rpdev = &sstar_lh_rtos_virtio_vprocs[i];

        ret = of_property_read_u32_index(np, "vdev-nums", i, &rpdev->vdev_nums);
        if (ret)
            rpdev->vdev_nums = 1;
        if (rpdev->vdev_nums > MAX_VDEV_NUMS)
        {
            pr_err("vdev-nums exceed the max %d\n", MAX_VDEV_NUMS);
            return -EINVAL;
        }

        if (!strcmp(rpdev->rproc_name, "rtos"))
        {
            ret = set_vring_phy_buf(pdev, rpdev, rpdev->vdev_nums);
            if (ret)
            {
                pr_err("No vring buffer.\n");
                return -ENOMEM;
            }
        }
        else
        {
            pr_err("No remote rtos processor.\n");
            return -ENODEV;
        }

        for (j = 0; j < rpdev->vdev_nums; j++)
        {
            pr_debug("%s rpdev%d vdev%d: vring0 0x%x, vring1 0x%x\n", __func__, i, rpdev->vdev_nums,
                     rpdev->ivdev[j].vring[0], rpdev->ivdev[j].vring[1]);
            rpdev->ivdev[j].vdev.id.device   = VIRTIO_ID_RPMSG;
            rpdev->ivdev[j].vdev.id.vendor   = (RPMsg_Device_LH_RTOS << 16) | 0x0;
            rpdev->ivdev[j].vdev.config      = &sstar_lh_rtos_virtio_config_ops;
            rpdev->ivdev[j].vdev.dev.parent  = &pdev->dev;
            rpdev->ivdev[j].vdev.dev.release = sstar_lh_rtos_virtio_vproc_release;
            rpdev->ivdev[j].base_vq_id       = j * 2;
            init_waitqueue_head(&rpdev->ivdev[j].wq);
#if defined(CONFIG_SMP) && defined(CONFIG_RPMSG_USE_SGI_TO_REPLACE_SMC_ON_GICV3)
            rpdev->ivdev[j].task = kthread_create_on_cpu(rpmsg_irq_handler, rpdev, 0 /*rtos_core_id*/, "rpmsg_irq/1");
#else
            rpdev->ivdev[j].task = kthread_create(rpmsg_irq_handler, rpdev, "rpmsg_irq/%d", j);
#endif
            if (IS_ERR(rpdev->ivdev[j].task))
            {
                pr_err("%s failed to create irq worker for vdev %d:%d, err %ld", __func__, i, j,
                       PTR_ERR(rpdev->ivdev[j].task));
                return PTR_ERR(rpdev->ivdev[j].task);
            }
            set_user_nice(rpdev->ivdev[j].task, MIN_NICE);
            wake_up_process(rpdev->ivdev[j].task);

            ret = register_virtio_device(&rpdev->ivdev[j].vdev);
            if (ret)
            {
                kthread_stop(rpdev->ivdev[j].task);
                pr_err("%s failed to register rpdev: %d\n", __func__, ret);
                return ret;
            }
        }
    }
    return ret;
}

static int disable_rtos_virtio = 0;
#if 1
static int __init disable_rtos_virtio_func(char *str)
{
    disable_rtos_virtio = simple_strtol(str, NULL, 10);
    return 0;
}
// early_param("disable_rtos_virtio", disable_rtos_virtio_func);
early_param("disable_rtos", disable_rtos_virtio_func);
#else
module_param(disable_rtos_virtio, int, 0644);
MODULE_PARM_DESC(disable_rtos_virtio, "Disable RTOS IPC");
#endif
static struct platform_driver sstar_lh_rtos_virtio_driver = {
    .driver =
        {
            .owner          = THIS_MODULE,
            .name           = "sstar-lh-rtos-virtio",
            .of_match_table = sstar_lh_rtos_virtio_dt_ids,
        },
    .probe = sstar_lh_rtos_virtio_probe,
};

static int __init sstar_lh_rtos_virtio_init(void)
{
    int ret;

    if (disable_rtos_virtio)
    {
        pr_info("RTOS RPMSG disabled - not registering driver");
        return 0;
    }

    ret = platform_driver_register(&sstar_lh_rtos_virtio_driver);
    if (ret)
        pr_err("Unable to initialize lh-rtos-virtio driver\n");
    else
        pr_info("sstar lh-rtos-virtio driver is registered.\n");

    return ret;
}

static void sstar_lh_rtos_virtio_init_kick_linkup(void)
{
#ifdef CONFIG_ARM_GIC_V3
#if defined(CONFIG_SS_AMP)
    sstar_lh_rtos_virtio_send_ipi(0x1, IPI_NR_LINUX_2_RTOS_RPMSG);
#else // LH or SMPLH
#if defined(CONFIG_RPMSG_USE_SGI_TO_REPLACE_SMC_ON_GICV3)
    int cpu_idx = 0;
    for (cpu_idx = 0; cpu_idx < CONFIG_NR_CPUS; cpu_idx++)
        sstar_lh_rtos_virtio_send_ipi(cpu_idx, IPI_NR_LINUX_2_RTOS_RPMSG);
#else
// NOP
#endif
#endif
#else // CONFIG_ARM_GIC_V2
#if defined(CONFIG_SS_AMP)
    struct arm_smccc_res res;
    arm_smccc_smc(INTEROS_SC_L2R_CALL, TARGET_BITS_CORE1, NSATT_BITS_GROUP0, SGIINTID_BITS_15, 0, 0, 0, 0, &res);
#else // LH or SMPLH
    // NOP
#endif
#endif
}

static int __init sstar_lh_rtos_virtio_shm_init_by_ipi(void)
{
    interos_call_mbox_args_t *ptr_mbox_args;
    u32                       marker = 0;
    int                       count  = 10000;

    ptr_mbox_args         = (interos_call_mbox_args_t *)(BASE_REG_MAILBOX_PA + BK_REG(0x60) + IO_OFFSET);
    ptr_mbox_args->arg0_l = 0;
    ptr_mbox_args->arg0_h = 0;
    ptr_mbox_args->ret_l  = 0;
    ptr_mbox_args->ret_h  = 0;
#if !defined(CONFIG_LINUX_ON_SS_HYPERVISOR)
    // Send Linux main HW core ID to RTOS
    ptr_mbox_args->arg3_l = cpu_logical_map(0);
#endif
    sstar_lh_rtos_virtio_init_kick_linkup();
#ifdef CONFIG_SS_PROFILING_TIME
    recode_timestamp(__LINE__, "rpmsg_shm_linkup");
#endif
    printk(KERN_INFO "[virtio] notify rpmsg shm linkup\n");
    udelay(10);
    while (count > 0)
    {
        marker = (ptr_mbox_args->arg0_h << 16) + ptr_mbox_args->arg0_l;
        if (marker == 0xf3f2f1f0)
            break;
        sstar_lh_rtos_virtio_init_kick_linkup();
        udelay(1000);
        --count;
    }

    if (marker == 0xf3f2f1f0)
    {
#ifdef CONFIG_ARM_LPAE
#ifdef CONFIG_MIU0_AT_1000000000
        sstar_lh_rtos_virtio_share_area = (u64)(u32)((ptr_mbox_args->ret_h << 16) + ptr_mbox_args->ret_l) + 0xFE0000000;
#else
        sstar_lh_rtos_virtio_share_area = (u64)(u32)((ptr_mbox_args->ret_h << 16) + ptr_mbox_args->ret_l);
#endif
#else
        sstar_lh_rtos_virtio_share_area = (u32)((ptr_mbox_args->ret_h << 16) + ptr_mbox_args->ret_l);
#endif
#if defined(CONFIG_RPMSG_USE_SGI_TO_REPLACE_SMC_ON_GICV3)
#if defined(CONFIG_LINUX_ON_SS_HYPERVISOR)
        rtos_core_id = signal_hyp(HYPERCALL_FID_VM_QUERY, HYPERCALL_SUBCMD_VM_QUERY_RTOS_MASTER, 0, 0);
#else
        rtos_core_id                    = (u32)ptr_mbox_args->arg3_l;
#endif
        // printk(KERN_INFO "rtos_core_id = %u\n", rtos_core_id);
#endif
#ifdef CONFIG_SS_PROFILING_TIME
        recode_timestamp(__LINE__, "rpmsg_shm_init");
#endif
    }
    else
    {
        printk(KERN_ERR "[virtio] failed to get address of rpmsg share area\n");
        disable_rtos_virtio             = 1;
        sstar_lh_rtos_virtio_share_area = 0x0;
        return 0;
    }

#ifdef CONFIG_ARM_LPAE
    printk(KERN_INFO "[virtio] address of rpmsg share area:0x%llx,%lu\n", sstar_lh_rtos_virtio_share_area,
           RPMSG_RING_SIZE);
#else
    printk(KERN_INFO "[virtio] address of rpmsg share area:0x%x,%lu\n", sstar_lh_rtos_virtio_share_area,
           RPMSG_RING_SIZE);
#endif
    return 0;
}

static int __init sstar_lh_rtos_virtio_shm_init_by_smc(void)
{
#ifdef CONFIG_ARM_LPAE
#ifdef CONFIG_MIU0_AT_1000000000
    sstar_lh_rtos_virtio_share_area = (u64)(u32)sstar_lh_rtos_virtio_smc(INTEROS_SC_L2R_RPMSG_HANDSHAKE) + 0xFE0000000;
#else
    sstar_lh_rtos_virtio_share_area = (u64)(u32)sstar_lh_rtos_virtio_smc(INTEROS_SC_L2R_RPMSG_HANDSHAKE);
#endif
    printk(KERN_INFO "address of rpmsg share area:0x%llx,%lu\n", sstar_lh_rtos_virtio_share_area, RPMSG_RING_SIZE);
#else
    sstar_lh_rtos_virtio_share_area = (u32)sstar_lh_rtos_virtio_smc(INTEROS_SC_L2R_RPMSG_HANDSHAKE);
    printk(KERN_INFO "address of rpmsg share area:0x%x,%lu\n", sstar_lh_rtos_virtio_share_area, RPMSG_RING_SIZE);
#endif
    return 0;
}

/*
 * Use pure_initcall to make sure that it will be called on CPU0.
 */
int __init sstar_lh_rtos_virtio_shm_init(void)
{
    if (disable_rtos_virtio)
    {
        pr_info("RTOS RPMSG disabled - not registering driver");
        return 0;
    }

#ifdef CONFIG_ARM_GIC_V3
#if defined(CONFIG_RPMSG_USE_SGI_TO_REPLACE_SMC_ON_GICV3)
    sstar_lh_rtos_virtio_shm_init_by_ipi();
#else
#if defined(CONFIG_SS_AMP)
    sstar_lh_rtos_virtio_shm_init_by_ipi();
#else // LH or SMPLH
    sstar_lh_rtos_virtio_shm_init_by_smc();
#endif
#endif
#else // CONFIG_ARM_GIC_V2
#if defined(CONFIG_SS_AMP)
    sstar_lh_rtos_virtio_shm_init_by_ipi();
#else // LH or SMPLH
    sstar_lh_rtos_virtio_shm_init_by_smc();
#endif
#endif

    return 0;
}
pure_initcall(sstar_lh_rtos_virtio_shm_init);

MODULE_AUTHOR("SSTAR");
MODULE_DESCRIPTION("SSTAR lh-rtos virtio device driver");
MODULE_LICENSE("GPL");
subsys_initcall(sstar_lh_rtos_virtio_init);
