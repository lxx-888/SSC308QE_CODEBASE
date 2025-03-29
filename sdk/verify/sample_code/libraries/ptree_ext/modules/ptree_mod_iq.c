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

#include "isp_cus3a_if.h"
#include "ssos_def.h"
#include "ssos_list.h"
#include "ptree_log.h"
#include "ssos_mem.h"
#include "ptree_sur_iq.h"
//#include "mi_iqserver.h"
#include "mi_isp_cus3a_api.h"
#include "ptree_mod_sys.h"
#include "ptree_mod_iq.h"
#include "ptree_maker.h"

#define LOG_INFO 0

#define PTREE_MOD_IQ_DEV_NUM 2

typedef struct PTREE_MOD_IQ_Obj_s PTREE_MOD_IQ_Obj_t;

struct PTREE_MOD_IQ_Obj_s
{
    PTREE_MOD_SYS_Obj_t base;
};

static int                 _PTREE_MOD_IQ_Init(PTREE_MOD_SYS_Obj_t *sysMod);
static int                 _PTREE_MOD_IQ_Deinit(PTREE_MOD_SYS_Obj_t *sysMod);
static PTREE_MOD_InObj_t * _PTREE_MOD_IQ_CreateModIn(PTREE_MOD_Obj_t *mod, unsigned int loopId);
static PTREE_MOD_OutObj_t *_PTREE_MOD_IQ_CreateModOut(PTREE_MOD_Obj_t *mod, unsigned int loopId);
static void                _PTREE_MOD_IQ_Free(PTREE_MOD_SYS_Obj_t *sysMod);

static const PTREE_MOD_SYS_Ops_t G_PTREE_MOD_IQ_SYS_OPS = {
    .init         = _PTREE_MOD_IQ_Init,
    .deinit       = _PTREE_MOD_IQ_Deinit,
    .createModIn  = _PTREE_MOD_IQ_CreateModIn,
    .createModOut = _PTREE_MOD_IQ_CreateModOut,
};
static const PTREE_MOD_SYS_Hook_t G_PTREE_MOD_IQ_SYS_HOOK = {
    .destruct = NULL,
    .free     = _PTREE_MOD_IQ_Free,
};

static MI_U32 g_u32Cus3aCreateDev[PTREE_MOD_IQ_DEV_NUM] = {0};

static int _PTREE_MOD_IQ_Cus3aAeInit(void *pdata, ISP_AE_INIT_PARAM *pInitState)
{
    UNUSED(pdata);
    PTREE_DBG("**ae_init ,shutter=%d,shutter_step=%d,sensor_gain_min=%d,sensor_gain_max=%d **", pInitState->shutter,
              pInitState->shutter_step, pInitState->sensor_gain, pInitState->sensor_gain_max);
    return 0;
}

static void _PTREE_MOD_IQ_Cus3aAeRelease(void *pdata)
{
    UNUSED(pdata);
    PTREE_DBG("**ae_release **");
}

