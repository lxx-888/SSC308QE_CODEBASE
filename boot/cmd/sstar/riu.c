/*
 * riu.c - Sigmastar
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
#include <log.h>
#include <command.h>
#include <asm/arch/mach/io.h>
#include <asm/arch/mach/platform.h>
#include <sstar_sys_utility.h>

extern void riu_dbg_timeout_dump(void);

int do_riu(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
    MS_U32 u32Target = 0;
    MS_U32 u32Value  = 0;
    MS_U8  u8Bit     = 0;
    MS_U8  u8Len     = 0;
    MS_U8  i         = 0;

    if (argc < 3)
    {
        cmd_usage(cmdtp);
        return 0;
    }

    if (0 == strncmp(argv[1], "wword", 2))
    {
        u32Target = simple_strtoul(argv[2], NULL, 16);
        u32Value  = simple_strtoul(argv[3], NULL, 16);
        Write2Byte(u32Target, (MS_U16)u32Value);
        u32Target = simple_strtoul(argv[2], NULL, 16);
        log_info("[%x]:0x%04x\n", (unsigned int)u32Target, (unsigned int)Read2Byte(u32Target));
    }
    else if (0 == strncmp(argv[1], "rword", 2))
    {
        u32Target = simple_strtoul(argv[2], NULL, 16);
        if (NULL != argv[3])
            u8Len = simple_strtoul(argv[3], NULL, 10);
        else
            u8Len = 1;

        if (1 == u8Len)
            printf("[%x]:0x%04x\n", (unsigned int)u32Target, (unsigned int)Read2Byte(u32Target));
        else
        {
            if (u32Target & 0x01)
            {
                printf("%06x: ", (unsigned int)u32Target - 1);
                u32Target = u32Target - 1;
            }
            else
                printf("%06x: ", (unsigned int)u32Target);

            for (i = 1; i <= u8Len / 2; i++)
            {
                printf("%04x ", (unsigned int)Read2Byte(u32Target + i * 2 - 1));

                if (0x80 <= ((u32Target + i - 1) & 0xFF))
                {
                    break;
                }

                if (0 == i % 4)
                    printf(" ");
                if (0 == i % 8)
                    printf("\n%06x: ", (unsigned int)(u32Target + i));
            }
            printf("\n");
        }
    }
    else if (0 == strncmp(argv[1], "wbyte", 2))
    {
        u32Target = simple_strtoul(argv[2], NULL, 16);
        u32Value  = simple_strtoul(argv[3], NULL, 16);
        WriteByte(u32Target, (MS_U8)u32Value);
        log_info("[%x]:0x%x\n", (unsigned int)u32Target, (unsigned int)ReadByte(u32Target));
    }
    else if (0 == strncmp(argv[1], "rbyte", 2))
    {
        u32Target = simple_strtoul(argv[2], NULL, 16);
        if (NULL != argv[3])
            u8Len = simple_strtoul(argv[3], NULL, 10);
        else
            u8Len = 1;

        if (1 == u8Len)
            printf("[%x]:0x%02x\n", (unsigned int)u32Target, (unsigned int)ReadByte(u32Target));
        else
        {
            MS_U32 tmpValue  = 0;
            MS_U32 tmpTarget = 0;

            if (u32Target & 0x01)
            {
                printf("%06x: ", (unsigned int)u32Target - 1);
                u32Target = u32Target - 1;
            }
            else
                printf("%06x: ", (unsigned int)u32Target);

            for (i = 1; i <= u8Len / 2; i++)
            {
                tmpTarget = u32Target + i * 2 - 1;
                tmpValue  = ReadByte(tmpTarget);
                tmpValue  = tmpValue << 8;
                tmpValue += ReadByte(tmpTarget - 1);
                printf("%04x ", (unsigned int)tmpValue);

                if (0xFF == (tmpTarget & 0xFF))
                {
                    break;
                }

                if (0 == i % 4)
                    printf(" ");
                if (0 == i % 8)
                    printf("\n%06x: ", (unsigned int)(u32Target + i * 2));
            }
            printf("\n");
        }
    }
    else if (0 == strncmp(argv[1], "bit", 2))
    {
        u32Target = simple_strtoul(argv[2], NULL, 16);
        u8Bit     = simple_strtoul(argv[3], NULL, 10);
        u32Value  = simple_strtoul(argv[4], NULL, 16);
        WriteRegBitPos(u32Target, u8Bit, (MS_U8)u32Value);
        log_info("[%x]:0x%02x\n", (unsigned int)u32Target, (unsigned int)ReadByte(u32Target));
    }
    else
    {
        cmd_usage(cmdtp);
    }

    /* check riu timeout & print */
    riu_dbg_timeout_dump();

    return 0;
}

U_BOOT_CMD(riu, 5, 0, do_riu, "riu command",
           "wword [target: bank+offset Addr(8bit mode)][value]\n"
           "riu rword [target: bank+offset Addr(8bit mode)][(len: default=1)]\n"
           "riu wbyte [target: bank+offset Addr(8bit mode)][value]\n"
           "riu rbyte [target: bank+offset Addr(8bit mode)][(len: default=1)]\n"
           "riu bit   [target: bank+offset Addr(8bit mode)][bit][(1/0)]\n");

