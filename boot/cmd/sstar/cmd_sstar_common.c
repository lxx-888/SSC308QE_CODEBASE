/*
 * cmd_sstar_common.c - Sigmastar
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

#include "include/cmd_sstar_common.h"
#include <asm/arch/mach/sstar_types.h>
#include <common.h>

char *get_script_next_line(char **line_buf_ptr)
{
    char *line_buf;
    char *next_line;
    int   i = 0;

    line_buf = (*line_buf_ptr);

    // strip '\r', '\n' and comment
    while (1)
    {
        // strip '\r' & '\n'
        if (IS_LINE_END(line_buf[0]))
        {
            line_buf++;
        }
        // strip comment
        else if (IS_COMMENT(line_buf[0]))
        {
            for (i = 0; !IS_LINE_END(line_buf[0]) && i <= SCRIPT_MAX_LINE_SIZE; i++)
            {
                line_buf++;
            }

            if (i > SCRIPT_MAX_LINE_SIZE)
            {
                line_buf[0] = SCRIPT_FILE_END;

                printf("Error: the max size of one line is %d!!!\n",
                       SCRIPT_MAX_LINE_SIZE); // <-@@@

                break;
            }
        }
        else
        {
            break;
        }
    }

    // get next line
    if (IS_FILE_END(line_buf[0]))
    {
        next_line = NULL;
    }
    else
    {
        next_line = line_buf;

        for (i = 0; !IS_LINE_END(line_buf[0]) && i <= SCRIPT_MAX_LINE_SIZE; i++)
        {
            line_buf++;
        }

        if (i > SCRIPT_MAX_LINE_SIZE)
        {
            next_line = NULL;

            printf("Error: the max size of one line is %d!!!\n",
                   SCRIPT_MAX_LINE_SIZE); // <-@@@
        }
        else
        {
            line_buf[0]   = '\0';
            *line_buf_ptr = line_buf + 1;
        }
    }

    return next_line;
}