static void _PTREE_MOD_IQ_Cus3aAeRun(void *pdata, const ISP_AE_INFO *pInfo, ISP_AE_RESULT *pResult)
{
    unsigned int u32Max         = pInfo->AvgBlkY * pInfo->AvgBlkX;
    unsigned int u32Avg         = 0;
    unsigned int u32Idx         = 0;
    unsigned int u32yLower      = 0x48;
    unsigned int u32yUpper      = 0x58;
    unsigned int u32ChangeRatio = 6; // percentage
    unsigned int u32GainMin     = 1024 * 2;
    unsigned int u32GainMax     = 1024 * 1000;
    unsigned int u32ShutterMin  = 150;
    unsigned int u32ShutterMax  = 33333;

    UNUSED(pdata);

    pResult->Change             = 0;
    pResult->i4BVx16384         = 16384;
    pResult->HdrRatio           = 16384; // user define hdr exposure ratio
    pResult->HdrRatio1          = 1024;
    pResult->IspGain            = 1024;
    pResult->SensorGain         = 4096;
    pResult->Shutter            = 20000;
    pResult->IspGainHdrShort    = 1024;
    pResult->SensorGainHdrShort = 1024;
    pResult->ShutterHdrShort    = 1000;

    for (u32Idx = 0; u32Idx < u32Max; ++u32Idx)
    {
        u32Avg += pInfo->avgs[u32Idx].y;
    }

    if (u32Max > 1)
    {
        u32Avg /= u32Max;
    }

    pResult->AvgY       = u32Avg;
    pResult->SensorGain = pInfo->SensorGain;
    pResult->Shutter    = pInfo->Shutter;

    if (u32Avg < u32yLower)
    {
        if (pInfo->Shutter < u32ShutterMax)
        {
            pResult->Shutter = pInfo->Shutter + (pInfo->Shutter * u32ChangeRatio / 100);
            if (pResult->Shutter > u32ShutterMax)
            {
                pResult->Shutter = u32ShutterMax;
            }
        }
        else
        {
            pResult->SensorGain = pInfo->SensorGain + (pInfo->SensorGain * u32ChangeRatio / 100);
            if (pResult->SensorGain > u32GainMax)
            {
                pResult->SensorGain = u32GainMax;
            }
        }
        pResult->Change = 1;
    }
    else if (u32Avg > u32yUpper)
    {
        if (pInfo->SensorGain > u32GainMin)
        {
            pResult->SensorGain = pInfo->SensorGain - (pInfo->SensorGain * u32ChangeRatio / 100);
            if (pResult->SensorGain < u32GainMin)
            {
                pResult->SensorGain = u32GainMin;
            }
        }
        else
        {
            pResult->Shutter = pInfo->Shutter - (pInfo->Shutter * u32ChangeRatio / 100);
            if (pResult->Shutter < u32ShutterMin)
            {
                pResult->Shutter = u32ShutterMin;
            }
        }
        pResult->Change = 1;
    }
#if LOG_INFO
    PTREE_DBG("Image avg = 0x%X ", u32Avg);
    PTREE_DBG("SensorGain: %d -> %d ", pInfo->SensorGain, pResult->SensorGain);
    PTREE_DBG("Shutter: %d -> %d ", pInfo->Shutter, pResult->Shutter);
#endif
}

static int _PTREE_MOD_IQ_Cus3aAwbInit(void *pdata, ISP_AWB_INIT_PARAM *param)
{
    UNUSED(pdata);
    UNUSED(param);
    PTREE_DBG("**awb_init **");
    return 0;
}

static void _PTREE_MOD_IQ_Cus3aAwbRun(void *pdata, const ISP_AWB_INFO *pInfo, ISP_AWB_RESULT *pResult)
{
    unsigned int u32Idx      = 0;
    int          s32AvgR     = 0;
    int          s32AvgG     = 0;
    int          s32AvgB     = 0;
    int          s32TarRgain = 1024;
    int          s32TarBgain = 1024;
    int          x           = 0;
    int          y           = 0;

    UNUSED(pdata);

    pResult->R_gain   = pInfo->CurRGain;
    pResult->G_gain   = pInfo->CurGGain;
    pResult->B_gain   = pInfo->CurBGain;
    pResult->Change   = 0;
    pResult->ColorTmp = 6000;

    if (++u32Idx % 4 == 0)
    {
        // center area YR/G/B avg
        for (y = 30; y < 60; ++y)
        {
            for (x = 32; x < 96; ++x)
            {
                s32AvgR += pInfo->avgs[pInfo->AvgBlkX * y + x].r;
                s32AvgG += pInfo->avgs[pInfo->AvgBlkX * y + x].g;
                s32AvgB += pInfo->avgs[pInfo->AvgBlkX * y + x].b;
            }
        }
        s32AvgR /= 30 * 64;
        s32AvgG /= 30 * 64;
        s32AvgB /= 30 * 64;

        if (s32AvgR < 1)
        {
            s32AvgR = 1;
        }
        if (s32AvgG < 1)
        {
            s32AvgG = 1;
        }
        if (s32AvgB < 1)
        {
            s32AvgB = 1;
        }

#if LOG_INFO
        PTREE_DBG("AVG R / G / B = %d, %d, %d ", s32AvgR, s32AvgG, s32AvgB);
#endif

        // calculate Rgain, Bgain
        s32TarRgain = s32AvgG * 1024 / s32AvgR;
        s32TarBgain = s32AvgG * 1024 / s32AvgB;

        if (s32TarRgain > pInfo->CurRGain)
        {
            if (s32TarRgain - pInfo->CurRGain < 384)
            {
                pResult->R_gain = s32TarRgain;
            }
            else
            {
                pResult->R_gain = pInfo->CurRGain + (s32TarRgain - pInfo->CurRGain) / 10;
            }
        }
        else
        {
            if (pInfo->CurRGain - s32TarRgain < 384)
            {
                pResult->R_gain = s32TarRgain;
            }
            else
            {
                pResult->R_gain = pInfo->CurRGain - (pInfo->CurRGain - s32TarRgain) / 10;
            }
        }

        if (s32TarBgain > pInfo->CurBGain)
        {
            if (s32TarBgain - pInfo->CurBGain < 384)
            {
                pResult->B_gain = s32TarBgain;
            }
            else
            {
                pResult->B_gain = pInfo->CurBGain + (s32TarBgain - pInfo->CurBGain) / 10;
            }
        }
        else
        {
            if (pInfo->CurBGain - s32TarBgain < 384)
            {
                pResult->B_gain = s32TarBgain;
            }
            else
            {
                pResult->B_gain = pInfo->CurBGain - (pInfo->CurBGain - s32TarBgain) / 10;
            }
        }

        pResult->Change = 1;
        pResult->G_gain = 1024;

#if LOG_INFO
        PTREE_DBG("[awb input] r=%d, g=%d, b=%d ", pInfo->CurRGain, pInfo->CurGGain, pInfo->CurBGain);
        PTREE_DBG("[awb output] r=%d, g=%d, b=%d ", pResult->R_gain, pResult->G_gain, pResult->B_gain);
#endif
    }
}

