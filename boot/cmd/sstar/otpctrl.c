/*
 * otpctrl.c- Sigmastar
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
#include <command.h>
#include <time.h>
#include <malloc.h>
#include "asm/arch/mach/platform.h"
#include "asm/arch/mach/io.h"
#include <asm/ptrace.h>
#include <linux/delay.h>
#include <asm/system.h>
#include <cpu_func.h>

#ifdef CONFIG_ARM_SMCCC
#include <linux/arm-smccc.h>
#include <optee_smc.h>
#include <optee_msg.h>
#include <optee_smc.h>
#include <tee.h>

/* UUID of the OTP example trusted application */
#define TA_OTP_UUID                                        \
    {                                                      \
        0x4ed4d78c, 0xbbcd, 0x11ed,                        \
        {                                                  \
            0xaf, 0xa1, 0x02, 0x42, 0xac, 0x12, 0x00, 0x02 \
        }                                                  \
    }

#define OPTEE_SMC_FAST_SSTAR_UT_PREFIX (0xFE00)
#define OPTEE_SMC_FAST_SSTAR_UT(id)    (OPTEE_SMC_FAST_SSTAR_UT_PREFIX + id)

/* OTP UT ID */
#define OPTEE_SMC_FUNCID_OTP_UT OPTEE_SMC_FAST_SSTAR_UT(0x01)
#define OPTEE_SMC_OTP_UT        OPTEE_SMC_FAST_CALL_VAL(OPTEE_SMC_FUNCID_OTP_UT)

#define UBOOT_CMD_OTP_CTRL 0xB001

#define UBOOT64_CMD_TF_A_UID 0xC200ff01
#define UBOOT32_CMD_TF_A_UID 0x8200ff01

#define TF_A_MSG_OS_UUID_0 0xB1DCCB02
#define TF_A_MSG_OS_UUID_1 0xE94CB670
#define TF_A_MSG_OS_UUID_2 0x8E0F99B1
#define TF_A_MSG_OS_UUID_3 0x339B40FB

#endif // CONFIG_ARM_SMCCC

#define CMD_OFFSET      24
#define OFFSET_ERR_CODE 0x0
#define OFFSET_DATA_SZ  0x2
#define OFFSET_DATA     0x4

#define SS_OTP_ARM32_SERVICE 0x82000300
#define SS_OTP_ARM64_SERVICE 0xC2000300

// 16: Error code(2byte) and read size(2byte).
// 1024:OTP Max read len, and aligned to 64byte
#define OTP_USED_MEM_SIZE (((OFFSET_DATA + 1024) + 0x3f) & (~0x3f))

#define OTP_W            0x1
#define OTP_R            0x2
#define TA_OTP_READ_CMD  0
#define TA_OTP_WRITE_CMD 1

#define DBUG_LOG_R_8(buf, _off)  (*(volatile unsigned char *)(buf + _off))
#define DBUG_LOG_R_16(buf, _off) (*(volatile unsigned short *)(buf + _off))
#define DBUG_LOG_W_16(buf, _off, _val)                                         \
    {                                                                          \
        (*((volatile unsigned short *)(buf + _off))) = (unsigned short)(_val); \
    }

