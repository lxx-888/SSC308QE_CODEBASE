/*
 * drv_rtcpwc.c- Sigmastar
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

#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/rtc.h>
#include <linux/clk.h>
#include <linux/clk-provider.h>
#include <linux/ctype.h>
#include <linux/delay.h>
#include <linux/irq.h>
#include <linux/of_irq.h>
#include <linux/input.h>
#include <linux/string.h>
#include <linux/ktime.h>
#include <ms_platform.h>
#include <ms_types.h>
#include <ms_msys.h>
#include <rtcpwc_os.h>
#include <hal_rtcpwc.h>
#include <cam_inter_os.h>
#ifdef CONFIG_SSTAR_MCU
#include <drv_mcu.h>
#endif
#define DTS_DEFAULT_DATE "default-date"

#ifdef CONFIG_INPUT
struct rtc_io_data
{
    u8  num;
    u32 keycode;
#ifdef CONFIG_SSTAR_PWC_IO_POLLING
    u8  last_state;
    u32 count;
    u32 threshold;
#endif /* CONFIG_SSTAR_PWC_IO_POLLING */
#ifdef CONFIG_SSTAR_PWC_IO_INTERRUPT
    u32                    virq;
    char                   irq_name[32];
    struct sstar_rtc_info *info;
#endif /* CONFIG_SSTAR_PWC_IO_INTERRUPT */
};

struct rtc_io_info
{
    u32 nio;
#ifdef CONFIG_SSTAR_PWC_IO_POLLING
    u32 poll_interval;
#endif /* CONFIG_SSTAR_PWC_IO_POLLING */
    struct rtc_io_data *data;
};
#endif /* CONFIG_INPUT */

struct sstar_rtc_info
{
    u8                      rtc_inited;
    dev_t                   devt;
    struct mutex            mutex;
    struct platform_device *rtc_pdev;
    struct device *         device;
    struct rtc_device *     rtc_dev;
    struct hal_rtcpwc_t     rtc_hal;
#ifdef CONFIG_SSTAR_RTC_WITH_ALARM
    u32 alarm_virq;
#ifdef CONFIG_PM_SLEEP
    u32     alarm_threshold;
    u32     alarm;
    ktime_t suspend_time;
    ktime_t resume_time;
#endif /*CONFIG_PM_SLEEP*/
#endif /* CONFIG_SSTAR_RTC_WITH_ALARM */
#ifdef CONFIG_INPUT
    struct rtc_io_info rtc_io;
    struct input_dev * rtc_input;
#endif /* CONFIG_INPUT */
};

static struct sstar_rtc_info *sstar_rtc_point;

static ssize_t count_status_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *                 str  = buf;
    char *                 end  = buf + PAGE_SIZE;
    struct sstar_rtc_info *info = dev_get_drvdata(dev);

    str += scnprintf(str, end - str, "iso   count  : %d\n", info->rtc_hal.fail_count.iso_fail);
    str += scnprintf(str, end - str, "read  count  : %d\n", info->rtc_hal.fail_count.read_fail);
    str += scnprintf(str, end - str, "clock count  : %d\n", info->rtc_hal.fail_count.clock_fail);
    return (str - buf);
}
DEVICE_ATTR(count_status, 0444, count_status_show, NULL);

#ifdef CONFIG_SSTAR_RTC_WITH_PWC
static ssize_t wakeup_event_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *                 src;
    char *                 str  = buf;
    char *                 end  = buf + PAGE_SIZE;
    struct sstar_rtc_info *info = dev_get_drvdata(dev);

    mutex_lock(&info->mutex);
    src = hal_rtc_get_wakeup_name(&info->rtc_hal);
    mutex_unlock(&info->mutex);

    str += scnprintf(str, end - str, "wakeup event: %s\n", src);
    return (str - buf);
}
DEVICE_ATTR(wakeup_event, 0444, wakeup_event_show, NULL);

static ssize_t event_state_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    int                    i;
    u16                    state;
    char **                name;
    char *                 str  = buf;
    char *                 end  = buf + PAGE_SIZE;
    struct sstar_rtc_info *info = dev_get_drvdata(dev);

    mutex_lock(&info->mutex);
    name  = hal_rtc_get_event_name(&info->rtc_hal);
    state = hal_rtc_get_event_state(&info->rtc_hal);
    mutex_unlock(&info->mutex);

    str += scnprintf(str, end - str, "event state:\n");

    for (i = 0; i < RTC_STATE_MAX; i++)
    {
        if (name[i])
        {
            if (state & (1 << i))
            {
                str += scnprintf(str, end - str, "%s : ON\n", name[i]);
            }
            else
            {
                str += scnprintf(str, end - str, "%s : OFF\n", name[i]);
            }
        }
    }

    return (str - buf);
}
DEVICE_ATTR(event_state, 0444, event_state_show, NULL);
#endif /* CONFIG_SSTAR_RTC_WITH_PWC */