static void _PTREE_MOD_IQ_Cus3aAwbRelease(void *pdata)
{
    UNUSED(pdata);
    PTREE_DBG("** awb_release **");
}

static int _PTREE_MOD_IQ_Cus3aAfInit(void *pdata, ISP_AF_INIT_PARAM *param)
{
#if 0
    int *data  = (int *)pdata;
    int  dev   = data[0];
    int  u32ch = data[1];
    PTREE_DBG("************ af_init **********");

    UNUSED(param);

#define USE_NORMAL_MODE 1

#ifdef USE_NORMAL_MODE
    //_Init Normal mode setting
    CusAFWin_t afwin = {AF_ROI_MODE_NORMAL,
                        1,
                        {{0, 0, 255, 255},
                         {256, 0, 511, 255},
                         {512, 0, 767, 255},
                         {768, 0, 1023, 255},
                         {0, 256, 255, 511},
                         {256, 256, 511, 511},
                         {512, 256, 767, 511},
                         {768, 256, 1023, 511},
                         {0, 512, 255, 767},
                         {256, 512, 511, 767},
                         {512, 512, 767, 767},
                         {768, 512, 1023, 767},
                         {0, 768, 255, 1023},
                         {256, 768, 511, 1023},
                         {512, 768, 767, 1023},
                         {768, 768, 1023, 1023}}};

    MI_ISP_CUS3A_SetAFWindow((CUS3A_ISP_DEV_e)dev, (CUS3A_ISP_CH_e)u32ch, &afwin);
#else
    //_Init Matrix mode setting
    CusAFWin_t afwin = {
        // full image, equal divide to 16x16
        // window setting need to multiple of two
        AF_ROI_MODE_MATRIX,
        16,
        {{0, 0, 62, 62},
         {64, 64, 126, 126},
         {128, 128, 190, 190},
         {192, 192, 254, 254},
         {256, 256, 318, 318},
         {320, 320, 382, 382},
         {384, 384, 446, 446},
         {448, 448, 510, 510},
         {512, 512, 574, 574},
         {576, 576, 638, 638},
         {640, 640, 702, 702},
         {704, 704, 766, 766},
         {768, 768, 830, 830},
         {832, 832, 894, 894},
         {896, 896, 958, 958},
         {960, 960, 1022, 1022}}

        /*
        //use two row only => 16x2 win
        //and set taf_roimode.u32_vertical_block_number = 2
        E_IQ_AF_ROI_MODE_MATRIX,
        2,
        {{0, 0, 62, 62},
        {64, 64, 126, 126},
        {128, 0, 190, 2},      //win2 v_str, v_end doesn't use, set to (0, 2)
        {192, 0, 254, 2},
        {256, 0, 318, 2},
        {320, 0, 382, 2},
        {384, 0, 446, 2},
        {448, 0, 510, 2},
        {512, 0, 574, 2},
        {576, 0, 638, 2},
        {640, 0, 702, 2},
        {704, 0, 766, 2},
        {768, 0, 830, 2},
        {832, 0, 894, 2},
        {896, 0, 958, 2},
        {960, 0, 1022, 2}}
        */
    };

    MI_ISP_CUS3A_SetAFWindow((CUS3A_ISP_DEV_e)dev, (CUS3A_ISP_CH_e)u32ch, &afwin);
#endif

    // set AF Filter
    CusAFFilter_t affilter = {
        // filter setting with sign value
        //{s9, s10, s9, s13, s13}

        //[0.3~0.6]  : 20,  37,  20, -4352, 3392; 20, -39,  20,  2176, 3264; 26, 0, -26, -1152, 2432;
        //[0.2~0.5]  : 20,  35,  20, -6144, 3520; 20, -39,  20,   -64, 2304; 26, 0, -26, -3328, 2432;
        //[0.1~0.6]  : 37,   0, -37, -6848, 3136; 37,   0, -37,  1600, 1792; 32, 0, -32, -2624,    0;
        //[0.08~0.24]: 13,  11,  13, -5568, 3456; 13, -26,  13, -7680, 3840; 15, 0, -15, -6592, 3200;
        //[0.03~0.25]: 19,   0, -19, -7808, 3776; 19,   0, -19, -4672, 2304; 17, 0, -17, -5824, 1920;
        //[0.07~0.1] :  8, -13,   8, -7680, 3968;  8, -16,   8, -7936, 4032;  3, 0,  -3, -7744, 3904;

        // here use [0.3~0.6] & [0.08~0.24]
        // convert to hw format (sign bit with msb)
        20,   37,          20,   4352 + 8192, 3392, 0,        1023,        0,           1023, 13,        11,
        13,   5568 + 8192, 3456, 0,           1023, 0,        1023,        1,           20,   39 + 1024, 20,
        2176, 3264,        1,    26,          0,    26 + 512, 1152 + 8192, 2432,        1,    13,        26 + 1024,
        13,   7680 + 8192, 3840, 1,           15,   0,        15 + 512,    6592 + 8192, 3200,
    };
    MI_ISP_CUS3A_SetAFFilter(dev, 0, &affilter);

    // set AF Sq
    CusAFFilterSq_t sq = {
        .bSobelYSatEn      = 0,
        .u16SobelYThd      = 1023,
        .bIIRSquareAccEn   = 1,
        .bSobelSquareAccEn = 0,
        .u16IIR1Thd        = 0,
        .u16IIR2Thd        = 0,
        .u16SobelHThd      = 0,
        .u16SobelVThd      = 0,
        .u8AFTbl1X =
            {
                6,
                7,
                7,
                6,
                6,
                6,
                7,
                6,
                6,
                7,
                6,
                6,
            },
        .u16AFTbl1Y = {0, 32, 288, 800, 1152, 1568, 2048, 3200, 3872, 4607, 6271, 7199, 8191},
        .u8AFTbl2X =
            {
                6,
                7,
                7,
                6,
                6,
                6,
                7,
                6,
                6,
                7,
                6,
                6,
            },
        .u16AFTbl2Y = {0, 32, 288, 800, 1152, 1568, 2048, 3200, 3872, 4607, 6271, 7199, 8191},
    };

    MI_ISP_CUS3A_SetAFFilterSq(dev, 0, &sq);

#if MOTOR_TEST
    mod1_isp_af_motor_init();
#endif
#endif //__linux__
    UNUSED(pdata);
    UNUSED(param);
    PTREE_DBG("**** af_init done ****");
    return 0;
}

