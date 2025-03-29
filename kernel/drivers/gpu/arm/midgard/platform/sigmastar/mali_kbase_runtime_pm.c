// SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note
/*
 *
 * (C) COPYRIGHT 2015-2021 ARM Limited. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you can access it online at
 * http://www.gnu.org/licenses/gpl-2.0.html.
 *
 */

#include <mali_kbase.h>
#include <mali_kbase_defs.h>
#include <linux/pm_runtime.h>
#include <linux/clk.h>
#include <linux/clk-provider.h>
#include <linux/delay.h>
#include <linux/regulator/consumer.h>

#include "mali_kbase_config_platform.h"

static bool gpu_power_on_or_off = false;

static void enable_gpu_power_control(struct kbase_device *kbdev)
{
    unsigned int i;

#if defined(CONFIG_REGULATOR)
    for (i = 0; i < kbdev->nr_regulators; i++) {
        if (WARN_ON(kbdev->regulators[i] == NULL))
            ;
        else if (!regulator_is_enabled(kbdev->regulators[i]))
            WARN_ON(regulator_enable(kbdev->regulators[i]));
    }
#endif

    for (i = 0; i < kbdev->nr_clocks; i++) {
        if (WARN_ON(kbdev->clocks[i] == NULL))
            ;
        else if (!__clk_is_enabled(kbdev->clocks[i]))
            WARN_ON(clk_prepare_enable(kbdev->clocks[i]));
    }
}

static void disable_gpu_power_control(struct kbase_device *kbdev)
{
    unsigned int i;

    for (i = 0; i < kbdev->nr_clocks; i++) {
        if (WARN_ON(kbdev->clocks[i] == NULL))
            ;
        else if (__clk_is_enabled(kbdev->clocks[i])) {
            clk_disable_unprepare(kbdev->clocks[i]);
            WARN_ON(__clk_is_enabled(kbdev->clocks[i]));
        }

    }

#if defined(CONFIG_REGULATOR)
    for (i = 0; i < kbdev->nr_regulators; i++) {
        if (WARN_ON(kbdev->regulators[i] == NULL))
            ;
        else if (regulator_is_enabled(kbdev->regulators[i]))
            WARN_ON(regulator_disable(kbdev->regulators[i]));
    }
#endif
}

/*
 * Attention: Only for P5 + GPU version
 * TODO: Need some operation for power on failed
 */
