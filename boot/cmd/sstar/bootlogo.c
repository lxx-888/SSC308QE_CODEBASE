/*
 * bootlogo.c - Sigmastar
 *
 * Copyright (c) [2019~2022] SigmaStar Technology.
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

//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
#include <common.h>
#include <command.h>
#include <bootstage.h>
#include <malloc.h>
#include <memalign.h>
#include <stdlib.h>
#include <cpu_func.h>
#include <stdlib.h>
#include <linux/delay.h>

//#include "asm/arch/mach/ms_types.h"
#include "asm/arch/mach/platform.h"
#include "asm/arch/mach/io.h"
#include <asm/gpio.h>

#include <ubi_uboot.h>

#ifdef CONFIG_SSTAR_DISP
#include "cam_os_wrapper.h"
#include "mi_common_datatype.h"
#include "mi_disp_impl_datatype.h"
#include "drv_disp_os_header.h"
#include "hal_disp_include.h"
#include "drv_disp_if.h"
#include <pwm.h>
#include <dm.h>
#endif

#ifdef CONFIG_SSTAR_HDMITX
#include "mhal_hdmitx_datatype.h"
#include "mhal_hdmitx.h"
#endif

#ifdef CONFIG_SSTAR_CJSON_PARSER
#include "cjson.h"
#endif

#ifdef CONFIG_FS_LITTLEFS
#include "littlefs.h"
#define fs_mount(partition, mnt_path)  littlefs_mount(partition, mnt_path)
#define fs_unmount()                   littlefs_unmount()
#define fs_open(filename, flags, mode) littlefs_open(filename, flags, mode)
#define fs_close(fd)                   littlefs_close(fd)
#define fs_read(fd, buf, count)        littlefs_read(fd, buf, count)
#define fs_write(fd, buf, count)       littlefs_write(fd, buf, count)
#define fs_lseek(fd, offset, whence)   littlefs_lseek(fd, offset, whence)

#elif defined(CONFIG_FS_FIRMWAREFS)
#include "firmwarefs.h"
#define fs_mount(partition, mnt_path)  firmwarefs_mount(partition, mnt_path)
#define fs_unmount()                   firmwarefs_unmount()
#define fs_open(filename, flags, mode) firmwarefs_open(filename, flags, mode)
#define fs_close(fd)                   firmwarefs_close(fd)
#define fs_read(fd, buf, count)        firmwarefs_read(fd, buf, count)
#define fs_write(fd, buf, count)       firmwarefs_write(fd, buf, count)
#define fs_lseek(fd, offset, whence)   firmwarefs_lseek(fd, offset, whence)

#elif defined(CONFIG_FS_EXT4)
#include "fs.h"
#include "ext4fs.h"
#define fs_mount(partition, mnt_path) ext4fs_mount_wrapper(partition, mnt_path)
#define fs_unmount()
#else
#error "CONFIG_FS_LITTLEFS or CONFIG_FS_FIRMWAREFS or CONFIG_FS_EXT4 must be defined"
#endif

#ifdef CONFIG_SSTAR_JPD
#ifdef CONFIG_JPD_SW
#include "jinclude.h"
#include "jpeglib.h"
#elif defined(CONFIG_JPD_HW)
#include "mhal_jpd.h"

#ifdef CONFIG_SSTAR_RGN
#include "mhal_rgn_datatype.h"
#include "mhal_rgn.h"
#endif

#define MAX_DEC_HEIGHT 8640
#define MAX_DEC_WIDTH  10000
#define ALIGNMENT_NEED 64
typedef struct
{
    void * InputAddr;
    void * InputpVirAddr;
    MI_U32 InputSize;

    void * OutputAddr;
    void * OutpVirAddr;
    MI_U32 OutputSize;

    void * InterAddr;
    void * InterVirAddr;
    MI_U32 InterSize;
} MHal_JPD_Addr_t;

#endif
#endif

#ifdef CONFIG_MS_PARTITION
#include "part_mxp.h"
#endif

#ifdef CONFIG_SSTAR_RGN
#include "mhal_rgn_datatype.h"
#include "mhal_rgn.h"
#endif

#include <linux/libfdt.h>
#include <fdt_support.h>
#include <mapmem.h>

#include "font_ascii_32x32.h"

#define BOOTLOGO_TO_VIRA(pa) ((MI_U32)((pa) + 0x20000000))
#define BOOTLOGO_TO_PHYA(va) ((MI_U32)((va)-0x20000000))
#define MAX_DISP_DEV_NUM     HAL_DISP_DEVICE_MAX
#define MAX_LOGO_FILE_NUM    (2)
#define MAX_STR_LEN          (128)
#define DISP_BASE_ALIGN      (16)

#define MAX_CMDSTABLE_NUMS (4)
#define MAX_CMDS_LEN       (128)
#define MAX_CMDLINE_NUMS   (256)
#define PAD_UNKNOWN        (65535)

//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------

#define GET_ARRAY_SIZE(x)             (sizeof(x) / sizeof((x)[0]))
#define BL_ALIGN_UP(val, alignment)   ((((val) + (alignment)-1) / (alignment)) * (alignment))
#define BL_ALIGN_DOWN(val, alignment) (((val) / (alignment)) * (alignment))
#define BL_MAX(a, b)                   \
    (                                  \
        {                              \
            typeof(a) _a = a;          \
            typeof(b) _b = b;          \
            (_a) > (_b) ? (_a) : (_b); \
        })

#define BL_MIN(a, b)                   \
    (                                  \
        {                              \
            typeof(a) _a = a;          \
            typeof(b) _b = b;          \
            (_a) < (_b) ? (_a) : (_b); \
        })

#define ASCII_COLOR_YELLOW "\033[1;33m"
#define ASCII_COLOR_GREEN  "\033[1;32m"
#define ASCII_COLOR_RED    "\033[1;31m"
#define ASCII_COLOR_BLUE   "\033[1;36m"
#define ASCII_COLOR_END    "\033[0m"

#define BOOTLOGO_DBG(dbglv, _fmt, _args...)                                                             \
    do                                                                                                  \
        if (dbglv & bootlogo_dbg_level)                                                                 \
        {                                                                                               \
            if (dbglv & (DBG_LEVEL_API))                                                                \
                printf("[BL API ][%09lu][%s %d]" ASCII_COLOR_BLUE, timer_get_us(), __func__, __LINE__); \
            else if (dbglv & DBG_LEVEL_ERR)                                                             \
                printf("[BL ERR ][%s %d]" ASCII_COLOR_RED, __func__, __LINE__);                         \
            else if (dbglv & DBG_LEVEL_WRN)                                                             \
                printf("[BL WRN ][%s %d]" ASCII_COLOR_YELLOW, __func__, __LINE__);                      \
            else                                                                                        \
                printf("[BL INFO][%s %d]" ASCII_COLOR_GREEN, __func__, __LINE__);                       \
            printf(_fmt, ##_args);                                                                      \
            printf(ASCII_COLOR_END);                                                                    \
        }                                                                                               \
    while (0)

#define BL_EXEC_FUNC(_func_, _except_ret)                                             \
    do                                                                                \
    {                                                                                 \
        MI_S32 _ret = 0;                                                              \
        if (_except_ret != (_ret = _func_))                                           \
        {                                                                             \
            BOOTLOGO_DBG(DBG_LEVEL_ERR, "exec func[%s] error. %#x\n", #_func_, _ret); \
            return -1;                                                                \
        }                                                                             \
        else                                                                          \
        {                                                                             \
            BOOTLOGO_DBG(DBG_LEVEL_FUNC, "exec func[%s] pass\n", #_func_);            \
        }                                                                             \
    } while (0);

#define YUV444_TO_YUV420_PIXEL_MAPPING(y_dst_addr, uv_dst_addr, dst_x, dst_y, dst_stride, src_addr, src_x, src_y, \
                                       src_w, src_h, rotate)                                                      \
    do                                                                                                            \
    {                                                                                                             \
        for (src_y = 0; src_y < src_h; src_y++)                                                                   \
        {                                                                                                         \
            for (src_x = 0; src_x < src_w; src_x++)                                                               \
            {                                                                                                     \
                *((char *)((char *)(y_dst_addr) + (dst_y) * (dst_stride) + (dst_x))) =                            \
                    *((char *)((char *)(src_addr) + (src_y) * (src_w * 3) + (src_x * 3)));                        \
                if ((src_y & 0x01) && (src_x & 0x01))                                                             \
                {                                                                                                 \
                    if (dst_y > 0)                                                                                \
                    {                                                                                             \
                        *((short *)((char *)(uv_dst_addr) + ((dst_y - 1) >> 1) * (dst_stride) + (dst_x - 1))) =   \
                            *((short *)((char *)(src_addr) + (src_y) * (src_w * 3) + (src_x * 3) + 1));           \
                    }                                                                                             \
                }                                                                                                 \
                if (rotate == LOGO_ROTATE_180)                                                                    \
                {                                                                                                 \
                    if (((dst_y + 1) == src_h) && (src_x & 0x01) && (src_y == 0))                                 \
                    {                                                                                             \
                        *((short *)((char *)(uv_dst_addr) + ((dst_y - 1) >> 1) * (dst_stride) + (dst_x - 1))) =   \
                            *((short *)((char *)(src_addr) + (src_y) * (src_w * 3) + (src_x * 3) + 1));           \
                    }                                                                                             \
                }                                                                                                 \
                else if (rotate == LOGO_ROTATE_270)                                                               \
                {                                                                                                 \
                    if ((src_x == 0) && (src_y & 0x01))                                                           \
                    {                                                                                             \
                        *((short *)((char *)(uv_dst_addr) + ((dst_y - 1) >> 1) * (dst_stride) + (dst_x - 1))) =   \
                            *((short *)((char *)(src_addr) + (src_y) * (src_w * 3) + (src_x * 3) + 1));           \
                    }                                                                                             \
                }                                                                                                 \
            }                                                                                                     \
        }                                                                                                         \
    } while (0);

#define RGB_PIXEL_MAPPING(dst_addr, dst_x, dst_y, dst_stride, src_addr, src_x, src_y, src_stride, src_w, src_h, type) \
    do                                                                                                                \
    {                                                                                                                 \
        for (src_y = 0; src_y < src_h; src_y++)                                                                       \
        {                                                                                                             \
            for (src_x = 0; src_x < src_w; src_x++)                                                                   \
            {                                                                                                         \
                *((type *)((char *)(dst_addr) + (dst_y) * (dst_stride) + (dst_x) * sizeof(type))) =                   \
                    *((type *)((char *)(src_addr) + (src_y) * (src_stride) + (src_x) * sizeof(type)));                \
            }                                                                                                         \
        }                                                                                                             \
    } while (0);

#define BOOTLOGO_PRINTARGV(argc, argv)                                  \
    do                                                                  \
    {                                                                   \
        MI_U16 i = 0;                                                   \
        for (i = 0; i < argc; i++)                                      \
        {                                                               \
            BOOTLOGO_DBG(DBG_LEVEL_INFO, "argv[%d]: %s\n", i, argv[i]); \
        }                                                               \
    } while (0);

#ifdef CONFIG_SSTAR_CJSON_PARSER
#define CJSON_GETINT(obj, key, exist_flag, default)                                                          \
    (                                                                                                        \
        {                                                                                                    \
            MI_S32 retval   = default;                                                                       \
            cJSON *pstCjson = cJSON_GetObjectItem(obj, key);                                                 \
            exist_flag      = FALSE;                                                                         \
            if (pstCjson)                                                                                    \
            {                                                                                                \
                retval     = pstCjson->valueint;                                                             \
                exist_flag = TRUE;                                                                           \
                BOOTLOGO_DBG(DBG_LEVEL_CJSON, "found key=%s value=%d\n", key, retval);                       \
            }                                                                                                \
            else                                                                                             \
            {                                                                                                \
                exist_flag = FALSE;                                                                          \
                BOOTLOGO_DBG(DBG_LEVEL_CJSON, "not found; Item name is %s; default val: %d\n", key, retval); \
            }                                                                                                \
            retval;                                                                                          \
        })

#define CJSON_GETSTR(obj, key, dec)                                                     \
    (                                                                                   \
        {                                                                               \
            cJSON *item = cJSON_GetObjectItem(obj, key);                                \
            if (item && item->valuestring != NULL)                                      \
            {                                                                           \
                strncpy((char *)dec, item->valuestring, strlen(item->valuestring) + 1); \
            }                                                                           \
            else                                                                        \
            {                                                                           \
                BOOTLOGO_DBG(DBG_LEVEL_CJSON, "not found; Item name is %s\n", key);     \
            }                                                                           \
        })

#define CJSON_GET_PNL_ARRAY(obj, key, cmdbuff, cmdcnt, data_type, exist_flag)                                     \
    do                                                                                                            \
    {                                                                                                             \
        cJSON *    pstCmdBuffChildNode = NULL;                                                                    \
        MI_S32     s32Cnt              = 0;                                                                       \
        MI_S32     s32Index            = 0;                                                                       \
        data_type *pBase               = NULL;                                                                    \
        if (obj->type != cJSON_Array)                                                                             \
        {                                                                                                         \
            BOOTLOGO_DBG(DBG_LEVEL_ERR, "PNL_ARRAY: %s is not a array\n", key);                                   \
            exist_flag = 0;                                                                                       \
            break;                                                                                                \
        }                                                                                                         \
        s32Cnt = cJSON_GetArraySize(obj);                                                                         \
        if (s32Cnt <= 0)                                                                                          \
        {                                                                                                         \
            BOOTLOGO_DBG(DBG_LEVEL_ERR, "PNL_ARRAY: %s nunmbers is error\n", key);                                \
            exist_flag = 0;                                                                                       \
            break;                                                                                                \
        }                                                                                                         \
        pBase = (data_type *)malloc_cache_aligned(s32Cnt * sizeof(data_type));                                    \
        if (pBase == NULL)                                                                                        \
        {                                                                                                         \
            BOOTLOGO_DBG(DBG_LEVEL_ERR, "PNL_ARRAY: %s malloc size: %d fail\n", key, s32Cnt * sizeof(data_type)); \
            exist_flag = 0;                                                                                       \
            break;                                                                                                \
        }                                                                                                         \
        memset(pBase, 0, s32Cnt * sizeof(data_type));                                                             \
        for (s32Index = 0; s32Index < s32Cnt; s32Index++)                                                         \
        {                                                                                                         \
            pstCmdBuffChildNode = cJSON_GetArrayItem(obj, s32Index);                                              \
            if (pstCmdBuffChildNode->type != cJSON_String)                                                        \
            {                                                                                                     \
                BOOTLOGO_DBG(DBG_LEVEL_ERR, "PNL_ARRAY: %s numbers is not string\n", key);                        \
                exist_flag = 0;                                                                                   \
                free(pBase);                                                                                      \
                break;                                                                                            \
            }                                                                                                     \
            *(pBase + s32Index) = (data_type)strtoul(pstCmdBuffChildNode->valuestring, NULL, 0);                  \
        }                                                                                                         \
        cmdcnt     = s32Cnt;                                                                                      \
        cmdbuff    = pBase;                                                                                       \
        exist_flag = 1;                                                                                           \
        BOOTLOGO_DBG(DBG_LEVEL_CJSON, "cmd_buf_node: %s; CmdBuffCnt: %d; CmdBuffParse done!\n", key, cmdcnt);     \
    } while (0);
#endif

typedef enum
{
    DBG_LEVEL_ERR   = 0x1,
    DBG_LEVEL_INFO  = 0x2,
    DBG_LEVEL_JPD   = 0x4,
    DBG_LEVEL_CJSON = 0x8,
    DBG_LEVEL_DTS   = 0x10,
    DBG_LEVEL_MHAL  = 0x20,
    DBG_LEVEL_FUNC  = 0x40,
    DBG_LEVEL_FILE  = 0x80,
    DBG_LEVEL_API   = 0x100,
    DBG_LEVEL_WRN   = 0x200,
} dbg_level;

typedef enum
{
    LOGO_ROTATE_NONE,
    LOGO_ROTATE_90,
    LOGO_ROTATE_180,
    LOGO_ROTATE_270
} logo_rotation;

typedef enum
{
    ASPECT_RATIO_ZOOM,
    ASPECT_RATIO_CENTER,
    ASPECT_RATIO_USER
} aspect_ratio;

typedef enum
{
    DISP_INTERFACE_DPI,
    DISP_INTERFACE_MIPI,
    DISP_INTERFACE_LVDS,
    DISP_INTERFACE_MIPI1,
    DISP_INTERFACE_LVDS1,
    DISP_INTERFACE_MAX,
} display_interface;

typedef struct
{
    MI_BOOL color_metrix_valid_flag;
    MI_U32  color_metrix_id;

    MI_BOOL gop_dst_valid_flag;
    MI_U32  gop_dst_id;
} disp_misc_config;

typedef struct
{
    char                 file_path[MAX_STR_LEN];
    MI_U32               file_data[2]; //[0]: VA  [1]:  PA
    MI_S32               file_size;
    MI_SYS_PixelFormat_e pixel_format;   // MI_SYS_PixelFormat_e
    MI_U32               decode_data[2]; //[0]: VA  [1]:  PA
    MI_U32               decode_data_size;
    MI_U32               img_stride;
    MI_U32               img_width;
    MI_U32               img_height;
    MI_BOOL              decoded_flag;
    MI_U32               img_crop_x;
    MI_U32               img_crop_y;
    MI_U32               img_crop_width;
    MI_U32               img_crop_height;
} disp_image_info;

typedef struct
{
    MI_U32       width;
    MI_U32       height;
    const MI_U8 *font;
} ui_font_info;

typedef struct
{
    MI_U32 length;   // bar length
    MI_U32 x1;       // bar start x
    MI_U32 y1;       // bar start y
    MI_U32 x2;       // bar end x
    MI_U32 y2;       // bar end y
    MI_U32 thick;    // bar thick
    MI_U32 bg_color; // bar background color bit[23, 0]rgb

    MI_U32 percent_info_length;
    MI_U32 percent_info_x;
    MI_U32 percent_info_y;
} ui_bar_info;

typedef struct
{
    MI_U32 dev_id;
    MI_U32 gwin_id;
    void * ctx;
    MI_S32 active;
} gop_config;

typedef struct
{
    MI_U32 pin;
    MI_U32 value;
    MI_U32 delay;
} gpio_config;

typedef struct
{
    MI_U32 pwm_pin;
    MI_U32 period;
    MI_U32 duty_cycle;
} backlight_config;

typedef struct
{
    MI_U32            logo_id;
    aspect_ratio      aspect_ratio;
    logo_rotation     rotate;
    display_interface disp_intf;
    MI_U32            disp_interface;
    MI_U32            disp_dev_id;
    MI_U32            layer_type;
    MI_U32            port_id;
    void *            dev_ctx;
    void *            vid_layer_ctx;
    void *            input_port_ctx;
    MI_BOOL           disp_config_parsed;
    MI_BOOL           disp_inited;
    MI_BOOL           disp_injected;
    MI_BOOL           show_blank_flag;
    MI_BOOL           ui_inited;
    MI_BOOL           backlight_inited;
#ifdef CONFIG_SSTAR_HDMITX
    void *  hdmitx_ctx;
    MI_BOOL hdmi_flag;
#endif
    MI_U32 width;
    MI_U32 height;
    MI_U32 fps;

    char                           target_node_name[MAX_STR_LEN];
    disp_image_info *              image_info;
    char                           pq_file[MAX_STR_LEN];
    disp_misc_config               disp_misc_cfg;
    MI_DISP_IMPL_MhalPanelConfig_t panel_cfg;
    gpio_config                    gpio_config[3]; // 0:power-gpios 1:reset-gpios 2:backlight-gpios
    MI_U32                         reset_dealy_ms;
    MI_U32                         reset_during_us;
    backlight_config               backlight_config;

#if defined(CONFIG_SSTAR_DISP)
    MI_DISP_IMPL_MhalPnlUnifiedParamConfig_t mhal_pnl_unified_param_cfg;
#endif
    gop_config gop_cfg;
} display_config;

typedef struct
{
    MI_U32 phy_addr;
    MI_U32 size;
    MI_U32 offest;
} disp_buffer_info;

#ifdef CONFIG_SSTAR_HDMITX
typedef struct
{
    const char *             timing_str;
    MI_DISP_OutputTiming_e   disp_timig;
    MhaHdmitxTimingResType_e hdmi_timig;
} timing_str2enum;
#endif

typedef struct
{
    char   cmdtable_name[MAX_STR_LEN];
    MI_S32 cmdtable_lines;
    char   cmdtable_data[MAX_CMDLINE_NUMS][MAX_CMDS_LEN];
} cmd_table;

typedef struct
{
    MI_U8     count;
    cmd_table tables[MAX_CMDLINE_NUMS];
} cmd_table_config;

typedef struct
{
    MI_U32 x1;
    MI_U32 y1;
    MI_U32 x2;
    MI_U32 y2;
    MI_U32 width;
    MI_U32 height;
} ui_blank_info;

typedef struct
{
    // common
    MI_BOOL                           logo_config_parsed;
    MI_BOOL                           common_inited;
    MI_BOOL                           cfg_data_init_flag;
    MI_BOOL                           repeat_flag;
    MI_U32                            disp_dev_count;
    MI_S32                            pq_size;
    MI_U32                            pq_data_addr[2]; //[0]: VA  [1]:  PA
    display_config                    disp_cfgs[MAX_DISP_DEV_NUM];
    disp_buffer_info                  disp_buf_info;
    disp_image_info                   logo_files[MAX_LOGO_FILE_NUM];
    disp_image_info                   ui_info[MAX_DISP_DEV_NUM];
    MI_DISP_IMPL_MhalMemAllocConfig_t phy_mem;
} bootlogo_contex;

typedef struct
{
    const char *disp_intf_str;
    MI_U32      disp_intf;
} disp_intf_str2enum;

bootlogo_contex g_bootlogo_contex;
MI_U32          bootlogo_dbg_level = DBG_LEVEL_ERR | DBG_LEVEL_WRN;

#ifdef CONFIG_SSTAR_HDMITX
timing_str2enum timing_map[] = {
    {"DACOUT_480I", -1, E_MI_DISP_OUTPUT_NTSC},
    {"DACOUT_480P", E_MHAL_HDMITX_RES_720X480P_60HZ, E_MI_DISP_OUTPUT_480P60},
    {"DACOUT_576I", -1, E_MI_DISP_OUTPUT_PAL},
    {"DACOUT_576P", E_MHAL_HDMITX_RES_720X576P_50HZ, E_MI_DISP_OUTPUT_576P50},
    {"DACOUT_720P_24", E_MHAL_HDMITX_RES_1280X720P_24HZ, E_MI_DISP_OUTPUT_720P24},
    {"DACOUT_720P_25", E_MHAL_HDMITX_RES_1280X720P_25HZ, E_MI_DISP_OUTPUT_720P25},
    {"DACOUT_720P_2997", E_MHAL_HDMITX_RES_1280X720P_2997HZ, E_MI_DISP_OUTPUT_1280x720_2997},
    {"DACOUT_720P_30", E_MHAL_HDMITX_RES_1280X720P_30HZ, E_MI_DISP_OUTPUT_720P30},
    {"DACOUT_720P_50", E_MHAL_HDMITX_RES_1280X720P_50HZ, E_MI_DISP_OUTPUT_720P50},
    {"DACOUT_720P_5994", E_MHAL_HDMITX_RES_1280X720P_5994HZ, E_MI_DISP_OUTPUT_1280x720_5994},
    {"DACOUT_720P_60", E_MHAL_HDMITX_RES_1280X720P_60HZ, E_MI_DISP_OUTPUT_720P60},
    {"DACOUT_1080P_2398", E_MHAL_HDMITX_RES_1920X1080P_2398HZ, E_MI_DISP_OUTPUT_1920x1080_2398},
    {"DACOUT_1080P_24", E_MHAL_HDMITX_RES_1920X1080P_24HZ, E_MI_DISP_OUTPUT_1080P24},
    {"DACOUT_1080P_25", E_MHAL_HDMITX_RES_1920X1080P_25HZ, E_MI_DISP_OUTPUT_1080P25},
    {"DACOUT_1080P_2997", E_MHAL_HDMITX_RES_1920X1080P_2997HZ, E_MI_DISP_OUTPUT_1920x1080_2997},
    {"DACOUT_1080P_30", E_MHAL_HDMITX_RES_1920X1080P_30HZ, E_MI_DISP_OUTPUT_1080P30},
    {"DACOUT_1080P_50", E_MHAL_HDMITX_RES_1920X1080P_50HZ, E_MI_DISP_OUTPUT_1080P50},
    {"DACOUT_1080P_5994", E_MHAL_HDMITX_RES_1920X1080P_5994HZ, E_MI_DISP_OUTPUT_1920x1080_5994},
    {"DACOUT_1080P_60", E_MHAL_HDMITX_RES_1920X1080P_60HZ, E_MI_DISP_OUTPUT_1080P60},
    {"DACOUT_1440P_50", E_MHAL_HDMITX_RES_2560X1440P_50HZ, E_MI_DISP_OUTPUT_2560x1440_50},
    {"DACOUT_2K2KP_30", E_MHAL_HDMITX_RES_2560X1440P_30HZ, E_MI_DISP_OUTPUT_2560x1440_30},
    {"DACOUT_2K2KP_50", E_MHAL_HDMITX_RES_2560X1440P_50HZ, E_MI_DISP_OUTPUT_2560x1440_50},
    {"DACOUT_2K2KP_60", E_MHAL_HDMITX_RES_2560X1440P_60HZ, E_MI_DISP_OUTPUT_2560x1440_60},
    {"DACOUT_4K2KP_25", E_MHAL_HDMITX_RES_3840X2160P_25HZ, E_MI_DISP_OUTPUT_3840x2160_25},
    {"DACOUT_4K2KP_2997", E_MHAL_HDMITX_RES_3840X2160P_2997HZ, E_MI_DISP_OUTPUT_3840x2160_2997},
    {"DACOUT_4K2KP_30", E_MHAL_HDMITX_RES_3840X2160P_30HZ, E_MI_DISP_OUTPUT_3840x2160_30},
    {"DACOUT_4K2KP_60", E_MHAL_HDMITX_RES_3840X2160P_60HZ, E_MI_DISP_OUTPUT_3840x2160_60},
    {"DACOUT_640x480P_60", E_MHAL_HDMITX_RES_640X480P_60HZ, E_MI_DISP_OUTPUT_640x480_60},
    {"DACOUT_800x600P_60", E_MHAL_HDMITX_RES_800X600P_60HZ, E_MI_DISP_OUTPUT_800x600_60},
    {"DACOUT_848x480P_60", E_MHAL_HDMITX_RES_848X480P_60HZ, E_MI_DISP_OUTPUT_848x480_60},
    {"DACOUT_1024x768P_60", E_MHAL_HDMITX_RES_1024X768P_60HZ, E_MI_DISP_OUTPUT_1024x768_60},
    {"DACOUT_1280x768P_60", E_MHAL_HDMITX_RES_1280X768P_60HZ, E_MI_DISP_OUTPUT_1280x768_60},
    {"DACOUT_1280x960P_60", E_MHAL_HDMITX_RES_1280X960P_60HZ, E_MI_DISP_OUTPUT_1280x960_60},
    {"DACOUT_1280x1024P_60", E_MHAL_HDMITX_RES_1280X1024P_60HZ, E_MI_DISP_OUTPUT_1280x1024_60},
    {"DACOUT_1360x768P_60", E_MHAL_HDMITX_RES_1360X768P_60HZ, E_MI_DISP_OUTPUT_1360x768_60},
    {"DACOUT_1366x768P_60", E_MHAL_HDMITX_RES_1366X768P_60HZ, E_MI_DISP_OUTPUT_1366x768_60},
    {"DACOUT_1400x1050P_60", E_MHAL_HDMITX_RES_1400X1050P_60HZ, E_MI_DISP_OUTPUT_1400x1050_60},
    {"DACOUT_1440x900P_60", E_MHAL_HDMITX_RES_1440X900P_60HZ, E_MI_DISP_OUTPUT_1440x900_60},
    {"DACOUT_1280x800P_60", E_MHAL_HDMITX_RES_1280X800P_60HZ, E_MI_DISP_OUTPUT_1280x800_60},
    {"DACOUT_1600x900P_60", E_MHAL_HDMITX_RES_1600X900P_60HZ, E_MI_DISP_OUTPUT_1600x900_60},
    {"DACOUT_1600x1200P_60", E_MHAL_HDMITX_RES_1600X1200P_60HZ, E_MI_DISP_OUTPUT_1600x1200_60},
    {"DACOUT_1680x1050P_60", E_MHAL_HDMITX_RES_1680X1050P_60HZ, E_MI_DISP_OUTPUT_1680x1050_60},
    {"DACOUT_1920x1200P_60", E_MHAL_HDMITX_RES_1920X1200P_60HZ, E_MI_DISP_OUTPUT_1920x1200_60},
    {"DACOUT_1920x1440P_60", E_MHAL_HDMITX_RES_1920X1440P_60HZ, E_MI_DISP_OUTPUT_1920x1440_60},
    {"DACOUT_2560x1600P_60", E_MHAL_HDMITX_RES_2560X1600P_60HZ, E_MI_DISP_OUTPUT_2560x1600_60},
};
#endif

disp_intf_str2enum disp_intf_map[] = {
    {"HDMI", HAL_DISP_INTF_HDMI},        {"CVBS", HAL_DISP_INTF_CVBS},
    {"VGA", HAL_DISP_INTF_VGA},          {"LCD", HAL_DISP_INTF_LCD},
    {"YPBPR", HAL_DISP_INTF_YPBPR},      {"TTL", HAL_DISP_INTF_TTL},
    {"TTL_SPI_IF", HAL_DISP_INTF_TTL},   {"MIPI", HAL_DISP_INTF_MIPIDSI},
    {"MIPI-1", HAL_DISP_INTF_MIPIDSI_1}, {"LVDS", HAL_DISP_INTF_LVDS},
    {"LVDS-1", HAL_DISP_INTF_LVDS_1},    {"LVDS-DUAL", HAL_DISP_INTF_DUAL_LVDS},
    {"BT656", HAL_DISP_INTF_BT656},      {"BT601", HAL_DISP_INTF_BT601},
    {"BT1120", HAL_DISP_INTF_BT1120},    {"BT1120-DDR", HAL_DISP_INTF_BT1120_DDR},
    {"MCU", HAL_DISP_INTF_MCU},          {"MCU_NOFLM", HAL_DISP_INTF_MCU_NOFLM},
    {"SRGB", HAL_DISP_INTF_SRGB},
};

static void dcache_all(void)
{
    flush_dcache_all();
    invalidate_dcache_all();
}

static void set_dbg_level(void)
{
    char *env = env_get("bldbglevel");
    if (env)
    {
        bootlogo_dbg_level = simple_strtoul(env, NULL, 0);
    }
    bootlogo_dbg_level |= DBG_LEVEL_ERR | DBG_LEVEL_WRN;
}

#ifdef CONFIG_FS_EXT4
static MI_S32 ext4fs_mount_wrapper(char *partition, char *mnt_path)
{
    (void)mnt_path;
    char dev_part_str[64];

    snprintf(dev_part_str, sizeof(dev_part_str), "0#%s", partition);
    if (fs_set_blk_dev("mmc", dev_part_str, FS_TYPE_EXT))
    {
        BOOTLOGO_DBG(DBG_LEVEL_WRN, "Can't set block device %s\n", dev_part_str);
        return -1;
    }
    return 0;
}

static MI_S32 ext4fs_size_wrapper(char *filename)
{
    loff_t size = 0;

    if (ext4fs_size(filename, &size) < 0)
        return -1;

    return size;
}

static MI_S32 ext4fs_load_wrapper(char *filename, char *buffer, MI_S32 len)
{
    loff_t actread = 0;

    if (ext4fs_read(buffer, 0, len, &actread) < 0)
        return -1;

    return actread;
}

static MI_S32 read_logo_file(char *szPath, char **outbuf)
{
    MI_S32 ret       = -1;
    MI_S32 file_size = 0;

    BOOTLOGO_DBG(DBG_LEVEL_API, ">>>\n");
    BOOTLOGO_DBG(DBG_LEVEL_FILE, "szPath: %s\n", szPath);

    file_size = ext4fs_size_wrapper(szPath);
    if (file_size <= 0)
        return -1;

    *outbuf = malloc_cache_aligned(file_size);
    if ((*outbuf) == NULL)
    {
        BOOTLOGO_DBG(DBG_LEVEL_FILE, "malloc[%d] fail\n", file_size);
        return -1;
    }

    ret = ext4fs_load_wrapper(szPath, *outbuf, file_size);
    if (ret != file_size)
    {
        BOOTLOGO_DBG(DBG_LEVEL_FILE, "read %s failed!\n", szPath);
        return -1;
    }

    BOOTLOGO_DBG(DBG_LEVEL_FILE, "Read size %d\n", ret);
    dcache_all();

    BOOTLOGO_DBG(DBG_LEVEL_API, "<<<\n");
    return ret;
}
#else
static MI_S32 read_logo_file(char *szPath, char **outbuf)
{
    MI_S32 ret       = -1;
    MI_S32 file_size = 0;

    BOOTLOGO_DBG(DBG_LEVEL_API, ">>>\n");
    BOOTLOGO_DBG(DBG_LEVEL_FILE, "szPath: %s\n", szPath);
    void *pFd = fs_open(szPath, O_RDONLY, 0);
    if (!pFd)
    {
        BOOTLOGO_DBG(DBG_LEVEL_FILE, "can't open %s\n", szPath);
        goto EXIT;
    }
    file_size = fs_lseek(pFd, 0, SEEK_END);
    if (file_size <= 0)
    {
        BOOTLOGO_DBG(DBG_LEVEL_FILE, "%s size is 0\n", szPath);
        goto EXIT;
    }
    BOOTLOGO_DBG(DBG_LEVEL_FILE, "File size %d\n", file_size);

    *outbuf = malloc_cache_aligned(file_size);
    if ((*outbuf) == NULL)
    {
        BOOTLOGO_DBG(DBG_LEVEL_FILE, "malloc[%d] fail\n", file_size);
        goto EXIT;
    }

    fs_lseek(pFd, 0, SEEK_SET);
    ret = fs_read(pFd, *outbuf, file_size);
    if (ret != file_size)
    {
        BOOTLOGO_DBG(DBG_LEVEL_FILE, "read %s failed!\n", szPath);
        ret = -1;
        goto EXIT;
    }

    BOOTLOGO_DBG(DBG_LEVEL_FILE, "Read size %d\n", ret);
    dcache_all();

EXIT:
    if (pFd)
    {
        fs_close(pFd);
    }
    BOOTLOGO_DBG(DBG_LEVEL_API, "<<<\n");
    return ret;
}
#endif

#ifdef CONFIG_SSTAR_HDMITX
static MI_U32 timing_str2enum(const char *timing, MI_BOOL is_hdmi)
{
    MI_S32 i   = 0;
    MI_S32 len = GET_ARRAY_SIZE(timing_map);
    for (i = 0; i++; i < len)
    {
        if (!strcasecmp(timing_map[i].timing_str, timing))
        {
            if (is_hdmi)
            {
                return timing_map[i].hdmi_timig;
            }
            else
            {
                return timing_map[i].disp_timig;
            }
        }
    }
    BOOTLOGO_DBG(DBG_LEVEL_ERR, "No such timing\n");
    return E_MI_DISP_OUTPUT_1080P60;
}

static MI_S32 parse_dac_config(cJSON *target_node, MI_DISP_IMPL_MhalPanelConfig_t *panel_cfg)
{
    MI_S32                                    ret           = 0;
    MI_S32                                    exist_flag    = 0;
    MI_DISP_IMPL_MhalPnlUnifiedParamConfig_t *pnl_param_cfg = &panel_cfg->stPnlUniParamCfg;

    BOOTLOGO_DBG(DBG_LEVEL_API, ">>>\n");
    if (target_node == NULL)
    {
        BOOTLOGO_DBG(DBG_LEVEL_ERR, "target_node is NULLs\n");
        ret = -1;
        return ret;
    }
    pnl_param_cfg->pPanelName = malloc_cache_aligned(MAX_STR_LEN);
    if (!pnl_param_cfg->pPanelName)
    {
        BOOTLOGO_DBG(DBG_LEVEL_ERR, "malloc[%d] fail\n", MAX_STR_LEN);
        ret = -1;
        return ret;
    }
    CJSON_GETSTR(target_node, "m_pPanelName", pnl_param_cfg->pPanelName);
    BOOTLOGO_DBG(DBG_LEVEL_INFO, "m_pPanelName: %s\n", pnl_param_cfg->pPanelName);

    pnl_param_cfg->bPanelDither = CJSON_GETINT(target_node, "m_panel_flagDither", exist_flag, 0);

    MI_DISP_IMPL_MhalPnlUnifiedTgnPolarityConfig_t *pstTgnPolarityInfo = &pnl_param_cfg->stTgnPolarityInfo;
    pstTgnPolarityInfo->u8InvDCLK = CJSON_GETINT(target_node, "m_panel_flagInvDCLK", exist_flag, 0);
    pnl_param_cfg->u8TgnPolarityFlag |= exist_flag;
    pstTgnPolarityInfo->u8InvDE = CJSON_GETINT(target_node, "m_panel_flagInvDE", exist_flag, 0);
    pnl_param_cfg->u8TgnPolarityFlag |= exist_flag;
    pstTgnPolarityInfo->u8InvHSync = CJSON_GETINT(target_node, "m_panel_flagInvHSync", exist_flag, 0);
    pnl_param_cfg->u8TgnPolarityFlag |= exist_flag;
    pstTgnPolarityInfo->u8InvVSync = CJSON_GETINT(target_node, "m_panel_flagInvVSync", exist_flag, 0);
    pnl_param_cfg->u8TgnPolarityFlag |= exist_flag;

    pnl_param_cfg->u8TgnTimingFlag                                = 1;
    MI_DISP_IMPL_MhalPnlUnifiedTgnTimingConfig_t *tgn_timing_info = &pnl_param_cfg->stTgnTimingInfo;
    tgn_timing_info->u16HSyncWidth     = CJSON_GETINT(target_node, "m_wPanelHSyncWidth", exist_flag, 0);
    tgn_timing_info->u16HSyncBackPorch = CJSON_GETINT(target_node, "m_wPanelHSyncBackPorch", exist_flag, 0);
    tgn_timing_info->u16VSyncWidth     = CJSON_GETINT(target_node, "m_wPanelVSyncWidth", exist_flag, 0);
    tgn_timing_info->u16VSyncBackPorch = CJSON_GETINT(target_node, "m_wPanelVBackPorch", exist_flag, 0);
    tgn_timing_info->u16HStart         = CJSON_GETINT(target_node, "m_wPanelHStart", exist_flag, 0);
    tgn_timing_info->u16VStart         = CJSON_GETINT(target_node, "m_wPanelVStart", exist_flag, 0);
    tgn_timing_info->u16HActive        = CJSON_GETINT(target_node, "m_wPanelWidth", exist_flag, 0);
    tgn_timing_info->u16VActive        = CJSON_GETINT(target_node, "m_wPanelHeight", exist_flag, 0);
    tgn_timing_info->u16HTotal         = CJSON_GETINT(target_node, "m_wPanelHTotal", exist_flag, 0);
    tgn_timing_info->u16VTotal         = CJSON_GETINT(target_node, "m_wPanelVTotal", exist_flag, 0);
    tgn_timing_info->u32Dclk           = CJSON_GETINT(target_node, "m_wPanelDCLK", exist_flag, 0);

    MI_DISP_IMPL_MhalPnlUnifiedTgnSpreadSpectrumConfig_t *pstTgnSscInfo = &pnl_param_cfg->stTgnSscInfo;
    pstTgnSscInfo->u16SpreadSpectrumStep = CJSON_GETINT(target_node, "m_wSpreadSpectrumFreq", exist_flag, 0);
    pnl_param_cfg->u8TgnSscFlag |= exist_flag;
    BOOTLOGO_DBG(DBG_LEVEL_INFO, "u16SpreadSpectrumStep: %d\n", pstTgnSscInfo->u16SpreadSpectrumStep);
    pstTgnSscInfo->u16SpreadSpectrumSpan = CJSON_GETINT(target_node, "m_wSpreadSpectrumRatio", exist_flag, 0);
    pnl_param_cfg->u8TgnSscFlag |= exist_flag;
    BOOTLOGO_DBG(DBG_LEVEL_INFO, "u16SpreadSpectrumSpan: %d\n", pstTgnSscInfo->u16SpreadSpectrumSpan);

    pnl_param_cfg->eOutputFormatBitMode = CJSON_GETINT(target_node, "m_eOutputFormatBitMode", exist_flag, 0);
    pnl_param_cfg->u8TgnOutputBitMdFlag |= exist_flag;
    BOOTLOGO_DBG(DBG_LEVEL_INFO, "u8TgnOutputBitMdFlag: %d\n", pnl_param_cfg->u8TgnOutputBitMdFlag);

    panel_cfg->bValid = TRUE;
    BOOTLOGO_DBG(DBG_LEVEL_API, "<<<\n");
    return 0;
}
#endif

static MI_U32 disp_intf_str_to_enum(const char *disp_intf)
{
    MI_S32 len = GET_ARRAY_SIZE(disp_intf_map);
    MI_S32 i   = 0;
    for (i = 0; i < len; i++)
    {
        if (!strcmp(disp_intf, disp_intf_map[i].disp_intf_str))
        {
            return disp_intf_map[i].disp_intf;
        }
    }
    return HAL_DISP_INTF_HDMI;
}

static MI_U32 malloc_from_reserved_buf(disp_buffer_info *disp_buf_info, MI_U32 malloc_size)
{
    MI_U32 phy_addr;
    MI_U32 vir_addr;
    MI_U32 total_size = 0;

    if (disp_buf_info->phy_addr == 0)
    {
        vir_addr = (MI_U32)malloc_cache_aligned(malloc_size);
    }
    else
    {
        total_size = disp_buf_info->size - disp_buf_info->offest;
        if (malloc_size > total_size)
        {
            BOOTLOGO_DBG(DBG_LEVEL_ERR, "Malloc Fail! MallocSize[%d] > TotalSize[%d]\n", malloc_size, total_size);
            return 0;
        }
        phy_addr = disp_buf_info->phy_addr + disp_buf_info->offest;
        disp_buf_info->offest += malloc_size;
        disp_buf_info->offest = BL_ALIGN_UP(disp_buf_info->offest, 4096);
        vir_addr              = BOOTLOGO_TO_VIRA(phy_addr);
    }
    memset((char *)vir_addr, 0, malloc_size);

    BOOTLOGO_DBG(DBG_LEVEL_INFO, "Malloc size[%d] successed ! Phy: %x; TotalSize: %d\n", malloc_size, phy_addr,
                 total_size);
    return vir_addr;
}

static void free_from_reserved_buf(disp_buffer_info *disp_buf_info, char *pvir_addr, MI_U32 free_size)
{
    MI_S32 free_size_offset = 0;

    free_size_offset = BL_ALIGN_UP(free_size, 4096);
    disp_buf_info->offest -= free_size_offset;
    BOOTLOGO_DBG(DBG_LEVEL_INFO, "Free successed ! vir: %p; FreeSize: %d; offest: %u\n", pvir_addr, free_size,
                 disp_buf_info->offest);
}

static void set_gpio(MI_U32 gpio_pin, MI_U32 value, MI_U32 delay)
{
    if (gpio_pin == PAD_UNKNOWN)
    {
        BOOTLOGO_DBG(DBG_LEVEL_INFO, "GPIO ID UNKNOWN.\n");
        return;
    }
    if (gpio_request(gpio_pin, "gpio_pin") < 0)
    {
        BOOTLOGO_DBG(DBG_LEVEL_INFO, "GPIO request faild.\n");
    }
    gpio_direction_output(gpio_pin, 1);
    gpio_set_value(gpio_pin, (value ? 0 : 1));
    mdelay(delay);
}

static MI_S32 hdmi_init(display_config *disp_cfg)
{
#if defined(CONFIG_SSTAR_HDMITX)
    MhalHdmitxAttrConfig_t stAttrCfg;
    memset(&stAttrCfg, 0, sizeof(stAttrCfg));
    MhalHdmitxSignalConfig_t stSignalCfg;
    memset(&stSignalCfg, 0, sizeof(stSignalCfg));
    MhalHdmitxMuteConfig_t stMuteCfg;
    memset(&stMuteCfg, 0, sizeof(stMuteCfg));

    BL_EXEC_FUNC(MhalHdmitxCreateInstance(&disp_cfg->hdmitx_ctx, disp_cfg->hdmi_flag), 0);
    // MhalHdmitxSetDebugLevel(disp_cfg->hdmitx_ctx, 0x3F);

    stMuteCfg.enType = E_MHAL_HDMITX_MUTE_AUDIO | E_MHAL_HDMITX_MUTE_VIDEO | E_MHAL_HDMITX_MUTE_AVMUTE;
    stMuteCfg.bMute  = 1;
    BL_EXEC_FUNC(MhalHdmitxSetMute(disp_cfg->hdmitx_ctx, &stMuteCfg), 0);
    stSignalCfg.bEn = 0;
    BL_EXEC_FUNC(MhalHdmitxSetSignal(disp_cfg->hdmitx_ctx, &stSignalCfg), 0);

    stAttrCfg.bVideoEn     = 1;
    stAttrCfg.enInColor    = E_MHAL_HDMITX_COLOR_RGB444;
    stAttrCfg.enOutColor   = E_MHAL_HDMITX_COLOR_RGB444;
    stAttrCfg.enOutputMode = E_MHAL_HDMITX_OUTPUT_MODE_HDMI;
    stAttrCfg.enColorDepth = E_MHAL_HDMITX_CD_24_BITS;
    stAttrCfg.enTiming     = timing_str2enum(disp_cfg->panel_cfg.stPanelAttr.m_pPanelName);

    stAttrCfg.bAudioEn    = 1;
    stAttrCfg.enAudioFreq = E_MHAL_HDMITX_AUDIO_FREQ_48K;
    stAttrCfg.enAudioCh   = E_MHAL_HDMITX_AUDIO_CH_2;
    stAttrCfg.enAudioFmt  = E_MHAL_HDMITX_AUDIO_FORMAT_PCM;
    stAttrCfg.enAudioCode = E_MHAL_HDMITX_AUDIO_CODING_PCM;
    BL_EXEC_FUNC(MhalHdmitxSetAttr(disp_cfg->hdmitx_ctx, &stAttrCfg), 0);
    stSignalCfg.bEn = 1;
    BL_EXEC_FUNC(MhalHdmitxSetSignal(disp_cfg->hdmitx_ctx, &stSignalCfg), 0);
    stMuteCfg.enType = E_MHAL_HDMITX_MUTE_AUDIO | E_MHAL_HDMITX_MUTE_VIDEO | E_MHAL_HDMITX_MUTE_AVMUTE;
    stMuteCfg.bMute  = 0;
    BL_EXEC_FUNC(MhalHdmitxSetMute(disp_cfg->hdmitx_ctx, &stMuteCfg), 0);
    BOOTLOGO_DBG(DBG_LEVEL_MHAL, "finish\n");
#endif
    return 0;
}

#if defined(CONFIG_SSTAR_HDMITX)
static MI_S32 hdmi_deinit(display_config *disp_cfg)
{
    MhalHdmitxSignalConfig_t stSignalCfg;
    memset(&stSignalCfg, 0, sizeof(stSignalCfg));
    MhalHdmitxMuteConfig_t stMuteCfg;
    memset(&stMuteCfg, 0, sizeof(stMuteCfg));

    stMuteCfg.enType = E_MHAL_HDMITX_MUTE_AUDIO | E_MHAL_HDMITX_MUTE_VIDEO | E_MHAL_HDMITX_MUTE_AVMUTE;
    stMuteCfg.bMute  = 1;
    BL_EXEC_FUNC(MhalHdmitxSetMute(disp_cfg->hdmitx_ctx, &stMuteCfg), 0);
    stSignalCfg.bEn = 0;
    BL_EXEC_FUNC(MhalHdmitxSetSignal(disp_cfg->hdmitx_ctx, &stSignalCfg), 0);
    stMuteCfg.enType = E_MHAL_HDMITX_MUTE_AUDIO | E_MHAL_HDMITX_MUTE_VIDEO | E_MHAL_HDMITX_MUTE_AVMUTE;
    stMuteCfg.bMute  = 0;
    BL_EXEC_FUNC(MhalHdmitxSetMute(disp_cfg->hdmitx_ctx, &stMuteCfg), 0);
    BL_EXEC_FUNC(MhalHdmitxDestroyInstance(disp_cfg->hdmitx_ctx), 0);
    BOOTLOGO_DBG(DBG_LEVEL_MHAL, "finish\n");
    return 0;
}
#endif

static MI_S32 cfg_data_init(bootlogo_contex *contex)
{
    MI_U32 i = 0;
    if (contex->cfg_data_init_flag)
    {
        BOOTLOGO_DBG(DBG_LEVEL_INFO, "CfgData already Init %d\n", contex->cfg_data_init_flag);
        return 0;
    }

    // u32 u32DispDbgLevel     = 0; // 0x1F;
    // MHAL_DISP_DbgLevel(&u32DispDbgLevel);

    MI_DISP_IMPL_MhalPanelConfig_t astPnlCfg[MAX_DISP_DEV_NUM];
    memset(astPnlCfg, 0, sizeof(astPnlCfg));
    for (i = 0; i < MAX_DISP_DEV_NUM; i++)
    {
        memcpy(&astPnlCfg[i], &contex->disp_cfgs[i].panel_cfg, sizeof(MI_DISP_IMPL_MhalPanelConfig_t));
        BOOTLOGO_DBG(DBG_LEVEL_INFO, "eTiming: %d, bValid: %d\n", astPnlCfg[i].stPnlUniParamCfg.eDisplayTiming,
                     astPnlCfg[i].bValid);
    }
    BL_EXEC_FUNC(DRV_DISP_IF_InitPanelConfig(astPnlCfg, MAX_DISP_DEV_NUM), 1);

    MI_DISP_IMPL_MhalPqConfig_t stMhalPqCfg;
    memset(&stMhalPqCfg, 0, sizeof(stMhalPqCfg));
    if (contex->pq_size > 0)
    {
        stMhalPqCfg.u32PqFlags  = E_MI_DISP_PQ_FLAG_LOAD_BIN;
        stMhalPqCfg.u32DataSize = contex->pq_size;
        stMhalPqCfg.pData       = (void *)contex->pq_data_addr[0];
        BOOTLOGO_DBG(DBG_LEVEL_INFO, "set PQ. pData = %px, size = %d\n", stMhalPqCfg.pData, stMhalPqCfg.u32DataSize);
        BL_EXEC_FUNC(DRV_DISP_IF_DeviceSetPqConfig(NULL, &stMhalPqCfg), 1);
    }

    contex->cfg_data_init_flag = 1;
    return 0;
}

#if (defined CONFIG_SSTAR_JPD) && (defined CONFIG_JPD_SW)
static void rgb_rotate(MI_U8 *pDstBuf, MI_U8 *pSrcBuf, MI_U16 u16Width, MI_U16 u16Height, logo_rotation rotate,
                       MI_U8 u8BytePerPixel)
{
    BOOTLOGO_DBG(DBG_LEVEL_API, ">>>\n");
    dcache_all();
    MI_U16 x = 0, y = 0;
    switch (rotate)
    {
        case LOGO_ROTATE_90:
        {
            if (u8BytePerPixel == 2)
            {
                RGB_PIXEL_MAPPING(pDstBuf, (u16Height - y - 1), x, u16Height * u8BytePerPixel, pSrcBuf, x, y,
                                  u8BytePerPixel * u16Width, u16Width, u16Height, MI_U16);
            }
            else if (u8BytePerPixel == 4)
            {
                RGB_PIXEL_MAPPING(pDstBuf, (u16Height - y - 1), x, u16Height * u8BytePerPixel, pSrcBuf, x, y,
                                  u8BytePerPixel * u16Width, u16Width, u16Height, MI_U32);
            }
            break;
        }
        case LOGO_ROTATE_180:
        {
            if (u8BytePerPixel == 2)
            {
                RGB_PIXEL_MAPPING(pDstBuf, (u16Width - x - 1), (u16Height - y - 1), u8BytePerPixel * u16Width, pSrcBuf,
                                  x, y, u8BytePerPixel * u16Width, u16Width, u16Height, MI_U16);
            }
            else if (u8BytePerPixel == 4)
            {
                RGB_PIXEL_MAPPING(pDstBuf, (u16Width - x - 1), (u16Height - y - 1), u8BytePerPixel * u16Width, pSrcBuf,
                                  x, y, u8BytePerPixel * u16Width, u16Width, u16Height, MI_U32);
            }
            break;
        }
        case LOGO_ROTATE_270:
        {
            if (u8BytePerPixel == 2)
            {
                RGB_PIXEL_MAPPING(pDstBuf, y, (u16Width - x - 1), u16Height * u8BytePerPixel, pSrcBuf, x, y,
                                  u8BytePerPixel * u16Width, u16Width, u16Height, MI_U16);
            }
            else if (u8BytePerPixel == 4)
            {
                RGB_PIXEL_MAPPING(pDstBuf, y, (u16Width - x - 1), u16Height * u8BytePerPixel, pSrcBuf, x, y,
                                  u8BytePerPixel * u16Width, u16Width, u16Height, MI_U32);
            }
            break;
        }
        default:
            return;
    }
    dcache_all();
    BOOTLOGO_DBG(DBG_LEVEL_API, "<<<\n");
}

static void yuv444_to_yuv420(char *inbuf, char *outbuf, MI_U16 *width, MI_U16 *height, logo_rotation rotate)
{
    MI_U16 x = 0, y = 0;
    MI_U8 *dst_y = NULL, *dts_uv = NULL;
    MI_U8 *src_yuv = NULL;

    src_yuv = inbuf;
    dst_y   = outbuf;
    dts_uv  = dst_y + (*width) * (*height);

    BOOTLOGO_DBG(DBG_LEVEL_API, ">>>\n");
    BOOTLOGO_DBG(DBG_LEVEL_JPD, "YUV 444 To 420, In:%p, Out:%p, Width:%d, Height:%d\n", inbuf, outbuf, *width, *height);

    switch (rotate)
    {
        case LOGO_ROTATE_NONE:
        {
            YUV444_TO_YUV420_PIXEL_MAPPING(dst_y, dts_uv, x, y, *width, src_yuv, x, y, *width, *height, rotate);
        }
        break;
        case LOGO_ROTATE_90:
        {
            YUV444_TO_YUV420_PIXEL_MAPPING(dst_y, dts_uv, *height - y, x, *height, src_yuv, x, y, *width, *height,
                                           rotate);
            *width ^= *height;
            *height ^= *width;
            *width ^= *height;
        }
        break;
        case LOGO_ROTATE_180:
        {
            YUV444_TO_YUV420_PIXEL_MAPPING(dst_y, dts_uv, (*width - x), (*height - y - 1), *width, src_yuv, x, y,
                                           *width, *height, rotate);
        }
        break;
        case LOGO_ROTATE_270:
        {
            YUV444_TO_YUV420_PIXEL_MAPPING(dst_y, dts_uv, y, (*width - x - 1), *height, src_yuv, x, y, *width, *height,
                                           rotate);
            *width ^= *height;
            *height ^= *width;
            *width ^= *height;
        }
        break;
        default:
            return;
    }
    BOOTLOGO_DBG(DBG_LEVEL_API, "<<<\n");
}

static MI_U8 *jpd_sw_to_yuv420sp(MI_U32 inbuf_size, MI_U8 *inbuf, MI_U32 *outbuf_size, MI_U32 *out_width,
                                 MI_U32 *out_height, MI_U32 *img_crop_width, MI_U32 *img_crop_height,
                                 MI_U32 *img_crop_x, MI_U32 *img_crop_y, logo_rotation rotate,
                                 disp_buffer_info *disp_buf_info)
{
    MI_U32 yuv420_size = 0;
    // Variables for the source jpg
    MI_U32 jpg_size = 0;
    MI_U8 *jpg_buf  = NULL;

    // Variables for the decompressor itself
    struct jpeg_decompress_struct cinfo;
    memset(&cinfo, 0, sizeof(cinfo));
    struct jpeg_error_mgr jerr;
    memset(&jerr, 0, sizeof(jerr));

    // Variables for the output buffer, and how long each row is
    MI_U32 bmp_size   = 0;
    MI_U8 *bmp_buf    = NULL;
    MI_U8 *yuv420_buf = NULL;
    MI_U16 row_stride = 0, width = 0, height = 0, pixel_size = 0;
    MI_S32 rc       = 0;
    jpg_size        = inbuf_size;
    jpg_buf         = (MI_U8 *)inbuf;
    MI_U32 pre_time = timer_get_us();

    BOOTLOGO_DBG(DBG_LEVEL_API, ">>>\n");
    BOOTLOGO_DBG(DBG_LEVEL_JPD, "Create Decompress struct\n");
    // Allocate a new decompress struct, with the default error handler.
    // The default error handler will exit() on pretty much any issue,
    // so it's likely you'll want to replace it or supplement it with
    // your own.
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);

    BOOTLOGO_DBG(DBG_LEVEL_JPD, "Set memory buffer as source\n");
    // Configure this decompressor to read its data from a memory
    // buffer starting at u8 *pu8JpgBuffer, which is jpg_size
    // long, and which must contain a complete jpg already.
    //
    // If you need something fancier than this, you must write your
    // own data source manager, which shouldn't be too hard if you know
    // what it is you need it to do. See jpeg-8d/jdatasrc.c for the
    // implementation of the standard jpeg_mem_src and jpeg_stdio_src
    // managers as examples to work from.
    jpeg_mem_src(&cinfo, jpg_buf, jpg_size);

    BOOTLOGO_DBG(DBG_LEVEL_JPD, "Read the JPEG header\n");
    // Have the decompressor scan the jpeg header. This won't populate
    // the cinfo struct output fields, but will indicate if the
    // jpeg is valid.
    rc = jpeg_read_header(&cinfo, TRUE);
    if (rc != 1)
    {
        BOOTLOGO_DBG(DBG_LEVEL_ERR, "File does not seem to be a normal JPEG\n");
        return NULL;
    }

    BOOTLOGO_DBG(DBG_LEVEL_JPD, "Initiate JPEG decompression\n");

    // output color space is yuv444 packet
    cinfo.out_color_space = JCS_YCbCr;

    jpeg_start_decompress(&cinfo);
    width      = cinfo.output_width;
    height     = cinfo.output_height;
    pixel_size = cinfo.output_components;
    BOOTLOGO_DBG(DBG_LEVEL_JPD, "Image is %d by %d with %d components\n", width, height, pixel_size);

    switch (rotate)
    {
        case LOGO_ROTATE_NONE:
        {
            *img_crop_width  = BL_ALIGN_DOWN(width, 2);
            *img_crop_height = BL_ALIGN_DOWN(height, 2);
            width            = BL_ALIGN_UP(width, DISP_BASE_ALIGN);
            *img_crop_x      = 0;
            *img_crop_y      = 0;
        }
        break;
        case LOGO_ROTATE_180:
        {
            *img_crop_width  = BL_ALIGN_DOWN(height, 2);
            *img_crop_height = BL_ALIGN_DOWN(height, 2);
            width            = BL_ALIGN_UP(width, DISP_BASE_ALIGN);
            *img_crop_x      = BL_ALIGN_UP(width - *img_crop_width, 2);
            *img_crop_y      = 0;
        }
        break;
        case LOGO_ROTATE_90:
        {
            *img_crop_width  = BL_ALIGN_DOWN(height, 2);
            *img_crop_height = BL_ALIGN_DOWN(width, 2);
            height           = BL_ALIGN_UP(height, DISP_BASE_ALIGN);
            *img_crop_x      = BL_ALIGN_UP(height - *img_crop_width, 2);
            *img_crop_y      = 0;
        }
        break;
        case LOGO_ROTATE_270:
        {
            *img_crop_width  = BL_ALIGN_DOWN(height, 2);
            *img_crop_height = BL_ALIGN_DOWN(width, 2);
            height           = BL_ALIGN_UP(height, DISP_BASE_ALIGN);
            *img_crop_x      = 0;
            *img_crop_y      = 0;
        }
        break;
    }

    yuv420_size = width * height * 3 / 2;
    yuv420_buf  = (MI_U8 *)malloc_from_reserved_buf(disp_buf_info, yuv420_size);
    if (yuv420_buf == NULL)
    {
        BOOTLOGO_DBG(DBG_LEVEL_ERR, "Yuv420 OutBufSize: %d Malloc fail\n", yuv420_size);
        return NULL;
    }
    BOOTLOGO_DBG(DBG_LEVEL_JPD, "yuv420_buf: 0x%p\n", yuv420_buf);

    bmp_size = width * height * pixel_size;
    bmp_buf  = (MI_U8 *)malloc_cache_aligned(bmp_size);
    if (bmp_buf == NULL)
    {
        free_from_reserved_buf(disp_buf_info, yuv420_buf, yuv420_size);
        yuv420_buf = NULL;
        BOOTLOGO_DBG(DBG_LEVEL_ERR, "malloc bmp_buf[%d] fail\n", bmp_size);
        return NULL;
    }
    BOOTLOGO_DBG(DBG_LEVEL_JPD, "BmpBuffer: 0x%p\n", bmp_buf);

    row_stride = width * pixel_size;
    BOOTLOGO_DBG(DBG_LEVEL_JPD, "Start reading scanlines\n");
    while (cinfo.output_scanline < cinfo.output_height)
    {
        MI_U8 *buffer_array[1];
        buffer_array[0] = bmp_buf + (cinfo.output_scanline) * row_stride;
        jpeg_read_scanlines(&cinfo, buffer_array, 1);
    }

    BOOTLOGO_DBG(DBG_LEVEL_JPD, "Done reading scanlines\n");
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);

    BOOTLOGO_DBG(DBG_LEVEL_JPD, "End of decompression\n");
    dcache_all();
    MI_U32 pre_time1 = timer_get_us();
    printf("JPD_SW_decode time: %u us\n", (pre_time1 - pre_time));
    yuv444_to_yuv420(bmp_buf, yuv420_buf, &width, &height, rotate);
    *out_width  = width;
    *out_height = height;
    flush_dcache_range((MI_U32)(yuv420_buf), (MI_U32)(yuv420_buf + width * height * 3 / 2));
    if (bmp_buf)
    {
        free(bmp_buf);
        bmp_buf = NULL;
    }
    (*outbuf_size) = yuv420_size;
    printf("JPD_SW_decode time: %lu us\n", (timer_get_us() - pre_time1));
    BOOTLOGO_DBG(DBG_LEVEL_API, "<<<\n");
    return yuv420_buf;
}

static MI_U8 *jpd_sw_to_argb8888(MI_U32 inbuf_size, MI_U8 *inbuf, MI_U32 *outbuf_size, MI_U32 *out_width,
                                 MI_U32 *out_height, MI_U32 *img_crop_width, MI_U32 *img_crop_height,
                                 MI_U32 *img_crop_x, MI_U32 *img_crop_y, logo_rotation rotate,
                                 disp_buffer_info *disp_buf_info)
{
    // Variables for the source jpg
    MI_U32 jpg_size = 0;
    MI_U8 *jpg_buf  = NULL;

    // Variables for the decompressor itself
    struct jpeg_decompress_struct cinfo;
    memset(&cinfo, 0, sizeof(cinfo));
    struct jpeg_error_mgr jerr;
    memset(&jerr, 0, sizeof(jerr));

    // Variables for the output buffer, and how long each row is
    MI_U8 *linebuffer     = NULL;
    MI_U16 width          = 0;
    MI_U16 height         = 0;
    MI_U16 pixel_size     = 0;
    MI_S32 rc             = 0;
    MI_S32 i              = 0;
    jpg_size              = inbuf_size;
    jpg_buf               = (MI_U8 *)inbuf;
    MI_U8 *outbuf         = NULL;
    MI_U8 *dstbuffer      = NULL;
    MI_U8 *rotate_pre_buf = NULL;

    BOOTLOGO_DBG(DBG_LEVEL_API, ">>>\n");
    BOOTLOGO_DBG(DBG_LEVEL_JPD, "Create Decompress struct\n");
    // Allocate a new decompress struct, with the default error handler.
    // The default error handler will exit() on pretty much any issue,
    // so it's likely you'll want to replace it or supplement it with
    // your own.
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);

    BOOTLOGO_DBG(DBG_LEVEL_JPD, "Set memory buffer as source\n");
    // Configure this decompressor to read its data from a memory
    // buffer starting at u8 *pu8JpgBuffer, which is jpg_size
    // long, and which must contain a complete jpg already.
    //
    // If you need something fancier than this, you must write your
    // own data source manager, which shouldn't be too hard if you know
    // what it is you need it to do. See jpeg-8d/jdatasrc.c for the
    // implementation of the standard jpeg_mem_src and jpeg_stdio_src
    // managers as examples to work from.
    jpeg_mem_src(&cinfo, jpg_buf, jpg_size);

    BOOTLOGO_DBG(DBG_LEVEL_JPD, "Read the JPEG header\n");
    // Have the decompressor scan the jpeg header. This won't populate
    // the cinfo struct output fields, but will indicate if the
    // jpeg is valid.
    rc = jpeg_read_header(&cinfo, TRUE);
    if (rc != 1)
    {
        BOOTLOGO_DBG(DBG_LEVEL_ERR, "File does not seem to be a normal JPEG\n");
        return NULL;
    }

    BOOTLOGO_DBG(DBG_LEVEL_JPD, "Initiate JPEG decompression\n");
    // output color space is rgb888 packet
    cinfo.out_color_space = JCS_RGB;
    BOOTLOGO_DBG(DBG_LEVEL_JPD, "out_color_space: %d\n", cinfo.out_color_space);
    jpeg_start_decompress(&cinfo);
    width      = cinfo.output_width;
    height     = cinfo.output_height;
    pixel_size = cinfo.output_components;

    BOOTLOGO_DBG(DBG_LEVEL_JPD, "Image is %d by %d with %d components\n", width, height, pixel_size);

    switch (rotate)
    {
        case LOGO_ROTATE_NONE:
        {
            *img_crop_width  = BL_ALIGN_DOWN(width, 2);
            *img_crop_height = BL_ALIGN_DOWN(height, 4);
            width            = BL_ALIGN_UP(width, DISP_BASE_ALIGN);
            *img_crop_x      = 0;
            *img_crop_y      = 0;
        }
        break;
        case LOGO_ROTATE_180:
        {
            *img_crop_width  = BL_ALIGN_DOWN(width, 4);
            *img_crop_height = BL_ALIGN_DOWN(height, 4);
            width            = BL_ALIGN_UP(width, DISP_BASE_ALIGN);
            *img_crop_x      = width - *img_crop_width;
            *img_crop_y      = 0;
        }
        break;
        case LOGO_ROTATE_90:
        {
            *img_crop_width  = BL_ALIGN_DOWN(height, 4);
            *img_crop_height = BL_ALIGN_DOWN(width, 4);
            height           = BL_ALIGN_UP(height, DISP_BASE_ALIGN);
            *img_crop_x      = height - *img_crop_width;
            *img_crop_y      = 0;
        }
        break;
        case LOGO_ROTATE_270:
        {
            *img_crop_width  = BL_ALIGN_DOWN(height, 4);
            *img_crop_height = BL_ALIGN_DOWN(width, 4);
            height           = BL_ALIGN_UP(height, DISP_BASE_ALIGN);
            *img_crop_x      = 0;
            *img_crop_y      = 0;
        }
        break;
    }

    MI_S32 alloc_size = width * height * 4;
    outbuf            = (MI_U8 *)malloc_from_reserved_buf(disp_buf_info, alloc_size);
    if (outbuf == NULL)
    {
        BOOTLOGO_DBG(DBG_LEVEL_ERR, "malloc_from_reserved_buf[%d] fail\n", alloc_size);
        return NULL;
    }
    dstbuffer  = outbuf;
    alloc_size = width * pixel_size;
    linebuffer = (MI_U8 *)malloc_cache_aligned(alloc_size);
    if (linebuffer == NULL)
    {
        BOOTLOGO_DBG(DBG_LEVEL_ERR, "malloc[%d] fail\n", alloc_size);
        return NULL;
    }
    BOOTLOGO_DBG(DBG_LEVEL_JPD, "linebuffer: 0x%px, size: %d\n", linebuffer, alloc_size);
    if (rotate != LOGO_ROTATE_NONE)
    {
        alloc_size = width * height * 4;
        dstbuffer  = (MI_U8 *)malloc_cache_aligned(alloc_size);
        if (dstbuffer == NULL)
        {
            free(linebuffer);
            BOOTLOGO_DBG(DBG_LEVEL_ERR, "malloc[%d] fail\n", alloc_size);
            return NULL;
        }
        rotate_pre_buf = dstbuffer;
        BOOTLOGO_DBG(DBG_LEVEL_JPD, "dstbuffer: 0x%px, size: %d\n", dstbuffer, alloc_size);
    }

    BOOTLOGO_DBG(DBG_LEVEL_JPD, "Start reading scanlines\n");
    while (cinfo.output_scanline < cinfo.output_height)
    {
        MI_U8 *buffer_array[1];
        buffer_array[0] = linebuffer;
        MI_U8 *pixel    = linebuffer;
        jpeg_read_scanlines(&cinfo, buffer_array, 1);
        for (i = 0; i < width; i++, pixel += cinfo.output_components)
        {
            *(((MI_S32 *)dstbuffer) + i) = 0xFF << 24 | (*(pixel)) << 16 | (*(pixel + 1)) << 8 | (*(pixel + 2));
        }
        dstbuffer += width * 4;
    }
    BOOTLOGO_DBG(DBG_LEVEL_JPD, "Done reading scanlines\n");
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    BOOTLOGO_DBG(DBG_LEVEL_JPD, "End of decompression\n");
    dcache_all();
    if (rotate != LOGO_ROTATE_NONE)
    {
        rgb_rotate(outbuf, rotate_pre_buf, width, height, rotate, 4);
        if (rotate == LOGO_ROTATE_90 || rotate == LOGO_ROTATE_270) // swap width & height
        {
            width ^= height;
            height ^= width;
            width ^= height;
        }
        free(rotate_pre_buf);
    }

    *out_width  = width;
    *out_height = height;
    free(linebuffer);
    BOOTLOGO_DBG(DBG_LEVEL_API, "<<<\n");
    return outbuf;
}
#endif

#if (defined CONFIG_SSTAR_JPD) && (defined CONFIG_JPD_HW)
static void yuv422_to_yuv420(MI_U8 *src_buf, MI_U8 *dest_buf, MI_U16 src_width, MI_U16 src_height, MI_U16 dest_width,
                             MI_U16 dest_height, logo_rotation rotate)
{
    MI_U8 *src_buf_yuv = src_buf;
    MI_U8 *dest_buf_y  = dest_buf;
    MI_U8 *dest_buf_uv = dest_buf + (dest_width * dest_height);
    MI_U16 src_x = 0, src_y = 0;
    MI_U16 dest_x = 0, dest_y = 0;
    MI_U16 dest_stride = dest_width;

    for (src_y = 0; src_y < src_height; src_y++)
    {
        for (src_x = 0; src_x < src_width; src_x++)
        {
            if (rotate == LOGO_ROTATE_NONE)
            {
                dest_x = src_x;
                dest_y = src_y;
            }
            if (rotate == LOGO_ROTATE_90)
            {
                dest_x = dest_width - src_y - 2; //
                dest_y = src_x;
            }
            else if (rotate == LOGO_ROTATE_180)
            {
                dest_x = dest_width - src_x - 2; //
                dest_y = dest_height - src_y - 1;
            }
            else if (rotate == LOGO_ROTATE_270)
            {
                dest_x = src_y;
                dest_y = dest_height - src_x - 1;
            }
            *(((dest_buf_y) + (dest_y) * (dest_stride) + (dest_x))) =
                *(((src_buf_yuv) + (src_y) * (src_width * 2) + (src_x * 2)));
            if (rotate == LOGO_ROTATE_NONE)
            {
                if ((src_y & 0x01))
                {
                    *(((dest_buf_uv) + ((dest_y - 1) >> 1) * (dest_stride) + (dest_x))) =
                        *(((src_buf_yuv) + (src_y) * (src_width * 2) + (src_x * 2) + 1));
                }
            }
            else
            {
                if ((src_y & 0x01))
                {
                    if (src_x & 0x01)
                    {
                        *(((dest_buf_uv) + ((dest_y) >> 1) * (dest_stride) + (dest_x))) =
                            *(((src_buf_yuv) + (src_y) * (src_width * 2) + (src_x * 2) + 1));
                    }
                }
                else
                {
                    if ((src_x % 2) == 0)
                    {
                        *(((dest_buf_uv) + ((dest_y) >> 1) * (dest_stride) + (dest_x))) =
                            *(((src_buf_yuv) + (src_y) * (src_width * 2) + (src_x * 2) + 1));
                    }
                }
            }
        }
    }
    BOOTLOGO_DBG(DBG_LEVEL_API, "<<<\n");
}

static MI_U8 *jpd_hw(MI_U32 inbuf_size, MI_U8 *inbuf, MI_U32 *outbuf_size, MI_U32 *out_width, MI_U32 *out_height,
                     MI_U32 *img_crop_width, MI_U32 *img_crop_height, MI_U32 *img_crop_x, MI_U32 *img_crop_y,
                     logo_rotation rotate, disp_buffer_info *disp_buf_info, MI_SYS_PixelFormat_e pixel_format)
{
    // int ret = -1;
    MI_U32 disp_dev_id       = 0;
    MI_U16 u16Width          = 0;
    MI_U16 u16Height         = 0;
    MI_U32 yuv420_size       = 0;
    void * pYuv422OutputAddr = NULL;
    char * yuv420_buf        = NULL;

    MHAL_JPD_DEV_HANDLE  pstJpdDevHandle = NULL; ///< might be void* or MS_U32
    MHAL_JPD_INST_HANDLE pstJpdInstCtx   = NULL;

    MHAL_JPD_InputBuf_t  stInputBuf;
    MHAL_JPD_JpgInfo_t   stJpgInfo;
    MHAL_JPD_JpgFrame_t  stJpgFrame;
    MHal_JPD_Addr_t      stJpdAddr;
    MHAL_JPD_MaxDecRes_t stJpdMaxinfo;
    MHAL_JPD_Status_e    eJpdStatusFlag;

    memset(&stInputBuf, 0x00, sizeof(stInputBuf));
    memset(&stJpgInfo, 0x00, sizeof(stJpgInfo));
    memset(&stJpgFrame, 0x00, sizeof(stJpgFrame));
    memset(&stJpdAddr, 0x00, sizeof(stJpdAddr));
    memset(&stJpdMaxinfo, 0x00, sizeof(stJpdMaxinfo));
    memset(&eJpdStatusFlag, 0x00, sizeof(eJpdStatusFlag));

    // InterBuffer allocate when create device

    // InputBuffer
    stJpdAddr.InputAddr     = (void *)inbuf;
    stJpdAddr.InputpVirAddr = stJpdAddr.InputAddr;
    stJpdAddr.InputSize     = inbuf_size;

    if (pixel_format != E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420)
    {
        BOOTLOGO_DBG(DBG_LEVEL_ERR, "not support.pixel_format: %d\n", pixel_format);
        return NULL;
    }

    BOOTLOGO_DBG(DBG_LEVEL_API, ">>>\n");
    // YUV422Buffer
    // We allocate YUV422Buffer when extra the width and height of image.
    if (0 != MHAL_JPD_CreateDevice(disp_dev_id, &pstJpdDevHandle))
    {
        BOOTLOGO_DBG(DBG_LEVEL_ERR, "MHAL_JPD_CreateDevice fail.\n");
        return NULL;
    }
    if (0 != MHAL_JPD_CreateInstance(pstJpdDevHandle, &pstJpdInstCtx))
    {
        BOOTLOGO_DBG(DBG_LEVEL_ERR, "MHAL_JPD_CreateInstance fail.\n");
        MHAL_JPD_DestroyDevice(pstJpdDevHandle);
        return NULL;
    }

    stJpdMaxinfo.u16MaxHeight    = MAX_DEC_HEIGHT;
    stJpdMaxinfo.u16MaxWidth     = MAX_DEC_WIDTH;
    stJpdMaxinfo.u16ProMaxHeight = MAX_DEC_HEIGHT;
    stJpdMaxinfo.u16ProMaxWidth  = MAX_DEC_WIDTH;
    if (0 != MHAL_JPD_SetParam(pstJpdInstCtx, E_JPD_PARAM_MAX_DEC_RES, &stJpdMaxinfo)) // set msg to sysinfo
    {
        BOOTLOGO_DBG(DBG_LEVEL_ERR, "MHAL_JPD_SetParam MAX_DEC_RES fail.\n");
        return NULL;
    }

    // We do not use Addr2 now
    stInputBuf.pVirtBufAddr1  = stJpdAddr.InputpVirAddr;
    stInputBuf.pVirtBufAddr2  = stJpdAddr.InputpVirAddr + stJpdAddr.InputSize;
    stInputBuf.u64PhyBufAddr1 = (MI_U32)stJpdAddr.InputAddr;
    stInputBuf.u64PhyBufAddr2 = (MI_U32)(stJpdAddr.InputAddr + stJpdAddr.InputSize);
    stInputBuf.u32BufSize1    = stJpdAddr.InputSize;
    stInputBuf.u32BufSize2    = 0;

    BOOTLOGO_DBG(DBG_LEVEL_JPD, "Start Extract Jpg Header Info.\n");
    if (0 != MHAL_JPD_ExtractJpgInfo(pstJpdInstCtx, &stInputBuf, &stJpgInfo))
    {
        BOOTLOGO_DBG(DBG_LEVEL_ERR, "MHAL_JPD_Extract Jpg Info fail.\n");
        goto Exit;
    }

    BOOTLOGO_DBG(DBG_LEVEL_JPD, "Get JpgInfo w = %d , h = %d.\n", stJpgInfo.u16AlignedWidth,
                 stJpgInfo.u16AlignedHeight);
    stJpgFrame.stInputBuf.pVirtBufAddr1  = stJpdAddr.InputpVirAddr;
    stJpgFrame.stInputBuf.pVirtBufAddr2  = stJpdAddr.InputpVirAddr + stJpdAddr.InputSize;
    stJpgFrame.stInputBuf.u64PhyBufAddr1 = (MI_U32)stJpdAddr.InputAddr;
    stJpgFrame.stInputBuf.u64PhyBufAddr2 = (MI_U32)(stJpdAddr.InputAddr + stJpdAddr.InputSize);
    stJpgFrame.stInputBuf.u32BufSize1    = stJpdAddr.InputSize;
    stJpgFrame.stInputBuf.u32BufSize2    = 0;

    // Get aligned width and height
    u16Width    = stJpgInfo.u16AlignedWidth;
    u16Height   = stJpgInfo.u16AlignedHeight;
    yuv420_size = u16Width * u16Height * 3 / 2;

    switch (rotate)
    {
        case LOGO_ROTATE_NONE:
        {
            *out_width       = BL_ALIGN_UP(u16Width, DISP_BASE_ALIGN);
            *out_height      = BL_ALIGN_UP(u16Height, 2);
            *img_crop_width  = BL_ALIGN_DOWN(u16Width, 2);
            *img_crop_height = BL_ALIGN_DOWN(u16Height, 2);
            *img_crop_x      = 0;
            *img_crop_y      = 0;
        }
        break;
        case LOGO_ROTATE_90:
        {
            *out_width       = BL_ALIGN_UP(u16Height, DISP_BASE_ALIGN);
            *out_height      = BL_ALIGN_UP(u16Width, 2);
            *img_crop_width  = BL_ALIGN_DOWN(u16Height, 2);
            *img_crop_height = BL_ALIGN_DOWN(u16Width, 2);
            *img_crop_x      = BL_ALIGN_UP(*out_width - *img_crop_width, 2);
            *img_crop_y      = 0;
        }
        break;
        case LOGO_ROTATE_180:
        {
            *out_width       = BL_ALIGN_UP(u16Width, DISP_BASE_ALIGN);
            *out_height      = BL_ALIGN_UP(u16Height, 2);
            *img_crop_width  = BL_ALIGN_DOWN(u16Width, 2);
            *img_crop_height = BL_ALIGN_DOWN(u16Height, 2);
            *img_crop_x      = BL_ALIGN_UP(*out_width - *img_crop_width, 2);
            *img_crop_y      = 0;
        }
        break;
        case LOGO_ROTATE_270:
        {
            *out_width       = BL_ALIGN_UP(u16Height, DISP_BASE_ALIGN);
            *out_height      = BL_ALIGN_UP(u16Width, 2);
            *img_crop_width  = BL_ALIGN_DOWN(u16Height, 2);
            *img_crop_height = BL_ALIGN_DOWN(u16Width, 2);
            *img_crop_x      = 0;
            *img_crop_y      = 0;
        }
        break;
    }
    yuv420_size = (*out_width) * (*out_height) * 3 / 2;
    // YUV422Buffer
    stJpdAddr.OutputSize = stJpgInfo.u16AlignedWidth * stJpgInfo.u16AlignedHeight * 2;

    yuv420_buf = (MI_U8 *)malloc_from_reserved_buf(disp_buf_info, yuv420_size);
    if (yuv420_buf == NULL)
    {
        BOOTLOGO_DBG(DBG_LEVEL_ERR, "Yuv420 OutBufSize: %d Malloc fail\n", yuv420_size);
        goto Exit;
    }

    pYuv422OutputAddr = (MI_U8 *)malloc_cache_aligned(stJpdAddr.OutputSize + ALIGNMENT_NEED);
    if (pYuv422OutputAddr == NULL)
    {
        BOOTLOGO_DBG(DBG_LEVEL_ERR, "pYuv422 OutBufSize: %d Malloc fail\n", (stJpdAddr.OutputSize + ALIGNMENT_NEED));
        free_from_reserved_buf(disp_buf_info, yuv420_buf, yuv420_size);
        yuv420_buf = NULL;
        goto Exit;
    }

    memset(pYuv422OutputAddr, 0x0, stJpdAddr.OutputSize + ALIGNMENT_NEED);
    // Align output buffer, we use 32 bytes alignment
    stJpdAddr.OutputAddr  = (void *)BL_ALIGN_UP((MI_U32)pYuv422OutputAddr, ALIGNMENT_NEED);
    stJpdAddr.OutpVirAddr = stJpdAddr.OutputAddr;

    stJpgFrame.stOutputBuf.pVirtBufAddr[0]  = stJpdAddr.OutpVirAddr;
    stJpgFrame.stOutputBuf.u64PhyBufAddr[0] = (MI_U32)stJpdAddr.OutputAddr;
    stJpgFrame.stOutputBuf.u32BufSize       = stJpdAddr.OutputSize;

    stJpgFrame.stOutputBuf.eScaleDownMode  = E_MHAL_JPD_SCALE_DOWN_ORG;
    stJpgFrame.stOutputBuf.eOutputFmt      = E_MHAL_JPD_OUTPUT_FMT_YUV422;
    stJpgFrame.stOutputBuf.u32BufStride[0] = stJpgInfo.u16AlignedWidth * 2;
    stJpgFrame.stOutputBuf.eOutputMode     = E_MHAL_JPD_OUTPUT_FRAME;
    stJpgFrame.stOutputBuf.u32LineNum      = 0;
    stJpgFrame.stOutputBuf.bProtectEnable  = 0;
    stJpgFrame.stOutputBuf.pCmdQ           = NULL;

    dcache_all();
    BOOTLOGO_DBG(DBG_LEVEL_JPD, "Start Decode One Frame.\n");
    if (0 != MHAL_JPD_StartDecodeOneFrame(pstJpdInstCtx, &stJpgFrame))
    {
        BOOTLOGO_DBG(DBG_LEVEL_ERR, "MHAL_JPD_StartDecodeOneFrame fail.\n");
        free_from_reserved_buf(disp_buf_info, yuv420_buf, yuv420_size);
        yuv420_buf = NULL;
        goto Exit;
    }

    BOOTLOGO_DBG(DBG_LEVEL_JPD, "Input buffer addr         = %p \n", inbuf);
    BOOTLOGO_DBG(DBG_LEVEL_JPD, "inbuf_size              = %d \n", inbuf_size);
    BOOTLOGO_DBG(DBG_LEVEL_JPD, "Yuv420Buffer size in need = %d  \n", yuv420_size);
    BOOTLOGO_DBG(DBG_LEVEL_JPD, "Yuv422 Output buffer addr = 0x%x \n", (MI_U32)stJpdAddr.OutputAddr);
    BOOTLOGO_DBG(DBG_LEVEL_JPD, "Yuv422 OutBufSize         = 0x%x  \n", stJpdAddr.OutputSize);

    if (0 == MHAL_JPD_CheckDecodeStatus(pstJpdInstCtx, &eJpdStatusFlag))
    {
        BOOTLOGO_DBG(DBG_LEVEL_JPD, "JPD Done.\n");
    }
    else
    {
        BOOTLOGO_DBG(DBG_LEVEL_ERR, "JPD Fail\n");
    }

    dcache_all();
    yuv422_to_yuv420(stJpdAddr.OutputAddr, yuv420_buf, u16Width, u16Height, *out_width, *out_height, rotate);
    flush_dcache_range((MI_U32)(yuv420_buf), (MI_U32)(yuv420_buf + u16Width * u16Height * 3 / 2));

Exit:
    MHAL_JPD_DestroyInstance(pstJpdInstCtx);
    MHAL_JPD_DestroyDevice(pstJpdDevHandle);
    if (pYuv422OutputAddr)
    {
        free(pYuv422OutputAddr);
    }
    (*outbuf_size) = yuv420_size;
    BOOTLOGO_DBG(DBG_LEVEL_API, "<<<\n");
    return yuv420_buf;
}
#endif

static MI_S32 jpg_header_tail_detection(disp_image_info *image_info, char *file_path)
{
    MI_U32 jpg_header_0 = 0; // jpg file header vaule
    MI_U32 jpg_header_1 = 0; // jpg file header + 1 vaule
    MI_U32 jpg_tail_0   = 0; // jpg file tail - 1 vaule
    MI_U32 jpg_tail_1   = 0; // jpg file tail vaule

    image_info->file_size = read_logo_file(file_path, (char **)&image_info->file_data[0]);
    if (image_info->file_size == -1)
    {
        BOOTLOGO_DBG(DBG_LEVEL_ERR, "Read %s failed!\n", image_info->file_path);
        return -1;
    }

    jpg_header_0 = *((char *)(image_info->file_data[0]));
    jpg_header_1 = *((char *)(image_info->file_data[0]) + 1);
    jpg_tail_0   = *((char *)(image_info->file_data[0]) + image_info->file_size - 2);
    jpg_tail_1   = *((char *)(image_info->file_data[0]) + image_info->file_size - 1);

    if (jpg_header_0 == 0xFF && jpg_header_1 == 0xD8 && jpg_tail_0 == 0xFF && jpg_tail_1 == 0xD9)
    {
        return 0;
    }
    else
    {
        BOOTLOGO_DBG(DBG_LEVEL_ERR, "Read jpeg header: %x %x, tail: %x %x\n", jpg_header_0, jpg_header_1, jpg_tail_0,
                     jpg_tail_1);
        return -1;
    }
}

static MI_U8 *jpg_decode(MI_U32 inbuf_size, MI_U8 *inbuf, MI_U32 *outbuf_size, MI_U32 *out_width, MI_U32 *out_height,
                         MI_U32 *img_crop_width, MI_U32 *img_crop_height, MI_U32 *img_crop_x, MI_U32 *img_crop_y,
                         logo_rotation rotate, disp_buffer_info *disp_buf_info, MI_SYS_PixelFormat_e pixel_format)
{
    MI_U8 *out_buffer = NULL;
#if (defined CONFIG_SSTAR_JPD) && (defined CONFIG_JPD_SW)
    if (pixel_format == E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420)
    {
        out_buffer = jpd_sw_to_yuv420sp(inbuf_size, inbuf, outbuf_size, out_width, out_height, img_crop_width,
                                        img_crop_height, img_crop_x, img_crop_y, rotate, disp_buf_info);
    }
    else if (pixel_format == E_MI_SYS_PIXEL_FRAME_ARGB8888)
    {
        out_buffer = jpd_sw_to_argb8888(inbuf_size, inbuf, outbuf_size, out_width, out_height, img_crop_width,
                                        img_crop_height, img_crop_x, img_crop_y, rotate, disp_buf_info);
    }
#endif
#if (defined CONFIG_SSTAR_JPD) && (defined CONFIG_JPD_HW)
    out_buffer = jpd_hw(inbuf_size, inbuf, outbuf_size, out_width, out_height, img_crop_width, img_crop_height,
                        img_crop_x, img_crop_y, rotate, disp_buf_info, pixel_format);
#endif
    return out_buffer;
}

static MI_U32 get_frame_rate(display_config *disp_cfg, MI_DISP_IMPL_MhalPnlUnifiedTgnTimingConfig_t *tgn_timing_info)
{
#if defined(CONFIG_SSTAR_DISP)
    MI_U32 frame_rate      = 0;
    MI_U32 frame_pixel_num = (tgn_timing_info->u16HTotal * tgn_timing_info->u16VTotal);

    if (disp_cfg->disp_interface == HAL_DISP_INTF_BT656 || disp_cfg->disp_interface == HAL_DISP_INTF_BT601)
    {
        frame_rate = tgn_timing_info->u32Dclk / frame_pixel_num / 2;
    }
    else if (disp_cfg->disp_interface == HAL_DISP_INTF_MIPIDSI || disp_cfg->disp_interface == HAL_DISP_INTF_MIPIDSI_1)
    {
        frame_rate = disp_cfg->fps;
    }
    else
    {
        frame_rate = tgn_timing_info->u32Dclk / frame_pixel_num;
    }
#endif
    return frame_rate;
}

static MI_S32 get_sync_info(display_config *disp_cfg, MI_DISP_SyncInfo_t *pstSyncInfo)
{
#if defined(CONFIG_SSTAR_DISP)
    MI_DISP_IMPL_MhalPnlUnifiedParamConfig_t *    pnl_param_cfg   = &disp_cfg->mhal_pnl_unified_param_cfg;
    MI_DISP_IMPL_MhalPnlUnifiedTgnTimingConfig_t *tgn_timing_info = &pnl_param_cfg->stTgnTimingInfo;

    pstSyncInfo->u16VStart = tgn_timing_info->u16VStart;
    pstSyncInfo->u16Vact   = tgn_timing_info->u16VActive;
    pstSyncInfo->u16Vbb    = tgn_timing_info->u16VSyncBackPorch;
    pstSyncInfo->u16Vfb =
        tgn_timing_info->u16VTotal
        - (tgn_timing_info->u16VSyncWidth + tgn_timing_info->u16VActive + tgn_timing_info->u16VSyncBackPorch);

    pstSyncInfo->u16HStart = tgn_timing_info->u16HStart;
    pstSyncInfo->u16Hact   = tgn_timing_info->u16HActive;
    pstSyncInfo->u16Hbb    = tgn_timing_info->u16HSyncBackPorch;
    pstSyncInfo->u16Hfb =
        tgn_timing_info->u16HTotal
        - (tgn_timing_info->u16HSyncWidth + tgn_timing_info->u16HActive + tgn_timing_info->u16HSyncBackPorch);

    pstSyncInfo->u16Hpw       = tgn_timing_info->u16HSyncWidth;
    pstSyncInfo->u16Vpw       = tgn_timing_info->u16VSyncWidth;
    pstSyncInfo->u32FrameRate = get_frame_rate(disp_cfg, tgn_timing_info);
#endif
    return 0;
}

static MI_S32 get_timing_size(display_config *disp_cfg, MI_S32 *width, MI_S32 *height)
{
    MI_S32 timing_width  = disp_cfg->panel_cfg.stPnlUniParamCfg.stTgnTimingInfo.u16HActive;
    MI_S32 timing_height = disp_cfg->panel_cfg.stPnlUniParamCfg.stTgnTimingInfo.u16VActive;

#if defined(CONFIG_SSTAR_DISP)
    if (disp_cfg->dev_ctx)
    {
        MI_DISP_IMPL_MhalPnlUnifiedParamConfig_t *    pnl_param_cfg   = &disp_cfg->mhal_pnl_unified_param_cfg;
        MI_DISP_IMPL_MhalPnlUnifiedTgnTimingConfig_t *tgn_timing_info = &pnl_param_cfg->stTgnTimingInfo;
        timing_width                                                  = tgn_timing_info->u16HActive;
        timing_height                                                 = tgn_timing_info->u16VActive;
    }
#endif
    *width  = timing_width;
    *height = timing_height;
    return 0;
}

// color: RGB888
#ifdef CONFIG_SSTAR_RGN
static MI_S32 ui_line_draw(MI_U32 x1, MI_U32 y1, MI_U32 x2, MI_U32 y2, MI_U32 thick, MI_U32 color, MI_U8 *pBuffer,
                           MI_U32 canvas_stride, MI_U32 canvas_height, MI_U32 bpp)
{
    if (x1 < 0 || x1 >= canvas_stride || x2 < 0 || x2 >= canvas_stride || y1 < 0 || y1 >= canvas_height || y2 < 0
        || y2 >= canvas_height)
    {
        BOOTLOGO_DBG(DBG_LEVEL_ERR,
                     "invalid param.x1y1(%d %d) x2y2(%d %d) thick: %d, color: %d, nStride: %d, height: %d\n", x1, y1,
                     x2, y2, thick, color, canvas_stride, canvas_height);
        return -1;
    }
    MI_S32 x = 0, y = 0, m = 0;
    MI_S32 min_x       = x1 < x2 ? x1 : x2;
    MI_S32 max_x       = x1 < x2 ? x2 : x1;
    MI_S32 min_y       = y1 < y2 ? y1 : y2;
    MI_S32 max_y       = y1 < y2 ? y2 : y1;
    MI_U8  b           = color & 0xff;         // b
    MI_U8  g           = (color >> 8) & 0xff;  // g
    MI_U8  r           = (color >> 16) & 0xff; // r
    MI_U32 pixel_color = (0xff << 24) | (r << 16) | (g << 8) | b;
    BOOTLOGO_DBG(DBG_LEVEL_INFO, "color: %#x, pixel_color: %#x, thick(%d), rgb(%d %d %d)\n", color, pixel_color, thick,
                 r, g, b);
    BOOTLOGO_DBG(DBG_LEVEL_INFO, "x1y1(%d %d) x2y2(%d %d) nStride: %d, height: %d, bpp: %d\n", x1, y1, x2, y2,
                 canvas_height, canvas_height, bpp);

    dcache_all();
    if (y1 == y2)
    {
        for (x = min_x; x < max_x; x++)
        {
            for (y = min_y; y < (max_y + thick); y++)
            {
                if (y < canvas_height)
                {
                    MI_U8 *q = pBuffer + (canvas_stride * y + x) * bpp;
                    MI_U8 *c = (MI_U8 *)&pixel_color;
                    for (m = 0; m < bpp; m++)
                    {
                        *q++ = *c++;
                    }
                }
            }
        }
    }
    if (x1 == x2)
    {
        for (y = min_y; y < max_y; y++)
        {
            for (x = min_x; x < (min_x + thick); x++)
            {
                if (x < canvas_stride)
                {
                    MI_U8 *q = pBuffer + (canvas_stride * y + x) * bpp;
                    MI_U8 *c = (MI_U8 *)&pixel_color;
                    for (m = 0; m < bpp; m++)
                    {
                        *q++ = *c++;
                    }
                }
            }
        }
    }
    dcache_all();

    return 0;
}

static MI_S32 ui_font_draw(MI_U8 *canvas_buf, MI_U32 x, MI_U32 y, MI_U32 stride, MI_U32 height, MI_U32 color,
                           MI_U32 bg_color, const MI_U8 *text, logo_rotation rot, ui_font_info *font_info, MI_U32 bpp)
{
    MI_S32       font_w = font_info->width;
    MI_S32       font_h = font_info->height;
    const MI_U8 *font   = font_info->font;
    char *       p = NULL, *q = NULL;
    MI_U8        word        = 0;
    MI_S32       line_offset = 0;
    MI_S32       i = 0, j = 0, k = 0, l = 0;
    MI_S32       len                    = strlen(text);
    MI_U8        fb                     = color & 0xff;         // b
    MI_U8        fg                     = (color >> 8) & 0xff;  // g
    MI_U8        fr                     = (color >> 16) & 0xff; // r
    MI_U32       pixel_color_foreground = (0xff << 24) | (fr << 16) | (fg << 8) | fb;
    MI_U8        bb                     = bg_color & 0xff;         // b
    MI_U8        bg                     = (bg_color >> 8) & 0xff;  // g
    MI_U8        br                     = (bg_color >> 16) & 0xff; // r
    MI_U32       pixel_color_background = (0xff << 24) | (br << 16) | (bg << 8) | bb;
    BOOTLOGO_DBG(DBG_LEVEL_INFO, "color: %#x, pixel_color_foreground: %#x, frgb(%d %d %d)\n", color,
                 pixel_color_foreground, fr, fg, fb);
    BOOTLOGO_DBG(DBG_LEVEL_INFO, "bg_color: %#x, pixel_color_background: %#x, brgb(%d %d %d)\n", bg_color,
                 pixel_color_background, br, bg, bb);
    BOOTLOGO_DBG(DBG_LEVEL_INFO, "xy(%d %d) stride: %d, height: %d, rot: %d, bpp: %d\n", x, y, stride, height, rot,
                 bpp);

    dcache_all();
    p = canvas_buf + x * bpp + y * stride;
    for (i = 0; i < len; i++)
    {
        for (j = 0; j < font_h * font_w / 8; j++)
        {
            word = font[(MI_U8)text[i] * (font_w * font_h / 8) + j]; // get a byte info.
            switch (rot)
            {
                case LOGO_ROTATE_NONE:
                    line_offset = font_w * i * bpp + ((j * 8) % font_w) * bpp + (j * 8) / font_w * stride;
                    break;
                case LOGO_ROTATE_270:
                    line_offset = -font_w * i * stride - ((j * 8) % font_w) * stride + (j * 8) / font_w * bpp;
                    break;
                case LOGO_ROTATE_180:
                    line_offset = -font_w * i * bpp - ((j * 8) % font_w) * bpp - (j * 8) / font_w * stride;
                    break;
                case LOGO_ROTATE_90:
                    line_offset = font_w * i * stride + ((j * 8) % font_w) * stride - (j * 8) / font_w * bpp;
                    break;
                default:
                    return -1;
            }
            for (k = 0; k < 8; k++)
            {
                switch (rot)
                {
                    case LOGO_ROTATE_NONE:
                        q = p + line_offset + k * bpp;
                        break;
                    case LOGO_ROTATE_270:
                        q = p + line_offset - k * stride;
                        break;
                    case LOGO_ROTATE_180:
                        q = p + line_offset - k * bpp;
                        break;
                    case LOGO_ROTATE_90:
                        q = p + line_offset + k * stride;
                        break;
                    default:
                        return -1;
                }
                // Draw point
                if (word & (1 << k))
                {
                    char *c = (char *)&pixel_color_foreground;
                    for (l = 0; l < bpp; l++)
                    {
                        *q++ = *c++;
                    }
                }
                else
                {
                    char *c = (char *)&pixel_color_background;
                    for (l = 0; l < bpp; l++)
                    {
                        *q++ = *c++;
                    }
                }
            }
        }
    }
    dcache_all();

    return 0;
}

static MI_S32 ui_font_init(ui_font_info *font_info)
{
    memset(font_info, 0x0, sizeof(ui_font_info));
    font_info->width  = 32;
    font_info->height = 32;
    font_info->font   = g_font_ascii_32x32;
    return 0;
}

static MI_S32 get_bar_info(ui_font_info *font_info, logo_rotation rot, MI_U32 canvas_stride, MI_U32 canvas_height,
                           ui_bar_info *bar_info)
{
    MI_U16 bar_len_percent   = 70; // 70%
    MI_U16 bar_thick_percent = 6;  // 6%
    MI_U16 bar_start_y       = 60; // 80%

    memset(bar_info, 0x0, sizeof(ui_bar_info));
    bar_info->percent_info_length = BL_ALIGN_DOWN(4 * font_info->width, 2); // max is 4 word (:100%)
    bar_info->bg_color            = 0xffffff;                               // 0xAEED; // 0x808080; // bit[23, 0] rgb888

    if (rot == LOGO_ROTATE_NONE)
    {
        bar_info->thick          = BL_ALIGN_DOWN(canvas_height * bar_thick_percent / 100, 2);
        bar_info->length         = BL_ALIGN_DOWN(canvas_stride * bar_len_percent / 100, 2);
        bar_info->x1             = BL_ALIGN_DOWN((canvas_stride - bar_info->length) / 2, 2);
        bar_info->y1             = BL_ALIGN_DOWN(canvas_height * bar_start_y / 100, 2);
        bar_info->x2             = bar_info->x1 + bar_info->length;
        bar_info->y2             = bar_info->y1;
        bar_info->percent_info_x = BL_ALIGN_DOWN((canvas_stride - bar_info->length) / 2, 2) + bar_info->length;
        bar_info->percent_info_y = BL_ALIGN_DOWN(bar_info->y1 + 4, 2);
    }
    else if (rot == LOGO_ROTATE_90)
    {
        bar_info->thick          = BL_ALIGN_DOWN(canvas_stride * bar_thick_percent / 100, 2);
        bar_info->length         = BL_ALIGN_DOWN(canvas_height * bar_len_percent / 100, 2);
        bar_info->y1             = BL_ALIGN_DOWN((canvas_height - bar_info->length) / 2, 2);
        bar_info->x1             = BL_ALIGN_DOWN((canvas_stride * (100 - bar_start_y) / 100), 2) - bar_info->thick;
        bar_info->y2             = bar_info->y1 + bar_info->length;
        bar_info->x2             = bar_info->x1;
        bar_info->percent_info_y = BL_ALIGN_DOWN((canvas_height - bar_info->percent_info_length) / 2, 2);
        bar_info->percent_info_x = BL_ALIGN_DOWN(bar_info->x1 + bar_info->thick + font_info->height + 8, 2);
    }
    else if (rot == LOGO_ROTATE_180)
    {
        bar_info->thick  = BL_ALIGN_DOWN(canvas_height * bar_thick_percent / 100, 2);
        bar_info->length = BL_ALIGN_DOWN(canvas_stride * bar_len_percent / 100, 2);
        bar_info->x2     = BL_ALIGN_DOWN((canvas_stride - bar_info->length) / 2, 2);
        bar_info->y2     = BL_ALIGN_DOWN(canvas_height * (100 - bar_start_y) / 100, 2) - bar_info->thick;
        bar_info->x1     = bar_info->x2 + bar_info->length;
        bar_info->y1     = bar_info->y2;
        bar_info->percent_info_x =
            BL_ALIGN_DOWN((canvas_stride - bar_info->percent_info_length) / 2, 2) + bar_info->percent_info_length;
        bar_info->percent_info_y = BL_ALIGN_DOWN(bar_info->y1 + bar_info->thick + font_info->height + 8, 2);
    }
    else if (rot == LOGO_ROTATE_270)
    {
        bar_info->thick  = BL_ALIGN_DOWN(canvas_stride * bar_thick_percent / 100, 2);
        bar_info->length = BL_ALIGN_DOWN(canvas_height * bar_len_percent / 100, 2);
        bar_info->y2     = BL_ALIGN_DOWN((canvas_height - bar_info->length) / 2, 2);
        bar_info->x2     = BL_ALIGN_DOWN((canvas_stride * bar_start_y / 100), 2);
        bar_info->y1     = bar_info->y2 + bar_info->length;
        bar_info->x1     = bar_info->x2;
        bar_info->percent_info_y =
            BL_ALIGN_DOWN((canvas_height - bar_info->percent_info_length) / 2, 2) + bar_info->percent_info_length;
        bar_info->percent_info_x = BL_ALIGN_DOWN(bar_info->x2 - font_info->height - 8, 2);
    }

    BOOTLOGO_DBG(DBG_LEVEL_INFO, "font width: %d, height: %d\n", font_info->width, font_info->height);
    BOOTLOGO_DBG(DBG_LEVEL_INFO, "bar length: %d, start(%d,%d) end(%d, %d) thick: %d, bg_color: %#x\n",
                 bar_info->length, bar_info->x1, bar_info->y1, bar_info->x2, bar_info->y2, bar_info->thick,
                 bar_info->bg_color);
    BOOTLOGO_DBG(DBG_LEVEL_INFO, "percent_info_length: %d, start(%d,%d)\n", bar_info->length, bar_info->percent_info_x,
                 bar_info->percent_info_y);

    return 0;
}

static MI_S32 show_ui_bar(bootlogo_contex *contex, MI_U32 percent, MI_U32 bar_color, MI_U32 text_color, MI_U32 bg_color,
                          MI_U32 disp_dev_id)
{
    display_config * disp_cfg   = NULL;
    disp_image_info *image_info = NULL;
    ui_bar_info      bar_info;
    ui_font_info     font_info;
    MI_U16           length = 0;
    MI_S32           x2     = 0;
    MI_S32           y2     = 0;
    char             szText[MAX_STR_LEN];

    if (percent > 100)
    {
        BOOTLOGO_DBG(DBG_LEVEL_ERR, "percent %d is invalid\n", percent);
        return -1;
    }
    disp_cfg   = &contex->disp_cfgs[disp_dev_id];
    image_info = disp_cfg->image_info;
    if (disp_cfg == NULL || image_info == NULL)
    {
        BOOTLOGO_DBG(DBG_LEVEL_ERR, "disp_cfg/image_info is NULL\n");
        return -1;
    }

    BOOTLOGO_DBG(DBG_LEVEL_INFO, "bar_color(%#x) text_color(%#x)\n", bar_color, text_color);

    ui_font_init(&font_info);
    get_bar_info(&font_info, disp_cfg->rotate, image_info->img_stride, image_info->img_height, &bar_info);
    length = BL_ALIGN_UP(bar_info.length * percent / 100, 2);
    if (disp_cfg->rotate == LOGO_ROTATE_NONE)
    {
        x2 = bar_info.x1 + length;
        y2 = bar_info.y1;
    }
    else if (disp_cfg->rotate == LOGO_ROTATE_90)
    {
        y2 = bar_info.y1 + length;
        x2 = bar_info.x1;
    }
    else if (disp_cfg->rotate == LOGO_ROTATE_180)
    {
        x2 = bar_info.x1 - length;
        y2 = bar_info.y1;
    }
    else if (disp_cfg->rotate == LOGO_ROTATE_270)
    {
        y2 = bar_info.y1 - length;
        x2 = bar_info.x1;
    }
    ui_line_draw(bar_info.x1, bar_info.y1, bar_info.x2, bar_info.y2, bar_info.thick, bar_info.bg_color,
                 (MI_U8 *)image_info->decode_data[0], image_info->img_stride, image_info->img_height, 4);
    ui_line_draw(bar_info.x1, bar_info.y1, x2, y2, bar_info.thick, bar_color, (MI_U8 *)image_info->decode_data[0],
                 image_info->img_stride, image_info->img_height, 4);
    const char *fmt = (percent <= 9) ? "  %d%%" : (percent <= 99) ? " %d%%" : "%d%%";
    sprintf(szText, fmt, percent);
    ui_font_draw((MI_U8 *)image_info->decode_data[0], bar_info.percent_info_x, bar_info.percent_info_y,
                 image_info->img_stride * 4, image_info->img_height, text_color, bg_color, szText, disp_cfg->rotate,
                 &font_info, 4);

    return 0;
}

static MI_S32 get_msg_info(MI_U32 src_x, MI_U32 src_y, ui_font_info *font_info, logo_rotation rot, MI_U32 canvas_width,
                           MI_U32 canvas_height, MI_U32 *dst_x, MI_U32 *dst_y, MI_U32 len)
{
    BOOTLOGO_DBG(DBG_LEVEL_INFO, "img_width: %d ; img_height: %d\n", canvas_width, canvas_height);
    switch (rot)
    {
        case LOGO_ROTATE_NONE:
            if ((src_x + font_info->width * len) > canvas_width || (src_y + font_info->height) > canvas_height)
            {
                BOOTLOGO_DBG(DBG_LEVEL_ERR, "X out of range [0 , %d] ; Y out of range [0 , %d]\n",
                             canvas_width - font_info->width * len, canvas_height - font_info->height);
                return -1;
            }
            *dst_x = src_x;
            *dst_y = src_y;
            break;
        case LOGO_ROTATE_270:
            if ((src_x + font_info->width * len) > canvas_height || (src_y + font_info->height) > canvas_width)
            {
                BOOTLOGO_DBG(DBG_LEVEL_ERR, "X out of range [0 , %d] ; Y out of range [0 , %d]\n",
                             canvas_height - font_info->width * len, canvas_width - font_info->height);
                BOOTLOGO_DBG(DBG_LEVEL_ERR, "xy(%d, %d)\n", src_x + font_info->width * len, src_y + font_info->height);
                return -1;
            }
            *dst_x = src_y;
            *dst_y = canvas_height - src_x;
            break;
        case LOGO_ROTATE_180:
            if ((src_x + font_info->width * len) > canvas_width || (src_y + font_info->height) > canvas_height)
            {
                BOOTLOGO_DBG(DBG_LEVEL_ERR, "X out of range [0 , %d] ; Y out of range [0 , %d]\n",
                             canvas_width - font_info->width * len, canvas_height - font_info->height);
                BOOTLOGO_DBG(DBG_LEVEL_ERR, "xy(%d, %d)\n", src_x + font_info->width * len, src_y + font_info->height);
                return -1;
            }

            *dst_x = canvas_width - src_x;
            *dst_y = canvas_height - src_y;
            break;
        case LOGO_ROTATE_90:
            if ((src_x + font_info->width * len) > canvas_height || (src_y + font_info->height) > canvas_width)
            {
                BOOTLOGO_DBG(DBG_LEVEL_ERR, "X out of range [0 , %d] ; Y out of range [0 , %d]\n",
                             canvas_height - font_info->width * len, canvas_width - font_info->height);
                return -1;
            }
            *dst_x = canvas_width - src_y;
            *dst_y = src_x;
            break;
        default:
            return -1;
    }
    return 0;
}

static MI_S32 show_ui_msg(bootlogo_contex *contex, const MI_U8 *text, MI_U32 x, MI_U32 y, MI_U32 text_color,
                          MI_U32 bg_color, MI_U32 disp_dev_id)
{
    MI_U32           dst_x      = 0;
    MI_U32           dst_y      = 0;
    display_config * disp_cfg   = NULL;
    disp_image_info *image_info = NULL;
    ui_font_info     font_info;

    if (text == NULL)
    {
        BOOTLOGO_DBG(DBG_LEVEL_WRN, "txet is NULL!\n");
        return 0;
    }
    disp_cfg   = &contex->disp_cfgs[disp_dev_id];
    image_info = disp_cfg->image_info;
    if (disp_cfg == NULL || image_info == NULL)
    {
        BOOTLOGO_DBG(DBG_LEVEL_ERR, "disp_cfg/image_info is NULL\n");
        return -1;
    }

    BOOTLOGO_DBG(DBG_LEVEL_INFO, "xy(%d %d)\n", x, y);
    BOOTLOGO_DBG(DBG_LEVEL_INFO, "text_color(%#x) bg_color(%#x)\n", text_color, bg_color);

    ui_font_init(&font_info);
    if (get_msg_info(x, y, &font_info, disp_cfg->rotate, disp_cfg->image_info->img_width,
                     disp_cfg->image_info->img_height, &dst_x, &dst_y, strlen(text))
        != 0)
    {
        BOOTLOGO_DBG(DBG_LEVEL_ERR, "get_msg_info error!\n");
        return -1;
    }

    ui_font_draw((char *)image_info->decode_data[0], dst_x, dst_y, image_info->img_stride * 4, image_info->img_height,
                 text_color, bg_color, text, disp_cfg->rotate, &font_info, 4);

    return 0;
}

static MI_S32 get_blank_info(MI_U32 x, MI_U32 y, MI_U32 width, MI_U32 height, MI_U32 canvas_stride,
                             MI_U32 canvas_height, logo_rotation rot, ui_blank_info *blank_info)
{
    memset(blank_info, 0, sizeof(ui_blank_info));
    switch (rot)
    {
        case LOGO_ROTATE_NONE:
            if ((x + width) > canvas_stride || (y + height) > canvas_height)
            {
                return -1;
            }
            blank_info->x1 = x;
            blank_info->y1 = y;
            blank_info->x2 = x + width;
            blank_info->y2 = y;
            break;
        case LOGO_ROTATE_270:
            if ((x + width) > canvas_height || (y + height) > canvas_stride)
            {
                return -1;
            }
            blank_info->x1 = y;
            blank_info->y1 = canvas_height - x;
            blank_info->x2 = blank_info->x1;
            blank_info->y2 = blank_info->y1 - width;
            break;
        case LOGO_ROTATE_180:
            if ((x + width) > canvas_stride || (y + height) > canvas_height)
            {
                return -1;
            }

            blank_info->x1 = canvas_stride - x;
            blank_info->y1 = canvas_height - y - height;
            blank_info->x2 = blank_info->x1 - width;
            blank_info->y2 = blank_info->y1;
            break;
        case LOGO_ROTATE_90:
            if ((x + width) > canvas_height || (y + height) > canvas_stride)
            {
                return -1;
            }
            blank_info->x1 = canvas_stride - y - height;
            blank_info->y1 = x;
            blank_info->x2 = blank_info->x1;
            blank_info->y2 = blank_info->y1 + width;
            break;
        default:
            return -1;
    }
    blank_info->width  = width;
    blank_info->height = height;
    BOOTLOGO_DBG(DBG_LEVEL_INFO, "x1y1(%d %d) x2y2(%d %d) wh(%d %d)\n", blank_info->x1, blank_info->y1, blank_info->x2,
                 blank_info->y2, blank_info->width, blank_info->height);
    return 0;
}

static MI_S32 show_ui_blank(bootlogo_contex *contex, MI_U32 x, MI_U32 y, MI_U32 w, MI_U32 h, MI_U32 blank_color,
                            MI_U32 disp_dev_id)
{
    ui_blank_info    blank_info;
    logo_rotation    rot;
    MI_U32           canvas_stride = 0;
    MI_U32           canvas_height = 0;
    disp_image_info *image_info    = NULL;
    display_config * disp_cfg      = NULL;

    disp_cfg   = &contex->disp_cfgs[disp_dev_id];
    image_info = disp_cfg->image_info;
    if (disp_cfg == NULL || image_info == NULL)
    {
        BOOTLOGO_DBG(DBG_LEVEL_ERR, "disp_cfg/image_info is NULL\n");
        return -1;
    }

    rot           = disp_cfg->rotate;
    canvas_stride = image_info->img_stride - 1;
    canvas_height = image_info->img_height - 1;
    if (get_blank_info(x, y, w, h, canvas_stride, canvas_height, rot, &blank_info) != 0)
    {
        BOOTLOGO_DBG(DBG_LEVEL_ERR, "get_blank_info error !\n");
        return -1;
    }
    ui_line_draw(blank_info.x1, blank_info.y1, blank_info.x2, blank_info.y2, blank_info.height, blank_color,
                 (MI_U8 *)image_info->decode_data[0], image_info->img_stride, image_info->img_height, 4);
    return 0;
}

static MI_S32 show_ui_update(bootlogo_contex *contex, int argc, char *const argv[])
{
    display_config * disp_cfg = NULL;
    ui_bar_info      bar_info;
    ui_font_info     font_info;
    disp_image_info *image_info = NULL;
    char * msg_list[] = {"UPGRADE SUCCESS!!!", "UPGRADE FAIL!!!", "UPGRADING SOFTWARE", "PLEASE DO NOT TURN OFF"};
    MI_U32 msg_x = 0, msg_y = 0;
    MI_U32 msg_list_start = 0, msg_list_end = 0, i = 0, count = 1;
    MI_U32 position_ratio = 0;
    MI_U32 disp_dev_id    = 0;
    MI_U32 screen_width = 0, screen_height = 0;
    MI_U32 text_color = 0;
    MI_U32 percent    = 0;

    disp_dev_id = simple_strtoul(argv[1], NULL, 0);
    disp_cfg    = &contex->disp_cfgs[disp_dev_id];
    image_info  = disp_cfg->image_info;
    if (disp_cfg == NULL || image_info == NULL)
    {
        BOOTLOGO_DBG(DBG_LEVEL_ERR, "disp_cfg/image_info is NULL\n");
        return -1;
    }

    ui_font_init(&font_info);
    get_bar_info(&font_info, disp_cfg->rotate, image_info->img_stride, image_info->img_height, &bar_info);
    if ((disp_cfg->rotate == LOGO_ROTATE_90) || (disp_cfg->rotate == LOGO_ROTATE_270))
    {
        screen_width  = image_info->img_height - 1;
        screen_height = image_info->img_width - 1;
    }
    else
    {
        screen_width  = image_info->img_width - 1;
        screen_height = image_info->img_height - 1;
    }

    if (!strcasecmp("success", argv[0]))
    {
        text_color                = 0xbd00;
        msg_list_start            = 0;
        msg_list_end              = 1;
        position_ratio            = 5;
        disp_cfg->show_blank_flag = true;
    }
    else if (!strcasecmp("fail", argv[0]))
    {
        text_color                = 0xff0000;
        msg_list_start            = 1;
        msg_list_end              = 2;
        position_ratio            = 5;
        disp_cfg->show_blank_flag = true;
    }
    else
    {
        percent        = simple_strtoul(argv[0], NULL, 0);
        text_color     = 0xffffff;
        msg_list_start = 2;
        msg_list_end   = 4;
        position_ratio = 2;
    }
    if (disp_cfg->show_blank_flag == true)
    {
        if (-1 == show_ui_blank(contex, 0, 0, screen_width, screen_height, 0x9dceff, disp_dev_id))
        {
            return -1;
        }
    }
    if (strcasecmp("success", argv[0]) && strcasecmp("fail", argv[0]))
    {
        if (-1 == show_ui_bar(contex, percent, 0xff00, text_color, 0x9dceff, disp_dev_id))
        {
            return -1;
        }
        disp_cfg->show_blank_flag = false;
    }

    for (i = msg_list_start; i < msg_list_end; i++)
    {
        if (disp_cfg->rotate == LOGO_ROTATE_NONE || disp_cfg->rotate == LOGO_ROTATE_180)
        {
            msg_x = (image_info->img_width - (strlen(msg_list[i]) * font_info.width)) / 2;
            msg_y = image_info->img_height * position_ratio * 0.10 * count;
        }
        else if (disp_cfg->rotate == LOGO_ROTATE_90 || disp_cfg->rotate == LOGO_ROTATE_270)
        {
            msg_x = (image_info->img_height - (strlen(msg_list[i]) * font_info.width)) / 2;
            msg_y = image_info->img_width * position_ratio * 0.10 * count;
        }
        else
        {
            BOOTLOGO_DBG(DBG_LEVEL_ERR, "wrong rotate param: %d!\n", disp_cfg->rotate);
        }
        if (-1 == show_ui_msg(contex, msg_list[i], msg_x, msg_y, text_color, 0x9dceff, disp_dev_id))
        {
            return -1;
        }
        count++;
    }
    return 0;
}

static MI_S32 gop_init(display_config *disp_cfg, MI_DISP_InputPortAttr_t *input_attr,
                       MI_DISP_IMPL_MhalVideoFrameData_t *video_frame_buffer, MI_DISP_VidWinRect_t *vid_winrect)
{
    gop_config *                   gop_cfg = &disp_cfg->gop_cfg;
    MHAL_RGN_GopStretchWinConfig_t stGopStretchWinCfg;
    memset(&stGopStretchWinCfg, 0, sizeof(stGopStretchWinCfg));
    MHAL_RGN_GopGwinPixelFmtConfig_t stGopPixelFmtCfg;
    memset(&stGopPixelFmtCfg, 0, sizeof(stGopPixelFmtCfg));
    MHAL_RGN_GopGwinWinConfig_t stGopWinCfg;
    memset(&stGopWinCfg, 0, sizeof(stGopWinCfg));
    MHAL_RGN_GopGwinAlphaConfig_t stGopAlphaCfg;
    memset(&stGopAlphaCfg, 0, sizeof(stGopAlphaCfg));
    MHAL_RGN_GopAlphaZeroOpaqueConfig_t stAlphaZeroOpaqueCfg;
    memset(&stAlphaZeroOpaqueCfg, 0, sizeof(stAlphaZeroOpaqueCfg));
    MHAL_RGN_GopGwinBufferConfig_t stGopBufferCfg;
    memset(&stGopBufferCfg, 0, sizeof(stGopBufferCfg));
    MHAL_RGN_GopGwinOnOffConfig_t gop_on_off_cfg;
    memset(&gop_on_off_cfg, 0, sizeof(gop_on_off_cfg));
    MHAL_RGN_CmdqConfig_t gop_cmdq_config;
    memset(&gop_cmdq_config, 0, sizeof(gop_cmdq_config));

    MI_S32 timing_width  = 0;
    MI_S32 timing_height = 0;
    get_timing_size(disp_cfg, &timing_width, &timing_height);
    BOOTLOGO_DBG(DBG_LEVEL_INFO, "timing_width: %d, timing_height: %d\n", timing_width, timing_height);
    gop_cfg->dev_id  = disp_cfg->disp_dev_id;
    gop_cfg->gwin_id = 0;
    MI_S32 bScaleup  = 0;
    if (input_attr->stDispWin.u16Width == timing_width && input_attr->stDispWin.u16Height == timing_height
        && (input_attr->stDispWin.u16Width > input_attr->u16SrcWidth
            || input_attr->stDispWin.u16Height > input_attr->u16SrcHeight)) // scale up to full srceen
    {
        bScaleup = 1;
    }

    /*u32 u32DbgLevel = 0;
    MhalRgnSetDbgLevel(&u32DbgLevel);*/
    if (!gop_cfg->ctx)
    {
        MHAL_RGN_DevAttr_t stRgnDevAttr;
        stRgnDevAttr.eDevType    = E_MHAL_RGN_DEV_TYPE_GOP;
        stRgnDevAttr.eTargetType = E_MHAL_RGN_TARGET_TYPE_DISP_CUR_0;
        stRgnDevAttr.u32Id       = 0;
        BL_EXEC_FUNC(MHAL_RGN_Create(&stRgnDevAttr, &gop_cfg->ctx), MHAL_SUCCESS);
    }
    if (!gop_cfg->active)
    {
        BL_EXEC_FUNC(MHAL_RGN_Active(gop_cfg->ctx), MHAL_SUCCESS);
        gop_cfg->active = 1;
    }

    if (bScaleup)
    {
        stGopStretchWinCfg.stSrcWinCfg.u32X      = 0;
        stGopStretchWinCfg.stSrcWinCfg.u32Y      = 0;
        stGopStretchWinCfg.stSrcWinCfg.u32Width  = vid_winrect->u16Width;
        stGopStretchWinCfg.stSrcWinCfg.u32Height = vid_winrect->u16Height;
    }
    else
    {
        stGopStretchWinCfg.stSrcWinCfg.u32X      = 0;
        stGopStretchWinCfg.stSrcWinCfg.u32Y      = 0;
        stGopStretchWinCfg.stSrcWinCfg.u32Width  = timing_width;
        stGopStretchWinCfg.stSrcWinCfg.u32Height = timing_height;
    }
    stGopStretchWinCfg.stDestWinCfg.u32X      = 0;
    stGopStretchWinCfg.stDestWinCfg.u32Y      = 0;
    stGopStretchWinCfg.stDestWinCfg.u32Width  = timing_width;
    stGopStretchWinCfg.stDestWinCfg.u32Height = timing_height;
    BOOTLOGO_DBG(DBG_LEVEL_MHAL, "dev: %d, src (%d %d %d %d), dst (%d %d %d %d)\n", gop_cfg->dev_id,
                 stGopStretchWinCfg.stSrcWinCfg.u32X, stGopStretchWinCfg.stSrcWinCfg.u32Y,
                 stGopStretchWinCfg.stSrcWinCfg.u32Width, stGopStretchWinCfg.stSrcWinCfg.u32Height,
                 stGopStretchWinCfg.stDestWinCfg.u32X, stGopStretchWinCfg.stDestWinCfg.u32Y,
                 stGopStretchWinCfg.stDestWinCfg.u32Width, stGopStretchWinCfg.stDestWinCfg.u32Height);
    BL_EXEC_FUNC(MHAL_RGN_GopSetBaseWindow(gop_cfg->ctx, &stGopStretchWinCfg), MHAL_SUCCESS);

    stGopPixelFmtCfg.u32GwinId    = gop_cfg->gwin_id;
    stGopPixelFmtCfg.ePixelFormat = E_MHAL_GOP_PIXEL_FORMAT_ARGB8888;
    BL_EXEC_FUNC(MHAL_RGN_GopGwinSetPixelFormat(gop_cfg->ctx, &stGopPixelFmtCfg), MHAL_SUCCESS);

    stGopWinCfg.u32GwinId          = gop_cfg->gwin_id;
    stGopWinCfg.u32Stride          = input_attr->u16SrcWidth * 4;
    stGopWinCfg.stWinCfg.u32X      = input_attr->stDispWin.u16X;
    stGopWinCfg.stWinCfg.u32Y      = input_attr->stDispWin.u16Y;
    stGopWinCfg.stWinCfg.u32Width  = BL_MIN(timing_width, vid_winrect->u16Width);
    stGopWinCfg.stWinCfg.u32Height = BL_MIN(timing_height, vid_winrect->u16Height);
    BOOTLOGO_DBG(DBG_LEVEL_MHAL, "gwinid: %d, stride: %d, wincfg (%d %d %d %d)\n", stGopWinCfg.u32GwinId,
                 stGopWinCfg.u32Stride, stGopWinCfg.stWinCfg.u32X, stGopWinCfg.stWinCfg.u32Y,
                 stGopWinCfg.stWinCfg.u32Width, stGopWinCfg.stWinCfg.u32Height);
    BL_EXEC_FUNC(MHAL_RGN_GopGwinSetWindow(gop_cfg->ctx, &stGopWinCfg), MHAL_SUCCESS);

    stGopAlphaCfg.u32GwinId       = gop_cfg->gwin_id;
    stGopAlphaCfg.eAlphaType      = E_MHAL_GOP_GWIN_ALPHA_CONSTANT;
    stGopAlphaCfg.u8ConstAlphaVal = 0xff;
    BL_EXEC_FUNC(MHAL_RGN_GopGwinSetAlphaType(gop_cfg->ctx, &stGopAlphaCfg), MHAL_SUCCESS);

    stAlphaZeroOpaqueCfg.bEn          = 0;
    stAlphaZeroOpaqueCfg.bConstAlpha  = 1;
    stAlphaZeroOpaqueCfg.ePixelFormat = stGopPixelFmtCfg.ePixelFormat;
    BL_EXEC_FUNC(MHAL_RGN_GopSetAlphaZeroOpaque(gop_cfg->ctx, &stAlphaZeroOpaqueCfg), MHAL_SUCCESS);

    stGopBufferCfg.u32GwinId  = gop_cfg->gwin_id;
    stGopBufferCfg.u16Xoffset = 0;
    stGopBufferCfg.phyAddr    = video_frame_buffer->aPhyAddr[0] + (vid_winrect->u16X * 4);
    BOOTLOGO_DBG(DBG_LEVEL_MHAL, "gwin id: %d, u16Xoffset: %d, phy_addr: 0x%llx\n", stGopBufferCfg.u32GwinId,
                 stGopBufferCfg.u16Xoffset, stGopBufferCfg.phyAddr);
    BL_EXEC_FUNC(MHAL_RGN_GopGwinSetBuffer(gop_cfg->ctx, &stGopBufferCfg), MHAL_SUCCESS);

    gop_on_off_cfg.u32GwinId = gop_cfg->gwin_id;
    gop_on_off_cfg.bEn       = 1;
    BL_EXEC_FUNC(MHAL_RGN_GopGwinSetOnOff(gop_cfg->ctx, &gop_on_off_cfg), MHAL_SUCCESS);
    gop_cmdq_config.pstCmdqInf = NULL;
    BL_EXEC_FUNC(MHAL_RGN_SetCmdq(gop_cfg->ctx, &gop_cmdq_config), MHAL_SUCCESS);
    BL_EXEC_FUNC(MHAL_RGN_Process(gop_cfg->ctx), MHAL_SUCCESS);

    return 0;
}

