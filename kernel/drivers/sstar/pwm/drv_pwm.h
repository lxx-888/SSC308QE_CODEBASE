/*
 * drv_pwm.h- Sigmastar
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
#ifndef __DRV_PWM_H__
#define __DRV_PWM_H__

#include <cam_os_wrapper.h>

struct pwm_ch_cfg
{
    u64 duty;
    u64 shift;
    u64 period;
    u8  enable;
    u32 channel;
    u32 polarity;

#ifdef CONFIG_SSTAR_PWM_DDT
    u64 p_ddt;
    u64 n_ddt;
    u8  ddt_en;
#endif
};

struct pwm_gp_cfg
{
    u64 duty;
    u64 shift;
    u32 group;
    u64 period;
    u8  enable;
    u8  stop_en;
    u32 polarity;
    u32 round_num;
};

#define PWM_IOC_MAXNR 6

#define IOCTL_PWM_SET_CHAN_CFG_NR  (0)
#define IOCTL_PWM_GET_CHAN_CFG_NR  (2)
#define IOCTL_PWM_SET_GROUP_CFG_NR (1)
#define IOCTL_PWM_GET_GROUP_CFG_NR (3)
#define IOCTL_PWM_GROUP_STOP_NR    (4)
#define IOCTL_PWM_GROUP_ROUND_NR   (6)

#define PWM_IOC_MAGIC           'p'
#define IOCTL_PWM_SET_CHAN_CFG  _IO(PWM_IOC_MAGIC, IOCTL_PWM_SET_CHAN_CFG_NR)
#define IOCTL_PWM_GET_CHAN_CFG  _IO(PWM_IOC_MAGIC, IOCTL_PWM_GET_CHAN_CFG_NR)
#define IOCTL_PWM_SET_GROUP_CFG _IO(PWM_IOC_MAGIC, IOCTL_PWM_SET_GROUP_CFG_NR)
#define IOCTL_PWM_GET_GROUP_CFG _IO(PWM_IOC_MAGIC, IOCTL_PWM_GET_GROUP_CFG_NR)
#define IOCTL_PWM_GROUP_STOP    _IO(PWM_IOC_MAGIC, IOCTL_PWM_GROUP_STOP_NR)
#define IOCTL_PWM_GROUP_ROUND   _IO(PWM_IOC_MAGIC, IOCTL_PWM_GROUP_ROUND_NR)

#ifdef CONFIG_SSTAR_SPWM
struct spwm_gp_cfg
{
    u8  mode;
    u8  step;
    u8  index;
    u8  scale;
    u32 group;
    u8  enable;
    u64 period;
    u8  stop_en;
    u8  clamp_en;
    u8  duty_len;
    u64 duty[64];
    u64 clamp_max;
    u64 clamp_min;
    u32 round_num;
};

#define SPWM_IOC_MAXNR 6

#define IOCTL_SPWM_SET_GROUP_CFG_NR (0)
#define IOCTL_SPWM_GET_GROUP_CFG_NR (1)
#define IOCTL_SPWM_UPDATE_PERIOD_NR (2)
#define IOCTL_SPWM_UPDATE_DUTY_NR   (3)
#define IOCTL_SPWM_GROUP_STOP_NR    (4)
#define IOCTL_SPWM_GROUP_ROUND_NR   (5)

#define SPWM_IOC_MAGIC           's'
#define IOCTL_SPWM_SET_GROUP_CFG _IO(SPWM_IOC_MAGIC, IOCTL_SPWM_SET_GROUP_CFG_NR)
#define IOCTL_SPWM_GET_GROUP_CFG _IO(SPWM_IOC_MAGIC, IOCTL_SPWM_GET_GROUP_CFG_NR)
#define IOCTL_SPWM_UPDATE_PERIOD _IO(SPWM_IOC_MAGIC, IOCTL_SPWM_UPDATE_PERIOD_NR)
#define IOCTL_SPWM_UPDATE_DUTY   _IO(SPWM_IOC_MAGIC, IOCTL_SPWM_UPDATE_DUTY_NR)
#define IOCTL_SPWM_GROUP_STOP    _IO(SPWM_IOC_MAGIC, IOCTL_SPWM_GROUP_STOP_NR)
#define IOCTL_SPWM_GROUP_ROUND   _IO(SPWM_IOC_MAGIC, IOCTL_SPWM_GROUP_ROUND_NR)
#endif

#endif
