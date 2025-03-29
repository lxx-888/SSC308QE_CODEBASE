/*
 * hal_rtcpwc.c- Sigmastar
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

#include <rtcpwc_os.h>
#include <hal_rtcpwc.h>
#include <hal_rtcpwcreg.h>
#include <registers.h>

#define RTC_CHECK_STATUS_DELAY_TIME_US 100

#define READ_WORD(_reg)        (*(volatile u16 *)(_reg))
#define WRITE_WORD(_reg, _val) (*((volatile u16 *)(_reg))) = (u16)(_val)
#define WRITE_WORD_MASK(_reg, _val, _mask) \
    (*((volatile u16 *)(_reg))) = ((*((volatile u16 *)(_reg))) & ~(_mask)) | ((u16)(_val) & (_mask))

#define RTC_READ(_reg_)         READ_WORD(hal->rtc_base + ((_reg_) << 2))
#define RTC_WRITE(_reg_, _val_) WRITE_WORD(hal->rtc_base + ((_reg_) << 2), (_val_))

#define RTC_WRITE_MASK(_reg_, _val_, mask) WRITE_WORD_MASK(hal->rtc_base + ((_reg_) << 2), (_val_), (mask))

bool hal_rtc_iso_ctrl_ex(struct hal_rtcpwc_t *hal)
{
    u16 reg   = 0;
    u8  retry = ISO_ACK_RETRY_TIME;

    RTC_WRITE(RTCPWC_ISO_EN_CLR, RTCPWC_ISO_EN_CLR_BIT);
    reg   = RTC_READ(RTCPWC_ISO_EN_ACK);
    reg   = reg & RTCPWC_ISO_EN_ACK_BIT;
    retry = ISO_ACK_RETRY_TIME;
    while ((reg) && (--retry))
    {
#ifdef CONFIG_HIGH_RES_TIMERS
        rtc_os_usleep(RTC_CHECK_STATUS_DELAY_TIME_US);
#else
        rtc_os_udelay(RTC_CHECK_STATUS_DELAY_TIME_US);
#endif
        reg = RTC_READ(RTCPWC_ISO_EN_ACK);
        reg = reg & RTCPWC_ISO_EN_ACK_BIT;
    }
    if (retry == 0)
    {
        return FALSE;
    }

    RTC_WRITE(RTCPWC_ISO_TRIG, RTCPWC_ISO_TRIG_BIT);
    reg = RTC_READ(RTCPWC_ISO_EN_ACK);
    reg = reg & RTCPWC_ISO_EN_ACK_BIT;
    while ((!reg) && (--retry))
    {
#ifdef CONFIG_HIGH_RES_TIMERS
        rtc_os_usleep(RTC_CHECK_STATUS_DELAY_TIME_US);
#else
        rtc_os_udelay(RTC_CHECK_STATUS_DELAY_TIME_US);
#endif
        reg = RTC_READ(RTCPWC_ISO_EN_ACK);
        reg = reg & RTCPWC_ISO_EN_ACK_BIT;
    }
    if (retry == 0)
    {
        return FALSE;
    }

    RTC_WRITE(RTCPWC_ISO_EN_CLR, RTCPWC_ISO_EN_CLR_BIT);
    reg   = RTC_READ(RTCPWC_ISO_EN_ACK);
    reg   = reg & RTCPWC_ISO_EN_ACK_BIT;
    retry = ISO_ACK_RETRY_TIME;
    while ((reg) && (--retry))
    {
#ifdef CONFIG_HIGH_RES_TIMERS
        rtc_os_usleep(RTC_CHECK_STATUS_DELAY_TIME_US);
#else
        rtc_os_udelay(RTC_CHECK_STATUS_DELAY_TIME_US);
#endif
        reg = RTC_READ(RTCPWC_ISO_EN_ACK);
        reg = reg & RTCPWC_ISO_EN_ACK_BIT;
    }
    if (retry == 0)
    {
        return FALSE;
    }

    return TRUE;
}

static int hal_rtc_has_1k_clk(struct hal_rtcpwc_t *hal)
{
    u16 reg = 0;

    reg = RTC_READ(RTC_PWC_PWC2DIG_STATE2);
    if (reg & RTC_PWC_PWC2DIG_32K_OK)
    {
        return 1;
    }
    return 0;
}

static void hal_rtc_iso_ctrl(struct hal_rtcpwc_t *hal)
{
    static u8 warn_once = 0;
    u8        retry     = ISO_ACK_RETRY_TIME;

    if (0 == hal_rtc_has_1k_clk(hal))
    {
        if (!warn_once)
        {
            warn_once = 1;
            RTC_ERR("[%s][%d] RTCPWC fail to enter correct state and possibly caused by no power supplied\n",
                    __FUNCTION__, __LINE__);
        }
        hal->fail_count.clock_fail++;
        return;
    }

    if (((RTC_READ(RTCPWC_DIG2RTC_BASE_CTRL0)
          & (RTCPWC_DIG2RTC_BASE_RD | RTCPWC_DIG2RTC_SW0_RD | RTCPWC_DIG2RTC_SW1_RD))
         == 0)
        && (((RTC_READ(RTCPWC_DIG2RTC_BASE_CTRL1) & RTCPWC_DIG2RTC_ALARM_RD)) == 0))
    {
        hal->default_state = TRUE;
    }
    else
    {
        hal->default_state = FALSE;
    }

    while (!hal_rtc_iso_ctrl_ex(hal) && (--retry))
    {
        RTC_WRITE_MASK(RTCPWC_ISO_AUTO_REGEN, RTCPWC_ISO_AUTO_REGEN_bit, RTCPWC_ISO_AUTO_REGEN_bit);
        hal->fail_count.iso_fail++;
#ifdef CONFIG_HIGH_RES_TIMERS
        rtc_os_usleep(RTC_CHECK_STATUS_DELAY_TIME_US);
#else
        rtc_os_udelay(RTC_CHECK_STATUS_DELAY_TIME_US);
#endif
    }
    if (RTC_READ(RTCPWC_ISO_AUTO_REGEN) & RTCPWC_ISO_AUTO_REGEN_bit)
    {
        RTC_WRITE_MASK(RTCPWC_ISO_AUTO_REGEN, 0, RTCPWC_ISO_AUTO_REGEN_bit);
    }
}

/**
 *  hal_rtc_get_sw0 - get sw0 register value from rtc (16-bits)
 *  @hal: hal driver handle
 *  Returns: sw0 register value
 */
