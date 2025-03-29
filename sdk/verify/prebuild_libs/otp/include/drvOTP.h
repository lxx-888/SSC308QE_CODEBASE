/*
 * Copyright (c) 2016-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef DRVOTP_H
#define DRVOTP_H

#include <stdint.h>

/* operation*/
#define OTP_W 0x1
#define OTP_R 0x2

#if ((defined(DRVOTP_IPL) || defined(DRVOTP_TFA)) && !defined(KERNEL_OTPCTRL))
#define RIU_BASE             (0x1F000000UL)
#endif

#define OTP_DEBUG_SRAM_ADDR  (0xA0010000UL)

/* otp command */
enum {
    OTP_RSA_N                       = 0X00,
    OTP_RSA_E                       = 0X01,
    OTP_SECURITY_BOOT               = 0X02,
    OTP_RSA_KEY_LOCK                = 0X03,
    OTP_RSA_KEY_BLOCK               = 0X04,
    OTP_KEY1                        = 0X05,
    OTP_KEY2                        = 0X06,
    OTP_KEY3                        = 0X07,
    OTP_KEY4                        = 0X08,
    OTP_KEY5                        = 0X09,
    OTP_KEY6                        = 0X0A,
    OTP_KEY7                        = 0X0B,
    OTP_KEY8                        = 0X0C,
    OTP_AES_KEY1_LOCK               = 0X0D,
    OTP_AES_KEY1_BLOCK              = 0X0E,
    OTP_AES_KEY2_LOCK               = 0X0F,
    OTP_AES_KEY2_BLOCK              = 0X10,
    OTP_AES_KEY3_LOCK               = 0X11,
    OTP_AES_KEY3_BLOCK              = 0X12,
    OTP_AES_KEY4_LOCK               = 0X13,
    OTP_AES_KEY4_BLOCK              = 0X14,
    OTP_AES_KEY5_LOCK               = 0X15,
    OTP_AES_KEY5_BLOCK              = 0X16,
    OTP_AES_KEY6_LOCK               = 0X17,
    OTP_AES_KEY6_BLOCK              = 0X18,
    OTP_AES_KEY7_LOCK               = 0X19,
    OTP_AES_KEY7_BLOCK              = 0X1A,
    OTP_AES_KEY8_LOCK               = 0X1B,
    OTP_AES_KEY8_BLOCK              = 0X1C,
    OTP_CID                         = 0X1D,
    OTP_CID_LOCK                    = 0X1E,
    OTP_DISABLE_JTAG_MODE           = 0X1F,
    OTP_PASSWORD_MODE               = 0X20,
    OTP_PASSWORD                    = 0X21,
    OTP_PASSWORD_LOCK               = 0X22,
    OTP_PASSWORD_BLOCK              = 0X23,
    OTP_ROM_SELECT_AES_KEY          = 0X24,
    OTP_ROM_SELECT_AES_KEY_LOCK     = 0X25,
    OTP_UUID2                       = 0X26,
    OTP_DID1                        = 0X27,
    OTP_DID1_LOCK                   = 0X28,
    OTP_DID2                        = 0X29,
    OTP_DID2_LOCK                   = 0X2A,
    OTP_VERSION_CTL                 = 0X2B,
    OTP_VERSION2_CTL                = 0X2C,
    OTP_FORCE_UARTBOOT              = 0X2D,
    OTP_UUID                        = 0X2E,
    OTP_RSA_KEY_LEN                 = 0X2F,
    OTP_PROTECT_ENABLE              = 0X30,
    OTP_RSA_KEY_N_CHK_ENABLE        = 0x31,
    OTP_PASSWORD_CHK_ENABLE         = 0x32,
    OTP_OTP_KEY_CHK_ENABLE          = 0x33,
    OTP_AES_MULTIPLE_NUM            = 0x34,
    OTP_SCA_PROTECT_ENABLE          = 0x35,
    OTP_PassWord_chk                = 0x36,
    OTP_Key1_chk                    = 0x37,
    OTP_Key2_chk                    = 0x38,
    OTP_Key3_chk                    = 0x39,
    OTP_Key4_chk                    = 0x3A,
    OTP_Key5_chk                    = 0x3B,
    OTP_Key6_chk                    = 0x3C,
    OTP_Key7_chk                    = 0x3D,
    OTP_Key8_chk                    = 0x3E,
    OTP_RSA_KEY_N_CHK               = 0x3F,
    OTP_TZ                          = 0x40,
#if (defined(DRVOTP_IPL) && !defined(KERNEL_OTPCTRL))
    OTP_IPL_ANTI_ROLLBACK           = 0x41,
    OTP_IPL_CUST_ANTI_ROLLBACK      = 0x42,
    OTP_TF_A_ANTI_ROLLBACK          = 0x43,
    OTP_OPTEE_ANTI_ROLLBACK         = 0x44,
    OTP_VMM_ANTI_ROLLBACK           = 0x45,
    OTP_UBOOT_ANTI_ROLLBACK         = 0x46,
    OTP_KERNEL_ANTI_ROLLBACK        = 0x47,
    OTP_RTOS_ANTI_ROLLBACK          = 0x48,
#endif
    OTP_CUSTOMER_AREA               = 0XA0,
    OTP_CUSTOMER_AREA_LOCK0         = 0XA1,
    OTP_CUSTOMER_AREA_LOCK1         = 0XA2,
    OTP_CUSTOMER_AREA_LOCK2         = 0XA3,
    OTP_CUSTOMER_AREA_LOCK3         = 0XA4,
    OTP_CUSTOMER_AREA_LOCK4         = 0XA5,
    OTP_CUSTOMER_AREA_LOCK5         = 0XA6,
    OTP_CUSTOMER_AREA_LOCK6         = 0XA7,
    OTP_CUSTOMER_AREA_LOCK7         = 0XA8,
    OTP_ALL                         = 0XFE,
    OTP_MAX                         = 0XFF
};

typedef void(*otp_sleep)(uint32_t);

typedef struct
{
    uint64_t base_addr;
    uint64_t buf;
    uint32_t op;
    uint32_t cmd;
    uint32_t val;
    uint32_t bufsize;
    otp_sleep fsleep;
} otp_config;

#if (defined(DRVOTP_IPL) && !defined(KERNEL_OTPCTRL))
void OTP_check_func(void);
int OTP_get_ver_start_addr(const char * name);
int OTP_get_ver_end_addr(const char * name);
void OTP_handler(unsigned int op,unsigned int cmd,unsigned int val);
#endif

void OTP_main(otp_config* otp_op);

#endif