static MI_S32 gop_deinit(display_config *disp_cfg)
{
    gop_config *                  gop_cfg = &disp_cfg->gop_cfg;
    MHAL_RGN_GopGwinOnOffConfig_t gop_on_off_cfg;
    memset(&gop_on_off_cfg, 0, sizeof(gop_on_off_cfg));
    if (gop_cfg->ctx)
    {
        gop_on_off_cfg.u32GwinId = gop_cfg->gwin_id;
        gop_on_off_cfg.bEn       = 0;
        BL_EXEC_FUNC(MHAL_RGN_GopGwinSetOnOff(gop_cfg->ctx, &gop_on_off_cfg), MHAL_SUCCESS);
        if (gop_cfg->active)
        {
            BL_EXEC_FUNC(MHAL_RGN_Deactive(gop_cfg->ctx), MHAL_SUCCESS);
        }
        BL_EXEC_FUNC(MHAL_RGN_Destory(gop_cfg->ctx), MHAL_SUCCESS);
        gop_cfg->active = 0;
        gop_cfg->ctx    = NULL;
        BOOTLOGO_DBG(DBG_LEVEL_MHAL, "Destory done\n");
    }

    return 0;
}

static MI_S32 ui_init(MI_U32 disp_dev_id)
{
    bootlogo_contex *                 contex  = &g_bootlogo_contex;
    disp_image_info *                 ui_info = &contex->ui_info[disp_dev_id];
    MI_DISP_IMPL_MhalVideoFrameData_t Video_frame_buffer;
    MI_DISP_VidWinRect_t              vid_winrect;
    memset(&vid_winrect, 0, sizeof(MI_DISP_VidWinRect_t));
    memset(&Video_frame_buffer, 0, sizeof(Video_frame_buffer));
    MI_DISP_InputPortAttr_t input_attr;
    memset(&input_attr, 0, sizeof(input_attr));
    display_config *disp_cfg      = &contex->disp_cfgs[disp_dev_id];
    MI_U32          timing_width  = 0;
    MI_U32          timing_height = 0;
    MI_U32          i             = 0;

    if (disp_cfg->ui_inited == true)
    {
        BOOTLOGO_DBG(DBG_LEVEL_INFO, "UI has inited\n");
        return 0;
    }
    get_timing_size(disp_cfg, &timing_width, &timing_height);

    ui_info->pixel_format     = E_MI_SYS_PIXEL_FRAME_ARGB8888;
    ui_info->decode_data_size = timing_width * timing_height * 4;

    ui_info->decode_data[0] = malloc_from_reserved_buf(&contex->disp_buf_info, ui_info->decode_data_size);
    if (ui_info->decode_data[0] == 0)
    {
        BOOTLOGO_DBG(DBG_LEVEL_ERR, "GOP malloc buffer is null!\n");
        return -1;
    }
    ui_info->decode_data[1] = (MI_U32)BOOTLOGO_TO_PHYA(ui_info->decode_data[0]);
    ui_info->img_stride     = timing_width;
    ui_info->img_width      = timing_width;
    ui_info->img_height     = timing_height;

    Video_frame_buffer.ePixelFormat  = E_MI_SYS_PIXEL_FRAME_ARGB8888;
    Video_frame_buffer.au32Stride[0] = ui_info->img_stride;
    Video_frame_buffer.au32Stride[1] = Video_frame_buffer.au32Stride[0];
    Video_frame_buffer.aPhyAddr[0]   = ui_info->decode_data[1];
    Video_frame_buffer.aPhyAddr[1] =
        Video_frame_buffer.aPhyAddr[0] + (Video_frame_buffer.au32Stride[0] * input_attr.u16SrcHeight);

    input_attr.u16SrcWidth  = timing_width;
    input_attr.u16SrcHeight = timing_height;
    ui_info->img_stride     = timing_width;
    ui_info->img_width      = timing_width;
    ui_info->img_height     = timing_height;
    vid_winrect.u16X        = 0;
    vid_winrect.u16Y        = 0;
    vid_winrect.u16Width    = timing_width;
    vid_winrect.u16Height   = timing_height;
    disp_cfg->image_info    = ui_info;

    while (4 * i < ui_info->decode_data_size)
    {
        ((MI_S32 *)ui_info->decode_data[0])[i++] = 0xff9dceff; // blue
    }
    gop_init(disp_cfg, &input_attr, &Video_frame_buffer, &vid_winrect);
    disp_cfg->ui_inited = true;
    return 0;
}
#endif