u32 hal_rtc_get_sw0(struct hal_rtcpwc_t *hal)
{
    u16 data_h = 0;
    u16 data_l = 0;
    u32 sw0    = 0;
    u16 reg    = 0;

    if (hal->sw0.is_vaild)
    {
        return hal->sw0.value;
    }

    // read sw0 register command
    reg = RTC_READ(RTCPWC_DIG2RTC_BASE_CTRL0);
    RTC_WRITE(RTCPWC_DIG2RTC_BASE_CTRL0, reg | RTCPWC_DIG2RTC_SW0_RD);
    hal_rtc_iso_ctrl(hal);

    // read data from data buffer register
    data_h = RTC_READ(RTCPWC_RTC2DIG_RDDATA_H);
    data_l = RTC_READ(RTCPWC_RTC2DIG_RDDATA_L);

    sw0 = data_h << 16;
    sw0 |= data_l;

    // reset control register
    reg = RTC_READ(RTCPWC_DIG2RTC_BASE_CTRL0);
    RTC_WRITE(RTCPWC_DIG2RTC_BASE_CTRL0, reg & ~RTCPWC_DIG2RTC_SW0_RD);

    // update shadow variable
    hal->sw0.value    = sw0;
    hal->sw0.is_vaild = TRUE;

    return sw0;
}

/**
 *  hal_rtc_set_sw0 - set sw0 register value from rtc (16-bits)
 *  @hal: hal driver handle
 *  @val: setting value
 */
void hal_rtc_set_sw0(struct hal_rtcpwc_t *hal, u32 val)
{
    u16 reg = 0;

    // set write sw0 register command
    reg = RTC_READ(RTCPWC_DIG2RTC_BASE_CTRL0);
    RTC_WRITE(RTCPWC_DIG2RTC_BASE_CTRL0, reg | RTCPWC_DIG2RTC_SW0_WR);

    // set sw password
    RTC_WRITE(RTCPWC_DIG2RTC_WRDATA_L, (val & 0xFFFF));
    RTC_DBG("set sw0 register to 0x%04x\n", (val & 0xFFFF));

    // trigger iso flow
    hal_rtc_iso_ctrl(hal);

    // reset control bits
    reg = RTC_READ(RTCPWC_DIG2RTC_BASE_CTRL0);
    RTC_WRITE(RTCPWC_DIG2RTC_BASE_CTRL0, reg & ~RTCPWC_DIG2RTC_SW0_WR);

    // update shadow variable
    hal->sw0.value    = val;
    hal->sw0.is_vaild = TRUE;
}

/**
 *  hal_rtc_get_sw2 - get sw2 register value from rtc (16-bits)
 *  @hal: hal driver handle
 *  Returns: sw2 register value
 */
u32 hal_rtc_get_sw2(struct hal_rtcpwc_t *hal)
{
    u16 sw2 = 0;

    // read data from data buffer register
    sw2 = RTC_READ(RTCPWC_SW2_RDDATA_REG);

    // update shadow variable
    hal->sw2.value    = sw2;
    hal->sw2.is_vaild = TRUE;

    return sw2;
}

/**
 *  hal_rtc_set_sw2 - set sw2 register value from rtc (16-bits)
 *  @hal: hal driver handle
 *  @val: setting value
 */
void hal_rtc_set_sw2(struct hal_rtcpwc_t *hal, u32 val)
{
    u16 reg = 0;

    // set write sw2 register command
    reg = RTC_READ(RTCPWC_DIG2RTC_BASE_CTRL0);
    RTC_WRITE(RTCPWC_DIG2RTC_BASE_CTRL0, reg | RTCPWC_DIG2RTC_SW2_WR);

    // set sw password
    RTC_WRITE(RTCPWC_DIG2RTC_WRDATA_L, (val & 0xFFFF));
    RTC_DBG("set sw2 register to 0x%04x\n", (val & 0xFFFF));

    // trigger iso flow
    hal_rtc_iso_ctrl(hal);

    // reset control bits
    reg = RTC_READ(RTCPWC_DIG2RTC_BASE_CTRL0);
    RTC_WRITE(RTCPWC_DIG2RTC_BASE_CTRL0, reg & ~RTCPWC_DIG2RTC_SW2_WR);

    // update shadow variable
    hal->sw2.value    = val;
    hal->sw2.is_vaild = TRUE;
}