static void _PTREE_MOD_IQ_Cus3aAfRelease(void *pdata)
{
    UNUSED(pdata);
    PTREE_DBG("**af_release **");
}

static void _PTREE_MOD_IQ_Cus3aAfRun(void *pdata, const ISP_AF_INFO *pInfo, ISP_AF_RESULT *pResult)
{
#if LOG_INFO
    int i = 0, x = 0;
    x = 0;
    // for (i = 0; i < 16; i++)
    for (i = 0; i < 2; i++)
    {
        PTREE_DBG(
            "[AF]win%d-%d iir0:0x%02x%02x%02x%02x%02x, iir1:0x%02x%02x%02x%02x%02x, luma:0x%02x%02x%02x%02x, "
            "sobelh:0x%02x%02x%02x%02x%02x, sobelv:0x%02x%02x%02x%02x%02x ysat:0x%02x%02x%02x",
            x, i, pInfo->pStats->stParaAPI[x].high_iir[4 + i * 5], pInfo->pStats->stParaAPI[x].high_iir[3 + i * 5],
            pInfo->pStats->stParaAPI[x].high_iir[2 + i * 5], pInfo->pStats->stParaAPI[x].high_iir[1 + i * 5],
            pInfo->pStats->stParaAPI[x].high_iir[0 + i * 5], pInfo->pStats->stParaAPI[x].low_iir[4 + i * 5],
            pInfo->pStats->stParaAPI[x].low_iir[3 + i * 5], pInfo->pStats->stParaAPI[x].low_iir[2 + i * 5],
            pInfo->pStats->stParaAPI[x].low_iir[1 + i * 5], pInfo->pStats->stParaAPI[x].low_iir[0 + i * 5],
            pInfo->pStats->stParaAPI[x].luma[3 + i * 4], pInfo->pStats->stParaAPI[x].luma[2 + i * 4],
            pInfo->pStats->stParaAPI[x].luma[1 + i * 4], pInfo->pStats->stParaAPI[x].luma[0 + i * 4],
            pInfo->pStats->stParaAPI[x].sobel_h[4 + i * 5], pInfo->pStats->stParaAPI[x].sobel_h[3 + i * 5],
            pInfo->pStats->stParaAPI[x].sobel_h[2 + i * 5], pInfo->pStats->stParaAPI[x].sobel_h[1 + i * 5],
            pInfo->pStats->stParaAPI[x].sobel_h[0 + i * 5], pInfo->pStats->stParaAPI[x].sobel_v[4 + i * 5],
            pInfo->pStats->stParaAPI[x].sobel_v[3 + i * 5], pInfo->pStats->stParaAPI[x].sobel_v[2 + i * 5],
            pInfo->pStats->stParaAPI[x].sobel_v[1 + i * 5], pInfo->pStats->stParaAPI[x].sobel_v[0 + i * 5],
            pInfo->pStats->stParaAPI[x].ysat[2 + i * 3], pInfo->pStats->stParaAPI[x].ysat[1 + i * 3],
            pInfo->pStats->stParaAPI[x].ysat[0 + i * 3]);
    }
#else
    UNUSED(pInfo);
#endif
    UNUSED(pdata);
    UNUSED(pResult);
}