#ifdef CONFIG_SSTAR_RTC_WITH_ALARM
static ssize_t alarm_timer_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t n)
{
    if (NULL != buf)
    {
        size_t                 len;
        const char *           str     = buf;
        u32                    now     = 0;
        u32                    seconds = 0;
        struct sstar_rtc_info *info    = dev_get_drvdata(dev);

        while (*str && !isspace(*str))
            str++;
        len = str - buf;
        if (len)
        {
            seconds = simple_strtoul(buf, NULL, 10);
            mutex_lock(&info->mutex);
            hal_rtc_read_time(&info->rtc_hal, &now);
            hal_rtc_set_alarm_and_enable(&info->rtc_hal, now + seconds);
            mutex_unlock(&info->mutex);
            return n;
        }
        return -EINVAL;
    }
    return -EINVAL;
}

static ssize_t alarm_timer_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    u32                    rest  = 0;
    u32                    now   = 0;
    u32                    alarm = 0;
    char *                 str   = buf;
    char *                 end   = buf + PAGE_SIZE;
    struct sstar_rtc_info *info  = dev_get_drvdata(dev);

    mutex_lock(&info->mutex);
    hal_rtc_read_time(&info->rtc_hal, &now);
    hal_rtc_get_alarm(&info->rtc_hal, &alarm);
    mutex_unlock(&info->mutex);

    rest = (alarm > now) ? (alarm - now) : 0;

    str += scnprintf(str, end - str, "countdown time = %d\n", rest);
    return (str - buf);
}
DEVICE_ATTR(alarm_timer, 0644, alarm_timer_show, alarm_timer_store);

#ifdef CONFIG_PM_SLEEP
static ssize_t alarm_threshold_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t n)
{
    if (NULL != buf)
    {
        char                   input_string[256] = {0};
        char *                 token;
        char *                 rest = input_string;
        struct sstar_rtc_info *info = dev_get_drvdata(dev);
        if (n >= 256)
            return -EINVAL;
        strncpy(input_string, buf, n);

        token = strsep(&rest, " ");
        if (token != NULL)
        {
            info->alarm = simple_strtoul(token, NULL, 10);
            RTC_DBG("alarm time: %u\n", info->alarm);
        }

        token = strsep(&rest, " ");
        if (token != NULL)
        {
            info->alarm_threshold = simple_strtoul(token, NULL, 10);
            RTC_DBG("alarm_threshold: %d\n", info->alarm_threshold);
        }

        return n;
    }
    return -EINVAL;
}

static ssize_t alarm_threshold_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *                 str  = buf;
    char *                 end  = buf + PAGE_SIZE;
    struct sstar_rtc_info *info = dev_get_drvdata(dev);

    str += scnprintf(str, end - str, "alarm threshold = %u\n", info->alarm_threshold);
    return (str - buf);
}
DEVICE_ATTR(alarm_threshold, 0644, alarm_threshold_show, alarm_threshold_store);

#endif /*CONFIG_PM_SLEEP*/

#endif /* CONFIG_SSTAR_RTC_WITH_ALARM */

#ifdef CONFIG_SSTAR_RTC_WITH_OFFSET
static ssize_t offset_count_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t n)
{
    int ret    = 0;
    s16 offset = 0;

    if (NULL != buf)
    {
        size_t                 len;
        const char *           str  = buf;
        struct sstar_rtc_info *info = dev_get_drvdata(dev);

        while (*str && !isspace(*str))
            str++;
        len = str - buf;
        if (len)
        {
            ret = kstrtos16(buf, 10, &offset);
            if (!ret)
            {
                if (offset > -256 && offset < 256)
                {
                    mutex_lock(&info->mutex);
                    hal_rtc_set_offset(&info->rtc_hal, offset);
                    mutex_unlock(&info->mutex);
                    return n;
                }
                return -ERANGE;
            }

            return ret;
        }
        return -EINVAL;
    }
    return -EINVAL;
}

static ssize_t offset_count_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    s16                    offset = 0;
    char *                 str    = buf;
    char *                 end    = buf + PAGE_SIZE;
    struct sstar_rtc_info *info   = dev_get_drvdata(dev);

    mutex_lock(&info->mutex);
    offset = hal_rtc_get_offset(&info->rtc_hal);
    mutex_unlock(&info->mutex);

    str += scnprintf(str, end - str, "offset count %d\n", offset);

    return (str - buf);
}
DEVICE_ATTR(offset_count, 0644, offset_count_show, offset_count_store);
#endif /* CONFIG_SSTAR_RTC_WITH_OFFSET */

static ssize_t sw3_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t n)
{
    if (NULL != buf)
    {
        size_t                 len;
        const char *           str   = buf;
        u16                    value = 0;
        struct sstar_rtc_info *info  = dev_get_drvdata(dev);

        while (*str && !isspace(*str))
            str++;
        len = str - buf;
        if (len)
        {
            value = simple_strtoul(buf, NULL, 10);
            mutex_lock(&info->mutex);
            if ((hal_rtc_get_sw3(&info->rtc_hal) != value) || (hal_rtc_get_sw2(&info->rtc_hal) != RTC_MAGIC_NUMBER))
            {
                hal_rtc_set_sw3(&info->rtc_hal, value);
            }
            mutex_unlock(&info->mutex);
            return n;
        }
        return -EINVAL;
    }
    return -EINVAL;
}

static ssize_t sw3_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    u32                    value = 0;
    char *                 str   = buf;
    char *                 end   = buf + PAGE_SIZE;
    struct sstar_rtc_info *info  = dev_get_drvdata(dev);

    mutex_lock(&info->mutex);
    value = hal_rtc_get_sw3(&info->rtc_hal);
    mutex_unlock(&info->mutex);

    str += scnprintf(str, end - str, "value save in sw3 = %d\n", value);
    return (str - buf);
}
DEVICE_ATTR(save_in_sw3, 0644, sw3_show, sw3_store);