int otp_tfa_call(unsigned long otp_cmd, unsigned long val, unsigned long buf, int cmd_func)
{
#ifdef CONFIG_ARM_SMCCC
    struct arm_smccc_res res;
    unsigned long        fid = 0;
    unsigned long        a1  = 0;
    unsigned long        a2  = 0;
    unsigned long        a3  = 0;
    unsigned long        a4  = 0;
    unsigned long        a5  = 0;
    unsigned long        a6  = 0;
    unsigned long        a7  = 0;

#ifdef CONFIG_ARM64
    arm_smccc_smc(UBOOT64_CMD_TF_A_UID, 0, 0, 0, 0, 0, 0, 0, &res);
    fid = SS_OTP_ARM64_SERVICE;
#else
    arm_smccc_smc(UBOOT32_CMD_TF_A_UID, 0, 0, 0, 0, 0, 0, 0, &res);
    fid = SS_OTP_ARM32_SERVICE;
#endif

    if (res.a0 == TF_A_MSG_OS_UUID_0 && res.a1 == TF_A_MSG_OS_UUID_1 && res.a2 == TF_A_MSG_OS_UUID_2
        && res.a3 == TF_A_MSG_OS_UUID_3)
    {
        invalidate_dcache_range(buf, buf + OTP_USED_MEM_SIZE);

        a1 = cmd_func;
        a2 = otp_cmd;
        a3 = val;
        a4 = buf;
        a5 = 0;
        a6 = 0;
        a7 = 0;
        arm_smccc_smc(fid, a1, a2, a3, a4, a5, a6, a7, &res);
        printf("[OTP ATF-%d] operation = %x, command = %x, data = %x\n", (fid == SS_OTP_ARM64_SERVICE) ? (64) : (32),
               0x1, (U32)otp_cmd, (U32)val);
    }
    else
        return -ENODEV;
#else
#ifdef CONFIG_ARM64
    printf("[OTP ATF-64] tf-A operation fail\r\n");
#else
    printf("[OTP ATF-32] tf-A operation fail\r\n");
#endif
#endif
    return 0;
}

int otp_optee_call(unsigned long otp_cmd, unsigned long val, unsigned long buf, int func)
{
#if defined(CONFIG_ARM_SMCCC) && defined(CONFIG_OPTEE)
    int                            ret  = 0;
    const struct tee_optee_ta_uuid uuid = TA_OTP_UUID;
    struct tee_open_session_arg    session;
    struct tee_invoke_arg          invoke;
    struct tee_param               param[4];
    struct udevice *               tee = NULL;
    struct tee_shm *               otp_buf;
    size_t                         otp_buf_size = OTP_USED_MEM_SIZE;

    tee = tee_find_device(tee, NULL, NULL, NULL);
    if (!tee)
    {
        // printf("[uboot] find optee device fail\n");
        return -ENODEV;
    }

    memset(&session, 0, sizeof(session));
    tee_optee_ta_uuid_to_octets(session.uuid, &uuid);
    if (tee_open_session(tee, &session, 0, NULL))
    {
        printf("[uboot] open optee session fail\n");
        return -ENXIO;
    }

    if (tee_shm_alloc(tee, otp_buf_size, TEE_SHM_ALLOC, &otp_buf))
    {
        ret = -ENXIO;
        goto optee_close;
    }

    memset(&param, 0, sizeof(param));

    if (func == TA_OTP_READ_CMD)
    {
        param[0].u.value.a     = otp_cmd;
        param[0].attr          = TEE_PARAM_ATTR_TYPE_VALUE_INPUT;
        param[1].attr          = TEE_PARAM_ATTR_TYPE_MEMREF_INOUT;
        param[1].u.memref.shm  = (struct tee_shm *)otp_buf;
        param[1].u.memref.size = otp_buf_size;
    }
    else if (func == TA_OTP_WRITE_CMD)
    {
        param[0].u.value.a     = otp_cmd;
        param[0].attr          = TEE_PARAM_ATTR_TYPE_VALUE_INPUT;
        param[1].u.value.a     = val;
        param[1].attr          = TEE_PARAM_ATTR_TYPE_VALUE_INPUT;
        param[2].u.memref.shm  = (struct tee_shm *)otp_buf;
        param[2].u.memref.size = otp_buf_size;
        param[2].attr          = TEE_PARAM_ATTR_TYPE_MEMREF_INOUT;
    }
    else
    {
        printf("[uboot] optee cmd fail\n");
        ret = -EPERM;
        goto optee_mem_free;
    }

    memset(&invoke, 0, sizeof(invoke));
    invoke.func    = func;
    invoke.session = session.session;

    if (tee_invoke_func(tee, &invoke, 3, param))
    {
        printf("invoke function fail\n");
        ret = -EIO;
        goto optee_mem_free;
    }

    memcpy((char *)buf, otp_buf->addr, otp_buf_size);

optee_mem_free:
    tee_shm_free(otp_buf);

optee_close:
    tee_close_session(tee, session.session);

    return ret;

#else
    return -ENODEV;
#endif
}

