/*
 * estar.c - Sigmastar
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

#include <asm/arch/mach/sstar_types.h>
#include <command.h>
#include <common.h>
#include <malloc.h>

#include "include/cmd_sstar_common.h"

#define ARG_SCRIPT_FILE_INDEX 1

#define IS_ARG_AVAILABLE_SCRIPT_FILE(x) ((x) > ARG_SCRIPT_FILE_INDEX)
#define ARG_SCRIPT_FILE(x)              (x)[ARG_SCRIPT_FILE_INDEX]

int do_estar(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
    char *buffer     = NULL;
    char *script_buf = NULL;
    int   ret        = -1;

    /*
     * script may call "estar" to download other script
     * so store script in heap.
     */
    buffer = (char *)malloc(SCRIPT_MAX_FILE_SIZE);
    if (buffer == NULL)
    {
        printf("no memory for command string!!\n");
        return -1;
    }

    /*
     * If LMB is enable, cmd "tftp" counld't download data
     * to memory where has been malloc, so first download to
     * CONFIG_SSTAR_ESTAR_LOAD_SCRIPT_ADDR then copy to heap.
     */
#if defined(CONFIG_LMB) && defined(CONFIG_SSTAR_ESTAR_LOAD_SCRIPT_ADDR)
    script_buf = (char *)CONFIG_SSTAR_ESTAR_LOAD_SCRIPT_ADDR;
#else
    script_buf = buffer;
#endif

    memset(script_buf, 0, SCRIPT_MAX_FILE_SIZE);
    sprintf(script_buf, "tftp %p %s", script_buf,
            IS_ARG_AVAILABLE_SCRIPT_FILE(argc) ? ARG_SCRIPT_FILE(argv) : DEFAULT_SCRIPT_FILE_NAME);
    run_command(script_buf, 0);

#if defined(CONFIG_LMB) && defined(CONFIG_SSTAR_ESTAR_LOAD_SCRIPT_ADDR)
    memcpy((void *)buffer, (void *)script_buf, SCRIPT_MAX_FILE_SIZE);
#endif

    script_buf = buffer;
    char *next_line;
    while ((next_line = get_script_next_line(&script_buf)) != NULL)
    {
        printf("\n>> %s \n", next_line);
        ret = 0; // at least one cmd

        // if any cmd failed, stop execute the script
        if (run_command(next_line, 0))
        {
            ret = -1;
            break;
        }
    }
    free(buffer);

    return ret;
}

U_BOOT_CMD(estar, CONFIG_SYS_MAXARGS, 1, do_estar, "script via network",
           "script via network\n"
           "estar [script_name.txt]");