#ifdef CONFIG_SSTAR_RTC_WITH_PWC
char *sstar_rtc_get_wakeup_source(void)
{
    if (!sstar_rtc_point)
        return NULL;

    if (!sstar_rtc_point->rtc_inited)
        return NULL;

    return hal_rtc_get_wakeup_name(&sstar_rtc_point->rtc_hal);
}
EXPORT_SYMBOL(sstar_rtc_get_wakeup_source);
#endif /* CONFIG_SSTAR_RTC_WITH_PWC */

static int sstar_rtc_read_time(struct device *dev, struct rtc_time *tm)
{
    u32                    seconds = 0;
    struct sstar_rtc_info *info    = dev_get_drvdata(dev);

    if (!info->rtc_inited)
        return -EIO;

    mutex_lock(&info->mutex);
    hal_rtc_read_time(&info->rtc_hal, &seconds);
    mutex_unlock(&info->mutex);

    rtc_time64_to_tm(seconds, tm);

    RTC_DBG("[%d,%d,%d,%d,%d,%d]\n", tm->tm_year, tm->tm_mon, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);

    return rtc_valid_tm(tm);
}

static int sstar_rtc_set_time(struct device *dev, struct rtc_time *tm)
{
    unsigned long          seconds;
    struct sstar_rtc_info *info = dev_get_drvdata(dev);

    if (!info->rtc_inited)
        return -EIO;

    RTC_DBG("[%d,%d,%d,%d,%d,%d]\n", tm->tm_year, tm->tm_mon, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);

    seconds = rtc_tm_to_time64(tm);

    mutex_lock(&info->mutex);
    hal_rtc_set_time(&info->rtc_hal, seconds);
    mutex_unlock(&info->mutex);

#ifdef CONFIG_PM_SLEEP
    info->resume_time = ktime_get();
    RTC_DBG("[%s] info->resume_time is %lld\n", __FUNCTION__, info->resume_time);
#endif
#ifdef CONFIG_RPMSG
    CamInterOsSignal(INTEROS_L2R_RTC_TIME_SET_INFO_SYNC, info->rtc_hal.base_time.value, info->rtc_hal.sw0.value, 0);
#endif

    return 0;
}

#ifdef CONFIG_SSTAR_RTC_WITH_ALARM
static int sstar_rtc_read_alarm(struct device *dev, struct rtc_wkalrm *alarm)
{
    u32                    seconds;
    struct sstar_rtc_info *info = dev_get_drvdata(dev);

    if (!info->rtc_inited)
        return -EIO;

    mutex_lock(&info->mutex);
    hal_rtc_get_alarm(&info->rtc_hal, &seconds);
    alarm->enabled = hal_rtc_get_alarm_enable(&info->rtc_hal);
    alarm->pending = hal_rtc_get_alarm_int(&info->rtc_hal);
    mutex_unlock(&info->mutex);

    RTC_DBG("seconds = %#x\r\n", seconds);

    rtc_time64_to_tm(seconds, &alarm->time);

    RTC_DBG("[%d,%d,%d,%d,%d,%d]\n", alarm->time.tm_year, alarm->time.tm_mon, alarm->time.tm_mday, alarm->time.tm_hour,
            alarm->time.tm_min, alarm->time.tm_sec);

    return rtc_valid_tm(&alarm->time);
}

static int sstar_rtc_set_alarm(struct device *dev, struct rtc_wkalrm *alarm)
{
    u32                    seconds;
    struct sstar_rtc_info *info = dev_get_drvdata(dev);

    if (!info->rtc_inited)
        return -EIO;

    RTC_DBG("[%d,%d,%d,%d,%d,%d]\n", alarm->time.tm_year, alarm->time.tm_mon, alarm->time.tm_mday, alarm->time.tm_hour,
            alarm->time.tm_min, alarm->time.tm_sec);

    seconds = rtc_tm_to_time64(&alarm->time);

    RTC_DBG("seconds = %#x\r\n", seconds);

    mutex_lock(&info->mutex);
    hal_rtc_set_alarm_and_enable(&info->rtc_hal, seconds);
    mutex_unlock(&info->mutex);

    return 0;
}

static int sstar_rtc_alarm_irq_enable(struct device *dev, unsigned int enabled)
{
    struct sstar_rtc_info *info = dev_get_drvdata(dev);

    if (!info->rtc_inited)
        return -EIO;

    mutex_lock(&info->mutex);
    hal_rtc_alarm_enable(&info->rtc_hal, enabled);
    mutex_unlock(&info->mutex);

    return 0;
}
#endif /* CONFIG_SSTAR_RTC_WITH_ALARM */

#ifdef CONFIG_SSTAR_RTC_WITH_OFFSET
static int sstar_rtc_read_offset(struct device *dev, long *offset)
{
    struct sstar_rtc_info *info = dev_get_drvdata(dev);

    if (!info->rtc_inited)
        return -EIO;

    mutex_lock(&info->mutex);
    *offset = hal_rtc_get_offset(&info->rtc_hal);
    mutex_unlock(&info->mutex);
    return 0;
}