#if defined(CONFIG_SSTAR_DISP)
static MI_S32 panel_init(display_config *disp_cfg)
{
    BL_EXEC_FUNC(DRV_DISP_IF_SetPnlUnifiedParamConfig(disp_cfg->dev_ctx, &disp_cfg->mhal_pnl_unified_param_cfg), 1);
    BOOTLOGO_DBG(DBG_LEVEL_MHAL, "finish\n");
    return 0;
}

static MI_S32 panel_deinit(display_config *disp_cfg)
{
    // free buffer
    MI_DISP_IMPL_MhalPnlUnifiedParamConfig_t *pnl_param_cfg = &disp_cfg->mhal_pnl_unified_param_cfg;
    if (pnl_param_cfg->pPanelName)
    {
        free((void *)pnl_param_cfg->pPanelName);
    }
    MI_DISP_IMPL_MhalPnlUnifiedMcuConfig_t *mcu_cfg = &pnl_param_cfg->stMcuInfo;
    if (mcu_cfg->stMcuInitCmd.pu32CmdBuf)
    {
        free(mcu_cfg->stMcuInitCmd.pu32CmdBuf);
        mcu_cfg->stMcuInitCmd.pu32CmdBuf    = NULL;
        mcu_cfg->stMcuInitCmd.u32CmdBufSize = 0;
    }
    if (mcu_cfg->stMcuAutoCmd.pu32CmdBuf)
    {
        free(mcu_cfg->stMcuAutoCmd.pu32CmdBuf);
        mcu_cfg->stMcuAutoCmd.pu32CmdBuf    = NULL;
        mcu_cfg->stMcuAutoCmd.u32CmdBufSize = 0;
    }
    MI_DISP_IMPL_MhalPnlUnifiedMipiDsiConfig_t *mipi_dsi_cfg = &pnl_param_cfg->stMpdInfo;
    if (mipi_dsi_cfg->pu8CmdBuf)
    {
        free(mipi_dsi_cfg->pu8CmdBuf);
        mipi_dsi_cfg->pu8CmdBuf     = NULL;
        mipi_dsi_cfg->u32CmdBufSize = 0;
    }

    BOOTLOGO_DBG(DBG_LEVEL_MHAL, "finish\n");
    return 0;
}