/**
 *  hal_rtc_get_sw3 - get sw3 register value from rtc (16-bits)
 *  @hal: hal driver handle
 *  Returns: sw3 register value
 */
u32 hal_rtc_get_sw3(struct hal_rtcpwc_t *hal)
{
    u16 sw3 = 0;

    // read data from data buffer register
    sw3 = RTC_READ(RTCPWC_SW3_RDDATA_REG);

    // update shadow variable
    hal->sw3.value    = sw3;
    hal->sw3.is_vaild = TRUE;

    return sw3;
}

/**
 *  hal_rtc_set_sw3 - set sw3 register value from rtc (16-bits)
 *  @hal: hal driver handle
 *  @val: setting value
 */
void hal_rtc_set_sw3(struct hal_rtcpwc_t *hal, u32 val)
{
    u16 reg = 0;

    if (hal_rtc_get_sw2(hal) != RTC_MAGIC_NUMBER)
    {
        hal_rtc_set_sw2(hal, RTC_MAGIC_NUMBER);
    }

    // set write sw3 register command
    reg = RTC_READ(RTCPWC_DIG2RTC_BASE_CTRL0);
    RTC_WRITE(RTCPWC_DIG2RTC_BASE_CTRL0, reg | RTCPWC_DIG2RTC_SW3_WR);

    // set sw password
    RTC_WRITE(RTCPWC_DIG2RTC_WRDATA_L, (val & 0xFFFF));
    RTC_DBG("set sw3 register to 0x%04x\n", (val & 0xFFFF));

    // trigger iso flow
    hal_rtc_iso_ctrl(hal);

    // reset control bits
    reg = RTC_READ(RTCPWC_DIG2RTC_BASE_CTRL0);
    RTC_WRITE(RTCPWC_DIG2RTC_BASE_CTRL0, reg & ~RTCPWC_DIG2RTC_SW3_WR);

    // update shadow variable
    hal->sw3.value    = val;
    hal->sw3.is_vaild = TRUE;
}

static u32 hal_rtc_get_base(struct hal_rtcpwc_t *hal)
{
    u16 reg;
    u16 base_h    = 0;
    u16 base_l    = 0;
    u32 base_time = 0;

    // set read bit of base time
    reg = RTC_READ(RTCPWC_DIG2RTC_BASE_CTRL0);
    RTC_WRITE(RTCPWC_DIG2RTC_BASE_CTRL0, reg | RTCPWC_DIG2RTC_BASE_RD);
    hal_rtc_iso_ctrl(hal);

    // read base time
    base_h    = RTC_READ(RTCPWC_RTC2DIG_RDDATA_H);
    base_l    = RTC_READ(RTCPWC_RTC2DIG_RDDATA_L);
    base_time = base_h << 16;
    base_time |= base_l;
    RTC_DBG("get base time is 0x%08x\n", base_time);

    // reset read bit of base time
    reg = RTC_READ(RTCPWC_DIG2RTC_BASE_CTRL0);
    RTC_WRITE(RTCPWC_DIG2RTC_BASE_CTRL0, reg & ~RTCPWC_DIG2RTC_BASE_RD);

    return base_time;
}

void hal_rtc_set_base_time(struct hal_rtcpwc_t *hal, u32 seconds)
{
    u16 reg;

    hal->base_time.is_vaild = TRUE;
    hal->base_time.value    = seconds;

    // set base time bit command
    reg = RTC_READ(RTCPWC_DIG2RTC_BASE_CTRL0);
    RTC_WRITE(RTCPWC_DIG2RTC_BASE_CTRL0, reg | RTCPWC_DIG2RTC_BASE_WR);

    // set value to base time register
    RTC_WRITE(RTCPWC_DIG2RTC_WRDATA_L, (u16)seconds);
    RTC_WRITE(RTCPWC_DIG2RTC_WRDATA_H, (u16)((seconds) >> 16));
    RTC_DBG("set baset time to 0x%08x\n", seconds);

    // trigger iso flow
    hal_rtc_iso_ctrl(hal);

    // set counter reset bit
    reg = RTC_READ(RTCPWC_DIG2RTC_BASE_CTRL0);
    RTC_WRITE(RTCPWC_DIG2RTC_BASE_CTRL0, reg | RTCPWC_DIG2RTC_CNT_RST_WR);

    // trigger iso flow
    hal_rtc_iso_ctrl(hal);
    reg = RTC_READ(RTCPWC_DIG2RTC_BASE_CTRL0);
    RTC_WRITE(RTCPWC_DIG2RTC_BASE_CTRL0, reg & ~RTCPWC_DIG2RTC_CNT_RST_WR);

    // reset control bits
    reg = RTC_READ(RTCPWC_DIG2RTC_SET);
    RTC_WRITE(RTCPWC_DIG2RTC_SET, reg & ~RTCPWC_DIG2RTC_SET_BIT);
    reg = RTC_READ(RTCPWC_DIG2RTC_BASE_CTRL0);
    RTC_WRITE(RTCPWC_DIG2RTC_BASE_CTRL0, reg & ~RTCPWC_DIG2RTC_BASE_WR);

    hal_rtc_set_sw0(hal, RTC_PASSWORD);
}