static int sstar_rtc_set_offset(struct device *dev, long offset)
{
    struct sstar_rtc_info *info = dev_get_drvdata(dev);

    if (!info->rtc_inited)
        return -EIO;

    mutex_lock(&info->mutex);
    hal_rtc_set_offset(&info->rtc_hal, (s16)offset);
    mutex_unlock(&info->mutex);
    return 0;
}
#endif /* CONFIG_SSTAR_RTC_WITH_OFFSET */

#ifdef CONFIG_SSTAR_RTC_WITH_ALARM
static irqreturn_t sstar_rtc_interrupt(int irq, void *dev_id)
{
    struct sstar_rtc_info *info = (struct sstar_rtc_info *)dev_id;

    RTC_DBG("rtc alarm interrupt is trigger\n");

    hal_rtc_interrupt(&info->rtc_hal);

    rtc_update_irq(info->rtc_dev, 1, RTC_AF);

    return IRQ_HANDLED;
}
#endif /* CONFIG_SSTAR_RTC_WITH_ALARM */

#ifdef CONFIG_SSTAR_PWC_IO_INTERRUPT
static irqreturn_t sstar_rtc_io_interrupt(int irq, void *dev_id)
{
    u16                    state;
    struct rtc_io_data *   data = (struct rtc_io_data *)dev_id;
    struct sstar_rtc_info *info = ((struct rtc_io_data *)dev_id)->info;

    state = hal_rtc_get_event_state(&info->rtc_hal);

    if (info->rtc_input)
    {
        input_event(info->rtc_input, EV_KEY, data->keycode, (state & (2 << data->num)) ? 1 : 0);
        input_sync(info->rtc_input);
    }

    return IRQ_HANDLED;
}
#endif /* CONFIG_SSTAR_PWC_IO_INTERRUPT */