static MI_S32 mop_init(display_config *disp_cfg, disp_image_info *image_info, MI_DISP_InputPortAttr_t *input_attr,
                       MI_DISP_IMPL_MhalVideoFrameData_t *video_frame_buffer, MI_DISP_VidWinRect_t *vid_winrect)
{
    bootlogo_contex *                  contex        = &g_bootlogo_contex;
    MI_DISP_IMPL_MhalMemAllocConfig_t *phy_mem       = &contex->phy_mem;
    void *                             vid_layer_ctx = disp_cfg->vid_layer_ctx;
    MI_U32                             port_id       = disp_cfg->port_id;

    if (!disp_cfg->input_port_ctx)
    {
        BL_EXEC_FUNC(DRV_DISP_IF_InputPortCreateInstance(phy_mem, vid_layer_ctx, port_id, &disp_cfg->input_port_ctx),
                     1);
    }
    BL_EXEC_FUNC(DRV_DISP_IF_InputPortSetAttr(disp_cfg->input_port_ctx, input_attr), 1);
    BL_EXEC_FUNC(DRV_DISP_IF_InputPortSetCropAttr(disp_cfg->input_port_ctx, vid_winrect), 1);
    BL_EXEC_FUNC(DRV_DISP_IF_InputPortFlip(disp_cfg->input_port_ctx, video_frame_buffer), 1);
    BL_EXEC_FUNC(DRV_DISP_IF_InputPortEnable(disp_cfg->input_port_ctx, TRUE), 1);

    MI_DISP_IMPL_MhalRegFlipConfig_t reg_flip_cfg;
    memset(&reg_flip_cfg, 0, sizeof(reg_flip_cfg));
    MI_DISP_IMPL_MhalRegWaitDoneConfig_t stRegWaitDoneCfg;
    memset(&stRegWaitDoneCfg, 0, sizeof(stRegWaitDoneCfg));
    reg_flip_cfg.bEnable  = 1;
    reg_flip_cfg.pCmdqInf = NULL;
    BL_EXEC_FUNC(DRV_DISP_IF_SetRegFlipConfig(disp_cfg->dev_ctx, &reg_flip_cfg), 1);
    stRegWaitDoneCfg.pCmdqInf = NULL;
    BL_EXEC_FUNC(DRV_DISP_IF_SetRegWaitDoneConfig(disp_cfg->dev_ctx, &stRegWaitDoneCfg), 1);

    return 0;
}
#endif