static void p5_gpu_power_on(struct kbase_device *kbdev)
{
    unsigned short val;
    unsigned int cnt_delay = 20000U;

    // 1.power on, iso release
    val = INREG16(BASE_REG_CHIPTOP_PA + REG_ID_6F);
    OUTREG16(BASE_REG_CHIPTOP_PA + REG_ID_6F, (0xff7f & val)); // power off

    while(cnt_delay--)
    {
        val = INREG16(BASE_REG_CHIPTOP_PA + REG_ID_6F); // wait for power on ready, reg_gpu_power_off_rb
        if(val & 0x200U)
        {
            break;
        }
        udelay(5U);  //max delay 0.1s
    }
    if (cnt_delay == 0) {
        dev_err(kbdev->dev, "GPU power on ready failed!\n");
        return;
    }

    OUTREG16(BASE_REG_CHIPTOP_PA + REG_ID_6F, (0xfe7f & val)); // release iso
    udelay(1);  //20ns

    // 2.open pll
    OUTREG16(BASE_REG_GPUPLL_PA + REG_ID_60, 0x1eb8);
    OUTREG16(BASE_REG_GPUPLL_PA + REG_ID_61, 0x0045); // set ref-clk 50M
    OUTREG16(BASE_REG_GPUPLL_PA + REG_ID_62, 0x0001);
    //OUTREG16(BASE_REG_GPUPLL_PA + REG_ID_12, 0x105a); //GPU_clk=650M
    OUTREG16(BASE_REG_GPUPLL_PA + REG_ID_12, 0x108c);  //GPU-clk=600M
    OUTREG16(BASE_REG_GPUPLL_PA + REG_ID_11, 0x0080); // open GPU clk
    udelay(50); //50us wait for pll oscillation, ask for analog for real chip

    // 2.5.miu clk for GPU
    OUTREG16(BASE_REG_CLKGEN_PA + REG_ID_07, 0x1010);
    udelay(1);

    // 3.release GPU clock gate
    OUTREG16(BASE_REG_CLKGEN_PA + REG_ID_56, 0x0008);
    udelay(1);  //300ns

    // 4.release marble_block_sw reset on CHIPTOP
    SETREG16(BASE_REG_CHIPTOP_PA + REG_ID_1F, 0x0002);
    udelay(1);  //10ns

    // 5.release clk
    OUTREG16(BASE_REG_MARBLE_GP_PA + REG_ID_30, 0x0003); // open & select fast clock

    // 6.open sram
    OUTREG16(BASE_REG_MARBLE_GP_PA + REG_ID_32, 0x000f);

    val = INREG16(BASE_REG_DIG_GP_CTRL_PA + REG_ID_12);
    OUTREG16(BASE_REG_DIG_GP_CTRL_PA + REG_ID_12, (0x7f & val));

    OUTREG16(BASE_REG_MARBLE_GP_PA + REG_ID_20, 0x0f00); // release all/nodie
    OUTREG16(BASE_REG_MARBLE_GP_PA + REG_ID_20, 0x0500); // release all/die
    udelay(3);  //3us

    // disable sram clock force
    OUTREG16(BASE_REG_MARBLE_GP_PA + REG_ID_32, 0x0000);

    // 7.release GPU core reset
    OUTREG16(BASE_REG_MARBLE_GP_PA + REG_ID_20, 0x0400);

    // 8. close slow clock
    OUTREG16(BASE_REG_CLKGEN_PA + REG_ID_56, 0x0001);
    gpu_power_on_or_off = true;
}

static void p5_gpu_power_off(struct kbase_device *kbdev)
{
    // 0. write GPU_IRQ_CLEAR and GPU_IRQ_MASK
    SETREG16(BASE_REG_GPU_PA + REG_ID_09, 0x0080);
    SETREG16(BASE_REG_GPU_PA + REG_ID_0A, 0x0080);

    // 1. GPU SW reset
    OUTREG16(BASE_REG_GPU_PA + REG_ID_0C, 0x0001);

    // 2. wait GPU SW reset done
    //udelay(3);
    /*while((INREG16(BASE_REG_GPU_PA + REG_ID_0B) & 0x0100) != 1)
        udelay(1);*/

    // 3. check AXI reg
    while((INREG16(BASE_REG_MARBLE_GP_PA + REG_ID_4A) | 0x8a) != 0x02aa ||
          (INREG16(BASE_REG_MARBLE_GP_PA + REG_ID_4B) | 0x8a) != 0x02aa)
        udelay(1);

    // 4. sram sleep, not need if GPU is about to power off
    OUTREG16(BASE_REG_MARBLE_GP_PA + REG_ID_32, 0x000f);
    SETREG16(BASE_REG_DIG_GP_CTRL_PA + REG_ID_12, 0x0080);
    udelay(5);

    // 5. reset GPU block
    OUTREG16(BASE_REG_MARBLE_GP_PA + REG_ID_20, 0xffff);

    // 6. clk
    CLRREG16(BASE_REG_CHIPTOP_PA + REG_ID_1F, 0x0002);

    // 7. iso
    SETREG16(BASE_REG_CHIPTOP_PA + REG_ID_6F, 0x0100);
    SETREG16(BASE_REG_CHIPTOP_PA + REG_ID_6F, 0x0180);

    // 8. close GPU clk and pll
    OUTREG16(BASE_REG_CLKGEN_PA + REG_ID_56, 0x0001);
    OUTREG16(BASE_REG_GPUPLL_PA + REG_ID_11, 0x0180);

    gpu_power_on_or_off = false;
}