static int sstar_rtc_init(struct sstar_rtc_info *info)
{
    int             ret = 0;
    int             num = 0;
    struct rtc_time tm  = {0};

#ifdef CONFIG_SSTAR_RTC_WITH_OFFSET
    s32 offset = 0;
#endif /* CONFIG_SSTAR_RTC_WITH_OFFSET */

#ifdef CONFIG_SSTAR_RTC_WITH_PWC
    u32 val      = 0;
    u32 array[2] = {0};
#ifdef CONFIG_INPUT
    u8 index = 0;
#ifdef CONFIG_SSTAR_PWC_IO_POLLING
    u32 debounce_interval;
#endif /* CONFIG_SSTAR_PWC_IO_POLLING */
    struct fwnode_handle *child;
#endif /* CONFIG_INPUT */
#endif /* CONFIG_SSTAR_RTC_WITH_PWC */

    info->rtc_hal.default_base = 0; // 1970/1/1 00:00:00
    if (0 < (num = of_property_count_elems_of_size(info->rtc_pdev->dev.of_node, DTS_DEFAULT_DATE, sizeof(int))))
    {
        if (!of_property_read_u32_array(info->rtc_pdev->dev.of_node, DTS_DEFAULT_DATE, (u32 *)&tm, num))
        {
            if (!rtc_valid_tm(&tm))
            {
                info->rtc_hal.default_base = rtc_tm_to_time64(&tm);
            }
            else
            {
                RTC_ERR("[%s]: Please check " DTS_DEFAULT_DATE " in dtsi\n", __func__);
            }
        }
    }

#ifdef CONFIG_SSTAR_RTC_WITH_PWC
    ret = of_property_read_bool(info->rtc_pdev->dev.of_node, "io0-hiz");
    if (ret)
    {
        info->rtc_hal.pwc_io0_hiz = 1;
        RTC_DBG("io0-hiz (%d)\n", 1);
    }

    ret = of_property_read_u32(info->rtc_pdev->dev.of_node, "io2-wos", &val);
    if (ret)
    {
        RTC_DBG("of_property_read_u32 fail (io2-wos) %d\n", ret);
    }
    else
    {
        info->rtc_hal.pwc_io2_valid = TRUE;
        info->rtc_hal.pwc_io2_cmp   = val;
        RTC_DBG("io2-wos (%d)\n", val);

        ret = of_property_read_u32_array(info->rtc_pdev->dev.of_node, "io2-wos-v", array, 2);
        if (ret)
        {
            RTC_DBG("of_property_read_u32_array fail (io2-wos-v) %d\n", ret);
        }
        else
        {
            info->rtc_hal.pwc_io2_vlsel = array[0];
            info->rtc_hal.pwc_io2_vhsel = array[1];
            RTC_DBG("io2-wos-v (%d %d)\n", array[0], array[1]);
        }
    }

    ret = of_property_read_bool(info->rtc_pdev->dev.of_node, "io3-pu");
    if (ret)
    {
        info->rtc_hal.pwc_io3_pu = 1;
        RTC_DBG("io3-pu (%d)\n", 1);
    }

    ret = of_property_read_u32(info->rtc_pdev->dev.of_node, "io4-enable", &val);
    if (ret)
    {
        RTC_DBG("of_property_read_u32 fail (io4-enable) %d\n", ret);
    }
    else
    {
        info->rtc_hal.pwc_io4_valid = TRUE;
        info->rtc_hal.pwc_io4_value = val;
        RTC_DBG("io4-enable (%d)\n", val);
    }

    ret = of_property_read_u32(info->rtc_pdev->dev.of_node, "io5-enable", &val);
    if (ret)
    {
        RTC_DBG("of_property_read_u32 fail (io5-enable) %d\n", ret);
    }
    else
    {
        info->rtc_hal.pwc_io5_valid = TRUE;
        info->rtc_hal.pwc_io5_value = val;
        RTC_DBG("io5-enable (%d)\n", val);
    }

    ret = of_property_read_bool(info->rtc_pdev->dev.of_node, "io5-no-poweroff");
    if (ret)
    {
        info->rtc_hal.pwc_io5_no_poweroff = 1;
    }
#endif /* CONFIG_SSTAR_RTC_WITH_PWC */

    ret = of_property_read_bool(info->rtc_pdev->dev.of_node, "iso-auto-regen");
    if (ret)
    {
        info->rtc_hal.iso_auto_regen = 1;
        RTC_DBG("iso-auto-regen (%d)\n", 1);
    }

#ifdef CONFIG_SSTAR_RTC_WITH_ALARM
    info->alarm_virq = irq_of_parse_and_map(info->rtc_pdev->dev.of_node, 0);
    if (info->alarm_virq == 0)
    {
        RTC_ERR("[%s]: can't find interrupts property\n", __func__);
        return -EINVAL;
    }
    ret = request_irq(info->alarm_virq, sstar_rtc_interrupt, IRQF_TRIGGER_RISING, "RTC Alarm", (void *)info);
    if (ret)
    {
        RTC_ERR("[%s]: interrupt \"%s\" register failed\n", __func__, "RTC Alarm");
        return -EINVAL;
    }

#ifdef CONFIG_PM_SLEEP
    enable_irq_wake(info->alarm_virq);
#endif
#endif /* CONFIG_SSTAR_RTC_WITH_ALARM */

#ifdef CONFIG_SSTAR_RTC_WITH_OFFSET
    ret = of_property_read_u32(info->rtc_pdev->dev.of_node, "offset-count", &offset);
    if (ret)
    {
        RTC_DBG("of_property_read_s32 fail (offset-count) %d\n", ret);
    }
    else
    {
        ret = of_property_read_bool(info->rtc_pdev->dev.of_node, "offset-nagative");
        if (ret)
        {
            offset = -offset;
        }

        if (offset > -256 && offset < 256)
        {
            info->rtc_hal.offset_count = (s16)offset;
        }
        else
        {
            info->rtc_hal.offset_count = 0;
        }
        RTC_DBG("offset-count (%d)\n", val);
    }
#endif /* CONFIG_SSTAR_RTC_WITH_OFFSET */

#ifdef CONFIG_INPUT
    info->rtc_io.nio = device_get_child_node_count(&info->rtc_pdev->dev);
    if (info->rtc_io.nio)
    {
        info->rtc_io.data =
            devm_kzalloc(&info->rtc_pdev->dev, sizeof(struct rtc_io_data) * info->rtc_io.nio, GFP_KERNEL);
#ifdef CONFIG_SSTAR_PWC_IO_POLLING
        if (device_property_read_u32(&info->rtc_pdev->dev, "poll-interval", &info->rtc_io.poll_interval))
        {
            info->rtc_io.poll_interval = 10;
        }
#endif /* CONFIG_SSTAR_PWC_IO_POLLING */
        device_for_each_child_node(&info->rtc_pdev->dev, child)
        {
            if (fwnode_property_read_u32(child, "num", &num))
            {
                RTC_ERR("[%s]: %s can't find num property\n", __func__, dev_name(child->dev));
                return -EINVAL;
            }
            info->rtc_io.data[index].num = num;

            if (fwnode_property_read_u32(child, "keycode", &info->rtc_io.data[index].keycode))
            {
                RTC_ERR("[%s]: %s can't find keycode property\n", __func__, dev_name(child->dev));
                return -EINVAL;
            }

#ifdef CONFIG_SSTAR_PWC_IO_POLLING
            if (fwnode_property_read_u32(child, "debounce-interval", &debounce_interval))
            {
                debounce_interval = 10;
            }
            info->rtc_io.data[index].threshold = DIV_ROUND_UP(debounce_interval, info->rtc_io.poll_interval);
#endif /* CONFIG_SSTAR_PWC_IO_POLLING */

#ifdef CONFIG_SSTAR_PWC_IO_INTERRUPT
            info->rtc_io.data[index].info = info;
            info->rtc_io.data[index].virq = irq_of_parse_and_map(to_of_node(child), 0);
            if (info->rtc_io.data[index].virq == 0)
            {
                RTC_ERR("[%s]: %s can't find interrupts property\n", __func__, dev_name(child->dev));
                return -EINVAL;
            }

            ret = scnprintf(info->rtc_io.data[index].irq_name, sizeof(info->rtc_io.data[index].irq_name), "RTCPWC IO%d",
                            info->rtc_io.data[index].num);
            if (!ret)
            {
                RTC_ERR("[%s]: irqname scnprintf failed\n", __func__);
                return -EINVAL;
            }

            ret = request_irq(info->rtc_io.data[index].virq, sstar_rtc_io_interrupt,
                              IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, info->rtc_io.data[index].irq_name,
                              (void *)&info->rtc_io.data[index]);
            if (ret)
            {
                RTC_ERR("[%s]: interrupt \"%s\" register failed\n", __func__, "RTC Alarm");
                return -EINVAL;
            }
#endif /* CONFIG_SSTAR_PWC_IO_INTERRUPT */

            index++;
        }
    }
#endif /* CONFIG_INPUT */

    return 0;
}

