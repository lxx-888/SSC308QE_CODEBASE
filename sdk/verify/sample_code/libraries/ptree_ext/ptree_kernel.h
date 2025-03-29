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

#ifndef __PTREE_KERNEL__
#define __PTREE_KERNEL__

/**
 * struct ptree_user_config - User mode's configuration for ptree.
 *
 * @is_using_binary  : Using ptree binary data or parsing script.
 * @config_data_size : Set data size of data[0].
 * @config_data      : Set config as an arena buffer pool if is_using_binary=1, or a file path if is_using_binary = 0.
 */
struct ptree_user_config
{
    unsigned char is_using_binary;
    unsigned int  config_data_size;
    char*         config_data;
};

#define PTREE_DEVICE_NAME              "sstar_ptree"
#define PTREE_IOCTL_CONSTRUCT_PIPELINE _IO('k', 0)
#define PTREE_IOCTL_DESTRUCT_PIPELINE  _IO('k', 1)
#define PTREE_IOCTL_START_PIPELINE     _IO('k', 2)
#define PTREE_IOCTL_STOP_PIPELINE      _IO('k', 3)
#define PTREE_IOCTL_SET_CONFIG         _IOW('k', 4, struct ptree_user_config)

#endif //__PTREE_KERNEL__