static MI_S32 common_init(bootlogo_contex *contex)
{
    char * env     = NULL;
    char * pHandle = NULL;
    MI_S32 miu     = 0;
    char   sz[MAX_STR_LEN];
    char   start[MAX_STR_LEN];
    char   end[MAX_STR_LEN];

    BOOTLOGO_DBG(DBG_LEVEL_API, ">>>\n");
    if (contex->common_inited)
    {
        BOOTLOGO_DBG(DBG_LEVEL_INFO, "Common resource already inited\n");
        return 0;
    }
    memset(contex, 0, sizeof(bootlogo_contex));

    if (!strcmp(CONFIG_SSTAR_LOGO_PART_NAME, ""))
    {
        BOOTLOGO_DBG(DBG_LEVEL_WRN, "logo part name is NULL!\n");
        return -1;
    }
    if (fs_mount(CONFIG_SSTAR_LOGO_PART_NAME, NULL) < 0)
    {
        // inited=1 and return -2 for empty slice upgrade
        contex->common_inited = 1;
        BOOTLOGO_DBG(DBG_LEVEL_WRN, "fs_mount partition failed!\n");
        return -2;
    }

    env = env_get("bootargs");
    if (!env)
    {
        BOOTLOGO_DBG(DBG_LEVEL_WRN, "env_get: bootargs fail\n");
        return -1;
    }
    pHandle = strstr(env, "mmap_reserved=fb");
    if (!pHandle)
    {
        BOOTLOGO_DBG(DBG_LEVEL_ERR, "strstr: mmap_reserved=fb fail\n");
        return -1;
    }

    sscanf(pHandle, "mmap_reserved=fb,miu=%d,sz=%[^,],max_start_off=%[^,],max_end_off=%[^ ]", &miu, sz, start, end);
    contex->disp_buf_info.size     = simple_strtoul(sz, NULL, 16);
    contex->disp_buf_info.phy_addr = simple_strtoul(start, NULL, 16);
    contex->disp_buf_info.offest   = 0;
    BOOTLOGO_DBG(DBG_LEVEL_INFO, "bootargs size 0x%x addr 0x%x\n", contex->disp_buf_info.size,
                 contex->disp_buf_info.phy_addr);
    if (!contex->disp_buf_info.phy_addr || !contex->disp_buf_info.size)
    {
        BOOTLOGO_DBG(DBG_LEVEL_ERR, "bootargs failed. size 0x%x addr 0x%x\n", contex->disp_buf_info.size,
                     contex->disp_buf_info.phy_addr);
        return -1;
    }

    contex->common_inited = 1;
    BOOTLOGO_DBG(DBG_LEVEL_API, "<<<\n");

    return 0;
}