static void sstar_rtc_deinit(struct sstar_rtc_info *info)
{
#ifdef CONFIG_SSTAR_PWC_IO_INTERRUPT
    u8 i;
    for (i = 0; i < info->rtc_io.nio; i++)
    {
        free_irq(info->rtc_io.data[i].virq, (void *)&info->rtc_io.data[i]);
        irq_dispose_mapping(info->rtc_io.data[i].virq);
    }
#endif /* CONFIG_SSTAR_PWC_IO_INTERRUPT */

#ifdef CONFIG_SSTAR_RTC_WITH_ALARM
    free_irq(info->alarm_virq, (void *)info);
    irq_dispose_mapping(info->alarm_virq);
#endif /* CONFIG_SSTAR_RTC_WITH_ALARM */
}

#ifdef CONFIG_SSTAR_PWC_IO_POLLING
static void sstar_rtc_io_polled_poll(struct input_dev *input)
{
    u32                    i;
    u16                    state;
    struct sstar_rtc_info *info = input_get_drvdata(input);

    state = hal_rtc_get_event_state(&info->rtc_hal);
    for (i = 0; i < info->rtc_io.nio; i++)
    {
        if ((state & (2 << info->rtc_io.data[i].num)) != info->rtc_io.data[i].last_state)
        {
            info->rtc_io.data[i].count++;
            if (info->rtc_io.data[i].count >= info->rtc_io.data[i].threshold)
            {
                info->rtc_io.data[i].last_state = state & (2 << info->rtc_io.data[i].num);

                input_event(input, EV_KEY, info->rtc_io.data[i].keycode,
                            (state & (2 << info->rtc_io.data[i].num)) ? 1 : 0);
                input_sync(input);
            }
        }
        else
        {
            info->rtc_io.data[i].count = 0;
        }
    }
}
#endif /* CONFIG_SSTAR_PWC_IO_POLLING */

void sstar_rtc_sw3_set(u32 val)
{
    if ((hal_rtc_get_sw3(&sstar_rtc_point->rtc_hal) != val)
        || (hal_rtc_get_sw2(&sstar_rtc_point->rtc_hal) != RTC_MAGIC_NUMBER))
    {
        hal_rtc_set_sw3(&sstar_rtc_point->rtc_hal, val);
    }
}
EXPORT_SYMBOL(sstar_rtc_sw3_set);

u32 sstar_rtc_sw3_get(void)
{
    return hal_rtc_get_sw3(&sstar_rtc_point->rtc_hal);
}
EXPORT_SYMBOL(sstar_rtc_sw3_get);

static const struct rtc_class_ops sstar_rtcpwc_ops = {
    .read_time = sstar_rtc_read_time,
    .set_time  = sstar_rtc_set_time,
#ifdef CONFIG_SSTAR_RTC_WITH_ALARM
    .read_alarm       = sstar_rtc_read_alarm,
    .set_alarm        = sstar_rtc_set_alarm,
    .alarm_irq_enable = sstar_rtc_alarm_irq_enable,
#endif /* CONFIG_SSTAR_RTC_WITH_ALARM */
#ifdef CONFIG_SSTAR_RTC_WITH_OFFSET
    .read_offset = sstar_rtc_read_offset,
    .set_offset  = sstar_rtc_set_offset,
#endif /* CONFIG_SSTAR_RTC_WITH_OFFSET */
};

#if defined(CONFIG_SSTAR_RTC_WITH_PWC) && !defined(CONFIG_PM_MCU_POWER_RESET)
static void sstar_rtcpwc_poweroff(void)
{
#ifdef CONFIG_SSTAR_MCU
    mcu_write_config();
#endif
    hal_rtc_power_off(&sstar_rtc_point->rtc_hal);
}
#endif

#ifdef CONFIG_RPMSG
u32 sstar_rtcpwc_time_sync(u32 arg0, u32 arg1, u32 arg2, u32 arg3)
{
    sstar_rtc_point->rtc_hal.base_time.is_vaild = 1;
    sstar_rtc_point->rtc_hal.base_time.value    = arg1;
    sstar_rtc_point->rtc_hal.sw0.is_vaild       = 1;
    sstar_rtc_point->rtc_hal.sw0.value          = arg2;
    return 0;
}
#endif

