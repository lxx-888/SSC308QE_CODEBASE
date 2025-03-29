/* SigmaStar trade secret */
/* Copyright (c) [2019~2020] SigmaStar Technology.
All rights reserved.

Unless otherwise stipulated in writing, any and all information contained
herein regardless in any format shall remain the sole proprietary of
SigmaStar and be kept in strict confidence
(SigmaStar Confidential Information) by the recipient.
Any unauthorized act including without limitation unauthorized disclosure,
copying, use, reproduction, sale, distribution, modification, disassembling,
reverse engineering and compiling of the contents of SigmaStar Confidential
Information is unlawful and strictly prohibited. SigmaStar hereby reserves the
rights to any and all damages, losses, costs and expenses resulting therefrom.
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "ptree_bin.h"
#include "ptree_kernel.h"

static inline int _file_name_check(const char *file) /* NOLINT */
{
    int surfix_off = 0;

    surfix_off = strlen(file);
    if (surfix_off > 6 && !strcmp(file + surfix_off - 5, ".json"))
    {
        printf("[PTREE_DB_JSON]:File name (%s) check pass.\n", file);
        return 0;
    }
    printf("[PTREE_DB_JSON]: File name (%s) check error.\n", file);
    return -1;
}

int main(int argc, char **argv)
{
    int                      fd;
    char                     open_path[32];
    char                     get_val = 0;
    struct ptree_user_config config;

    if (argc != 2)
    {
        printf("Usage: %s [binary name] | [json/ini file]\n", argv[0]);
        return -1;
    }
    // Open the device
    snprintf(open_path, 32, "/dev/%s", PTREE_DEVICE_NAME);
    fd = open(open_path, O_RDWR);
    if (fd < 0)
    {
        perror("Failed to open the device");
        return -1;
    }
    memset(&config, 0, sizeof(struct ptree_user_config));
    if (_file_name_check(argv[1]) == 0)
    {
        config.config_data_size = strlen(argv[1]) + 1;
        config.config_data      = argv[1];
    }
    else
    {
        const PTREE_BIN_Info_t *binInfo = NULL;
        binInfo                         = _PTREE_BIN_GetBinInfo(argv[1]);
        if (!binInfo)
        {
            printf("Can not find the pipeline name: %s\n", argv[1]);
            return -1;
        }
        config.is_using_binary  = 1;
        config.config_data      = binInfo->data;
        config.config_data_size = binInfo->size;
    }
    if (ioctl(fd, PTREE_IOCTL_SET_CONFIG, &config) < 0)
    {
        goto ERR_IOCTL0;
    }
    if (ioctl(fd, PTREE_IOCTL_CONSTRUCT_PIPELINE) < 0)
    {
        goto ERR_IOCTL0;
    }
    if (ioctl(fd, PTREE_IOCTL_START_PIPELINE) < 0)
    {
        goto ERR_IOCTL1;
    }
    while (1)
    {
        printf("Type 'q' to exit!\n");
        get_val = getchar();
        if (get_val == 'q')
        {
            break;
        }
    }
    if (ioctl(fd, PTREE_IOCTL_STOP_PIPELINE) < 0)
    {
        goto ERR_IOCTL1;
    }
    if (ioctl(fd, PTREE_IOCTL_DESTRUCT_PIPELINE) < 0)
    {
        goto ERR_IOCTL0;
    }

    // Close the device
    close(fd);
    return 0;
ERR_IOCTL1:
    perror("IOCTL1 failed");
    ioctl(fd, PTREE_IOCTL_DESTRUCT_PIPELINE);
ERR_IOCTL0:
    perror("IOCTL0 failed");
    close(fd);
    return -1;
}