void otp_ctrl_W(unsigned long otp_cmd, unsigned long val, unsigned long buf)
{
    if (otp_optee_call(otp_cmd, val, buf, TA_OTP_WRITE_CMD) == 0)
        return;
    else if (otp_tfa_call(otp_cmd, val, buf, OTP_W) == 0)
        return;
    else
        printf("[uboot] otpctrl write fail\n");
}

void otp_ctrl_R(unsigned long otp_cmd, unsigned long buf)
{
    if (otp_optee_call(otp_cmd, 0, buf, TA_OTP_READ_CMD) == 0)
        return;
    if (otp_tfa_call(otp_cmd, 0, buf, OTP_R) == 0)
        return;
    else
        printf("[uboot] otpctrl read fail\n");
}

static void otp_show_results(unsigned long buf, unsigned long op)
{
    int err_code = DBUG_LOG_R_16(buf, OFFSET_ERR_CODE);

    if (err_code)
    {
        printf("[ERR] otp control failure, addr %lx of error code:%x\n", buf, err_code);
        return;
    }

    if (op == OTP_R)
    {
        int           i, j, k;
        unsigned long size = DBUG_LOG_R_16(buf, OFFSET_DATA_SZ);

        if (size <= 0)
            return;

        printf("Results: \n\r");

        for (i = 0; i < size; i += 16)
        {
            k = (size - i) > 16 ? 16 : (size - i);
            printf("0x%03x:: ", i);
            for (j = i; j < i + k; j++)
            {
                printf("0x%02x ", DBUG_LOG_R_8(buf, j + OFFSET_DATA));
            }

            printf("\n\r");
        }

        DBUG_LOG_W_16(buf, OFFSET_DATA_SZ, 0);
    }
}

int do_otp(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
    unsigned long cmd = 0;
    unsigned long off = 0;
    unsigned long val = 0;
    unsigned long buf = 0;
    int           ret = CMD_RET_SUCCESS;

    if (argc < 3)
    {
        return CMD_RET_USAGE;
    }

#ifdef OTPCTRL_RESULT_DRAM
    buf = (unsigned long)(char *)memalign(64, OTP_USED_MEM_SIZE);
    memset((char *)buf, 0, OTP_USED_MEM_SIZE);
    // printf("buf addr %lx\n", buf);
#else
    buf = CONFIG_OTPCTRL_LOG_ADDRESS;
#endif

    if ((!strncmp(argv[1], "-w", strlen("-w"))))
    {
        if (argc < 5)
        {
            ret = CMD_RET_USAGE;
            goto mem_free;
        }

        cmd = (u32)simple_strtoul(argv[2], NULL, 16);
        if (cmd >= 0x100)
        {
            printf("[ERR] otp cmd 0x%lx out of range 0~0xFF\n", cmd);
            ret = CMD_RET_USAGE;
            goto mem_free;
        }

        off = (u32)simple_strtoul(argv[3], NULL, 16);
        val = (u32)simple_strtoul(argv[4], NULL, 16);

        otp_ctrl_W((cmd << CMD_OFFSET | off), val, buf);
        otp_show_results(buf, OTP_W);
    }
    else if ((!strncmp(argv[1], "-r", strlen("-r"))))
    {
        cmd = (u32)simple_strtoul(argv[2], NULL, 16);
        if (cmd >= 0x100)
        {
            printf("[ERR] otp cmd 0x%lx out of range 0~0xFF\n", cmd);
            ret = CMD_RET_USAGE;
            goto mem_free;
        }

        off = 0;
        otp_ctrl_R((cmd << CMD_OFFSET | off), buf);
        otp_show_results(buf, OTP_R);
    }

mem_free:
#ifdef OTPCTRL_RESULT_DRAM
    free((char *)buf);
#endif
    return ret;
}