static u32 hal_rtc_get_base_time(struct hal_rtcpwc_t *hal)
{
    u32 password  = 0;
    u32 base_time = 0;

    if (hal->base_time.is_vaild)
    {
        return hal->base_time.value;
    }

    password  = hal_rtc_get_sw0(hal);
    base_time = hal_rtc_get_base(hal);

    RTC_DBG("read password 0x%08x baset time 0x%08x\n", password, base_time);

    if (password != RTC_PASSWORD)
    {
        RTC_ERR("please set rtc timer (hwclock -w)\n");
        hal_rtc_set_base_time(hal, hal->default_base);
        hal_rtc_set_sw0(hal, RTC_PASSWORD);
        base_time = hal->default_base;
    }

    hal->base_time.is_vaild = TRUE;
    hal->base_time.value    = base_time;

    return base_time;
}

static u32 hal_rtc_get_count(struct hal_rtcpwc_t *hal)
{
    u16 reg       = 0;
    u32 run_sec   = 0;
    u32 chk_times = 5;
    u16 counter_h = 0;
    u16 counter_l = 0;

    if (!hal->default_state)
    {
        // set read bit of rtc counter
        reg = RTC_READ(RTCPWC_DIG2RTC_BASE_CTRL1);
        RTC_WRITE(RTCPWC_DIG2RTC_BASE_CTRL1, reg | RTCPWC_DIG2RTC_CNT_RD);

        // trigger iso flow
        hal_rtc_iso_ctrl(hal);
    }

    chk_times = 5;

    // Latch RTC counter and Check valid bit of RTC counter
    do
    {
        reg = RTC_READ(RTCPWC_DIG2RTC_CNT_RD_TRIG);
        RTC_WRITE(RTCPWC_DIG2RTC_CNT_RD_TRIG, reg | RTCPWC_DIG2RTC_CNT_RD_TRIG_BIT);

        // Note : The first to retrieve RTC counter will failed without below delay
#ifdef CONFIG_HIGH_RES_TIMERS
        rtc_os_usleep(200);
#else
        rtc_os_udelay(200);
#endif
    } while ((RTC_READ(RTCPWC_RTC2DIG_CNT_UPDATING) & RTCPWC_RTC2DIG_CNT_UPDATING_BIT) && (chk_times--));

    if (chk_times == 0)
    {
        hal->fail_count.read_fail++;
        RTC_ERR("Check valid bit of RTC counter failed!\n");
        // Reset read bit of RTC counter
        reg = RTC_READ(RTCPWC_DIG2RTC_BASE_CTRL1);
        RTC_WRITE(RTCPWC_DIG2RTC_BASE_CTRL1, reg & ~RTCPWC_DIG2RTC_CNT_RD);
        return 0;
    }

    // read RTC counter
    counter_h = RTC_READ(RTCPWC_REG_RTC2DIG_RDDATA_CNT_H);
    counter_l = RTC_READ(RTCPWC_REG_RTC2DIG_RDDATA_CNT_L);
    run_sec   = counter_h << 16;
    run_sec |= counter_l;
    RTC_DBG("get counter is 0x%08x\r\n", run_sec);

    // reset read bit of rtc counter
    reg = RTC_READ(RTCPWC_DIG2RTC_BASE_CTRL1);
    RTC_WRITE(RTCPWC_DIG2RTC_BASE_CTRL1, reg & ~RTCPWC_DIG2RTC_CNT_RD);

    return run_sec;
}

int hal_rtc_read_time(struct hal_rtcpwc_t *hal, u32 *seconds)
{
    u32 count     = 0;
    u32 base_time = 0;

    // read base time register
    base_time = hal_rtc_get_base_time(hal);
    RTC_DBG("get base time is 0x%08x\r\n", base_time);

    // read count register
    count = hal_rtc_get_count(hal);
    RTC_DBG("get count is 0x%08x\n", count);

    if ((base_time + count) > 0xFFFFFFFF)
    {
        *seconds = 0xFFFFFFFF;
    }
    else
    {
        *seconds = base_time + count;
    }

    return 0;
}

int hal_rtc_set_time(struct hal_rtcpwc_t *hal, u32 seconds)
{
    hal_rtc_set_base_time(hal, seconds);
    return 0;
}

void hal_rtc_get_alarm(struct hal_rtcpwc_t *hal, u32 *seconds)
{
    u16 reg;
    u32 base;
    u32 alarm;

    base = hal_rtc_get_base_time(hal);

    if (hal->alm_inited)
    {
        // set read bit of alarm
        reg = RTC_READ(RTCPWC_DIG2RTC_BASE_CTRL1);
        RTC_WRITE(RTCPWC_DIG2RTC_BASE_CTRL1, reg | RTCPWC_DIG2RTC_ALARM_RD);

        // trigger iso flow flow
        hal_rtc_iso_ctrl(hal);

        // reset control bits
        reg = RTC_READ(RTCPWC_DIG2RTC_BASE_CTRL1);
        RTC_WRITE(RTCPWC_DIG2RTC_BASE_CTRL1, reg & ~RTCPWC_DIG2RTC_ALARM_RD);

        alarm = RTC_READ(RTCPWC_RTC2DIG_RDDATA_L) | (RTC_READ(RTCPWC_RTC2DIG_RDDATA_H) << 16);
    }
    else
    {
        // when reading alarm for the first time, assign alarm to 0.
        hal->alm_inited = 1;
        alarm           = 0;
    }

    *seconds = base + alarm;
}