#ifdef CONFIG_SSTAR_BOOTLOGO_UT
static MI_S32 load_boot_cmds(cJSON *root_node, cmd_table_config *pstCmdstableCfg)
{
    MI_S32 i             = 0;
    MI_S32 j             = 0;
    MI_S32 s32CmdlineCnt = 0;
    MI_S32 count         = 0;
    cJSON *disp_node     = NULL;
    cJSON *cmdlines_node = NULL;
    cJSON *subnode       = NULL;
    cJSON *node          = NULL;
    cJSON *bootcmds_node = NULL;

    BOOTLOGO_DBG(DBG_LEVEL_API, ">>>\n");
    BOOTLOGO_DBG(DBG_LEVEL_INFO, "Star Init cmdstable...\n");

    disp_node = cJSON_GetObjectItem(root_node, "bootlogo");
    if (disp_node == NULL)
    {
        BOOTLOGO_DBG(DBG_LEVEL_ERR, "Get disp_node Fail!\n");
        goto Exit;
    }
    bootcmds_node = cJSON_GetObjectItem(disp_node, "bootCmds");
    if (bootcmds_node == NULL)
    {
        BOOTLOGO_DBG(DBG_LEVEL_ERR, "Get bootcmds_node Fail!\n");
        goto Exit;
    }
    count = cJSON_GetArraySize(bootcmds_node);
    if (count > 0)
    {
        BOOTLOGO_DBG(DBG_LEVEL_INFO, "count: %d !\n", count);
        pstCmdstableCfg->count = count;

        for (i = 0; i < count; i++)
        {
            subnode = cJSON_GetArrayItem(bootcmds_node, i);
            CJSON_GETSTR(subnode, "cmdsName", pstCmdstableCfg->tables[i].cmdtable_name);
            cmdlines_node = cJSON_GetObjectItem(subnode, "cmdlines");

            s32CmdlineCnt                             = cJSON_GetArraySize(cmdlines_node);
            pstCmdstableCfg->tables[i].cmdtable_lines = s32CmdlineCnt;

            for (j = 0; j < s32CmdlineCnt; j++)
            {
                node = cJSON_GetArrayItem(cmdlines_node, j);
                if (strlen(node->valuestring) + 1 > MAX_STR_LEN)
                {
                    BOOTLOGO_DBG(DBG_LEVEL_ERR, "This cmdlines[%s] is too long !\n", node->valuestring);
                    continue;
                }
                strncpy(pstCmdstableCfg->tables[i].cmdtable_data[j], node->valuestring, strlen(node->valuestring) + 1);
                BOOTLOGO_DBG(DBG_LEVEL_INFO, "cmdtable[%d]: cmdlines: %d cmd: %s\n", i, j, node->valuestring);
            }
        }
    }
    else
    {
        BOOTLOGO_DBG(DBG_LEVEL_INFO, "Cmdstable is NULL\n");
    }

    BOOTLOGO_DBG(DBG_LEVEL_API, "<<<\n");
Exit:

    return 0;
}

static cmd_table *bootlogo_get_cmdtable_by_cmdname(cmd_table_config *cmdtable_cfg, MI_U8 *cmdtable_name)
{
    MI_S32 i = 0;

    for (i = 0; i < cmdtable_cfg->count; i++)
    {
        if (!strcasecmp(cmdtable_name, cmdtable_cfg->tables[i].cmdtable_name))
        {
            BOOTLOGO_DBG(DBG_LEVEL_INFO, "Found: %s cmdtable!\n", cmdtable_name);
            return &cmdtable_cfg->tables[i];
        }
    }
    BOOTLOGO_DBG(DBG_LEVEL_ERR, "Not found: %s cmdtable! \n", cmdtable_name);
    return NULL;
}
#endif

#ifdef CONFIG_SSTAR_CJSON_PARSER
static MI_S32 load_pq_file(bootlogo_contex *contex, display_config *disp_cfg)
{
    BOOTLOGO_DBG(DBG_LEVEL_API, ">>>\n");
    if (!contex->pq_data_addr[1] && (strlen(disp_cfg->pq_file) != 0))
    {
        contex->pq_size = read_logo_file(disp_cfg->pq_file, (char **)&contex->pq_data_addr[0]);
        if (contex->pq_data_addr[0])
        {
            contex->pq_data_addr[1] = BOOTLOGO_TO_PHYA(contex->pq_data_addr[0]);
        }
    }

    BOOTLOGO_DBG(DBG_LEVEL_API, "<<<\n");
    return 0;
}

static MI_S32 load_logofile_name(bootlogo_contex *contex, cJSON *logo_file_node)
{
    MI_S32           ret             = -1;
    MI_S32           i               = 0;
    disp_image_info *image_info      = NULL;
    cJSON *          child_file_node = NULL;

    BOOTLOGO_DBG(DBG_LEVEL_API, ">>>\n");
    for (i = 0; i < MAX_LOGO_FILE_NUM; i++)
    {
        if (i < cJSON_GetArraySize(logo_file_node))
        {
            child_file_node = cJSON_GetArrayItem(logo_file_node, i);
            image_info      = &contex->logo_files[i];
            CJSON_GETSTR(child_file_node, "file_name", image_info->file_path);
            BOOTLOGO_DBG(DBG_LEVEL_INFO, "Get logo_file[%d], file_name: %s\n", i, image_info->file_path);
        }
    }
    ret = 0;
    BOOTLOGO_DBG(DBG_LEVEL_API, "<<<\n");
    return ret;
}

cJSON *get_json_root(char *file_path)
{
    char * file_data = NULL;
    MI_S32 file_size = 0;
    cJSON *root_node = NULL;

    file_size = read_logo_file(file_path, (char **)&file_data);
    if (file_size == -1)
    {
        BOOTLOGO_DBG(DBG_LEVEL_ERR, "read %s failed!\n", file_path);
        return root_node;
    }
    root_node = cJSON_Parse(file_data);
    if (root_node == NULL)
    {
        BOOTLOGO_DBG(DBG_LEVEL_INFO, "Parse Cjosn failed!\n");
        goto Exit;
    }
    BOOTLOGO_DBG(DBG_LEVEL_INFO, "json_parser_load done\n");
Exit:

    if (file_data)
    {
        free(file_data);
    }
    return root_node;
}

static MI_S32 parse_backlight_config(display_config *disp_cfg, cJSON *backlight_node)
{
    MI_U32 exist_flag        = 0;
    MI_S32 backlight_percent = 0;

    BOOTLOGO_DBG(DBG_LEVEL_API, ">>>\n");
    disp_cfg->gpio_config[0].pin       = CJSON_GETINT(backlight_node, "power_gpio", exist_flag, PAD_UNKNOWN);
    disp_cfg->gpio_config[0].value     = CJSON_GETINT(backlight_node, "power_gpio_value", exist_flag, 0);
    disp_cfg->gpio_config[1].pin       = CJSON_GETINT(backlight_node, "reset_gpio", exist_flag, PAD_UNKNOWN);
    disp_cfg->gpio_config[1].value     = CJSON_GETINT(backlight_node, "reset_gpio_value", exist_flag, 0);
    disp_cfg->gpio_config[2].pin       = CJSON_GETINT(backlight_node, "backlight_gpio", exist_flag, PAD_UNKNOWN);
    disp_cfg->gpio_config[2].value     = CJSON_GETINT(backlight_node, "backlight_gpio_value", exist_flag, 0);
    disp_cfg->backlight_config.pwm_pin = CJSON_GETINT(backlight_node, "pwm_pin", exist_flag, PAD_UNKNOWN);
    disp_cfg->backlight_config.period  = CJSON_GETINT(backlight_node, "pwm_period", exist_flag, 0);
    backlight_percent                  = CJSON_GETINT(backlight_node, "backlight_percent", exist_flag, 100);
    if (backlight_percent > 100)
    {
        BOOTLOGO_DBG(DBG_LEVEL_ERR, "backlight percent error!\n");
        return 0;
    }
    disp_cfg->backlight_config.duty_cycle = backlight_percent * disp_cfg->backlight_config.period / 100;
    BOOTLOGO_DBG(DBG_LEVEL_API, "<<<\n");
    return 0;
}
#endif

static MI_S32 parse_logo_config(bootlogo_contex *contex, cJSON *root_node)
{
    MI_U32          disp_dev_count = 0;
    display_config *disp_cfg       = NULL;
    cJSON *         logo_node      = NULL;
    cJSON *         blg_node_arry  = NULL;
    cJSON *         sub_node       = NULL;
    cJSON *         logo_file_node = NULL;
    MI_S32          exist_flag     = 0;
    MI_U32          rotate         = 0;
    BOOTLOGO_DBG(DBG_LEVEL_API, ">>>\n");
    if (contex->logo_config_parsed == true)
    {
        BOOTLOGO_DBG(DBG_LEVEL_INFO, "bootlogo config already init\n");
        return 0;
    }

    logo_node = cJSON_GetObjectItem(root_node, "bootlogo");
    if (logo_node == NULL)
    {
        BOOTLOGO_DBG(DBG_LEVEL_ERR, "get bootlogo node failed!\n");
        return -1;
    }

    blg_node_arry = cJSON_GetObjectItem(logo_node, "property");
    if (blg_node_arry == NULL)
    {
        BOOTLOGO_DBG(DBG_LEVEL_ERR, "get bootlogo list fail\n");
        return -1;
    }

    disp_dev_count = cJSON_GetArraySize(blg_node_arry);
    if (disp_dev_count <= 0)
    {
        BOOTLOGO_DBG(DBG_LEVEL_ERR, "get bootlogo node content is NULL \n");
        return -1;
    }
    contex->disp_dev_count = disp_dev_count;

    for (MI_U32 i = 0; i < disp_dev_count && i < MAX_DISP_DEV_NUM; i++)
    {
        disp_cfg = &contex->disp_cfgs[i];
        sub_node = cJSON_GetArrayItem(blg_node_arry, i);
        if (sub_node == NULL)
        {
            BOOTLOGO_DBG(DBG_LEVEL_ERR, "get bootlogo node fail\n");
            return -1;
        }
        char interface_str[MAX_STR_LEN];
        memset(interface_str, 0x0, MAX_STR_LEN);
        CJSON_GETSTR(sub_node, "device_type", interface_str);
        disp_cfg->disp_interface = disp_intf_str_to_enum(interface_str);

        memset(disp_cfg->target_node_name, 0, MAX_STR_LEN);
        CJSON_GETSTR(sub_node, "param_target", disp_cfg->target_node_name);
        if (!strlen(disp_cfg->target_node_name))
        {
            BOOTLOGO_DBG(DBG_LEVEL_INFO, "disp_cfg->szTargetSec: %s\n", disp_cfg->target_node_name);
        }
        disp_cfg->disp_dev_id = CJSON_GETINT(sub_node, "dev_id", exist_flag, 0);
        rotate                = CJSON_GETINT(sub_node, "rotate", exist_flag, 0);
        switch (rotate)
        {
            case 90:
                disp_cfg->rotate = LOGO_ROTATE_90;
                break;
            case 180:
                disp_cfg->rotate = LOGO_ROTATE_180;
                break;
            case 270:
                disp_cfg->rotate = LOGO_ROTATE_270;
                break;
            default:
                disp_cfg->rotate = LOGO_ROTATE_NONE;
        }
        disp_cfg->fps = CJSON_GETINT(sub_node, "fps", exist_flag, 0);
        CJSON_GETSTR(sub_node, "pq_file_name", disp_cfg->pq_file);
        if (disp_cfg->pq_file)
        {
            load_pq_file(contex, disp_cfg);
        }
    }
    logo_file_node = cJSON_GetObjectItem(logo_node, "logo_file");
    if (load_logofile_name(contex, logo_file_node) == -1)
    {
        return -1;
    }
    contex->logo_config_parsed = true;
    BOOTLOGO_DBG(DBG_LEVEL_API, "<<<\n");

    return 0;
}

static MI_S32 parse_panel_config(cJSON *target_node, display_config *disp_cfg)
{
    MI_S32                                    exist_flag    = 0;
    cJSON *                                   cmd_buf_node  = NULL;
    MI_DISP_IMPL_MhalPnlUnifiedParamConfig_t *pnl_param_cfg = &disp_cfg->mhal_pnl_unified_param_cfg;

    BOOTLOGO_DBG(DBG_LEVEL_API, ">>>\n");
    if (target_node == NULL)
    {
        BOOTLOGO_DBG(DBG_LEVEL_ERR, "target_node is NULL\n");
        return -1;
    }

    CJSON_GETSTR(target_node, "m_pPanelName", pnl_param_cfg->pPanelName);
    BOOTLOGO_DBG(DBG_LEVEL_INFO, "m_pPanelName: %s\n", pnl_param_cfg->pPanelName);

    MI_DISP_IMPL_MhalPnlUnifiedTgnPolarityConfig_t *pstTgnPolarityInfo = &pnl_param_cfg->stTgnPolarityInfo;
    pstTgnPolarityInfo->u8InvDCLK = CJSON_GETINT(target_node, "m_panel_flagInvDCLK", exist_flag, 0);
    pnl_param_cfg->u8TgnPolarityFlag |= exist_flag;
    pstTgnPolarityInfo->u8InvDE = CJSON_GETINT(target_node, "m_panel_flagInvDE", exist_flag, 0);
    pnl_param_cfg->u8TgnPolarityFlag |= exist_flag;
    pstTgnPolarityInfo->u8InvHSync = CJSON_GETINT(target_node, "m_panel_flagInvHSync", exist_flag, 0);
    pnl_param_cfg->u8TgnPolarityFlag |= exist_flag;
    pstTgnPolarityInfo->u8InvVSync = CJSON_GETINT(target_node, "m_panel_flagInvVSync", exist_flag, 0);
    pnl_param_cfg->u8TgnPolarityFlag |= exist_flag;

    pnl_param_cfg->u8TgnTimingFlag                                = 1;
    MI_DISP_IMPL_MhalPnlUnifiedTgnTimingConfig_t *tgn_timing_info = &pnl_param_cfg->stTgnTimingInfo;
    tgn_timing_info->u16HSyncWidth     = CJSON_GETINT(target_node, "m_wPanelHSyncWidth", exist_flag, 0);
    tgn_timing_info->u16HSyncBackPorch = CJSON_GETINT(target_node, "m_wPanelHSyncBackPorch", exist_flag, 0);
    tgn_timing_info->u16VSyncWidth     = CJSON_GETINT(target_node, "m_wPanelVSyncWidth", exist_flag, 0);
    tgn_timing_info->u16VSyncBackPorch = CJSON_GETINT(target_node, "m_wPanelVBackPorch", exist_flag, 0);
    tgn_timing_info->u16HStart         = CJSON_GETINT(target_node, "m_wPanelHStart", exist_flag, 0);
    tgn_timing_info->u16VStart         = CJSON_GETINT(target_node, "m_wPanelVStart", exist_flag, 0);
    tgn_timing_info->u16HActive        = CJSON_GETINT(target_node, "m_wPanelWidth", exist_flag, 0);
    tgn_timing_info->u16VActive        = CJSON_GETINT(target_node, "m_wPanelHeight", exist_flag, 0);
    tgn_timing_info->u16HTotal         = CJSON_GETINT(target_node, "m_wPanelHTotal", exist_flag, 0);
    tgn_timing_info->u16VTotal         = CJSON_GETINT(target_node, "m_wPanelVTotal", exist_flag, 0);
    tgn_timing_info->u32Dclk           = CJSON_GETINT(target_node, "m_wPanelDCLK", exist_flag, 0);

    MI_DISP_IMPL_MhalPnlUnifiedTgnSpreadSpectrumConfig_t *pstTgnSscInfo = &pnl_param_cfg->stTgnSscInfo;
    pstTgnSscInfo->u16SpreadSpectrumStep = CJSON_GETINT(target_node, "m_wSpreadSpectrumFreq", exist_flag, 0);
    pnl_param_cfg->u8TgnSscFlag |= exist_flag;
    BOOTLOGO_DBG(DBG_LEVEL_INFO, "u16SpreadSpectrumStep: %d\n", pstTgnSscInfo->u16SpreadSpectrumStep);
    pstTgnSscInfo->u16SpreadSpectrumSpan = CJSON_GETINT(target_node, "m_wSpreadSpectrumRatio", exist_flag, 0);
    pnl_param_cfg->u8TgnSscFlag |= exist_flag;
    BOOTLOGO_DBG(DBG_LEVEL_INFO, "u16SpreadSpectrumSpan: %d\n", pstTgnSscInfo->u16SpreadSpectrumSpan);

    pnl_param_cfg->eOutputFormatBitMode = CJSON_GETINT(target_node, "m_eOutputFormatBitMode", exist_flag, 0);
    pnl_param_cfg->u8TgnOutputBitMdFlag |= exist_flag;
    BOOTLOGO_DBG(DBG_LEVEL_INFO, "u8TgnOutputBitMdFlag: %d\n", pnl_param_cfg->u8TgnOutputBitMdFlag);
    MI_DISP_IMPL_MhalPnlUnifiedTgnRgbOutputSwapConfig_t *pstTgnRgbSwapInfo = &pnl_param_cfg->stTgnRgbSwapInfo;
    pstTgnRgbSwapInfo->u8SwapChnR = CJSON_GETINT(target_node, "m_ucPanelSwapChnR", exist_flag, 0);
    pnl_param_cfg->u8TgnRgbSwapFlag |= exist_flag;
    pstTgnRgbSwapInfo->u8SwapChnG = CJSON_GETINT(target_node, "m_ucPanelSwapChnG", exist_flag, 0);
    pnl_param_cfg->u8TgnRgbSwapFlag |= exist_flag;
    pstTgnRgbSwapInfo->u8SwapChnB = CJSON_GETINT(target_node, "m_ucPanelSwapChnB", exist_flag, 0);
    pnl_param_cfg->u8TgnRgbSwapFlag |= exist_flag;
    pstTgnRgbSwapInfo->u8SwapRgbML = CJSON_GETINT(target_node, "m_ucPanelSwapRgbML", exist_flag, 0);
    pnl_param_cfg->u8TgnRgbSwapFlag |= exist_flag;

    // rgb data related
    MI_DISP_IMPL_MhalPnlUnifiedRgbDataConfig_t *pstRgbDataInfo = &pnl_param_cfg->stRgbDataInfo;
    pstRgbDataInfo->eRgbDtype = CJSON_GETINT(target_node, "m_ePanelRgbDataType", exist_flag, 0);
    pnl_param_cfg->u8RgbDataFlag |= exist_flag;
    pstRgbDataInfo->u8RgbDswap = CJSON_GETINT(target_node, "m_ePanelRgbDataSwap", exist_flag, 0);
    pnl_param_cfg->u8RgbDataFlag |= exist_flag;

    MI_DISP_IMPL_MhalPnlUnifiedRgbDeltaConfig_t *pstRgbDeltaInfo = &pnl_param_cfg->stRgbDeltaInfo;
    pstRgbDeltaInfo->eOddLine = CJSON_GETINT(target_node, "m_ePanelRgbDeltaOddMode", exist_flag, 0);
    pnl_param_cfg->u8RgbDeltaMdFlag |= exist_flag;
    pstRgbDeltaInfo->eEvenLine = CJSON_GETINT(target_node, "m_ePanelRgbDeltaEvenMode", exist_flag, 0);
    pnl_param_cfg->u8RgbDeltaMdFlag |= exist_flag;

    pnl_param_cfg->u32FDclk = CJSON_GETINT(target_node, "m_wPanelFixedDCLK", exist_flag, 0);
    pnl_param_cfg->u8TgnFixedDclkFlag |= exist_flag;

    if (HAL_DISP_INTF_MCU == disp_cfg->disp_interface || HAL_DISP_INTF_MCU_NOFLM == disp_cfg->disp_interface)
    {
        MI_DISP_IMPL_MhalPnlUnifiedMcuConfig_t *mcu_cfg = &pnl_param_cfg->stMcuInfo;
        pnl_param_cfg->u8McuConfigFlag                  = 1;
        pnl_param_cfg->u8McuPhase                       = CJSON_GETINT(target_node, "m_wMcuPhase", exist_flag, 0);
        pnl_param_cfg->u8McuPhaseFlag |= exist_flag;

        pnl_param_cfg->u8McuPolarity = CJSON_GETINT(target_node, "m_wMcuPolarity", exist_flag, 0);
        pnl_param_cfg->u8McuPolarityFlag |= exist_flag;

        pnl_param_cfg->u8McuRsPolarity = CJSON_GETINT(target_node, "m_wMcuRsPolarity", exist_flag, 0);
        pnl_param_cfg->u8McuRsPolarityFlag |= exist_flag;

        mcu_cfg->u32HActive         = CJSON_GETINT(target_node, "m_wMcuHActive", exist_flag, 0);
        mcu_cfg->u32VActive         = CJSON_GETINT(target_node, "m_wMcuVActive", exist_flag, 0);
        mcu_cfg->u8WRCycleCnt       = CJSON_GETINT(target_node, "m_wMcuWRCycleCnt", exist_flag, 0);
        mcu_cfg->u8CSLeadWRCycleCnt = CJSON_GETINT(target_node, "m_wMcuCSLeadWRCycleCnt", exist_flag, 0);
        mcu_cfg->u8RSLeadCSCycleCnt = CJSON_GETINT(target_node, "m_wMcuRSLeadCSCycleCnt", exist_flag, 0);
        mcu_cfg->enMcuType          = CJSON_GETINT(target_node, "m_eMcuType", exist_flag, 0);
        mcu_cfg->enMcuDataBusCfg    = CJSON_GETINT(target_node, "m_eMcuDataBusCfg", exist_flag, 0);
        cmd_buf_node                = cJSON_GetObjectItem(target_node, "m_pMcuInitCmdBuff");
        if (cmd_buf_node != NULL)
        {
            CJSON_GET_PNL_ARRAY(cmd_buf_node, "m_pMcuInitCmdBuff", mcu_cfg->stMcuInitCmd.pu32CmdBuf,
                                mcu_cfg->stMcuInitCmd.u32CmdBufSize, MI_U32, exist_flag);
            cmd_buf_node = NULL;
        }
        else
        {
            BOOTLOGO_DBG(DBG_LEVEL_ERR, "Can't get m_pMcuInitCmdBuff node .\n");
        }

        cmd_buf_node = cJSON_GetObjectItem(target_node, "m_pMcuAutoCmdBuff");
        if (cmd_buf_node != NULL)
        {
            CJSON_GET_PNL_ARRAY(cmd_buf_node, "m_pMcuAutoCmdBuff", mcu_cfg->stMcuAutoCmd.pu32CmdBuf,
                                mcu_cfg->stMcuAutoCmd.u32CmdBufSize, MI_U32, exist_flag);
            cmd_buf_node = NULL;
        }
        else
        {
            BOOTLOGO_DBG(DBG_LEVEL_ERR, "Can't get m_pMcuAutoCmdBuff node .\n");
        }
    }

    if (HAL_DISP_INTF_BT1120 == disp_cfg->disp_interface || HAL_DISP_INTF_BT1120_DDR == disp_cfg->disp_interface)
    {
        pnl_param_cfg->u8Bt1120Phase = CJSON_GETINT(target_node, "m_wBt1120Phase", exist_flag, 0);
        pnl_param_cfg->u8Bt1120PhaseFlag |= exist_flag;
    }
    if (HAL_DISP_INTF_BT1120 == disp_cfg->disp_interface || HAL_DISP_INTF_BT1120_DDR == disp_cfg->disp_interface
        || HAL_DISP_INTF_BT601 == disp_cfg->disp_interface || HAL_DISP_INTF_BT656 == disp_cfg->disp_interface)
    {
        pnl_param_cfg->ePnlScanMode = CJSON_GETINT(target_node, "m_ePanelScanMode", exist_flag, 0);
        if (pnl_param_cfg->ePnlScanMode == E_MI_DISP_MHALPNL_SCAN_INTERLACE_EXT)
        {
            tgn_timing_info->u16OddVfdeStart  = CJSON_GETINT(target_node, "m_wPanelOddVfdeStart", exist_flag, 0);
            tgn_timing_info->u16OddVfdeEnd    = CJSON_GETINT(target_node, "m_wPanelOddVfdeEnd", exist_flag, 0);
            tgn_timing_info->u16EvenVfdeStart = CJSON_GETINT(target_node, "m_wPanelEvenVfdeStart", exist_flag, 0);
            tgn_timing_info->u16EvenVfdeEnd   = CJSON_GETINT(target_node, "m_wPanelEvenVfdeEnd", exist_flag, 0);
            tgn_timing_info->u16Field0Start   = CJSON_GETINT(target_node, "m_wPanelField0Start", exist_flag, 0);
            tgn_timing_info->u16Field0End     = CJSON_GETINT(target_node, "m_wPanelField0End", exist_flag, 0);
        }
    }

    MI_DISP_IMPL_MhalPnlUnifiedPadDrvngConfig_t *pstPadDrvngInfo = &pnl_param_cfg->stPadDrvngInfo;
    pstPadDrvngInfo->u8PadDrvngLvl = CJSON_GETINT(target_node, "m_wPadDrvngLvl", exist_flag, 0);
    pnl_param_cfg->u8PadDrvngFlag |= exist_flag;
    // pstPadDrvngInfo->u32PadMode = CJSON_GETINT(target_node, "m_wPadMode", exist_flag, 0);
    // pnl_param_cfg->u8PadDrvngPadMdFlag |= exist_flag;

    if (HAL_DISP_INTF_MIPIDSI == disp_cfg->disp_interface || HAL_DISP_INTF_MIPIDSI_1 == disp_cfg->disp_interface)
    {
        MI_DISP_IMPL_MhalPnlUnifiedMipiDsiConfig_t *mipi_dsi_cfg = &pnl_param_cfg->stMpdInfo;
        pnl_param_cfg->u8MpdFlag                                 = 1;

        mipi_dsi_cfg->u8HsTrail   = CJSON_GETINT(target_node, "m_wHsTrail", exist_flag, 0);
        mipi_dsi_cfg->u8HsPrpr    = CJSON_GETINT(target_node, "m_wHsPrpr", exist_flag, 0);
        mipi_dsi_cfg->u8HsZero    = CJSON_GETINT(target_node, "m_wHsZero", exist_flag, 0);
        mipi_dsi_cfg->u8ClkHsPrpr = CJSON_GETINT(target_node, "m_wClkHsPrpr", exist_flag, 0);
        mipi_dsi_cfg->u8ClkHsExit = CJSON_GETINT(target_node, "m_wClkHsExit", exist_flag, 0);
        mipi_dsi_cfg->u8ClkTrail  = CJSON_GETINT(target_node, "m_wClkTrail", exist_flag, 0);
        mipi_dsi_cfg->u8ClkZero   = CJSON_GETINT(target_node, "m_wClkZero", exist_flag, 0);
        mipi_dsi_cfg->u8ClkHsPost = CJSON_GETINT(target_node, "m_wClkHsPost", exist_flag, 0);
        mipi_dsi_cfg->u8DaHsExit  = CJSON_GETINT(target_node, "m_wDaHsExit", exist_flag, 0);
        mipi_dsi_cfg->u8ContDet   = CJSON_GETINT(target_node, "m_wContDet", exist_flag, 0);
        mipi_dsi_cfg->u8Lpx       = CJSON_GETINT(target_node, "m_wLpx", exist_flag, 0);
        mipi_dsi_cfg->u8TaGet     = CJSON_GETINT(target_node, "m_wTaGet", exist_flag, 0);
        mipi_dsi_cfg->u8TaSure    = CJSON_GETINT(target_node, "m_wTaSure", exist_flag, 0);
        mipi_dsi_cfg->u8TaGo      = CJSON_GETINT(target_node, "m_wTaGo", exist_flag, 0);
        mipi_dsi_cfg->u16Bllp     = CJSON_GETINT(target_node, "m_wBllp", exist_flag, 0);
        mipi_dsi_cfg->u16Fps      = CJSON_GETINT(target_node, "m_wFps", exist_flag, 0);
        disp_cfg->fps             = mipi_dsi_cfg->u16Fps;
        mipi_dsi_cfg->enLaneNum   = CJSON_GETINT(target_node, "m_eLaneNum", exist_flag, 0);
        mipi_dsi_cfg->enFormat    = CJSON_GETINT(target_node, "m_eFormat", exist_flag, 0);
        mipi_dsi_cfg->enCtrl      = CJSON_GETINT(target_node, "m_eCtrlMode", exist_flag, 0);

        mipi_dsi_cfg->u16Hactive = tgn_timing_info->u16HActive;
        mipi_dsi_cfg->u16Hpw     = tgn_timing_info->u16HSyncWidth;
        mipi_dsi_cfg->u16Hbp     = tgn_timing_info->u16HSyncBackPorch;
        mipi_dsi_cfg->u16Hfp = tgn_timing_info->u16HTotal - tgn_timing_info->u16HActive - tgn_timing_info->u16HSyncWidth
                               - tgn_timing_info->u16HSyncBackPorch;
        mipi_dsi_cfg->u16Vactive = tgn_timing_info->u16VActive;
        mipi_dsi_cfg->u16Vpw     = tgn_timing_info->u16VSyncWidth;
        mipi_dsi_cfg->u16Vbp     = tgn_timing_info->u16VSyncBackPorch;
        mipi_dsi_cfg->u16Vfp = tgn_timing_info->u16VTotal - tgn_timing_info->u16VActive - tgn_timing_info->u16VSyncWidth
                               - tgn_timing_info->u16VSyncBackPorch;

        mipi_dsi_cfg->u8SyncCalibrate = CJSON_GETINT(target_node, "m_wSyncCalibrate", exist_flag, 0);
        mipi_dsi_cfg->u16VirHsyncSt   = CJSON_GETINT(target_node, "m_wVirHsyncSt", exist_flag, 0);
        mipi_dsi_cfg->u16VirHsyncEnd  = CJSON_GETINT(target_node, "m_wVirHsyncEnd", exist_flag, 0);
        mipi_dsi_cfg->u16VsyncRef     = CJSON_GETINT(target_node, "m_wVsyncRef", exist_flag, 0);
        mipi_dsi_cfg->u16DataClkSkew  = CJSON_GETINT(target_node, "m_wDataClkSkew", exist_flag, 0);

        mipi_dsi_cfg->u8PolCh0 = CJSON_GETINT(target_node, "m_ucPolCh0", exist_flag, 0);
        mipi_dsi_cfg->u8PolCh1 = CJSON_GETINT(target_node, "m_ucPolCh1", exist_flag, 0);
        mipi_dsi_cfg->u8PolCh2 = CJSON_GETINT(target_node, "m_ucPolCh2", exist_flag, 0);
        mipi_dsi_cfg->u8PolCh3 = CJSON_GETINT(target_node, "m_ucPolCh3", exist_flag, 0);
        mipi_dsi_cfg->u8PolCh4 = CJSON_GETINT(target_node, "m_ucPolCh4", exist_flag, 0);

        mipi_dsi_cfg->enCh[0] = CJSON_GETINT(target_node, "m_ucClkLane", exist_flag, 0);
        mipi_dsi_cfg->enCh[1] = CJSON_GETINT(target_node, "m_ucDataLane0", exist_flag, 0);
        mipi_dsi_cfg->enCh[2] = CJSON_GETINT(target_node, "m_ucDataLane1", exist_flag, 0);
        mipi_dsi_cfg->enCh[3] = CJSON_GETINT(target_node, "m_ucDataLane2", exist_flag, 0);
        mipi_dsi_cfg->enCh[4] = CJSON_GETINT(target_node, "m_ucDataLane3", exist_flag, 0);

        mipi_dsi_cfg->enPacketType = CJSON_GETINT(target_node, "m_ePacketType", exist_flag, 0);

        cmd_buf_node = cJSON_GetObjectItem(target_node, "m_pCmdBuff");
        if (cmd_buf_node != NULL)
        {
            CJSON_GET_PNL_ARRAY(cmd_buf_node, "m_pCmdBuff", mipi_dsi_cfg->pu8CmdBuf, mipi_dsi_cfg->u32CmdBufSize, MI_U8,
                                exist_flag);
            cmd_buf_node = NULL;
        }
        else
        {
            BOOTLOGO_DBG(DBG_LEVEL_ERR, "Can't get m_pCmdBuff node .\n");
        }
    }
    if (HAL_DISP_INTF_LVDS == disp_cfg->disp_interface || HAL_DISP_INTF_LVDS_1 == disp_cfg->disp_interface
        || HAL_DISP_INTF_DUAL_LVDS == disp_cfg->disp_interface)
    {
        MI_DISP_IMPL_MhalPnlUnifiedLvdsTxConfig_t *pstLvdsInfo = &pnl_param_cfg->stLvdsTxInfo;
        pnl_param_cfg->u8LvdsTxFlag                            = 1;
        pstLvdsInfo->enFormat      = CJSON_GETINT(target_node, "lvds_eFormat", exist_flag, 0);
        pstLvdsInfo->eLaneNum      = CJSON_GETINT(target_node, "lvds_eLaneNum", exist_flag, 0);
        pstLvdsInfo->u8SwapOddEven = CJSON_GETINT(target_node, "lvds_u8SwapOddEven", exist_flag, 0);
        pstLvdsInfo->u8SwapML      = CJSON_GETINT(target_node, "lvds_u8SwapML", exist_flag, 0);
        pstLvdsInfo->u8PolHsync    = CJSON_GETINT(target_node, "lvds_u8PolHsync", exist_flag, 0);
        pstLvdsInfo->u8PolVsync    = CJSON_GETINT(target_node, "lvds_u8PolVsync", exist_flag, 0);
        pstLvdsInfo->u8ClkLane     = CJSON_GETINT(target_node, "lvds_u8ClkLane", exist_flag, 0);
        pstLvdsInfo->u8PolLane0    = CJSON_GETINT(target_node, "lvds_u8PolLane0", exist_flag, 0);
        pstLvdsInfo->u8PolLane1    = CJSON_GETINT(target_node, "lvds_u8PolLane1", exist_flag, 0);
        pstLvdsInfo->u8PolLane2    = CJSON_GETINT(target_node, "lvds_u8PolLane2", exist_flag, 0);
        pstLvdsInfo->u8PolLane3    = CJSON_GETINT(target_node, "lvds_u8PolLane3", exist_flag, 0);
        pstLvdsInfo->u8PolLane4    = CJSON_GETINT(target_node, "lvds_u8PolLane4", exist_flag, 0);
        pstLvdsInfo->u8SwapLane0   = CJSON_GETINT(target_node, "lvds_u8SwapLane0", exist_flag, 0);
        pstLvdsInfo->u8SwapLane1   = CJSON_GETINT(target_node, "lvds_u8SwapLane1", exist_flag, 0);
        pstLvdsInfo->u8SwapLane2   = CJSON_GETINT(target_node, "lvds_u8SwapLane2", exist_flag, 0);
        pstLvdsInfo->u8SwapLane3   = CJSON_GETINT(target_node, "lvds_u8SwapLane3", exist_flag, 0);
        pstLvdsInfo->u8SwapLane4   = CJSON_GETINT(target_node, "lvds_u8SwapLane4", exist_flag, 0);
    }

    BOOTLOGO_DBG(DBG_LEVEL_API, "<<<\n");

    return 0;
}

