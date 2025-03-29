/*
 * io_check.c- Sigmastar
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <riu.h>
#include <padmux.h>
#include <hal_pinmux.h>
#include <hal_gpio_common.h>

static int get_mode_flag = 0;
static int set_mode_flag = 0;
static int verify_flag   = 0;
static int mode          = -1;
static int io_index      = -1;

static void print_usage(const char *prog)
{
    printf("Usage: %s\n", prog);
    puts(
        "  -s --set mode (e.g. \"prog_io_check -i <index> -s -m <mode>\")\n"
        "  -g --get mode (e.g. \"prog_io_check -i <index> -g\")\n"
        "  -v --verify mode reuse (e.g. \"prog_io_check -i <index> -v -m <mode>\")\n");
    exit(1);
}

static void parse_opts(int argc, char *argv[])
{
    while (1)
    {
        static const struct option lopts[] = {
            {"index", 1, 0, 'i'}, {"set", 1, 0, 's'}, {"get", 1, 0, 'g'}, {"verify", 1, 0, 'v'}, {"mode", 1, 0, 'm'},
        };
        int c;

        c = getopt_long(argc, argv, "i:sgm:v", lopts, NULL);

        if (c == -1)
            break;

        switch (c)
        {
            case 'i':
                io_index = atoi(optarg);
                break;
            case 's':
                set_mode_flag = 1;
                break;
            case 'g':
                get_mode_flag = 1;
                break;
            case 'm':
                mode = atoi(optarg);
                break;
            case 'v':
                verify_flag = 1;
                break;
            default:
                print_usage(argv[0]);
                break;
        }
    }

    if (io_index < 0 || io_index > gpio_table_size)
    {
        printf("Io index no found !\n");
        print_usage(argv[0]);
    }
    else
    {
        printf("pad name:%s, pad id:%d\n", gpio_table[io_index].p_name, io_index);
    }
}

static void gpio_pad_get_val(void)
{
    u16 mask       = 0;
    u16 content    = 0;
    u16 temp       = 0;
    u32 bank       = 0;
    u32 offset     = 0;
    u32 mode_index = 65535;

    for (int i = 0; i < m_hal_gpio_st_padmux_entry[io_index].size; i++)
    {
        mask    = m_hal_gpio_st_padmux_entry[io_index].padmux[i].mask;
        bank    = m_hal_gpio_st_padmux_entry[io_index].padmux[i].base >> 8;
        offset  = m_hal_gpio_st_padmux_entry[io_index].padmux[i].offset;
        content = 0;

        printf("0x%04x%02x", bank, offset);
        riu_r(bank, offset, &content, false);
        printf(" = 0x%04x                ", content);

        temp = content & mask;
        if (temp == m_hal_gpio_st_padmux_entry[io_index].padmux[i].val)
        {
            mode_index = mode_index < i ? mode_index : i;
            printf("\033[33m%d->%s\033[0m\n", i,
                   m_hal_gpio_st_padmode_info_tbl[m_hal_gpio_st_padmux_entry[io_index].padmux[i].mode].pad_name);
        }
        else
        {
            printf("%d->%s\n", i,
                   m_hal_gpio_st_padmode_info_tbl[m_hal_gpio_st_padmux_entry[io_index].padmux[i].mode].pad_name);
        }
    }

    if (mode_index == 65535)
    {
        printf("This Pin Mode Not Set\n");
    }
    else
    {
        printf("This Pin Mode Set %s\n",
               m_hal_gpio_st_padmode_info_tbl[m_hal_gpio_st_padmux_entry[io_index].padmux[mode_index].mode].pad_name);
    }
}
static void gpio_pad_check(struct hal_gpio_st_pad_check info)
{
    u16 mask   = 0;
    u32 bank   = 0;
    u32 offset = 0;

    for (int i = 0; i < m_hal_gpio_st_padmux_entry[io_index].size; i++)
    {
        mask   = m_hal_gpio_st_padmux_entry[io_index].padmux[i].mask;
        bank   = m_hal_gpio_st_padmux_entry[io_index].padmux[i].base >> 8;
        offset = m_hal_gpio_st_padmux_entry[io_index].padmux[i].offset;

        if ((info.base == bank) && (info.offset == offset) && (info.mask == mask))
        {
            printf("  mode %d->%s\n", i,
                   m_hal_gpio_st_padmode_info_tbl[m_hal_gpio_st_padmux_entry[io_index].padmux[i].mode].pad_name);
        }
    }
}

int main(int argc, char **argv)
{
    int i = 0;

    printf("Note: This tool is only used on platform %s\n", CHIP_NAME);

    parse_opts(argc, argv);

    /* set padmux mode */
    if (set_mode_flag && mode >= 0)
    {
        if (hal_gpio_pad_set_val(io_index, m_hal_gpio_st_padmux_entry[io_index].padmux[mode].mode))
        {
            printf("Set mode fail\n");
            return -1;
        }
    }

    /* get padmux message */
    gpio_pad_get_val();

    if (verify_flag && mode >= 0)
    {
        if (hal_gpio_pad_check_val(io_index, m_hal_gpio_st_padmux_entry[io_index].padmux[mode].mode))
        {
            printf("Warning: To set pin %s to %s, you need to set the following modes:\n", gpio_table[io_index].p_name,
                   m_hal_gpio_st_padmode_info_tbl[m_hal_gpio_st_padmux_entry[io_index].padmux[mode].mode].pad_name);
            for (i = 0; i < m_hal_gpio_st_pad_checkVal.infocount; i++)
            {
                gpio_pad_check(m_hal_gpio_st_pad_checkVal.infos[i]);
            }
        }
        else
        {
            printf("The current pin %s is already in %s mode\n", gpio_table[io_index].p_name,
                   m_hal_gpio_st_padmode_info_tbl[m_hal_gpio_st_padmux_entry[io_index].padmux[mode].mode].pad_name);
        }
    }

    return 0;
}