void hal_rtc_set_alarm_and_enable(struct hal_rtcpwc_t *hal, u32 seconds)
{
    u16 reg;
    u32 base;
    u32 alarm;

    base  = hal_rtc_get_base_time(hal);
    alarm = seconds - base;

    // enable alarm
    reg = RTC_READ(RTCPWC_DIG2RTC_BASE_CTRL1);
    reg |= RTCPWC_DIG2RTC_ALARM_EN;
    RTC_WRITE(RTCPWC_DIG2RTC_BASE_CTRL1, reg);

    // clear alarm interrupt
    reg = RTC_READ(RTCPWC_DIG2RTC_BASE_CTRL1);
    RTC_WRITE(RTCPWC_DIG2RTC_BASE_CTRL1, reg | RTCPWC_DIG2RTC_INT_CLR);

    // set base time bit
    reg = RTC_READ(RTCPWC_DIG2RTC_BASE_CTRL0);
    RTC_WRITE(RTCPWC_DIG2RTC_BASE_CTRL0, reg | RTCPWC_DIG2RTC_ALARM_WR);

    // set rtc alarm time
    RTC_WRITE(RTCPWC_DIG2RTC_WRDATA_L, (u16)alarm);
    RTC_WRITE(RTCPWC_DIG2RTC_WRDATA_H, (u16)((alarm) >> 16));
    RTC_DBG("set alarm time to 0x%08x\n", alarm);

    // trigger iso flow flow
    hal_rtc_iso_ctrl(hal);

    // reset control bits
    reg = RTC_READ(RTCPWC_DIG2RTC_BASE_CTRL0);
    RTC_WRITE(RTCPWC_DIG2RTC_BASE_CTRL0, reg & ~RTCPWC_DIG2RTC_ALARM_WR);

    // reset control bits
    reg = RTC_READ(RTCPWC_DIG2RTC_BASE_CTRL1);
    RTC_WRITE(RTCPWC_DIG2RTC_BASE_CTRL1, reg & ~RTCPWC_DIG2RTC_INT_CLR);
}

void hal_rtc_alarm_enable(struct hal_rtcpwc_t *hal, u8 enable)
{
    u16 reg;

    reg = RTC_READ(RTCPWC_DIG2RTC_BASE_CTRL1);
    if (enable)
    {
        reg |= RTCPWC_DIG2RTC_ALARM_EN;
    }
    else
    {
        reg &= ~RTCPWC_DIG2RTC_ALARM_EN;
    }

    RTC_WRITE(RTCPWC_DIG2RTC_BASE_CTRL1, reg);

    // trigger iso flow flow
    hal_rtc_iso_ctrl(hal);
}

u8 hal_rtc_get_alarm_enable(struct hal_rtcpwc_t *hal)
{
    return RTC_READ(RTCPWC_RTC2DIG_ISO_ALARM_INT) & RTCPWC_RTC2DIG_ALARM_EN_BIT;
}

static void hal_rtc_power_init(struct hal_rtcpwc_t *hal)
{
    u16 reg;

    if (hal->pwc_io4_valid)
    {
        reg = RTC_READ(RTCPWC_SW_CTRL);
        if (!(hal->pwc_io4_value & BIT2))
        {
            RTC_WRITE(RTCPWC_SW_CTRL, reg & ~RTCPWC_SW_CTRL_IO4_BIT);
        }
        else
        {
            RTC_WRITE(RTCPWC_SW_CTRL, reg | RTCPWC_SW_CTRL_IO4_BIT);
        }

        reg = RTC_READ(RTCPWC_DIG2PWC_PWR_EN_CTRL);
        if (!(hal->pwc_io4_value & BIT1))
        {
            RTC_WRITE(RTCPWC_DIG2PWC_PWR_EN_CTRL, reg & ~RTCPWC_ALARM_ON_EN);
        }
        else
        {
            RTC_WRITE(RTCPWC_DIG2PWC_PWR_EN_CTRL, reg | RTCPWC_ALARM_ON_EN);
        }

        reg = RTC_READ(RTCPWC_DIG2PWC_PWR_EN_CTRL);
        if (!(hal->pwc_io4_value & BIT0))
        {
            RTC_WRITE(RTCPWC_DIG2PWC_PWR_EN_CTRL, reg & ~RTCPWC_PWR_EN);
        }
        else
        {
            RTC_WRITE(RTCPWC_DIG2PWC_PWR_EN_CTRL, reg | RTCPWC_PWR_EN);
        }

        // trigger iso flow flow
        hal_rtc_iso_ctrl(hal);
    }

    if (hal->pwc_io5_valid)
    {
        reg = RTC_READ(RTCPWC_SW_CTRL);
        if (!(hal->pwc_io5_value & BIT2))
        {
            RTC_WRITE(RTCPWC_SW_CTRL, reg & ~RTCPWC_SW_CTRL_IO5_BIT);
        }
        else
        {
            RTC_WRITE(RTCPWC_SW_CTRL, reg | RTCPWC_SW_CTRL_IO5_BIT);
        }

        reg = RTC_READ(RTCPWC_DIG2PWC_PWR2_EN_CTRL);
        if (!(hal->pwc_io5_value & BIT1))
        {
            RTC_WRITE(RTCPWC_DIG2PWC_PWR2_EN_CTRL, reg & ~RTCPWC_ALARM2_ON_EN);
        }
        else
        {
            RTC_WRITE(RTCPWC_DIG2PWC_PWR2_EN_CTRL, reg | RTCPWC_ALARM2_ON_EN);
        }

        reg = RTC_READ(RTCPWC_DIG2PWC_PWR2_EN_CTRL);
        if (!(hal->pwc_io5_value & BIT0))
        {
            RTC_WRITE(RTCPWC_DIG2PWC_PWR2_EN_CTRL, reg & ~RTCPWC_PWR2_EN);
        }
        else
        {
            RTC_WRITE(RTCPWC_DIG2PWC_PWR2_EN_CTRL, reg | RTCPWC_PWR2_EN);
        }

        // trigger iso flow flow
        hal_rtc_iso_ctrl(hal);
    }
}