static MI_S32 set_backlight(MI_U32 disp_dev_num, MI_U32 disp_dev_id)
{
    bootlogo_contex *contex   = &g_bootlogo_contex;
    display_config * disp_cfg = NULL;
    MI_U32           i        = 0;

    for (i = 0; i < disp_dev_num; i++)
    {
        if (disp_dev_num == 1)
        {
            i = disp_dev_id;
        }
        disp_cfg = &contex->disp_cfgs[i];
        // backlight set
        if (!disp_cfg->backlight_inited && disp_cfg->disp_config_parsed)
        {
            if (disp_cfg->gpio_config[2].pin != PAD_UNKNOWN)
            {
                set_gpio(disp_cfg->gpio_config[2].pin, disp_cfg->gpio_config[2].value, 0);
            }
            else if (disp_cfg->backlight_config.pwm_pin != PAD_UNKNOWN)
            {
                struct udevice *dev;
                uclass_get_device(UCLASS_PWM, 0, &dev);
                pwm_set_config(dev, disp_cfg->backlight_config.pwm_pin, disp_cfg->backlight_config.period,
                               disp_cfg->backlight_config.duty_cycle);
                pwm_set_enable(dev, disp_cfg->backlight_config.pwm_pin, 1);
            }
            disp_cfg->backlight_inited = true;
        }
    }
    return 0;
}
static MI_S32 parse_display_config(bootlogo_contex *contex, MI_U32 disp_dev_id)
{
    BOOTLOGO_DBG(DBG_LEVEL_API, ">>>\n");
    display_config *disp_cfg = &contex->disp_cfgs[disp_dev_id];
    if (disp_cfg->disp_config_parsed == true)
    {
        BOOTLOGO_DBG(DBG_LEVEL_INFO, "BOOTLOGO LoadOutputTimingCfg already inited!\n");
        return 0;
    }

    cJSON *root_node      = NULL;
    cJSON *target_node    = NULL;
    cJSON *disp_node      = NULL;
    cJSON *backlight_node = NULL;

    root_node = get_json_root("config.json");
    if (!root_node)
    {
        return -1;
    }
    disp_node = cJSON_GetObjectItem(root_node, "mi_disp");
    if (disp_node == NULL)
    {
        BOOTLOGO_DBG(DBG_LEVEL_ERR, "Get disp_node failed!\n");
        return -1;
    }

    // parse backlight get
    backlight_node = cJSON_GetObjectItem(disp_node, "backlight");
    parse_backlight_config(disp_cfg, backlight_node);

    target_node = cJSON_GetObjectItem(disp_node, disp_cfg->target_node_name);
    if (target_node == NULL)
    {
        BOOTLOGO_DBG(DBG_LEVEL_INFO, "Get target_node failed!\n");
        return 0;
    }
    if (disp_cfg->disp_interface == HAL_DISP_INTF_HDMI)
    {
#ifdef CONFIG_SSTAR_HDMITX
        parse_dac_config(target_node, disp_cfg->panel_cfg);
#endif
    }
    else
    {
        parse_panel_config(target_node, disp_cfg);
    }
    disp_cfg->disp_config_parsed = true;
    BOOTLOGO_DBG(DBG_LEVEL_API, "<<<\n");
    return 0;
}

MI_S32 do_disp_init(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
    MI_S32           ret         = 0;
    MI_U32           disp_dev_id = 0;
    MI_U32           i           = 0;
    bootlogo_contex *contex      = &g_bootlogo_contex;
    display_config * disp_cfg    = NULL;
    cJSON *          root_node   = NULL;

    set_dbg_level();
    BOOTLOGO_DBG(DBG_LEVEL_INFO, "bootlogo_dbg_level: %#x\n", bootlogo_dbg_level);
    BOOTLOGO_DBG(DBG_LEVEL_API, ">>>\n");
    BOOTLOGO_PRINTARGV(argc, argv);

    ret = common_init(contex);
    /* The value ret == -1 indicates that common init is wrong.
       The value ret == -2 indicates that logo partition could not be found when empty slice upgrade. */
    if (0 != ret)
    {
        return ret;
    }

    if (argc > 2)
    {
        contex->repeat_flag = simple_strtoul(argv[2], NULL, 0);
    }
    if (argc > 1)
    {
        disp_dev_id = simple_strtoul(argv[1], NULL, 0);
    }
    disp_cfg = &contex->disp_cfgs[disp_dev_id];
    if (!disp_cfg)
    {
        BOOTLOGO_DBG(DBG_LEVEL_ERR, "disp_cfg is NULL\n");
        return -1;
    }

    if (disp_cfg->disp_inited && !contex->repeat_flag)
    {
        BOOTLOGO_DBG(DBG_LEVEL_INFO,
                     "display already init/enable."
                     "disp_inited: %d, repeat_flag: %d\n",
                     disp_cfg->disp_inited, contex->repeat_flag);
        return 0;
    }
    root_node = get_json_root("logo_configuration.json");
    if (root_node)
    {
        if (parse_logo_config(contex, root_node) == -1)
        {
            return -1;
        }
        cJSON_Delete(root_node);
        root_node = NULL;
    }

    for (i = 0; i <= disp_dev_id; i++)
    {
        disp_cfg = &contex->disp_cfgs[i];
        ret      = parse_display_config(contex, i);
        /* The value ret == -1 indicates that no available panel config.
           The value ret == -2 indicates that panel config can't be found. */
        if (0 != ret)
        {
            return ret;
        }
    }

    // BL_EXEC_FUNC(MHAL_DISP_SetDispPowerOn(), 1);
    BL_EXEC_FUNC(DRV_DISP_IF_ClkOn(), 1);
    // mdelay(1);
    cfg_data_init(contex);

    // set power/reset pin
    // deactive
    set_gpio(disp_cfg->gpio_config[0].pin, !disp_cfg->gpio_config[0].value, 0);
    set_gpio(disp_cfg->gpio_config[1].pin, !disp_cfg->gpio_config[1].value, 0);
    // set power
    set_gpio(disp_cfg->gpio_config[0].pin, disp_cfg->gpio_config[0].value, disp_cfg->gpio_config[0].delay);
    // set reset
    set_gpio(disp_cfg->gpio_config[1].pin, disp_cfg->gpio_config[1].value, disp_cfg->reset_dealy_ms);
    set_gpio(disp_cfg->gpio_config[1].pin, !disp_cfg->gpio_config[1].value, 0);
    udelay(disp_cfg->reset_during_us);
    set_gpio(disp_cfg->gpio_config[1].pin, disp_cfg->gpio_config[1].value, disp_cfg->gpio_config[1].delay);

    // init dev and layer
    MI_U32                           disp_interface = 0;
    MI_BOOL                          panel_flag     = 0;
    MI_DISP_IMPL_MhalRegFlipConfig_t reg_flip_cfg;
    memset(&reg_flip_cfg, 0, sizeof(reg_flip_cfg));
    MI_DISP_VideoLayerAttr_t layer_attr;
    memset(&layer_attr, 0, sizeof(layer_attr));
    MI_DISP_IMPL_MhalDeviceTimingInfo_t dev_timing_info;
    memset(&dev_timing_info, 0, sizeof(dev_timing_info));
    MI_DISP_SyncInfo_t disp_sync_info;
    memset(&disp_sync_info, 0, sizeof(disp_sync_info));

    disp_cfg->layer_type      = E_MI_DISP_VIDEOLAYER_MULTIWIN;
    dev_timing_info.eTimeType = disp_cfg->panel_cfg.stPnlUniParamCfg.eDisplayTiming;

    //----------disp misc config----------
    MI_DISP_IMPL_MhalDeviceConfig_t dev_Cfg;
    memset(&dev_Cfg, 0, sizeof(dev_Cfg));
    if (disp_cfg->disp_misc_cfg.color_metrix_valid_flag)
    {
        dev_Cfg.eType |= E_MI_DISP_DEV_CFG_COLORID;
        dev_Cfg.u8ColorId = disp_cfg->disp_misc_cfg.color_metrix_id;
    }
    if (disp_cfg->disp_misc_cfg.gop_dst_valid_flag)
    {
        dev_Cfg.eType |= E_MI_DISP_DEV_CFG_GOPBLENDID;
        dev_Cfg.u8GopBlendId = disp_cfg->disp_misc_cfg.gop_dst_id;
    }
    if (dev_Cfg.eType != E_MI_DISP_DEV_CFG_NONE)
    {
        BOOTLOGO_DBG(DBG_LEVEL_MHAL, "disp_dev_id: %d\n", disp_cfg->disp_dev_id);
        BOOTLOGO_DBG(DBG_LEVEL_MHAL, "eType: %d\n", dev_Cfg.eType);
        BOOTLOGO_DBG(DBG_LEVEL_MHAL, "u8ColorId: %d\n", dev_Cfg.u8ColorId);
        BOOTLOGO_DBG(DBG_LEVEL_MHAL, "u8GopBlendId: %d\n", dev_Cfg.u8GopBlendId);
        BL_EXEC_FUNC(DRV_DISP_IF_SetDeviceConfig(disp_cfg->disp_dev_id, &dev_Cfg), 1);
    }

    disp_interface = disp_cfg->disp_interface;
    if (disp_interface & HAL_DISP_INTF_TTL || disp_interface & HAL_DISP_INTF_MIPIDSI
        || disp_interface & HAL_DISP_INTF_MIPIDSI_1 || disp_interface & HAL_DISP_INTF_LVDS
        || disp_interface & HAL_DISP_INTF_LVDS_1 || disp_interface & HAL_DISP_INTF_DUAL_LVDS
        || disp_interface & HAL_DISP_INTF_BT656 || disp_interface & HAL_DISP_INTF_BT601
        || disp_interface & HAL_DISP_INTF_BT1120 || disp_interface & HAL_DISP_INTF_BT1120_DDR
        || disp_interface & HAL_DISP_INTF_MCU || disp_interface & HAL_DISP_INTF_MCU_NOFLM
        || disp_interface & HAL_DISP_INTF_SRGB)
    {
        panel_flag                  = 1;
        dev_timing_info.pstSyncInfo = &disp_sync_info;
        get_sync_info(disp_cfg, dev_timing_info.pstSyncInfo);
        dev_timing_info.eTimeType = E_MI_DISP_OUTPUT_USER;
    }

    BOOTLOGO_DBG(DBG_LEVEL_MHAL, "dev: %d, layer: %d\n", disp_cfg->disp_dev_id, disp_cfg->layer_type);
    BOOTLOGO_DBG(DBG_LEVEL_MHAL, "interface: %#x\n", disp_interface);
    BOOTLOGO_DBG(DBG_LEVEL_MHAL, "eTimeType: %d\n", dev_timing_info.eTimeType);

    MI_DISP_IMPL_MhalMemAllocConfig_t *phy_mem = &contex->phy_mem;
    BL_EXEC_FUNC(DRV_DISP_IF_DeviceCreateInstance(phy_mem, disp_cfg->disp_dev_id, &disp_cfg->dev_ctx), 1);
    BL_EXEC_FUNC(DRV_DISP_IF_VideoLayerCreateInstance(phy_mem, disp_cfg->layer_type, &disp_cfg->vid_layer_ctx), 1);
    BL_EXEC_FUNC(DRV_DISP_IF_DeviceSetBackGroundColor(disp_cfg->dev_ctx, 0x800080), 1);
    BL_EXEC_FUNC(DRV_DISP_IF_DeviceAddOutInterface(disp_cfg->dev_ctx, disp_interface), 1);
    BL_EXEC_FUNC(DRV_DISP_IF_DeviceSetOutputTiming(disp_cfg->dev_ctx, disp_interface, &dev_timing_info), 1);
    BL_EXEC_FUNC(DRV_DISP_IF_DeviceEnable(disp_cfg->dev_ctx, 1), 1);
    reg_flip_cfg.bEnable  = TRUE;
    reg_flip_cfg.pCmdqInf = NULL;
    BL_EXEC_FUNC(DRV_DISP_IF_SetRegFlipConfig(disp_cfg->dev_ctx, &reg_flip_cfg), 1);

    if (panel_flag)
    {
        BL_EXEC_FUNC(panel_init(disp_cfg), 0);
    }
    else
    {
        BL_EXEC_FUNC(hdmi_init(disp_cfg), 0);
    }

    BL_EXEC_FUNC(DRV_DISP_IF_VideoLayerBind(disp_cfg->vid_layer_ctx, disp_cfg->dev_ctx), 1);
    layer_attr.stVidLayerSize.u16Width     = disp_cfg->panel_cfg.stPnlUniParamCfg.stTgnTimingInfo.u16HActive;
    layer_attr.stVidLayerSize.u16Height    = disp_cfg->panel_cfg.stPnlUniParamCfg.stTgnTimingInfo.u16VActive;
    layer_attr.stVidLayerDispWin.u16Width  = layer_attr.stVidLayerSize.u16Width;
    layer_attr.stVidLayerDispWin.u16Height = layer_attr.stVidLayerSize.u16Height;
    BL_EXEC_FUNC(DRV_DISP_IF_VideoLayerSetAttr(disp_cfg->vid_layer_ctx, &layer_attr), 1);
    BL_EXEC_FUNC(DRV_DISP_IF_VideoLayerEnable(disp_cfg->vid_layer_ctx, 1), 1);
    BL_EXEC_FUNC(DRV_DISP_IF_SetRegFlipConfig(disp_cfg->dev_ctx, &reg_flip_cfg), 1);
    disp_cfg->disp_inited = 1;
    dcache_all();
    BOOTLOGO_DBG(DBG_LEVEL_API, "<<<\n");
    return ret;
}

MI_S32 do_disp_deinit(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
    bootlogo_contex *contex   = &g_bootlogo_contex;
    MI_S32           i        = 0;
    display_config * disp_cfg = NULL;

    BOOTLOGO_DBG(DBG_LEVEL_API, ">>>\n");
    for (i = 0; i < MAX_DISP_DEV_NUM; i++)
    {
        disp_cfg = &contex->disp_cfgs[i];
        if (disp_cfg->disp_injected)
        {
            BL_EXEC_FUNC(DRV_DISP_IF_InputPortEnable(disp_cfg->input_port_ctx, FALSE), 1);
            MI_DISP_IMPL_MhalRegFlipConfig_t reg_flip_cfg;
            memset(&reg_flip_cfg, 0, sizeof(reg_flip_cfg));
            reg_flip_cfg.bEnable  = 1;
            reg_flip_cfg.pCmdqInf = NULL;
            BL_EXEC_FUNC(DRV_DISP_IF_SetRegFlipConfig(disp_cfg->dev_ctx, &reg_flip_cfg), 1);
            BL_EXEC_FUNC(DRV_DISP_IF_InputPortDestroyInstance(disp_cfg->input_port_ctx), 1);
            disp_cfg->disp_injected = false;
        }
#ifdef CONFIG_SSTAR_RGN
        gop_deinit(disp_cfg);
#endif
        if (disp_cfg->disp_inited)
        {
            BL_EXEC_FUNC(DRV_DISP_IF_VideoLayerEnable(disp_cfg->vid_layer_ctx, FALSE), 1);
            BL_EXEC_FUNC(DRV_DISP_IF_VideoLayerDestoryInstance(disp_cfg->vid_layer_ctx), 1);
            panel_deinit(disp_cfg);
            BL_EXEC_FUNC(DRV_DISP_IF_DeviceEnable(disp_cfg->dev_ctx, FALSE), 1);
            BL_EXEC_FUNC(DRV_DISP_IF_DeviceDestroyInstance(disp_cfg->dev_ctx), 1);
            disp_cfg->disp_inited = false;
        }
#ifdef CONFIG_SSTAR_HDMITX
        if (disp_cfg->hdmitx_ctx)
        {
            hdmi_deinit(disp_cfg);
        }
#endif

        if (contex->pq_data_addr[0])
        {
            free((void *)contex->pq_data_addr[0]);
            contex->pq_data_addr[0] = 0;
            contex->pq_data_addr[1] = 0;
            contex->pq_size         = 0;
        }
    }
    fs_unmount();
    if (contex->disp_buf_info.phy_addr == 0)
    {
        if (contex->ui_info[0].decode_data[0])
        {
            free((void *)contex->ui_info[0].decode_data[0]);
        }
        if (contex->ui_info[1].decode_data[0])
        {
            free((void *)contex->ui_info[1].decode_data[0]);
        }
    }
    memset(contex, 0, sizeof(bootlogo_contex));
    BOOTLOGO_DBG(DBG_LEVEL_API, "<<<\n");
    return 0;
}

MI_S32 do_disp_decode(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
    bootlogo_contex * contex        = &g_bootlogo_contex;
    disp_image_info * image_info    = NULL;
    disp_buffer_info *disp_buf_info = &contex->disp_buf_info;
    MI_U32            logo_id       = 0;
    MI_U32            disp_dev_id   = 0;
    display_config *  disp_cfg      = NULL;
    display_config *  disp_cfg_old  = NULL;

    BOOTLOGO_DBG(DBG_LEVEL_API, ">>>\n");
    BOOTLOGO_PRINTARGV(argc, argv);
    if (!contex->common_inited)
    {
        BOOTLOGO_DBG(DBG_LEVEL_ERR, "Common resource don't init. bInit: %d\n", contex->common_inited);
        return -1;
    }

    if (argc > 2)
    {
        disp_dev_id = simple_strtoul(argv[2], NULL, 0);
    }
    if (argc > 1)
    {
        logo_id = simple_strtoul(argv[1], NULL, 0);
    }
    BOOTLOGO_DBG(DBG_LEVEL_INFO, "disp_dev_id %d, logo_id: %d\n", disp_dev_id, logo_id);
    disp_cfg = &contex->disp_cfgs[disp_dev_id];
    if (!disp_cfg)
    {
        BOOTLOGO_DBG(DBG_LEVEL_ERR, "disp_cfg is NULL\n");
        return -1;
    }
    if (logo_id >= MAX_LOGO_FILE_NUM)
    {
        BOOTLOGO_DBG(DBG_LEVEL_ERR, "logo_id[%d] is out of range\n", logo_id);
        return -1;
    }
    image_info = &contex->logo_files[logo_id];
    if (disp_dev_id > 0)
    {
        disp_cfg_old = &contex->disp_cfgs[disp_dev_id - 1];
        if ((disp_cfg->rotate == disp_cfg_old->rotate) && (image_info->decoded_flag == true))
        {
            BOOTLOGO_DBG(DBG_LEVEL_INFO, "logo_id: %d already decode!\n", logo_id);
            return 0;
        }
    }
    if (image_info->file_data[0] || (strlen(image_info->file_path) == 0))
    {
        BOOTLOGO_DBG(DBG_LEVEL_ERR, "Empty input logo file!\n");
        return -1;
    }

    if (jpg_header_tail_detection(image_info, image_info->file_path) == -1)
    {
        return -2;
    }

    if (image_info->file_data[0])
    {
        image_info->file_data[1] = BOOTLOGO_TO_PHYA(image_info->file_data[0]);
    }

    // need pixel fmt after decode done
    image_info->pixel_format = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;

#ifdef CONFIG_SSTAR_RGN
    image_info->pixel_format = E_MI_SYS_PIXEL_FRAME_ARGB8888;
#endif

    image_info->decode_data[0] = (MI_U32)jpg_decode(
        image_info->file_size, (MI_U8 *)image_info->file_data[0], &image_info->decode_data_size, &image_info->img_width,
        &image_info->img_height, &image_info->img_crop_width, &image_info->img_crop_height, &image_info->img_crop_x,
        &image_info->img_crop_y, disp_cfg->rotate, disp_buf_info, image_info->pixel_format);

    image_info->decode_data[1] = (MI_U32)BOOTLOGO_TO_PHYA(image_info->decode_data[0]);
    image_info->img_stride     = image_info->img_width;

    if (image_info->img_width % DISP_BASE_ALIGN != 0)
    {
        BOOTLOGO_DBG(DBG_LEVEL_WRN, "img_width %d is not align[%d]\n", image_info->img_width, DISP_BASE_ALIGN);
    }
    BOOTLOGO_DBG(DBG_LEVEL_INFO, "file_data %x %x %d\n", image_info->file_data[0], image_info->file_data[1],
                 image_info->file_size);
    BOOTLOGO_DBG(DBG_LEVEL_INFO, "decode_data %x %x %d\n", image_info->decode_data[0], image_info->decode_data[1],
                 image_info->decode_data_size);
    BOOTLOGO_DBG(DBG_LEVEL_INFO, "resolution %d %d %d\n", image_info->img_stride, image_info->img_width,
                 image_info->img_height);
    BOOTLOGO_DBG(DBG_LEVEL_INFO, "pixel_format %d, rotate: %d\n", image_info->pixel_format, disp_cfg->rotate);

    dcache_all();
    image_info->decoded_flag = true;
    // free filedata
    if (image_info->file_data[0])
    {
        free((void *)image_info->file_data[0]);
        image_info->file_data[0] = 0;
        image_info->file_data[1] = 0;
        image_info->file_size    = 0;
    }

    BOOTLOGO_DBG(DBG_LEVEL_API, "<<<\n");
    return 0;
}