U_BOOT_CMD(otpctrl, CONFIG_SYS_MAXARGS, 1, do_otp, "Used for otp program/read.",
           "Usage: otpctrl [-r] [otp command] for reading data from otp\n\r"
           "               otpctrl [-w] [otp command] [address offset] [data] for programing data to otp\n\r"
           "otp command:   0x0 > RSA Public N Key\n\r"
           "               0x1 > RSA Public E Key\n\r");

#ifdef OTP_CUSTOMER_ARES
#define READ_WORD(_reg) (*(volatile unsigned short *)(_reg))
#define WRITE_WORD(_reg, _val)                                           \
    {                                                                    \
        (*((volatile unsigned short *)(_reg))) = (unsigned short)(_val); \
    }

#define REG_ADDR_BASE_OTP5   (0x1f000000UL + (0x1025UL << 9))
#define REG_ADDR_BASE_OTP0   (0x1f000000UL + (0x1018UL << 9))
#define GET_REG16_ADDR(x, y) (x + (y)*4)

#define OTP5_READ(addr)       READ_WORD(GET_REG16_ADDR(REG_ADDR_BASE_OTP5, addr))
#define OTP5_WRITE(addr, val) WRITE_WORD(GET_REG16_ADDR(REG_ADDR_BASE_OTP5, addr), (val))

#define OTP0_READ(addr)       READ_WORD(GET_REG16_ADDR(REG_ADDR_BASE_OTP0, addr))
#define OTP0_WRITE(addr, val) WRITE_WORD(GET_REG16_ADDR(REG_ADDR_BASE_OTP0, addr), (val))

static void otp_read_form_point(u32 point)
{
    u32 local;

    if (point >= 8)
    {
        printf("[U-Boot] unknow status\n");
        return;
    }
    else if (point == 0)
    {
        point = 8;
    }

    local = (point - 1) * 4;

    printf("[U-Boot] addr1:p%x %x\n\r", point - 1, OTP5_READ(0x40 + local) | (OTP5_READ(0x41 + local) << 16));
    printf("[U-Boot] addr2:p%x %x\n\r", point - 1, OTP5_READ(0x42 + local) | (OTP5_READ(0x43 + local) << 16));
}

int do_otp_read_trig(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
    u16 addr1 = 0, addr2 = 0;
    u32 value = 0;

    if (argc < 3)
    {
        return CMD_RET_USAGE;
    }

    addr1 = (u16)simple_strtoul(argv[1], NULL, 16);
    addr2 = (u16)simple_strtoul(argv[2], NULL, 16);
    OTP0_WRITE(0x0, 0x9503); // setting trig cmd
    OTP0_WRITE(0x1, 0x8001); // cmd trig
    udelay(11);
    // check trig enable
    while ((OTP5_READ(0x32) & 0x1) != 0x1)
        ;

    // Set the otp read add1/addr2 trig and wait 1us,then can read value of two addrs in otp.
    OTP5_WRITE(0x36, addr1); // set customer addr1
    OTP5_WRITE(0x37, addr2); // set customer addr2
    OTP5_WRITE(0x30, 1);     // trig read
    udelay(1);
    value = OTP5_READ(0x34) & 0x7; // get piont
    otp_read_form_point(value);

    OTP5_WRITE(0x30, 0x100); // exit trig read mode

    return CMD_RET_SUCCESS;
}

U_BOOT_CMD(otpread, CONFIG_SYS_MAXARGS, 1, do_otp_read_trig, "Only read customer area otp.Read two addresses at once",
           "Usage: otpread [Customer OTP Addr1] [Customer OTP Addr2]\n\r");
#endif