static int sstar_rtcpwc_remove(struct platform_device *pdev)
{
    struct device *        dev  = &pdev->dev;
    struct sstar_rtc_info *info = dev_get_drvdata(dev);

#ifdef CONFIG_SSTAR_RTC_WITH_PWC
    device_remove_file(info->device, &dev_attr_event_state);
    device_remove_file(info->device, &dev_attr_wakeup_event);
#endif /* CONFIG_SSTAR_RTC_WITH_PWC */
#ifdef CONFIG_SSTAR_RTC_WITH_OFFSET
    device_remove_file(info->device, &dev_attr_offset_count);
#endif /* CONFIG_SSTAR_RTC_WITH_OFFSET */
    device_remove_file(info->device, &dev_attr_count_status);
#ifdef CONFIG_SSTAR_RTC_WITH_ALARM
    device_remove_file(info->device, &dev_attr_alarm_timer);
#ifdef CONFIG_PM_SLEEP
    device_remove_file(info->device, &dev_attr_alarm_threshold);
#endif /* CONFIG_PM_SLEEP */
#endif /* CONFIG_SSTAR_RTC_WITH_ALARM */
    device_remove_file(info->device, &dev_attr_save_in_sw3);

#ifdef CONFIG_SSTAR_RTC_WITH_PWC
    pm_power_off = NULL;
#ifdef CONFIG_INPUT
    input_unregister_device(info->rtc_input);
#endif /* CONFIG_INPUT */
#endif /* CONFIG_SSTAR_RTC_WITH_PWC */

    device_destroy(msys_get_sysfs_class(), info->devt);
    unregister_chrdev_region(info->devt, 1);

    if (info)
    {
        sstar_rtc_point = 0;
    }

    sstar_rtc_deinit(info);

    return 0;
}

static int sstar_rtcpwc_probe(struct platform_device *pdev)
{
    struct sstar_rtc_info *info;
    struct resource *      res;
    struct device *        device;
    dev_t                  devt;
    int                    ret = 0;
    void __iomem *         rtc_base;
#ifdef CONFIG_INPUT
    u8                i;
    struct input_dev *input;
#endif /* CONFIG_INPUT */

    info = devm_kzalloc(&pdev->dev, sizeof(struct sstar_rtc_info), GFP_KERNEL);
    if (!info)
        return -ENOMEM;
    RTC_DBG("RTC initial\n");
    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if (!res)
    {
        RTC_ERR("[%s]: failed to get IORESOURCE_MEM\n", __func__);
        return -ENODEV;
    }
    rtc_base = devm_ioremap_resource(&pdev->dev, res);

    if (IS_ERR(rtc_base))
        return PTR_ERR(rtc_base);

    info->rtc_hal.rtc_base = (unsigned long)rtc_base;
    info->rtc_pdev         = pdev;

    platform_set_drvdata(pdev, info);
    mutex_init(&info->mutex);

    if (sstar_rtc_init(info))
    {
        return -EINVAL;
    }

    hal_rtc_init(&info->rtc_hal);

    sstar_rtc_point  = info;
    info->rtc_inited = 1;

#ifdef CONFIG_PM_SLEEP
    device_set_wakeup_capable(&pdev->dev, true);
    device_wakeup_enable(&pdev->dev);
#endif

    info->rtc_dev = devm_rtc_device_register(&pdev->dev, dev_name(&pdev->dev), &sstar_rtcpwc_ops, THIS_MODULE);

    if (IS_ERR(info->rtc_dev))
    {
        ret = PTR_ERR(info->rtc_dev);
        RTC_ERR("[%s]: unable to register device (err=%d).\n", __func__, ret);
        return ret;
    }

    // Note: is it needed?
    // device_set_wakeup_capable(&pdev->dev, 1);
    // device_wakeup_enable(&pdev->dev);

    // init rtc
    RTC_DBG("[%s]: hardware initialize\n", __func__);
    if (0 != (ret = alloc_chrdev_region(&devt, 0, 1, "rtcpwc")))
    {
        return ret;
    }
    info->devt = devt;

    device = device_create(msys_get_sysfs_class(), NULL, devt, NULL, "rtcpwc");
    if (IS_ERR(device))
    {
        return -ENODEV;
    }

    dev_set_drvdata(device, info);

#ifdef CONFIG_SSTAR_RTC_WITH_PWC
#ifdef CONFIG_INPUT
    input = devm_input_allocate_device(&pdev->dev);
    if (!input)
    {
        RTC_ERR("[%s]: no memory for rtc input device\n", __func__);
        return -ENOMEM;
    }

    input_set_drvdata(input, info);

    input->name = "rtcpwc";
    input->phys = "rtcpwc/input0";

    input->id.bustype = BUS_HOST;
    input->id.vendor  = 0x0001;
    input->id.product = 0x0001;
    input->id.version = 0x0100;

    info->rtc_input = input;

    __set_bit(EV_KEY, input->evbit);

    for (i = 0; i < info->rtc_io.nio; i++)
    {
        input_set_capability(input, EV_KEY, info->rtc_io.data[i].keycode);
    }

#ifdef CONFIG_SSTAR_PWC_IO_POLLING
    ret = input_setup_polling(input, sstar_rtc_io_polled_poll);
    if (ret)
    {
        RTC_ERR("[%s]: rtcpwc unable to set up polling, err=%d\n", __func__, ret);
        return ret;
    }

    input_set_poll_interval(input, info->rtc_io.poll_interval);
#endif /* CONFIG_INPUT */

    ret = input_register_device(input);
    if (ret)
    {
        RTC_ERR("[%s]: rtcpwc unable to register polled device, err=%d\n", __func__, ret);
        return ret;
    }
#endif /* CONFIG_INPUT */
#ifndef CONFIG_PM_MCU_POWER_RESET
    pm_power_off = sstar_rtcpwc_poweroff;
#endif
#endif

#ifdef CONFIG_SSTAR_RTC_WITH_ALARM
    ret |= device_create_file(device, &dev_attr_alarm_timer);
    // ret |= device_create_file(device, &dev_attr_alarm_threshold);
#ifdef CONFIG_PM_SLEEP
    ret |= device_create_file(device, &dev_attr_alarm_threshold);
#endif // CONFIG_PM_SLEEP
#endif /* CONFIG_SSTAR_RTC_WITH_ALARM */
    ret |= device_create_file(device, &dev_attr_count_status);
#ifdef CONFIG_SSTAR_RTC_WITH_OFFSET
    ret |= device_create_file(device, &dev_attr_offset_count);
#endif /* CONFIG_SSTAR_RTC_WITH_OFFSET */
#ifdef CONFIG_SSTAR_RTC_WITH_PWC
    ret |= device_create_file(device, &dev_attr_wakeup_event);
    ret |= device_create_file(device, &dev_attr_event_state);
#endif /* CONFIG_SSTAR_RTC_WITH_PWC */
    ret |= device_create_file(device, &dev_attr_save_in_sw3);

#ifdef CONFIG_RPMSG
    CamInterOsSignalReg(INTEROS_R2L_RTC_TIME_SET_INFO_SYNC, sstar_rtcpwc_time_sync, "rtos_time_set");
#endif

    return ret;
}