static int pm_callback_power_on(struct kbase_device *kbdev)
{
    int ret = 1; /* Assume GPU has been powered off */
    int error;
    unsigned long flags;

    dev_dbg(kbdev->dev, "%s %p\n", __func__,
            (void *)kbdev->dev->pm_domain);
    if (gpu_power_on_or_off == false) {
        p5_gpu_power_on(kbdev);
    }
    spin_lock_irqsave(&kbdev->hwaccess_lock, flags);
    WARN_ON(kbdev->pm.backend.gpu_powered);
#if MALI_USE_CSF
    if (likely(kbdev->csf.firmware_inited)) {
        WARN_ON(!kbdev->pm.active_count);
        WARN_ON(kbdev->pm.runtime_active);
    }
    spin_unlock_irqrestore(&kbdev->hwaccess_lock, flags);

    enable_gpu_power_control(kbdev);
    CSTD_UNUSED(error);
#else
    spin_unlock_irqrestore(&kbdev->hwaccess_lock, flags);

    enable_gpu_power_control(kbdev);
    error = pm_runtime_get_sync(kbdev->dev);

    if (error == 1) {
        /*
         * Let core know that the chip has not been
         * powered off, so we can save on re-initialization.
         */
        ret = 0;
    }

    dev_dbg(kbdev->dev, "pm_runtime_get_sync returned %d\n", error);
#endif /* MALI_USE_CSF */

    return ret;
}

static void pm_callback_power_off(struct kbase_device *kbdev)
{
    unsigned long flags;
    dev_dbg(kbdev->dev, "%s\n", __func__);

    spin_lock_irqsave(&kbdev->hwaccess_lock, flags);
    WARN_ON(kbdev->pm.backend.gpu_powered);
#if MALI_USE_CSF
    if (likely(kbdev->csf.firmware_inited)) {
        WARN_ON(kbase_csf_scheduler_get_nr_active_csgs(kbdev));
        WARN_ON(kbdev->pm.backend.mcu_state != KBASE_MCU_OFF);
    }
    spin_unlock_irqrestore(&kbdev->hwaccess_lock, flags);

    /* Power down the GPU immediately */
    disable_gpu_power_control(kbdev);
#else  /* MALI_USE_CSF */
    spin_unlock_irqrestore(&kbdev->hwaccess_lock, flags);

#ifdef KBASE_PM_RUNTIME
    pm_runtime_mark_last_busy(kbdev->dev);
    pm_runtime_put_autosuspend(kbdev->dev);
#else
    /* Power down the GPU immediately as runtime PM is disabled */
    disable_gpu_power_control(kbdev);
#endif
#endif /* MALI_USE_CSF */
}

#if MALI_USE_CSF && defined(KBASE_PM_RUNTIME)
static void pm_callback_runtime_gpu_active(struct kbase_device *kbdev)
{
    unsigned long flags;
    int error;

    lockdep_assert_held(&kbdev->pm.lock);

    spin_lock_irqsave(&kbdev->hwaccess_lock, flags);
    WARN_ON(!kbdev->pm.backend.gpu_powered);
    WARN_ON(!kbdev->pm.active_count);
    WARN_ON(kbdev->pm.runtime_active);
    spin_unlock_irqrestore(&kbdev->hwaccess_lock, flags);

    if (pm_runtime_status_suspended(kbdev->dev)) {
        error = pm_runtime_get_sync(kbdev->dev);
        dev_dbg(kbdev->dev, "pm_runtime_get_sync returned %d", error);
    } else {
        /* Call the async version here, otherwise there could be
         * a deadlock if the runtime suspend operation is ongoing.
         * Caller would have taken the kbdev->pm.lock and/or the
         * scheduler lock, and the runtime suspend callback function
         * will also try to acquire the same lock(s).
         */
        error = pm_runtime_get(kbdev->dev);
        dev_dbg(kbdev->dev, "pm_runtime_get returned %d", error);
    }

    kbdev->pm.runtime_active = true;
}