MI_S32 do_disp_inject(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
    bootlogo_contex *        contex        = &g_bootlogo_contex;
    MI_S32                   disp_dev_id   = 0;
    MI_S32                   timing_width  = 0;
    MI_S32                   timing_height = 0;
    MI_DISP_VideoLayerAttr_t layer_attr;
    memset(&layer_attr, 0, sizeof(layer_attr));
    MI_DISP_IMPL_MhalVideoFrameData_t Video_frame_buffer;
    memset(&Video_frame_buffer, 0, sizeof(Video_frame_buffer));
    MI_DISP_InputPortAttr_t input_attr;
    memset(&input_attr, 0, sizeof(input_attr));
    disp_image_info *    image_info = NULL;
    MI_DISP_VidWinRect_t vid_winrect;
    memset(&vid_winrect, 0, sizeof(MI_DISP_VidWinRect_t));
    display_config *disp_cfg = NULL;
    MI_U32          dst_x = 0, dst_y = 0;

    BOOTLOGO_DBG(DBG_LEVEL_API, ">>>\n");
    BOOTLOGO_PRINTARGV(argc, argv);

    if (argc > 5)
    {
        disp_dev_id = simple_strtoul(argv[5], NULL, 0);
    }
    BOOTLOGO_DBG(DBG_LEVEL_INFO, "[disp_dev_id: %d]\n", disp_dev_id);
    disp_cfg = &contex->disp_cfgs[disp_dev_id];
    if (!disp_cfg)
    {
        BOOTLOGO_DBG(DBG_LEVEL_ERR, "disp_cfg is NULL\n");
        return -1;
    }
    if (disp_cfg->disp_injected == true)
    {
        BOOTLOGO_DBG(DBG_LEVEL_INFO, "display already inject!\n");
        return 0;
    }
    if (!contex->common_inited || !disp_cfg->disp_inited)
    {
        BOOTLOGO_DBG(DBG_LEVEL_ERR,
                     "Common resource don't init, and display don't init."
                     "bInit: %d, disp_inited: %d\n",
                     contex->common_inited, disp_cfg->disp_inited);
        return -1;
    }

    if (argc > 4)
    {
        dst_y = simple_strtoul(argv[4], NULL, 0);
    }
    if (argc > 3)
    {
        dst_x = simple_strtoul(argv[3], NULL, 0);
    }
    if (argc > 2)
    {
        disp_cfg->aspect_ratio = simple_strtoul(argv[2], NULL, 0);
    }
    if (argc > 1)
    {
        disp_cfg->logo_id = simple_strtoul(argv[1], NULL, 0);
    }

    BOOTLOGO_DBG(DBG_LEVEL_INFO, "logo_id: %d, out_aspect_ratio: %d, dst_x: %d, dst_y: %d\n", disp_cfg->logo_id,
                 disp_cfg->aspect_ratio, dst_x, dst_y);
    if (disp_cfg->logo_id >= MAX_LOGO_FILE_NUM)
    {
        BOOTLOGO_DBG(DBG_LEVEL_ERR, "logo_id[%d] is out of range\n", disp_cfg->logo_id);
        return -1;
    }

    image_info              = &contex->logo_files[disp_cfg->logo_id];
    input_attr.u16SrcWidth  = image_info->img_width;
    input_attr.u16SrcHeight = image_info->img_height;
    get_timing_size(disp_cfg, &timing_width, &timing_height);
    BOOTLOGO_DBG(DBG_LEVEL_INFO, "timing_width: %d, timing_height: %d\n", timing_width, timing_height);

    vid_winrect.u16X               = image_info->img_crop_x;
    vid_winrect.u16Y               = image_info->img_crop_y;
    vid_winrect.u16Width           = image_info->img_crop_width;
    vid_winrect.u16Height          = image_info->img_crop_height;
    MI_DISP_VidWinRect_t *disp_win = &input_attr.stDispWin;
    if (disp_cfg->aspect_ratio == ASPECT_RATIO_CENTER)
    {
        disp_win->u16X      = (timing_width - input_attr.u16SrcWidth) / 2;
        disp_win->u16Y      = (timing_height - input_attr.u16SrcHeight) / 2;
        disp_win->u16Width  = input_attr.u16SrcWidth;
        disp_win->u16Height = input_attr.u16SrcHeight;
    }
    else if (disp_cfg->aspect_ratio == ASPECT_RATIO_USER)
    {
        disp_win->u16X      = dst_x;
        disp_win->u16Y      = dst_y;
        disp_win->u16Width  = input_attr.u16SrcWidth;
        disp_win->u16Height = input_attr.u16SrcHeight;
    }
    else // ASPECT_RATIO_ZOOM
    {
        disp_win->u16X      = 0;
        disp_win->u16Y      = 0;
        disp_win->u16Width  = timing_width;
        disp_win->u16Height = timing_height;
    }

    Video_frame_buffer.ePixelFormat  = image_info->pixel_format;
    Video_frame_buffer.au32Stride[0] = image_info->img_stride;
    Video_frame_buffer.au32Stride[1] = Video_frame_buffer.au32Stride[0];
    Video_frame_buffer.aPhyAddr[0]   = image_info->decode_data[1];
    Video_frame_buffer.aPhyAddr[1] =
        Video_frame_buffer.aPhyAddr[0] + (Video_frame_buffer.au32Stride[0] * input_attr.u16SrcHeight);

    BOOTLOGO_DBG(DBG_LEVEL_MHAL, "[layer:%d][hw_port:%d] addr:%llx-%llx, stride:%d-%d, pixelformat:%d\n",
                 disp_cfg->layer_type, disp_cfg->port_id, Video_frame_buffer.aPhyAddr[0],
                 Video_frame_buffer.aPhyAddr[1], Video_frame_buffer.au32Stride[0], Video_frame_buffer.au32Stride[1],
                 Video_frame_buffer.ePixelFormat);
    BOOTLOGO_DBG(DBG_LEVEL_MHAL, "[layer:%d][hw_port:%d] srcw:%d, srch:%d, show:%d-%d-%d-%d\n", disp_cfg->layer_type,
                 disp_cfg->port_id, input_attr.u16SrcWidth, input_attr.u16SrcHeight, disp_win->u16X, disp_win->u16Y,
                 disp_win->u16Width, disp_win->u16Height);
    if (!timing_width || !timing_height || !input_attr.u16SrcWidth || !input_attr.u16SrcHeight || !disp_win->u16Width
        || !disp_win->u16Height || !Video_frame_buffer.au32Stride[0] || !Video_frame_buffer.aPhyAddr[0])
    {
        BOOTLOGO_DBG(DBG_LEVEL_ERR, "param error\n");
        return -1;
    }
    if (timing_width < (disp_win->u16X + disp_win->u16Width) || timing_height < (disp_win->u16Y + disp_win->u16Height))
    {
        BOOTLOGO_DBG(DBG_LEVEL_WRN, "Warning: disp window > timing window\n");
        disp_win->u16X      = 0;
        disp_win->u16Y      = 0;
        disp_win->u16Width  = BL_MIN(disp_win->u16Width, timing_width);
        disp_win->u16Height = BL_MIN(disp_win->u16Height, timing_height);
    }
    if (timing_width < disp_win->u16Width || timing_height < disp_win->u16Height)
    {
        BOOTLOGO_DBG(DBG_LEVEL_WRN, "Warning: disp window > timing window\n");
        disp_win->u16Width  = BL_MIN(disp_win->u16Width, timing_width);
        disp_win->u16Height = BL_MIN(disp_win->u16Height, timing_height);
    }
    if (input_attr.u16SrcWidth > disp_win->u16Width || input_attr.u16SrcHeight > disp_win->u16Height)
    {
        BOOTLOGO_DBG(DBG_LEVEL_WRN, "Warning: src window > disp window\n");
        vid_winrect.u16Width  = BL_MIN(image_info->img_crop_width, disp_win->u16Width);
        vid_winrect.u16Height = BL_MIN(image_info->img_crop_height, disp_win->u16Height);
    }
    BOOTLOGO_DBG(DBG_LEVEL_MHAL, "[layer:%d][hw_port:%d]  show:%d-%d-%d-%d\n", disp_cfg->layer_type, disp_cfg->port_id,
                 disp_win->u16X, disp_win->u16Y, disp_win->u16Width, disp_win->u16Height);
    BOOTLOGO_DBG(DBG_LEVEL_MHAL, "[layer:%d][hw_port:%d] crop:%d-%d-%d-%d\n", disp_cfg->layer_type, disp_cfg->port_id,
                 vid_winrect.u16X, vid_winrect.u16Y, vid_winrect.u16Width, vid_winrect.u16Height);

#ifdef CONFIG_SSTAR_RGN
    gop_init(disp_cfg, &input_attr, &Video_frame_buffer, &vid_winrect);
#else
    mop_init(disp_cfg, image_info, &input_attr, &Video_frame_buffer, &vid_winrect);
#endif
    disp_cfg->image_info    = image_info;
    disp_cfg->disp_injected = true;
    BOOTLOGO_DBG(DBG_LEVEL_API, "<<<\n");
    return 0;
}

MI_S32 do_bootlogo(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
    MI_S32 ret            = 0;
    char   logo_id_str[2] = {}, aspect_ratio_str[2] = {};
    char   x_str[5] = {}, y_str[5] = {};
    char   disp_dev_id_str[2] = {};
    char   repeat_str[2]      = {};
    MI_U32 argv_ints[6]       = {0};
    MI_U32 i = 0, j = 0, disp_dev_num = 1;
    MI_U32 pre_time = timer_get_us();

    bootstage_mark_name(1, "bootlogo+");

    set_dbg_level();
    BOOTLOGO_DBG(DBG_LEVEL_API, ">>>\n");
    BOOTLOGO_PRINTARGV(argc, argv);

    if (argc > 7)
    {
        BOOTLOGO_DBG(DBG_LEVEL_ERR, "invalid cmdline.\n");
        return -1;
    }
    if (argc < 6)
    {
        disp_dev_num = MAX_DISP_DEV_NUM;
    }
    for (i = 0; i < disp_dev_num; i++)
    {
        for (j = 1; j < argc; j++)
        {
            argv_ints[j - 1] = simple_strtol(argv[j], NULL, 0);
        }
        if (disp_dev_num == MAX_DISP_DEV_NUM)
        {
            argv_ints[4] = i;
        }
        if (argv_ints[4] >= MAX_DISP_DEV_NUM)
        {
            BOOTLOGO_DBG(DBG_LEVEL_ERR, "disp_dev_id %d is invalid\n", argv_ints[4]);
            return -1;
        }

        sprintf(logo_id_str, "%d", argv_ints[0]);
        sprintf(aspect_ratio_str, "%d", argv_ints[1]);
        sprintf(x_str, "%d", argv_ints[2]);
        sprintf(y_str, "%d", argv_ints[3]);
        sprintf(disp_dev_id_str, "%d", argv_ints[4]);
        sprintf(repeat_str, "%d", argv_ints[5]);

        // init disp
        char *init_argv[] = {"disp_init", disp_dev_id_str, repeat_str};
        ret               = do_disp_init(cmdtp, flag, sizeof(init_argv) / sizeof(init_argv[0]), init_argv);
        if (-1 == ret)
        {
            return ret;
        }
        if (-2 == ret)
        {
            /*  The value ret == -2 indicates that panel config can't be found or logo partition could not be found.
                The logo partition could not be found when empty slice upgrade.
                To keep the upgrade up and running,we retuen 0 here.
            */
            ret = 0;
            break;
        }

        // decode image buffer
        char *decode_argv[] = {"disp_decode", logo_id_str, disp_dev_id_str};
        ret                 = do_disp_decode(cmdtp, flag, sizeof(decode_argv) / sizeof(decode_argv[0]), decode_argv);
        if (-1 == ret)
        {
            return ret;
        }
        if (-2 == ret)
        {
            /*  If the user has stored the wrong picture,we want to not jam the boot process,so return 0 here. */
            ret = 0;
            break;
        }

        // inject buffer
        char *inject_argv[] = {"disp_inject", logo_id_str, aspect_ratio_str, x_str, y_str, disp_dev_id_str};
        if (-1 == do_disp_inject(cmdtp, flag, sizeof(inject_argv) / sizeof(inject_argv[0]), inject_argv))
        {
            return -1;
        }
    }

    set_backlight(disp_dev_num, argv_ints[4]);
    printf("cost time: %lu us\n", (timer_get_us() - pre_time));
    BOOTLOGO_DBG(DBG_LEVEL_API, "<<<\n");
    bootstage_mark_name(1, "bootlogo-");
    return ret;
}

#ifdef CONFIG_SSTAR_RGN
MI_S32 do_disp_ui(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
    MI_U32 ret         = -1;
    MI_U32 disp_dev_id = 0;
    MI_U32 i = 0, disp_dev_num = 1;
    char   disp_dev_id_str[2] = {};
    memset(disp_dev_id_str, 0, sizeof(disp_dev_id_str));
    MI_U32 bar_color   = 0xff00;   // green  //bit[23, 0] rgb888
    MI_U32 text_color  = 0xffffff; // white
    MI_U32 bg_color    = 0x9dceff; // blue
    MI_U32 blank_color = bg_color;
    MI_U32 x = 0, y = 0, w = 0, h = 0;
    MI_U32 percent = 0;

    bootlogo_contex *contex   = &g_bootlogo_contex;
    MI_U32           pre_time = timer_get_us();

    BOOTLOGO_DBG(DBG_LEVEL_API, ">>>\n");
    BOOTLOGO_PRINTARGV(argc, argv);

    if (argc < 3 || argc > 9)
    {
        BOOTLOGO_DBG(DBG_LEVEL_ERR, "invalid cmdline.\n");
        return -1;
    }
    if (argc > 6 && !strcmp(argv[1], "bar"))
    {
        disp_dev_id = simple_strtoul(argv[6], NULL, 0);
    }
    else if (argc > 7 && !strcmp(argv[1], "msg"))
    {
        disp_dev_id = simple_strtoul(argv[7], NULL, 0);
    }
    else if (argc > 8 && !strcmp(argv[1], "blank"))
    {
        disp_dev_id = simple_strtoul(argv[8], NULL, 0);
    }
    else
    {
        disp_dev_num = MAX_DISP_DEV_NUM;
    }
    if (disp_dev_id >= MAX_DISP_DEV_NUM)
    {
        BOOTLOGO_DBG(DBG_LEVEL_ERR, "disp_dev_id %d is invalid\n", disp_dev_id);
        return -1;
    }

    for (i = 0; i < disp_dev_num; i++)
    {
        if (disp_dev_num == MAX_DISP_DEV_NUM)
        {
            disp_dev_id = i;
        }
        sprintf(disp_dev_id_str, "%d", disp_dev_id);

        char *_argv[] = {"disp_init", disp_dev_id_str};
        ret           = do_disp_init(cmdtp, flag, sizeof(_argv) / sizeof(_argv[0]), _argv);
        if (-1 == ret)
        {
            return ret;
        }
        if (-2 == ret)
        {
            /*  The value ret == -2 indicates that panel config can't be found or logo partition could not be found.
                The logo partition could not be found when empty slice upgrade.
                To keep the upgrade up and running,we retuen 0 here.
            */
            ret = 0;
            break;
        }

        if (-1 == ui_init(disp_dev_id))
        {
            return -1;
        }

        if (!strcasecmp("bar", argv[1]))
        {
            if (argc > 5)
            {
                bg_color = simple_strtoul(argv[5], NULL, 0);
            }
            if (argc > 4)
            {
                text_color = simple_strtoul(argv[4], NULL, 0);
            }
            if (argc > 3)
            {
                bar_color = simple_strtoul(argv[3], NULL, 0);
            }
            if (argc > 2)
            {
                percent = simple_strtoul(argv[2], NULL, 0);
            }
            ret = show_ui_bar(contex, percent, bar_color, text_color, bg_color, disp_dev_id);
        }
        else if (!strcasecmp("msg", argv[1]))
        {
            if (argc > 6)
            {
                bg_color = simple_strtoul(argv[6], NULL, 0);
            }
            if (argc > 5)
            {
                text_color = simple_strtoul(argv[5], NULL, 0);
            }
            if (argc > 4)
            {
                y = simple_strtoul(argv[4], NULL, 0);
            }
            if (argc > 3)
            {
                x = simple_strtoul(argv[3], NULL, 0);
            }
            ret = show_ui_msg(contex, argv[2], x, y, text_color, bg_color, disp_dev_id);
        }
        else if (!strcasecmp("blank", argv[1]))
        {
            if (argc > 6)
            {
                blank_color = simple_strtoul(argv[6], NULL, 0);
            }
            if (argc > 5)
            {
                h = simple_strtoul(argv[5], NULL, 0);
            }
            if (argc > 4)
            {
                w = simple_strtoul(argv[4], NULL, 0);
            }
            if (argc > 3)
            {
                y = simple_strtoul(argv[3], NULL, 0);
            }
            if (argc > 2)
            {
                x = simple_strtoul(argv[2], NULL, 0);
            }
            ret = show_ui_blank(contex, x, y, w, h, blank_color, disp_dev_id);
        }
        else
        {
            BOOTLOGO_DBG(DBG_LEVEL_ERR, "Param error! plz [help disp_ui]\n");
        }
    }

    set_backlight(disp_dev_num, disp_dev_id);
    printf("cost time: %lu us\n", (timer_get_us() - pre_time));
    BOOTLOGO_DBG(DBG_LEVEL_API, "<<<\n");
    return ret;
}

MI_S32 do_disp_ui_update(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
    MI_S32 ret                = 0;
    MI_U32 disp_dev_id        = 0;
    char   disp_dev_id_str[2] = {};
    MI_U32 i = 0, disp_dev_num = 1;
    memset(disp_dev_id_str, 0, sizeof(disp_dev_id_str));

    bootlogo_contex *contex   = &g_bootlogo_contex;
    MI_U32           pre_time = timer_get_us();

    BOOTLOGO_DBG(DBG_LEVEL_API, ">>>\n");
    BOOTLOGO_PRINTARGV(argc, argv);

    if (argc < 2 || argc > 3)
    {
        BOOTLOGO_DBG(DBG_LEVEL_ERR, "invalid cmdline.\n");
        return -1;
    }
    if (argc == 2)
    {
        disp_dev_num = MAX_DISP_DEV_NUM;
    }

    for (i = 0; i < disp_dev_num; i++)
    {
        if (disp_dev_num == MAX_DISP_DEV_NUM)
        {
            disp_dev_id = i;
        }
        else
        {
            disp_dev_id = simple_strtoul(argv[2], NULL, 0);
        }
        if (disp_dev_id >= MAX_DISP_DEV_NUM)
        {
            BOOTLOGO_DBG(DBG_LEVEL_ERR, "disp_dev_id %d is invalid\n", disp_dev_id);
            return -1;
        }
        sprintf(disp_dev_id_str, "%d", disp_dev_id);

        char *_argv[] = {"disp_init", disp_dev_id_str};
        ret           = do_disp_init(cmdtp, flag, sizeof(_argv) / sizeof(_argv[0]), _argv);
        if (-1 == ret)
        {
            return ret;
        }
        if (-2 == ret)
        {
            /*  The value ret == -2 indicates that panel config can't be found or logo partition could not be found.
                The logo partition could not be found when empty slice upgrade.
                To keep the upgrade up and running,we retuen 0 here.
            */
            ret = 0;
            break;
        }

        if (-1 == ui_init(disp_dev_id))
        {
            return -1;
        }

        char *ui_update_argv[] = {argv[1], disp_dev_id_str};
        if (-1
            == show_ui_update(contex, sizeof(ui_update_argv) / sizeof(ui_update_argv[0]),
                              (char *const *)ui_update_argv))
        {
            return -1;
        }
    }

    set_backlight(disp_dev_num, disp_dev_id);
    printf("cost time: %lu us\n", (timer_get_us() - pre_time));
    BOOTLOGO_DBG(DBG_LEVEL_API, "<<<\n");
    return ret;
}
#endif

#if (defined CONFIG_SSTAR_BOOTLOGO_UT)
MI_S32 do_run_cmds(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
    MI_S32           ret    = -1;
    MI_S32           i      = 0;
    bootlogo_contex *contex = &g_bootlogo_contex;
    cmd_table_config cmdtable_cfg;
    cmd_table *      cmdtable = NULL;
    char             cmdtable_path[MAX_STR_LEN];
    char             cmdtable_name[MAX_STR_LEN];
    cJSON *          root_node = NULL;

    MI_U32 pre_time = timer_get_us();
    set_dbg_level();
    BOOTLOGO_DBG(DBG_LEVEL_API, ">>>\n");
    BOOTLOGO_PRINTARGV(argc, argv);
    memset(&cmdtable_cfg, 0, sizeof(cmd_table_config));
    strcpy(cmdtable_path, "config.json");
    strcpy(cmdtable_name, "bootcmds_hdmi");

    if (argc >= 2)
    {
        strcpy(cmdtable_name, argv[1]);
    }
    if (argc >= 3)
    {
        strcpy(cmdtable_path, argv[2]);
    }

    if (common_init(contex) == -1)
    {
        return ret;
    }
    root_node = get_json_root(cmdtable_path);
    if (!root_node)
    {
        return ret;
    }
    if (parse_logo_config(contex, root_node) == -1)
    {
        return ret;
    }
    if (load_boot_cmds(root_node, &cmdtable_cfg) == -1)
    {
        return ret;
    }

    cJSON_Delete(root_node);
    root_node = NULL;

    BOOTLOGO_DBG(DBG_LEVEL_INFO, "cmds: %s\n", cmdtable_name);

    cmdtable = bootlogo_get_cmdtable_by_cmdname(&cmdtable_cfg, cmdtable_name);
    if (cmdtable == NULL)
    {
        return ret;
    }

    for (i = 0; i < cmdtable->cmdtable_lines; i++)
    {
        BOOTLOGO_DBG(DBG_LEVEL_INFO, "Start running: %s \n", cmdtable->cmdtable_data[i]);
        run_command((char *)(cmdtable->cmdtable_data[i]), 0);
    }
    ret = 0;
    printf("cost time: %lu us\n", (timer_get_us() - pre_time));
    BOOTLOGO_DBG(DBG_LEVEL_API, "<<<\n");

    return ret;
}

static char sz_run_cmds[] =
    "run_cmds [cmds]\n"
    "\t cmds :user custom cmds; default is bootcmds_hdmi\n";
#endif

static char sz_bootlogo[] =
    "bootlogo [logo_id] [aspect ratio] [x] [y] [device_id] [repeat]\n"
    "\t logo_id: image id. default is 0\n"
    "\t aspect_ratio: 0: zoom, 1: center, 2: usr. default is 0\n"
    "\t x: show horizontal start. it's valid when aspect_ratio is 2. default is 0\n"
    "\t y: show vertical start. it's valid when aspect_ratio is 2. default is 0\n"
    "\t device_id: devcie ID. default is 0\n"
    "\t repeat. 0:not repeat 1:repeat. default is 0\n"
    "\t Example:\n"
    "\t bootlogo \n"
    "\t bootlogo 1 \n"
    "\t bootlogo 0 0 \n"
    "\t bootlogo 0 1 \n"
    "\t bootlogo 0 2 32 64 0 \n"
    "\t Note:\n"
    "\t bootlogo inner flow: disp_init -> disp_decode -> disp_inject\n";

static char sz_disp_init[] =
    "disp_init [device_id] [repeat]\n"
    "\t device_id: devcie ID. default is 0\n"
    "\t repeat. 0:not repeat 1:repeat. default is 0\n";

static char sz_disp_decode[] =
    "disp_decode [logo_id] [device_id]\n"
    "\t logo_id: image id. default is 0\n"
    "\t device_id: devcie ID. default is 0\n";

static char sz_disp_inject[] =
    "disp_inject [logo_id] [aspect ratio] [x] [y] [device_id]\n"
    "\t logo_id: image id. default is 0\n"
    "\t aspect_ratio: 0: zoom, 1: center, 2: usr. default is 0\n"
    "\t x: show horizontal start. it's valid when aspect_ratio is 2. default is 0\n"
    "\t y: show vertical start. it's valid when aspect_ratio is 2. default is 0\n"
    "\t device_id: devcie ID. default is 0\n";
#ifdef CONFIG_SSTAR_RGN
static char sz_disp_ui[] =
    "disp_ui [bar/msg/blank] [percentage] [bar_color] [text_color] [bg_color] [device_id]\n"
    "\t bar: sub cmd. \n"
    "\t percentage: percentage. range [0, 100]\n"
    "\t bar_color: show bar color. fmt is rgb(bit[23, 0]). default is green\n"
    "\t text_color: show text color. default is white\n"
    "\t bg_color: show bar background. default is blue\n"
    "\t device_id: devcie ID. default is 0\n"
    "\t Example:\n"
    "\t disp_ui bar 10 \n"
    "\t disp_ui bar 50 \n"
    "\t disp_ui bar 100 \n"
    "\t disp_ui msg I am UI\n"
    "\t disp_ui [msg] [text] [x] [y] [text_color] [bg_color] [DevId]\n"
    "\t msg: sub cmd. \n"
    "\t text: ascii text. default is blank\n"
    "\t x: show text horizontal start. default is 0\n"
    "\t y: show text vertical start. default is 0\n"
    "\t text_color: show text color. default is white\n"
    "\t bg_color: show bar background. default is blue\n"
    "\t DevId: devcie ID. default is 0.\n"
    "\t Example:\n"
    "\t disp_ui msg update \n"
    "\t \n"
    "\t disp_ui [blank] [x] [y] [w] [h] [BlankColor] [DevId]\n"
    "\t Example:\n"
    "\t disp_ui blank 0 0 20 20 0xFFFFFF \n";

static char sz_disp_ui_update[] =
    "\t disp_ui_update [percentage/success/fail] [device_id]\n"
    "\t percentage: percentage. range [0, 100]\n"
    "\t device_id: devcie ID. default is 0\n"
    "\t Example:\n"
    "\t disp_ui_update 10 \n"
    "\t disp_ui_update success \n"
    "\t disp_ui_update fail \n";
#endif

U_BOOT_CMD(disp_init, CONFIG_SYS_MAXARGS, 1, do_disp_init, "disp init", sz_disp_init);
U_BOOT_CMD(disp_deinit, CONFIG_SYS_MAXARGS, 1, do_disp_deinit, "disp deinit", NULL);
U_BOOT_CMD(disp_decode, CONFIG_SYS_MAXARGS, 1, do_disp_decode, "disp decode image", sz_disp_decode);
U_BOOT_CMD(disp_inject, CONFIG_SYS_MAXARGS, 1, do_disp_inject, "disp inject image", sz_disp_inject);
U_BOOT_CMD(bootlogo, CONFIG_SYS_MAXARGS, 1, do_bootlogo, "show bootlogo", sz_bootlogo);
#ifdef CONFIG_SSTAR_RGN
U_BOOT_CMD(disp_ui, CONFIG_SYS_MAXARGS, 1, do_disp_ui, "show ui", sz_disp_ui);
U_BOOT_CMD(disp_ui_update, CONFIG_SYS_MAXARGS, 1, do_disp_ui_update, "disp ui update", sz_disp_ui_update);
#endif
#ifdef CONFIG_SSTAR_BOOTLOGO_UT
U_BOOT_CMD(run_cmds, CONFIG_SYS_MAXARGS, 1, do_run_cmds, "run cmds", sz_run_cmds);
#endif