static void hal_rtc_alarm_init(struct hal_rtcpwc_t *hal)
{
    hal->alm_inited = 0;
    hal_rtc_alarm_enable(hal, 0);
    hal_rtc_power_init(hal);
}

void hal_rtc_event_init(struct hal_rtcpwc_t *hal)
{
    u16 reg;

    if (hal->pwc_io0_hiz)
    {
        reg = RTC_READ(RTCPWC_DIG2PWC_PWR2_EN_CTRL);
        reg |= RTCPWC_IO0_HIZ_EN_BIT;
        RTC_WRITE(RTCPWC_DIG2PWC_PWR2_EN_CTRL, reg);

        // trigger iso flow flow
        hal_rtc_iso_ctrl(hal);
    }

    if (hal->pwc_io2_valid)
    {
        reg = RTC_READ(RTCPWC_WOS_CTRL_REG);
        reg &= ~RTCPWC_WOS_CLR_BIT;
        reg |= RTCPWC_WOS_HP_EN_BIT;
        reg &= ~RTCPWC_WOS_CMPOUT_SEL_MASK;
        reg |= ((hal->pwc_io2_cmp << RTCPWC_WOS_CMPOUT_SEL_SHIFT) & RTCPWC_WOS_CMPOUT_SEL_MASK);
        RTC_WRITE(RTCPWC_WOS_CTRL_REG, reg);

        if (hal->pwc_io2_vlsel > 7)
        {
            hal->pwc_io2_vlsel = 7;
        }
        if (hal->pwc_io2_vhsel > 7)
        {
            hal->pwc_io2_vhsel = 7;
        }
        reg = (hal->pwc_io2_vlsel << 3) | hal->pwc_io2_vhsel;
        RTC_WRITE(RTCPWC_WOS_V_SEL_REG, reg);

        // trigger iso flow flow
        hal_rtc_iso_ctrl(hal);
    }

    if (hal->pwc_io3_pu)
    {
        reg = RTC_READ(RTCPWC_DIG2PWC_PWR2_EN_CTRL);
        reg &= ~RTCPWC_IO3_POL_BIT;
        reg |= RTCPWC_IO3_PUPD_SEL_BIT;
        RTC_WRITE(RTCPWC_DIG2PWC_PWR2_EN_CTRL, reg);

        // trigger iso flow flow
        hal_rtc_iso_ctrl(hal);
    }
}

u32 hal_rtc_get_alarm_int(struct hal_rtcpwc_t *hal)
{
    return (RTC_READ(RTCPWC_RTC2DIG_ISO_ALARM_INT) & RTCPWC_RTC2DIG_ISO_ALARM_INT_BIT);
}

void hal_rtc_clear_alarm_int(struct hal_rtcpwc_t *hal)
{
    u16 reg;

    // clear alarm interrupt
    reg = RTC_READ(RTCPWC_DIG2RTC_BASE_CTRL1);
    RTC_WRITE(RTCPWC_DIG2RTC_BASE_CTRL1, reg | RTCPWC_DIG2RTC_INT_CLR);

    // trigger iso flow flow
    hal_rtc_iso_ctrl(hal);

    // reset control bits
    reg = RTC_READ(RTCPWC_DIG2RTC_BASE_CTRL1);
    RTC_WRITE(RTCPWC_DIG2RTC_BASE_CTRL1, reg & ~RTCPWC_DIG2RTC_INT_CLR);
}

