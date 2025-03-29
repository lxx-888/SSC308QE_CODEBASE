/*
 * chkotp.c- Sigmastar
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
#include "include/chkotp.h"
#include <stdio.h>
#include <command.h>

#ifndef RIU_BASE_ADDR
#define RIU_BASE_ADDR 0x1f000000
#endif
#define INREG16(x)                READ_REGISTER_USHORT(x)
#define READ_REGISTER_USHORT(reg) (*(volatile unsigned short *)(reg))

#define GET_REG8_ADDR(x, y)     (x + (y)*2 - ((y)&1))
#define GET_REG16_ADDR(x, y)    (x + (y)*4)
#define BANK_TO_REG8_ADDR(bank) GET_REG8_ADDR(RIU_BASE_ADDR, bank << 8)
#define BIT_MASK(nbits)         ((1 << (nbits)) - 1)

extern struct Category    category_arr[CATEGORY_NUM];
extern struct SpecialNote specialnote_arr[SPECIALNOTE_NUM];
extern struct TryIc       tryic_a_list[TRYIC_A_NUM];
extern struct TryIc       tryic_b_list[TRYIC_B_NUM];
extern struct TryIc       tryic_c_list[TRYIC_C_NUM];
extern struct TryIc       tryic_d_list[TRYIC_D_NUM];

U16 _read_trim_value_u16(U32 BankStart, U32 HighAddr, U32 LowAddr, U32 LocMSB, U32 LocLSB)
{
    U16 val;
    if (HighAddr == LowAddr)
    {
        val = (INREG16(GET_REG16_ADDR(BankStart, LowAddr)) >> LocLSB) & BIT_MASK(LocMSB - LocLSB + 1);
    }
    else
    {
        if (LocMSB > 15)
        {
            LocMSB -= 15;
        }
        val = (INREG16(GET_REG16_ADDR(BankStart, LowAddr)) >> LocLSB) & BIT_MASK(16 - LocLSB);
        val |= ((INREG16(GET_REG16_ADDR(BankStart, HighAddr)) & BIT_MASK(LocMSB)) << (16 - LocLSB));
    }
    return val;
}

void do_chkotp(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
    struct TryIc *tryic     = NULL;
    U16           tryic_num = 0;
    printf("trim category!\r\n");
    printf("argc = %d \r\n", argc);
    for (int i = 0; i < argc; i++)
    {
        printf("argv[%d] = %-30s\n", i, argv[i]);
    }
    if (argc >= 3)
    {
        printf("eg.trim or trim (A|B|C|D)\r\n");
        return;
    }
    if (argc == 1)
        ;
    else if (!strcmp(argv[1], "A"))
    {
        tryic     = tryic_a_list;
        tryic_num = TRYIC_A_NUM;
    }
    else if (!strcmp(argv[1], "B"))
    {
        tryic     = tryic_b_list;
        tryic_num = TRYIC_B_NUM;
    }
    else if (!strcmp(argv[1], "C"))
    {
        tryic     = tryic_c_list;
        tryic_num = TRYIC_C_NUM;
    }
    else if (!strcmp(argv[1], "D"))
    {
        tryic     = tryic_d_list;
        tryic_num = TRYIC_D_NUM;
    }
    else
    {
        printf("eg.chkotp or chkotp (A|B|C|D)\r\n");
        return;
    }

    printf("===============check trim program (category)!\r\n");
    for (int i = 0; i < CATEGORY_NUM; i++)
    {
        U16 otp_val = _read_trim_value_u16(BANK_TO_REG8_ADDR(category_arr[i].otp_addr.bank),
                                           category_arr[i].otp_addr.high_address, category_arr[i].otp_addr.low_address,
                                           category_arr[i].otp_addr.loc_msb, category_arr[i].otp_addr.loc_lsb);
        U16 ipctrl_val =
            _read_trim_value_u16(BANK_TO_REG8_ADDR(category_arr[i].ipctrl_addr.bank),
                                 category_arr[i].ipctrl_addr.high_address, category_arr[i].ipctrl_addr.low_address,
                                 category_arr[i].ipctrl_addr.loc_msb, category_arr[i].ipctrl_addr.loc_lsb);
        U16 activate_val =
            _read_trim_value_u16(BANK_TO_REG8_ADDR(category_arr[i].activateefuse.bank),
                                 category_arr[i].activateefuse.high_address, category_arr[i].activateefuse.low_address,
                                 category_arr[i].activateefuse.loc_msb, category_arr[i].activateefuse.loc_lsb);
        // printf("activate_bank=%x high_address=%x low_address=%x loc_msb %d loc_lsb=%d val=%x \r\n",
        //        BANK_TO_REG8_ADDR(category_arr[i].activateefuse.bank), category_arr[i].activateefuse.high_address,
        //        category_arr[i].activateefuse.low_address, category_arr[i].activateefuse.loc_msb,
        //        category_arr[i].activateefuse.loc_lsb, activate_val);
        if (activate_val)
        {
            if (ipctrl_val == otp_val)
            {
                printf(
                    "\033[1;32mindex=%03d\tactivate\tip=%-30s\tcategory=%-30s\t(ipctrl)0x%02x==(otp)0x%"
                    "02x\tPASS\r\n\033["
                    "0m",
                    i, category_arr[i].ip, category_arr[i].category, ipctrl_val, otp_val);
            }
            else
            {
                printf(
                    "\033[1;31mindex=%03d\tactivate\tip=%-30s\tcategory=%-30s\t(ipctrl)0x%02x!=(otp)0x%"
                    "02x\tFAIL\r\n\033["
                    "0m",
                    i, category_arr[i].ip, category_arr[i].category, ipctrl_val, otp_val);
            }
        }
        else
        {
            if (ipctrl_val == category_arr[i].def_val)
            {
                printf(
                    "\033[1;32mindex=%03d\tno "
                    "activate\tip=%-30s\tcategory=%-30s\t(ipctrl)0x%02x==(def)0x%02x\tPASS\r\n\033[0m",
                    i, category_arr[i].ip, category_arr[i].category, ipctrl_val, category_arr[i].def_val);
            }
            else
            {
                printf(
                    "\033[1;31mindex=%03d\tno "
                    "activate\tip=%-30s\tcategory=%-30s\t(ipctrl)0x%02x!=(def)0x%02x\tFAIL\r\n\033[0m",
                    i, category_arr[i].ip, category_arr[i].category, ipctrl_val, category_arr[i].def_val);
            }
        }
    }

    printf("===============check trim program (specialnote)!\r\n");
    for (int i = 0; i < SPECIALNOTE_NUM; i++)
    {
        U16 ipctrl_val = _read_trim_value_u16(BANK_TO_REG8_ADDR(specialnote_arr[i].bank),
                                              specialnote_arr[i].high_address, specialnote_arr[i].low_address,
                                              specialnote_arr[i].loc_msb, specialnote_arr[i].loc_lsb);
        if (ipctrl_val == specialnote_arr[i].def_val)
        {
            printf(
                "\033[1;32mindex=%03d\tip=%-30s\tspecialnote=%-30s\t(ipctrl_val)0x%02x==(def)0x%02x\tPASS\r\n\033[0m",
                i, specialnote_arr[i].ip, specialnote_arr[i].category, ipctrl_val, specialnote_arr[i].def_val);
        }
        else
        {
            printf(
                "\033[1;31mindex=%03d\tip=%-30s\tspecialnote=%-30s\t(ipctrl_val)0x%02x!=(def)0x%02x\tFail\r\n\033[0m",
                i, specialnote_arr[i].ip, specialnote_arr[i].category, ipctrl_val, specialnote_arr[i].def_val);
        }
    }

    if (tryic != NULL)
    {
        printf("===============check otp program tryIC %s\r\n", argv[1]);
        for (int i = 0; i < tryic_num; i++)
        {
            U16 otp_val = _read_trim_value_u16(BANK_TO_REG8_ADDR(tryic[i].bank), tryic[i].high_address,
                                               tryic[i].low_address, tryic[i].loc_msb, tryic[i].loc_lsb);
            if (otp_val == tryic[i].def_val)
            {
                printf("\033[1;32mindex=%03d\tip=%-30s\tcategory=%-30s\t(otp_val)0x%02x==(def)0x%02x\tPASS\r\n\033[0m",
                       i, tryic[i].ip, tryic[i].category, otp_val, tryic[i].def_val);
            }
            else
            {
                printf("\033[1;31mindex=%03d\tip=%-30s\tcategory=%-30s\t(otp_val)0x%02x==(def)0x%02x\tFAIL\r\n\033[0m",
                       i, tryic[i].ip, tryic[i].category, otp_val, tryic[i].def_val);
            }
        }
    }
}

U_BOOT_CMD(chkotp, 5, 1, do_chkotp, "eg.chkotp or chkotp (A|B|C|D)\r\n", "eg.chkotp or chkotp (A|B|C|D)\r\n");