static int _PTREE_MOD_IQ_Init(PTREE_MOD_SYS_Obj_t *sysMod)
{
    PTREE_SUR_IQ_Info_t *         iqInfo  = CONTAINER_OF(sysMod->base.info, PTREE_SUR_IQ_Info_t, base.base);
    CUS3A_ISP_DEV_e               eIspDev = 0;
    CUS3A_ISP_CH_e                eIspChn = 0;
    int                           i       = 0;
    MI_ISP_IQ_ParamInitInfoType_t status;

    eIspDev = (CUS3A_ISP_DEV_e)iqInfo->base.base.devId;
    eIspChn = (CUS3A_ISP_CH_e)iqInfo->base.base.chnId;

    if (!g_u32Cus3aCreateDev[iqInfo->base.base.devId])
    {
        if (1 == iqInfo->u8OpenIqServer)
        {
            // MI_IQSERVER_Open();
            PTREE_DBG("iq server open");
        }
        PTREE_DBG("CUS3A_Init>> SEC %s", iqInfo->base.base.sectionName);
        /*CUS3A global init*/
        // CUS3A_Init(eIspDev);
    }
    g_u32Cus3aCreateDev[iqInfo->base.base.devId]++;
    memset(&status, 0x0, sizeof(MI_ISP_IQ_ParamInitInfoType_t));
    MI_ISP_IQ_GetParaInitStatus(eIspDev, eIspChn, &status);
    if (1 != status.stParaAPI.bFlag)
    {
        PTREE_ERR("check isp & cus3a status failed! not load iq file!\n");
        return SSOS_DEF_FAIL;
    }
    for (i = 0; i < iqInfo->u32CaliCfgCnt; i++)
    {
        PTREE_DBG("isp(%d, %d) item: %d load califile(%s)\n", eIspDev, eIspChn, iqInfo->arrCaliCfgs[i].caliItem,
                  (char *)iqInfo->arrCaliCfgs[i].caliFile);
        MI_ISP_ApiCmdLoadCaliData(eIspDev, eIspChn, (MI_ISP_IQ_CaliItem_e)iqInfo->arrCaliCfgs[i].caliItem,
                                  (char *)iqInfo->arrCaliCfgs[i].caliFile);
    }

    if (0 == iqInfo->u8InitCus3a)
    {
        PTREE_DBG("Cus3a not open");
        return SSOS_DEF_OK;
    }

    /*CUS3A cteate channel*/
    // CUS3A_CreateChannel(eIspDev, eIspChn);

    if (!strcmp(iqInfo->cus3aType, "sigma"))
    {
        PTREE_DBG("== PTREE CUS3A DISABLE ==");

        CUS3A_SetAlgoAdaptor(eIspDev, eIspChn, E_ALGO_ADAPTOR_NATIVE, E_ALGO_TYPE_AE);
        CUS3A_SetAlgoAdaptor(eIspDev, eIspChn, E_ALGO_ADAPTOR_NATIVE, E_ALGO_TYPE_AWB);
        CUS3A_SetAlgoAdaptor(eIspDev, eIspChn, E_ALGO_ADAPTOR_NATIVE, E_ALGO_TYPE_AF);

        CUS3A_RegInterfaceEX(eIspDev, eIspChn, E_ALGO_ADAPTOR_1, E_ALGO_TYPE_AE, NULL);
        CUS3A_RegInterfaceEX(eIspDev, eIspChn, E_ALGO_ADAPTOR_1, E_ALGO_TYPE_AWB, NULL);
        CUS3A_RegInterfaceEX(eIspDev, eIspChn, E_ALGO_ADAPTOR_1, E_ALGO_TYPE_AF,
                             NULL); // if no need ,set algo to NULL
    }
    else if (!strcmp(iqInfo->cus3aType, "cus")) // use customer 3A
    {
        ISP_AE_INTERFACE   tAeIf;
        ISP_AWB_INTERFACE  tAwbIf;
        ISP_AF_INTERFACE   tAfIf;
        ISP_AE_INTERFACE * pAeIf      = NULL;
        ISP_AWB_INTERFACE *pAwbIf     = NULL;
        ISP_AF_INTERFACE * pAfIf      = NULL;
        MI_BOOL            bAEenable  = iqInfo->u8Cus3aAe;
        MI_BOOL            bAFenable  = iqInfo->u8Cus3aAf;
        MI_BOOL            bAWBenable = iqInfo->u8Cus3aAwb;
        int                s32IqModInfo[2];

        PTREE_DBG("== PTREE CUS3A ENABLE == , %d %d %d", bAEenable, bAWBenable, bAFenable);

        if (bAEenable)
        {
            /*AE*/
            s32IqModInfo[0] = eIspDev;
            s32IqModInfo[1] = eIspChn;
            tAeIf.ctrl      = NULL;
            tAeIf.pdata     = s32IqModInfo;
            tAeIf.init      = _PTREE_MOD_IQ_Cus3aAeInit;
            tAeIf.release   = _PTREE_MOD_IQ_Cus3aAeRelease;
            tAeIf.run       = _PTREE_MOD_IQ_Cus3aAeRun;
            pAeIf           = &tAeIf;

            CUS3A_RegInterfaceEX(eIspDev, eIspChn, E_ALGO_ADAPTOR_1, E_ALGO_TYPE_AE, pAeIf);
            CUS3A_SetAlgoAdaptor(eIspDev, eIspChn, E_ALGO_ADAPTOR_1, E_ALGO_TYPE_AE);
        }
        if (bAWBenable)
        {
            /*AWB*/
            s32IqModInfo[0] = eIspDev;
            s32IqModInfo[1] = eIspChn;
            tAwbIf.ctrl     = NULL;
            tAwbIf.pdata    = s32IqModInfo;
            tAwbIf.init     = _PTREE_MOD_IQ_Cus3aAwbInit;
            tAwbIf.release  = _PTREE_MOD_IQ_Cus3aAwbRelease;
            tAwbIf.run      = _PTREE_MOD_IQ_Cus3aAwbRun;
            pAwbIf          = &tAwbIf;

            CUS3A_RegInterfaceEX(eIspDev, eIspChn, E_ALGO_ADAPTOR_1, E_ALGO_TYPE_AWB, pAwbIf);
            CUS3A_SetAlgoAdaptor(eIspDev, eIspChn, E_ALGO_ADAPTOR_1, E_ALGO_TYPE_AWB);
        }
        if (bAFenable)
        {
            /*AF*/
            s32IqModInfo[0] = eIspDev;
            s32IqModInfo[1] = eIspChn;
            tAfIf.pdata     = s32IqModInfo;
            tAfIf.init      = _PTREE_MOD_IQ_Cus3aAfInit;
            tAfIf.release   = _PTREE_MOD_IQ_Cus3aAfRelease;
            tAfIf.run       = _PTREE_MOD_IQ_Cus3aAfRun;
            pAfIf           = &tAfIf;

            CUS3A_RegInterfaceEX(eIspDev, eIspChn, E_ALGO_ADAPTOR_1, E_ALGO_TYPE_AF,
                                 pAfIf); // if no need ,set algo to NULL
            CUS3A_SetAlgoAdaptor(eIspDev, eIspChn, E_ALGO_ADAPTOR_1, E_ALGO_TYPE_AF);
        }
    }

    return SSOS_DEF_OK;
}
static int _PTREE_MOD_IQ_Deinit(PTREE_MOD_SYS_Obj_t *sysMod)
{
    PTREE_SUR_IQ_Info_t *iqInfo = CONTAINER_OF(sysMod->base.info, PTREE_SUR_IQ_Info_t, base.base);

    if (1 == iqInfo->u8OpenIqServer)
    {
        // MI_IQSERVER_Close();
        PTREE_DBG("iq server close");
    }

    g_u32Cus3aCreateDev[iqInfo->base.base.devId]--;
    if (!g_u32Cus3aCreateDev[iqInfo->base.base.devId])
    {
        // CUS3A_Release(iqInfo->base.base.devId);
    }

    return SSOS_DEF_OK;
}
static PTREE_MOD_InObj_t *_PTREE_MOD_IQ_CreateModIn(PTREE_MOD_Obj_t *mod, unsigned int loopId)
{
    (void)mod;
    (void)loopId;
    return NULL;
}
static PTREE_MOD_OutObj_t *_PTREE_MOD_IQ_CreateModOut(PTREE_MOD_Obj_t *mod, unsigned int loopId)
{
    (void)mod;
    (void)loopId;
    return NULL;
}
static void _PTREE_MOD_IQ_Free(PTREE_MOD_SYS_Obj_t *sysMod)
{
    SSOS_MEM_Free(CONTAINER_OF(sysMod, PTREE_MOD_IQ_Obj_t, base));
}