void hal_rtc_power_off(struct hal_rtcpwc_t *hal)
{
    u16 reg;

    WRITE_WORD_MASK(IO_ADDRESS(PM_MB_BANK) + (PM_MB_OFFSET << 2), 0, RTC_IO4_IO5_MASK);
    if (hal->pwc_io5_no_poweroff)
    {
        // clear sw reset & wdt reset flag when poweroff
        CLRREG16(BASE_REG_PMPOR_PA + REG_ID_01, BIT0);
        SETREG16(BASE_REG_WDT_PA + REG_ID_02, BIT0);
    }

    // enable all power control for wakeup
    reg = RTC_READ(RTCPWC_DIG2PWC_PWR2_EN_CTRL);
    reg |= RTCPWC_PWR2_EN | RTCPWC_ALARM2_ON_EN;
    RTC_WRITE(RTCPWC_DIG2PWC_PWR2_EN_CTRL, reg);

    // trigger iso flow
    hal_rtc_iso_ctrl(hal);

    // enable pwc1 alarm for wakeup
    reg = RTC_READ(RTCPWC_DIG2PWC_PWR_EN_CTRL);
    reg |= RTCPWC_ALARM_ON_EN;
    RTC_WRITE(RTCPWC_DIG2PWC_PWR_EN_CTRL, reg);

    // trigger iso flow flow
    hal_rtc_iso_ctrl(hal);

#ifdef CONFIG_SSTAR_MCU
    // select mcu51 clock to fro and wakeup mcu51
    reg = RTC_READ(RTCPWC_DIG2PWC_MCU_CTRL);
    reg |= RTCPWC_DIG2PWC_MCU_FRO_BIT;
    RTC_WRITE(RTCPWC_DIG2PWC_MCU_CTRL, reg);

    // trigger iso flow flow
    hal_rtc_iso_ctrl(hal);

    reg = RTC_READ(RTCPWC_DIG2PWC_MCU_CTRL);
    reg |= RTCPWC_DIG2PWC_MCU_ON_BIT;
    RTC_WRITE(RTCPWC_DIG2PWC_MCU_CTRL, reg);

    // trigger iso flow flow
    hal_rtc_iso_ctrl(hal);

    if (hal->pwc_io5_no_poweroff)
    {
        WRITE_WORD_MASK(IO_ADDRESS(PM_MB_BANK) + (PM_MB_OFFSET << 2), RTC_IO4_OFF, RTC_IO4_IO5_MASK);
    }
    else
    {
        WRITE_WORD_MASK(IO_ADDRESS(PM_MB_BANK) + (PM_MB_OFFSET << 2), RTC_IO4_IO5_OFF, RTC_IO4_IO5_MASK);
    }

    return;
#endif

    // clear alarm interrupt
    reg = RTC_READ(RTCPWC_DIG2RTC_BASE_CTRL1);
    reg |= RTCPWC_DIG2RTC_INT_CLR;
    RTC_WRITE(RTCPWC_DIG2RTC_BASE_CTRL1, reg);

    // turn off power enable
    reg = RTC_READ(RTCPWC_DIG2PWC_PWR_EN_CTRL);
    reg &= ~RTCPWC_PWR_EN;
    RTC_WRITE(RTCPWC_DIG2PWC_PWR_EN_CTRL, reg);

    // turn off sw ctrl io4
    reg = RTC_READ(RTCPWC_SW_CTRL);
    reg &= ~(RTCPWC_SW_CTRL_IO4_BIT);
    RTC_WRITE(RTCPWC_SW_CTRL, reg);

    // turn off sw ctrl io5
    if (!hal->pwc_io5_no_poweroff)
    {
        reg = RTC_READ(RTCPWC_SW_CTRL);
        reg &= ~(RTCPWC_SW_CTRL_IO5_BIT);
        RTC_WRITE(RTCPWC_SW_CTRL, reg);
    }

    // trigger iso flow flow
    hal_rtc_iso_ctrl(hal);
}

void hal_rtc_interrupt(struct hal_rtcpwc_t *hal) {}

s16 hal_rtc_get_offset(struct hal_rtcpwc_t *hal)
{
    u16 reg;
    s16 offset;

    // read offset reg
    reg    = RTC_READ(RTCPWC_OFFSET_REG);
    offset = (reg & RTCPWC_OFFSET_MASK) >> RTCPWC_OFFSET_SHIFT;
    if (!(reg & RTCPWC_OFFSET_SIGN_BIT))
    {
        offset = -offset;
    }
    return offset;
}

void hal_rtc_set_offset(struct hal_rtcpwc_t *hal, s16 offset)
{
    u16 reg;

    if (offset > 0)
    {
        reg = offset << RTCPWC_OFFSET_SHIFT;
        reg |= RTCPWC_OFFSET_SIGN_BIT;
    }
    else
    {
        reg = -offset;
        reg = reg << RTCPWC_OFFSET_SHIFT;
        reg &= ~RTCPWC_OFFSET_SIGN_BIT;
    }

    // write offset reg
    RTC_WRITE(RTCPWC_OFFSET_REG, reg);
    // trigger iso flow
    hal_rtc_iso_ctrl(hal);
}

void hal_rtc_init(struct hal_rtcpwc_t *hal)
{
    if (hal->iso_auto_regen)
    {
        RTC_WRITE_MASK(RTCPWC_ISO_AUTO_REGEN, BIT0, BIT0);
    }

    // set iso trigger timer to 3ms
    RTC_WRITE(RTCPWC_ISO_TIMER_L, (U16)RTC_ISO_TRI_TIMER);
    RTC_WRITE(RTCPWC_ISO_TIMER_H, (U16)((RTC_ISO_TRI_TIMER) >> 16));

    hal_rtc_alarm_init(hal);
    hal_rtc_event_init(hal);
    hal_rtc_set_offset(hal, hal->offset_count);
}

static char *hal_rtc_wakeup_name[RTC_WAKEUP_MAX] = {
    [RTC_IO0_WAKEUP] = "rtc_io0", [RTC_IO1_WAKEUP] = "rtc_io1",     [RTC_IO2_WAKEUP] = "rtc_io2",
    [RTC_IO3_WAKEUP] = "rtc_io3", [RTC_ALARM_WAKEUP] = "rtc_alarm",
};

