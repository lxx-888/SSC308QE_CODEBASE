/*
 * usbstar.c - Sigmastar
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
#include "asm/arch/mach/platform.h"
#include "include/cmd_sstar_common.h"
#include <malloc.h>
#include <linux/delay.h>

extern int usb_stor_curr_dev; /* current device */
#define ENV_USB_UPGRADEIMAGE         "UsbUpgradeImage"
#define msleep(a)                    udelay(a * 1000)
#define USBSTAR_SCRIPT_MAX_FILE_SIZE 8192

#define ENV_USB_UPGRADEIMAGE "UsbUpgradeImage"
int do_usbstar(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
    char *buffer = NULL;
    char *script_buf;
    char *next_line;
    char  tmp[100];
    int   ret        = -1;
    char *image_name = NULL;
    int   count      = 0;

    if ((buffer = (char *)malloc(USBSTAR_SCRIPT_MAX_FILE_SIZE)) == NULL)
    {
        printf("no memory for command string!!\n");
        return -1;
    }

    image_name = env_get(ENV_USB_UPGRADEIMAGE);
    if (image_name == NULL)
    {
        printf("UsbUpgradeImage env is not set,use default mass storage rootdir file(SigmastarUpgrade.bin)\n");
        image_name = "SigmastarUpgrade.bin";
        run_command("setenv UpgradeImage SigmastarUpgrade.bin", 0);
    }

again:
    // usb reset
    if ((ret = run_command("usb reset", 0)) != 0)
    {
        printf("usb reset fail\n");
        goto end;
    }

    if (usb_stor_curr_dev == -1)
    {
        if (count++ > 10)
        {
            printf("counts exceed 10 ,exit usbstar\n");
            goto end;
        }
        msleep(3000);
        goto again;
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
    memset(script_buf, 0, USBSTAR_SCRIPT_MAX_FILE_SIZE);

    memset(tmp, 0, sizeof(tmp));
    snprintf(tmp, sizeof(tmp) - 1, "fatload usb 0 0x%X %s 0x4000 0x0", (U32)script_buf, image_name);
    printf("[USBSTAR runcmd] %s\n", tmp);
    if ((ret = run_command(tmp, 0)) != 0)
    {
        printf("execute cmd:%s fail\n", tmp);
        goto end;
    }

#if defined(CONFIG_LMB) && defined(CONFIG_SSTAR_ESTAR_LOAD_SCRIPT_ADDR)
    memcpy((void *)buffer, (void *)script_buf, USBSTAR_SCRIPT_MAX_FILE_SIZE);
    script_buf = buffer;
#endif

    while ((next_line = get_script_next_line(&script_buf)) != NULL)
    {
        printf("[USBSTAR runcmd] %s\n", next_line);
        if ((ret = run_command(next_line, 0)) != 0)
        {
            printf("[USBSTAR runcmd] %s fail!!!!!\n", next_line);
            goto end;
        }
    }
end:
    free(buffer);
    return ret;
}
U_BOOT_CMD(usbstar, CONFIG_SYS_MAXARGS, 1, do_usbstar, "script via USB package", "");