int do_riu_r(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
    u32           bank;
    u32           ofst;
    u32           val;
    void __iomem *addr;

    if (argc < 2 || argc > 3)
        return CMD_RET_USAGE;

    bank = simple_strtoul(argv[1], NULL, 16);
    if (argc == 2)
    {
        u8 i = 0;

        printf("BANK:0x%04X\n", bank);
        for (i = 0; i <= 0x7f; i += 1)
        {
            if (i % 0x8 == 0x0)
                printf("%02X: ", i);

            addr = (void __iomem *)IO_PHYS + (bank * 0x200) + (i << 2);
            val  = INREG16(addr);
            printf("0x%04X ", val);
            if (i % 0x8 == 0x7)
                printf("\n");
        }
    }
    else if (argc == 3)
    {
        ofst = simple_strtoul(argv[2], NULL, 16);
        printf("BANK:0x%04X 16bit-offset 0x%02X\n", bank, ofst);
        addr = (void __iomem *)IO_PHYS + (bank * 0x200) + (ofst << 2);
        val  = INREG16(addr);
        printf("0x%04X\n", val);
    }
    else
    {
        return CMD_RET_USAGE;
    }

    /* check riu timeout & print */
    riu_dbg_timeout_dump();

    return 0;
}

U_BOOT_CMD(riu_r, 3, 0, do_riu_r, "read RIU bank registers",
           "<bank> [<offset>]\n"
           "  <bank>   = register bank in 16-bit HEX format\n"
           "  <offset> = register offset in 8-bit HEX format\n");

int do_riu_w(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
    u32           bank;
    u32           ofst;
    u32           val;
    void __iomem *addr;

    if (argc != 4)
        return CMD_RET_USAGE;

    bank = simple_strtoul(argv[1], NULL, 16);
    ofst = simple_strtoul(argv[2], NULL, 16);
    val  = simple_strtoul(argv[3], NULL, 16);

    printf("BANK:0x%04X 16bit-offset 0x%02X\n", bank, ofst);
    addr = (void __iomem *)IO_PHYS + (bank * 0x200) + (ofst << 2);
    OUTREG16(addr, val);
    val = INREG16(addr);
    printf("0x%04X\n", val);

    /* check riu timeout & print */
    riu_dbg_timeout_dump();

    return 0;
}

U_BOOT_CMD(riu_w, 4, 4, do_riu_w, "write RIU bank registers",
           "<bank> <offset> <value>\n"
           "  <bank>   = register bank in 16-bit HEX format\n"
           "  <offset> = register offset in 8-bit HEX format\n"
           "  <value>  = value to write in 16-bit HEX format\n");

int do_riux32_r(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
    u32           bank;
    u32           ofst;
    u32           val;
    void __iomem *addr;

    if (argc < 2 || argc > 3)
        return CMD_RET_USAGE;

    bank = simple_strtoul(argv[1], NULL, 16);
    if (argc == 2)
    {
        u8 i = 0;

        printf("BANK:0x%04X\n", bank);
        for (i = 0; i <= 0x7f; i += 1)
        {
            if (i % 0x8 == 0x0)
                printf("%02X: ", i);

            addr = (void __iomem *)IO_PHYS + (bank * 0x200) + (i << 2);
            val  = INREG32(addr);
            printf("0x%08X ", val);
            if (i % 0x8 == 0x7)
                printf("\n");
        }
    }
    else if (argc == 3)
    {
        ofst = simple_strtoul(argv[2], NULL, 16);
        printf("BANK:0x%04X 16bit-offset 0x%02X\n", bank, ofst);
        addr = (void __iomem *)IO_PHYS + (bank * 0x200) + (ofst << 2);
        val  = INREG32(addr);
        printf("0x%08X\n", val);
    }
    else
    {
        return CMD_RET_USAGE;
    }

    /* check riu timeout & print */
    riu_dbg_timeout_dump();

    return 0;
}

U_BOOT_CMD(riux32_r, 3, 0, do_riux32_r, "read X32 bank registers",
           "[bank] [<offset>]\n"
           "  <bank>   = register bank in 16-bit HEX format\n"
           "  <offset> = register offset in 8-bit HEX format\n");

int do_riux32_w(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
    u32           bank;
    u32           ofst;
    u32           val;
    void __iomem *addr;

    if (argc != 4)
        return CMD_RET_USAGE;

    bank = simple_strtoul(argv[1], NULL, 16);
    ofst = simple_strtoul(argv[2], NULL, 16);
    val  = simple_strtoul(argv[3], NULL, 16);

    printf("BANK:0x%04X 16bit-offset 0x%02X\n", bank, ofst);
    addr = (void __iomem *)IO_PHYS + (bank * 0x200) + (ofst << 2);
    OUTREG32(addr, val);
    val = INREG32(addr);
    printf("0x%08X\n", val);

    /* check riu timeout & print */
    riu_dbg_timeout_dump();

    return 0;
}

U_BOOT_CMD(riux32_w, 4, 4, do_riux32_w, "write X32 bank registers",
           "<bank> <offset> <value>\n"
           "  <bank>   = register bank in 16-bit HEX format\n"
           "  <offset> = register offset in 8-bit HEX format\n"
           "  <value>  = value to write in 32-bit HEX format\n");