#ifdef CONFIG_PM_SLEEP
static int sstar_rtc_suspend(struct device *dev)
{
    struct sstar_rtc_info *info              = dev_get_drvdata(dev);
    u32                    now               = 0;
    u32                    seconds           = 0;
    long long int          alive_duration    = 0;
    long long int          alive_duration_ns = 0;
#ifdef CONFIG_SSTAR_MCU
    mcu_write_config();
#endif

    if (info->alarm)
    {
        seconds            = info->alarm;
        info->suspend_time = ktime_get();
        if (info->alarm == 1 && (info->suspend_time > info->resume_time))
        {
            alive_duration = (info->suspend_time - info->resume_time);

            RTC_ERR("Current suspend time in seconds: %lld\n", info->suspend_time);
            RTC_ERR("Current resume time in seconds: %lld\n", info->resume_time);
            alive_duration_ns = alive_duration % 1000000000;
            RTC_ERR("alive_duration %lld ns\n", alive_duration);
            if ((alive_duration_ns >= (long long int)info->alarm_threshold * 1000000)
                || (hal_rtc_get_wakeup_value(&info->rtc_hal) <= (RTC_IO1_WAKEUP)))
            {
                seconds = info->alarm + 1;
            }
        }
        RTC_ERR("alarm ====> %u s\n", seconds);
        // set rtc alarm before suspend
        mutex_lock(&info->mutex);
        hal_rtc_read_time(&info->rtc_hal, &now);
        hal_rtc_set_alarm_and_enable(&info->rtc_hal, now + seconds);
        mutex_unlock(&info->mutex);
    }

    hal_rtc_suspend(&info->rtc_hal);
    return 0;
}

static int sstar_rtc_resume(struct device *dev)
{
#ifdef CONFIG_INPUT
    u8 i;
#endif
#if defined(CONFIG_SSTAR_MCU) || defined(CONFIG_INPUT)
    u16 wakeup;
#endif
    struct sstar_rtc_info *info = dev_get_drvdata(dev);
    info->resume_time           = ktime_get();
    hal_rtc_resume(&info->rtc_hal);

#ifdef CONFIG_SSTAR_MCU
    wakeup = mcu_get_wakeup();
    if (wakeup)
    {
        return 0;
    }
#endif

#ifdef CONFIG_INPUT
    wakeup = hal_rtc_get_wakeup_value(&info->rtc_hal);
    if (wakeup)
    {
        for (i = 0; i < info->rtc_io.nio; i++)
        {
            if ((wakeup - 1) == info->rtc_io.data[i].num)
            {
                input_event(info->rtc_input, EV_KEY, info->rtc_io.data[i].keycode, 1);
                input_sync(info->rtc_input);

                input_event(info->rtc_input, EV_KEY, info->rtc_io.data[i].keycode, 0);
                input_sync(info->rtc_input);
            }
        }
    }
#endif /* CONFIG_INPUT */

    return 0;
}
#endif /* CONFIG_PM_SLEEP */

static const struct of_device_id sstar_rtcpwc_of_match_table[] = {{.compatible = "sstar,rtcpwc"}, {}};
MODULE_DEVICE_TABLE(of, sstar_rtcpwc_of_match_table);

#ifdef CONFIG_PM_SLEEP
static const struct dev_pm_ops sstar_rtc_pm_ops = {
    .suspend_noirq = sstar_rtc_suspend,
    .resume_noirq  = sstar_rtc_resume,
};
#endif /* CONFIG_PM_SLEEP */

static struct platform_driver sstar_rtcpwc_driver = {
    .remove = sstar_rtcpwc_remove,
    .probe  = sstar_rtcpwc_probe,
    .driver =
        {
            .name           = "sstar,rtcpwc",
            .owner          = THIS_MODULE,
            .of_match_table = sstar_rtcpwc_of_match_table,
#ifdef CONFIG_PM_SLEEP
            .pm = &sstar_rtc_pm_ops,
#endif /* CONFIG_PM_SLEEP */
        },
};

module_platform_driver(sstar_rtcpwc_driver);

MODULE_AUTHOR("SSTAR");
MODULE_DESCRIPTION("Sstar RTCPWC Driver");
MODULE_LICENSE("GPL v2");
