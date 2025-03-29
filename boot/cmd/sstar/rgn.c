/*
 * rgn.c - Sigmastar
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

#include <command.h>
#include <common.h>
#include <malloc.h>
#include "drv_rgn_os.h"
#include "mhal_cmdq.h"
#include "mhal_common.h"
#include "mhal_rgn_datatype.h"
#include "mhal_rgn.h"

struct gop_config
{
    u32 in_width;
    u32 in_height;
    u32 out_width;
    u32 out_height;
    u32 device_id;
};

static void *rgn_gop_ctx = NULL;

static char rgn_help_text[] =
    "gop [in width] [in height] [out width] [out height] [device id]\n"
    "dbgmg [level]\n";

void gopshow(struct gop_config *gop_cfg)
{
#define GOP_GWIN_ID 0

    s32 s32Ret = MHAL_SUCCESS;

    printf("%s %d, In(%d, %d) Out(%d %d) DeviceId:%d\n", __FUNCTION__, __LINE__, gop_cfg->in_width, gop_cfg->in_height,
           gop_cfg->out_width, gop_cfg->out_height, gop_cfg->device_id);

    if (rgn_gop_ctx == NULL)
    {
        u32 device_id = gop_cfg->device_id == 0 ? 1 : gop_cfg->device_id == 1 ? 0 : device_id;
        s32Ret        = MhalRgnGopCreate(E_MHAL_RGN_GOP_TYPE_DISP_UI, device_id, &rgn_gop_ctx);
    }

    if (s32Ret == MHAL_SUCCESS && rgn_gop_ctx)
    {
        MhalRgnGopStretchWinConfig_t   stGopStretchWinCfg;
        MhalRgnGopGwinPixelFmtConfig_t stGopPixelFmtCfg;
        MhalRgnGopGwinWinConfig_t      stGopWinCfg;
        MhalRgnGopGwinAlphaConfig_t    stGopAlphaCfg;
        MhalRgnGopGwinBufferConfig_t   stGopBufferCfg;
        MhalRgnGopGwinOnOffConfig_t    stGopOnOffCfg;

        stGopStretchWinCfg.tSrcWinCfg.u32X      = 0;
        stGopStretchWinCfg.tSrcWinCfg.u32Y      = 0;
        stGopStretchWinCfg.tSrcWinCfg.u32Width  = gop_cfg->out_width;
        stGopStretchWinCfg.tSrcWinCfg.u32Height = gop_cfg->out_height;

        stGopStretchWinCfg.tDestWinCfg.u32X      = 0;
        stGopStretchWinCfg.tDestWinCfg.u32Y      = 0;
        stGopStretchWinCfg.tDestWinCfg.u32Width  = gop_cfg->out_width;
        stGopStretchWinCfg.tDestWinCfg.u32Height = gop_cfg->out_height;

        stGopPixelFmtCfg.u32GwinId    = GOP_GWIN_ID;
        stGopPixelFmtCfg.ePixelFormat = E_MHAL_GOP_PIXEL_FORMAT_ARGB1555;

        stGopWinCfg.u32GwinId         = GOP_GWIN_ID;
        stGopWinCfg.u32Stride         = gop_cfg->in_width * 2;
        stGopWinCfg.tWinCfg.u32X      = 0;
        stGopWinCfg.tWinCfg.u32Y      = 0;
        stGopWinCfg.tWinCfg.u32Width  = gop_cfg->in_width;
        stGopWinCfg.tWinCfg.u32Height = gop_cfg->in_height;

        stGopAlphaCfg.u32GwinId       = GOP_GWIN_ID;
        stGopAlphaCfg.eAlphaType      = E_MHAL_GOP_GWIN_ALPHA_CONSTANT;
        stGopAlphaCfg.u8ConstAlphaVal = 0x00;

        stGopBufferCfg.u32GwinId  = GOP_GWIN_ID;
        stGopBufferCfg.u16Xoffset = 0;
        stGopBufferCfg.phyAddr    = 0x1234556;

        stGopOnOffCfg.u32GwinId = GOP_GWIN_ID;
        stGopOnOffCfg.bEn       = 1;

        MhalRgnGopActive(rgn_gop_ctx);
        MhalRgnGopSetBaseWindow(rgn_gop_ctx, &stGopStretchWinCfg);
        MhalRgnGopGwinSetPixelFormat(rgn_gop_ctx, &stGopPixelFmtCfg);
        MhalRgnGopGwinSetWindow(rgn_gop_ctx, &stGopWinCfg);
        MhalRgnGopGwinSetAlphaType(rgn_gop_ctx, &stGopAlphaCfg);
        MhalRgnGopGwinSetBuffer(rgn_gop_ctx, &stGopBufferCfg);
        MhalRgnGopGwinSetOnOff(rgn_gop_ctx, &stGopOnOffCfg);
        MhalRgnGopProcess(rgn_gop_ctx);
    }
    else
    {
        printf("%s %d, MhalRgnGopCreate Fail\n", __FUNCTION__, __LINE__);
    }
}

int do_rgn(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
    struct gop_config gop_cfg;

    if (argc >= 2 && !strcmp(argv[1], "gop"))
    {
        gop_cfg.in_width   = simple_strtoul(argv[2], NULL, 10);
        gop_cfg.in_height  = simple_strtoul(argv[3], NULL, 10);
        gop_cfg.out_width  = simple_strtoul(argv[4], NULL, 10);
        gop_cfg.out_height = simple_strtoul(argv[5], NULL, 10);
        gop_cfg.device_id  = simple_strtoul(argv[6], NULL, 10);
        gopshow(&gop_cfg);
    }
    else if (argc >= 2 && !strcmp(argv[1], "dbgmg"))
    {
        u32 u32DbgLevel = simple_strtoul(argv[2], NULL, 16);
        MhalRgnSetDbgLevel(&u32DbgLevel);
    }
    else if (!strcmp(argv[1], "delinst"))
    {
        if (rgn_gop_ctx)
        {
            MhalRgnGopDeActive(rgn_gop_ctx);
            MhalRgnGopDestory(rgn_gop_ctx);
            rgn_gop_ctx = NULL;
        }
        else
        {
            printf("%s %d, NULL Point\n", __FUNCTION__, __LINE__);
        }
    }
    else
    {
        printf("%s", rgn_help_text);
    }
    return 0;
}

U_BOOT_CMD(rgn, CONFIG_SYS_MAXARGS, 1, do_rgn, "rgn command", rgn_help_text);