char *hal_rtc_get_wakeup_name(struct hal_rtcpwc_t *hal)
{
    u16 reg;

    reg = RTC_READ(RTC_PWC_PWC2DIG_FLAG) & RTC_PWC_PWC2DIG_FLAG_MASK;
    if (reg >= RTC_WAKEUP_MAX)
    {
        return NULL;
    }
    else
    {
        return hal_rtc_wakeup_name[reg];
    }
}

u16 hal_rtc_get_wakeup_value(struct hal_rtcpwc_t *hal)
{
    return RTC_READ(RTC_PWC_PWC2DIG_FLAG) & RTC_PWC_PWC2DIG_FLAG_MASK;
}

static char *hal_rtc_event_name[RTC_STATE_MAX] = {
    [RTC_IO0_STATE] = "rtc_io0",
    [RTC_IO1_STATE] = "rtc_io1",
    [RTC_IO2_STATE] = "rtc_io2",
    [RTC_IO3_STATE] = "rtc_io3",
};

u16 hal_rtc_get_event_state(struct hal_rtcpwc_t *hal)
{
    return RTC_READ(RTC_PWC_PWC2DIG_STATE) & RTC_PWC_PWC2DIG_STATE_MASK;
}

char **hal_rtc_get_event_name(struct hal_rtcpwc_t *hal)
{
    return &hal_rtc_event_name[0];
}

void hal_rtc_suspend(struct hal_rtcpwc_t *hal)
{
    u16 reg;

    reg = hal_rtc_get_sw3(hal);
    hal_rtc_set_sw3(hal, ((SUSPEND_MAGIC & 0xFFF0) | (reg & 0xF)));

    hal->rtc_suspend_info.cmd0   = RTC_READ(RTCPWC_DIG2RTC_BASE_CTRL0);
    hal->rtc_suspend_info.cmd1   = RTC_READ(RTCPWC_DIG2RTC_BASE_CTRL1);
    hal->rtc_suspend_info.pwc0   = RTC_READ(RTCPWC_DIG2PWC_PWR_EN_CTRL);
    hal->rtc_suspend_info.pwc1   = RTC_READ(RTCPWC_DIG2PWC_PWR2_EN_CTRL);
    hal->rtc_suspend_info.wosv   = RTC_READ(RTCPWC_WOS_V_SEL_REG);
    hal->rtc_suspend_info.wosc   = RTC_READ(RTCPWC_WOS_CTRL_REG);
    hal->rtc_suspend_info.swctrl = RTC_READ(RTCPWC_SW_CTRL);

#ifdef CONFIG_SSTAR_MCU
    WRITE_WORD_MASK(IO_ADDRESS(PM_TOP_BANK) + ((PM_XTAL_OFFSET) << 2), 0, PM_XTAL_OFF);

    reg = RTC_READ(RTCPWC_DIG2PWC_MCU_CTRL);
    reg |= RTCPWC_DIG2PWC_MCU_FRO_BIT;
    RTC_WRITE(RTCPWC_DIG2PWC_MCU_CTRL, reg);
    hal_rtc_iso_ctrl(hal);

    reg = RTC_READ(RTCPWC_DIG2PWC_MCU_CTRL);
    reg |= RTCPWC_DIG2PWC_MCU_FRO_BIT | RTCPWC_DIG2PWC_MCU_ON_BIT;
    RTC_WRITE(RTCPWC_DIG2PWC_MCU_CTRL, reg);
    hal_rtc_iso_ctrl(hal);
#endif
}

void hal_rtc_resume(struct hal_rtcpwc_t *hal)
{
    u16 reg;

    RTC_WRITE(RTCPWC_DIG2RTC_BASE_CTRL0, hal->rtc_suspend_info.cmd0);
    RTC_WRITE(RTCPWC_DIG2RTC_BASE_CTRL1, hal->rtc_suspend_info.cmd1);
    RTC_WRITE(RTCPWC_DIG2PWC_PWR_EN_CTRL, hal->rtc_suspend_info.pwc0);
    RTC_WRITE(RTCPWC_DIG2PWC_PWR2_EN_CTRL, hal->rtc_suspend_info.pwc1);
    RTC_WRITE(RTCPWC_WOS_V_SEL_REG, hal->rtc_suspend_info.wosv);
    RTC_WRITE(RTCPWC_WOS_CTRL_REG, hal->rtc_suspend_info.wosc);
    RTC_WRITE(RTCPWC_SW_CTRL, hal->rtc_suspend_info.swctrl);

    reg = hal_rtc_get_sw3(hal);
    hal_rtc_set_sw3(hal, reg & 0xF);

#ifdef CONFIG_SSTAR_MCU
    reg = RTC_READ(RTCPWC_DIG2PWC_MCU_CTRL);
    reg &= ~RTCPWC_DIG2PWC_MCU_FRO_BIT;
    RTC_WRITE(RTCPWC_DIG2PWC_MCU_CTRL, reg);
    hal_rtc_iso_ctrl(hal);

    reg = RTC_READ(RTCPWC_DIG2PWC_MCU_CTRL);
    reg &= ~(RTCPWC_DIG2PWC_MCU_FRO_BIT | RTCPWC_DIG2PWC_MCU_ON_BIT);
    RTC_WRITE(RTCPWC_DIG2PWC_MCU_CTRL, reg);
    hal_rtc_iso_ctrl(hal);
#endif
}
