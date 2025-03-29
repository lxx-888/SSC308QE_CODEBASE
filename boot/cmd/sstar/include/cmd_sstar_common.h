/*
 * cmd_sstar_common.h - Sigmastar
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

#define DEFAULT_SCRIPT_FILE_NAME "auto_update.txt"
#define SCRIPT_MAX_LINE_SIZE     8000
#define SCRIPT_MAX_FILE_SIZE     4096

#define SCRIPT_FILE_COMMENT '#'
#define SCRIPT_FILE_END     '%'
#define IS_COMMENT(x)       (SCRIPT_FILE_COMMENT == (x))
#define IS_FILE_END(x)      (SCRIPT_FILE_END == (x))
#define IS_LINE_END(x)      (('\r' == (x)) || ('\n' == (x)))
extern char *get_script_next_line(char **line_buf_ptr);