static void pm_callback_runtime_gpu_idle(struct kbase_device *kbdev)
{
    unsigned long flags;

    lockdep_assert_held(&kbdev->pm.lock);

    dev_dbg(kbdev->dev, "%s", __func__);

    spin_lock_irqsave(&kbdev->hwaccess_lock, flags);
    WARN_ON(!kbdev->pm.backend.gpu_powered);
    WARN_ON(kbdev->pm.backend.l2_state != KBASE_L2_OFF);
    WARN_ON(kbdev->pm.active_count);
    WARN_ON(!kbdev->pm.runtime_active);
    spin_unlock_irqrestore(&kbdev->hwaccess_lock, flags);

    pm_runtime_mark_last_busy(kbdev->dev);
    pm_runtime_put_autosuspend(kbdev->dev);
    kbdev->pm.runtime_active = false;
}
#endif

#ifdef KBASE_PM_RUNTIME
static int kbase_device_runtime_init(struct kbase_device *kbdev)
{
    int ret = 0;

    dev_dbg(kbdev->dev, "kbase_device_runtime_init\n");

    pm_runtime_set_autosuspend_delay(kbdev->dev, AUTO_SUSPEND_DELAY);
    pm_runtime_use_autosuspend(kbdev->dev);

    pm_runtime_set_active(kbdev->dev);
    pm_runtime_enable(kbdev->dev);

    if (!pm_runtime_enabled(kbdev->dev)) {
        dev_warn(kbdev->dev, "pm_runtime not enabled");
        ret = -EINVAL;
    } else if (atomic_read(&kbdev->dev->power.usage_count)) {
        dev_warn(kbdev->dev,
             "%s: Device runtime usage count unexpectedly non zero %d",
            __func__, atomic_read(&kbdev->dev->power.usage_count));
        ret = -EINVAL;
    }

    return ret;
}

static void kbase_device_runtime_disable(struct kbase_device *kbdev)
{
    dev_dbg(kbdev->dev, "kbase_device_runtime_disable\n");

    if (atomic_read(&kbdev->dev->power.usage_count))
        dev_warn(kbdev->dev,
             "%s: Device runtime usage count unexpectedly non zero %d",
            __func__, atomic_read(&kbdev->dev->power.usage_count));

    pm_runtime_disable(kbdev->dev);
}
#endif /* KBASE_PM_RUNTIME */

static int pm_callback_runtime_on(struct kbase_device *kbdev)
{
    dev_dbg(kbdev->dev, "pm_callback_runtime_on\n");
    if (gpu_power_on_or_off == false) {
        p5_gpu_power_on(kbdev);
    }
    enable_gpu_power_control(kbdev);
    return 0;
}

static void pm_callback_runtime_off(struct kbase_device *kbdev)
{
    dev_dbg(kbdev->dev, "pm_callback_runtime_off\n");
    if (gpu_power_on_or_off == true) {
        p5_gpu_power_off(kbdev);
    }
    disable_gpu_power_control(kbdev);
}

static void pm_callback_resume(struct kbase_device *kbdev)
{
    int ret = pm_callback_runtime_on(kbdev);

    WARN_ON(ret);
}

static void pm_callback_suspend(struct kbase_device *kbdev)
{
    pm_callback_runtime_off(kbdev);
}

struct kbase_pm_callback_conf pm_callbacks = {
    .power_on_callback = pm_callback_power_on,
    .power_off_callback = pm_callback_power_off,
    .power_suspend_callback = pm_callback_suspend,
    .power_resume_callback = pm_callback_resume,
#ifdef KBASE_PM_RUNTIME
    .power_runtime_init_callback = kbase_device_runtime_init,
    .power_runtime_term_callback = kbase_device_runtime_disable,
    .power_runtime_on_callback = pm_callback_runtime_on,
    .power_runtime_off_callback = pm_callback_runtime_off,
#else               /* KBASE_PM_RUNTIME */
    .power_runtime_init_callback = NULL,
    .power_runtime_term_callback = NULL,
    .power_runtime_on_callback = NULL,
    .power_runtime_off_callback = NULL,
#endif              /* KBASE_PM_RUNTIME */

#if MALI_USE_CSF && defined(KBASE_PM_RUNTIME)
    .power_runtime_gpu_idle_callback = pm_callback_runtime_gpu_idle,
    .power_runtime_gpu_active_callback = pm_callback_runtime_gpu_active,
#else
    .power_runtime_gpu_idle_callback = NULL,
    .power_runtime_gpu_active_callback = NULL,
#endif
};