PTREE_MOD_Obj_t *PTREE_MOD_IQ_New(PARENA_Tag_t *tag)
{
    PTREE_MOD_IQ_Obj_t *iqMod = NULL;

    iqMod = SSOS_MEM_Alloc(sizeof(PTREE_MOD_IQ_Obj_t));
    if (!iqMod)
    {
        PTREE_ERR("Alloc err");
        goto ERR_MEM_ALLOC;
    }
    memset(iqMod, 0, sizeof(PTREE_MOD_IQ_Obj_t));
    if (SSOS_DEF_OK != PTREE_MOD_SYS_ObjInit(&iqMod->base, &G_PTREE_MOD_IQ_SYS_OPS, tag, E_MI_MODULE_ID_IQSERVER))
    {
        goto ERR_MOD_SYS_INIT;
    }

    if (iqMod->base.base.info->devId >= PTREE_MOD_IQ_DEV_NUM)
    {
        PTREE_ERR("Dev id %d is not support, max number is %d", iqMod->base.base.info->devId, PTREE_MOD_IQ_DEV_NUM);
        goto ERR_DEV_OUT_OF_RANGE;
    }

    PTREE_MOD_SYS_ObjRegister(&iqMod->base, &G_PTREE_MOD_IQ_SYS_HOOK);
    return &iqMod->base.base;

ERR_DEV_OUT_OF_RANGE:
    PTREE_MOD_ObjDel(&iqMod->base.base);
ERR_MOD_SYS_INIT:
    SSOS_MEM_Free(iqMod);
ERR_MEM_ALLOC:
    return NULL;
}

PTREE_MAKER_MOD_INIT(IQ, PTREE_MOD_IQ_New);
