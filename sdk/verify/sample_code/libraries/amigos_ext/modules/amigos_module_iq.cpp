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
#include <unistd.h>
#include "mi_isp.h"
#include "mi_isp_cus3a_api.h"
#include "isp_cus3a_if.h"
#include "mi_iqserver.h"
#include "amigos_module_init.h"
#include "amigos_module_iq.h"

std::map<unsigned int, unsigned int> AmigosModuleIq::mapIqCreateDev;

static int mod1_isp_ae_init(void *pdata, ISP_AE_INIT_PARAM *init_state)
{
    AMIGOS_INFO("****[%s] ae_init ,shutter=%d,shutter_step=%d,sensor_gain_min=%d,sensor_gain_max=%d ****\n",
              __FUNCTION__,
              init_state->shutter,
              init_state->shutter_step,
              init_state->sensor_gain,
              init_state->sensor_gain_max
             );
    return 0;
}

static void mod1_isp_ae_release(void *pdata)
{
     AMIGOS_INFO("****[%s] cus3e release ****\n", __FUNCTION__);
}

static void mod1_isp_ae_run(void *pdata, const ISP_AE_INFO *info, ISP_AE_RESULT *result)
{
#define log_info 0

    // Only one can be chosen (the following three define)
#define shutter_test  0
#define gain_test     0
#define AE_sample     1


    //static int AE_period = 4;
    //static unsigned int fcount = 0;
    unsigned int max = info->AvgBlkY*info->AvgBlkX;
    unsigned int avg=0;
    unsigned int n;

    result->Change              = 0;
    result->i4BVx16384          = 16384;
    result->HdrRatio            = 16384; //user define hdr exposure ratio
    result->HdrRatio1           = 1024;
    result->IspGain             = 1024;
    result->SensorGain          = 4096;
    result->Shutter             = 20000;
    result->IspGainHdrShort     = 1024;
    result->SensorGainHdrShort  = 1024;
    result->ShutterHdrShort     = 1000;
    //result->Size         = sizeof(CusAEResult_t);

    for(n=0; n<max; ++n)
    {
        avg += info->avgs[n].y;
    }
    if(max)
    {
        avg /= max;
    }

    result->AvgY         = avg;
//#if shutter_test // shutter test under constant sensor gain
#if 0
    MI_U32 Shutter_Step = 100; //per frame
    MI_U32 Shutter_Max = 33333;
    MI_U32 Shutter_Min = 150;
    MI_U32 Gain_Constant = 10240;
    static int tmp=0;
    static unsigned int fcount = 0;
    result->SensorGain = Gain_Constant;
    result->Shutter = info->Shutter;

    if(++fcount%AE_period == 0)
    {
        if (tmp==0)
        {
            result->Shutter = info->Shutter + Shutter_Step*AE_period;
            // AMIGOS_INFO("[shutter-up] result->Shutter = %d \n", result->SensorGain);
        }
        else
        {
            result->Shutter = info->Shutter - Shutter_Step*AE_period;
            // AMIGOS_INFO("[shutter-down] result->Shutter = %d \n", result->SensorGain);
        }
        if (result->Shutter >= Shutter_Max)
        {
            result->Shutter = Shutter_Max;
            tmp=1;
        }
        if (result->Shutter <= Shutter_Min)
        {
            result->Shutter = Shutter_Min;
            tmp=0;
        }
    }
#if log_info
     AMIGOS_INFO("fcount = %d, Image avg = 0x%X \n", fcount, avg);
     AMIGOS_INFO("tmp = %d, Shutter: %d -> %d \n", tmp, info->Shutter, result->Shutter);
#endif
#endif

//#if gain_test // gain test under constant shutter
#if 0
    MI_U32 Gain_Step = 1024; //per frame
    MI_U32 Gain_Max = 1024*100;
    MI_U32 Gain_Min = 1024*2;
    MI_U32 Shutter_Constant = 20000;
    static int tmp1=0;
    result->SensorGain = info->SensorGain;
    result->Shutter = Shutter_Constant;

    if(++fcount%AE_period == 0)
    {
        if (tmp1==0)
        {
            result->SensorGain = info->SensorGain + Gain_Step*AE_period;
            // AMIGOS_INFO("[gain-up] result->SensorGain = %d \n", result->SensorGain);
        }
        else
        {
            result->SensorGain = info->SensorGain - Gain_Step*AE_period;
            // AMIGOS_INFO("[gain-down] result->SensorGain = %d \n", result->SensorGain);
        }
        if (result->SensorGain >= Gain_Max)
        {
            result->SensorGain = Gain_Max;
            tmp1=1;
        }
        if (result->SensorGain <= Gain_Min)
        {
            result->SensorGain = Gain_Min;
            tmp1=0;
        }
    }
#if log_info
     AMIGOS_INFO("fcount = %d, Image avg = 0x%X \n", fcount, avg);
     AMIGOS_INFO("tmp = %d, SensorGain: %d -> %d \n", tmp, info->SensorGain, result->SensorGain);
#endif
#endif

#if AE_sample
    //MI_U32 y_lower = 0x28;
    //MI_U32 y_upper = 0x38;
    //MI_U32 change_ratio = 10; // percentage
    //MI_U32 Gain_Min = 1024*2;
    //MI_U32 Gain_Max = 1024*1000;
    //MI_U32 Shutter_Min = 150;
    //MI_U32 Shutter_Max = 33333;

    result->SensorGain = 1024;
    result->Shutter = 3000;
    result->SensorGainHdrShort = 1600;
    result->ShutterHdrShort = 100;
    result->Change = 1;
    /*
    result->SensorGain = info->SensorGain;
    result->Shutter = info->Shutter;
    result->SensorGainHdrShort = info->SensorGainHDRShort;
    result->ShutterHdrShort = info->ShutterHDRShort;*/
    /*
        if(fcount++ % AE_period == 0)
        {
            if(avg<y_lower)
            {
                if (info->Shutter<Shutter_Max)
                {
                    result->Shutter = info->Shutter + (info->Shutter*change_ratio/100);
                    if (result->Shutter > Shutter_Max) result->Shutter = Shutter_Max;
                }
                else
                {
                    result->SensorGain = info->SensorGain + (info->SensorGain*change_ratio/100);
                    if (result->SensorGain > Gain_Max) result->SensorGain = Gain_Max;
                }
                result->Change = 1;
            }
            else if(avg>y_upper)
            {
                if (info->SensorGain>Gain_Min)
                {
                    result->SensorGain = info->SensorGain - (info->SensorGain*change_ratio/100);
                    if (result->SensorGain < Gain_Min) result->SensorGain = Gain_Min;
                }
                else
                {
                    result->Shutter = info->Shutter - (info->Shutter*change_ratio/100);
                    if (result->Shutter < Shutter_Min) result->Shutter = Shutter_Min;
                }
                result->Change = 1;
            }
            else if(fcount == 1)
            {
                result->Change = 1;
            }

            //hdr demo code
            result->SensorGainHdrShort = result->SensorGain;
            result->ShutterHdrShort = result->Shutter / result->HdrRatio;

        }
    */
#if log_info
    AMIGOS_INFO("fcount = %d, Image avg = 0x%X \n", fcount, avg);
    AMIGOS_INFO("SensorGain: %d -> %d \n", info->SensorGain, result->SensorGain);
    AMIGOS_INFO("Shutter: %d -> %d \n", info->Shutter, result->Shutter);
    AMIGOS_INFO("SensorGainHDR: %d -> %d \n", info->SensorGainHDRShort, result->SensorGainHdrShort);
    AMIGOS_INFO("ShutterHDR: %d -> %d \n", info->ShutterHDRShort, result->ShutterHdrShort);
#endif

#endif

}

static int mod1_isp_awb_init(void *pdata,ISP_AWB_INIT_PARAM *param)
{
    AMIGOS_INFO("**** awb_init ****\n");
    return 0;
}

static void mod1_isp_awb_run(void *pdata, const ISP_AWB_INFO *info, ISP_AWB_RESULT *result)
{
    int *data = (int*)pdata;
    int dev = data[0];
    int ch = data[1];
    result->R_gain = 1024;
    result->G_gain = 1024;
    result->B_gain = 1024;
    result->Change = 1;
    result->ColorTmp = 6000;
/********************************************************************/
    MI_ISP_HISTO_HW_STATISTICS_t stHist;
    MI_ISP_AE_HW_STATISTICS_t *pAe=NULL;
    unsigned int index = 0,y=0;

    if(0 != MI_ISP_AE_GetHisto0HwStats(dev, ch, &stHist))
    {
        AMIGOS_ERR("GetHisto0HwStats error\n");
    }
    else
    {
        AMIGOS_INFO("Show AeHwHist:\n");
        for(index = 0; index < 6; ++index)
        {
            /*if(index % 8 == 0)
            {
                printf("\n");
            }*/
            printf("%3d:%5d ", index, stHist.nHisto[index]);
        }

        for(index =122 ; index < 128; ++index)
        {
            /*if(index % 8 == 0)
            {
                printf("\n");
            }*/
            printf("%3d:%5d ", index, stHist.nHisto[index]);
        }
        printf("\n");
        // AMIGOS_INFO("Show AeHwHist end.\n");
    }

    pAe = (MI_ISP_AE_HW_STATISTICS_t *)malloc(sizeof(MI_ISP_AE_HW_STATISTICS_t));
    if(0 != MI_ISP_AE_GetAeHwAvgStats(dev, ch, pAe))
    {
        AMIGOS_ERR("GetAwbHwAvgStats error\n");
    }
    else
    {
        AMIGOS_INFO("Show AwbHwAvgStats:\n");
        for(y = 0; y < 16; ++y)
        {
            printf("(%d,%d,%d,%d) ", pAe->nAvg[y].u8AvgR,pAe->nAvg[y].u8AvgG,pAe->nAvg[y].u8AvgB,pAe->nAvg[y].u8AvgY);
        }
        printf("\n");
        // AMIGOS_INFO("Show AwbHwAvgStats end.\n");
    }
    free(pAe);
}

static void mod1_isp_awb_release(void *pdata)
{
    AMIGOS_INFO("****[%s] awb_release ****\n", __FUNCTION__);
}

static int mod1_isp_af_init(void *pdata, ISP_AF_INIT_PARAM *param)
{
    int *data = (int*)pdata;
    int dev = data[0];
    int u32ch = data[1];

    AMIGOS_INFO("************ af_init **********\n");

#define USE_NORMAL_MODE 1

#ifdef USE_NORMAL_MODE
    //_Init Normal mode setting
    CusAFWin_t afwin =
    {
        AF_ROI_MODE_NORMAL,
        1,
        {   {   0,    0,  255,  255},
            { 256,    0,  511,  255},
            { 512,    0,  767,  255},
            { 768,    0, 1023,  255},
            {   0,  256,  255,  511},
            { 256,  256,  511,  511},
            { 512,  256,  767,  511},
            { 768,  256, 1023,  511},
            {   0,  512,  255,  767},
            { 256,  512,  511,  767},
            { 512,  512,  767,  767},
            { 768,  512, 1023,  767},
            {   0,  768,  255, 1023},
            { 256,  768,  511, 1023},
            { 512,  768,  767, 1023},
            { 768,  768, 1023, 1023}
        }
    };

    MI_ISP_CUS3A_SetAFWindow((CUS3A_ISP_DEV_e)dev, (CUS3A_ISP_CH_e)u32ch, &afwin);
#else
    //_Init Matrix mode setting
    CusAFWin_t afwin =
    {
        //full image, equal divide to 16x16
        //window setting need to multiple of two
        AF_ROI_MODE_MATRIX,
        16,
        {   {0, 0, 62, 62},
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
            {960, 960, 1022, 1022}
        }

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


    //set AF Filter
    CusAFFilter_t affilter =
    {
        //filter setting with sign value
        //{s9, s10, s9, s13, s13}

        //[0.3~0.6]  : 20,  37,  20, -4352, 3392; 20, -39,  20,  2176, 3264; 26, 0, -26, -1152, 2432;
        //[0.2~0.5]  : 20,  35,  20, -6144, 3520; 20, -39,  20,   -64, 2304; 26, 0, -26, -3328, 2432;
        //[0.1~0.6]  : 37,   0, -37, -6848, 3136; 37,   0, -37,  1600, 1792; 32, 0, -32, -2624,    0;
        //[0.08~0.24]: 13,  11,  13, -5568, 3456; 13, -26,  13, -7680, 3840; 15, 0, -15, -6592, 3200;
        //[0.03~0.25]: 19,   0, -19, -7808, 3776; 19,   0, -19, -4672, 2304; 17, 0, -17, -5824, 1920;
        //[0.07~0.1] :  8, -13,   8, -7680, 3968;  8, -16,   8, -7936, 4032;  3, 0,  -3, -7744, 3904;

        //here use [0.3~0.6] & [0.08~0.24]
        //convert to hw format (sign bit with msb)
           20,      37,     20, 4352+8192, 3392, 0, 1023, 0, 1023,
           13,      11,     13, 5568+8192, 3456, 0, 1023, 0, 1023,
        1, 20, 39+1024,     20,      2176, 3264,
        1, 26,       0, 26+512, 1152+8192, 2432,
        1, 13, 26+1024,     13, 7680+8192, 3840,
        1, 15,       0, 15+512, 6592+8192, 3200,
    };
    MI_ISP_CUS3A_SetAFFilter(dev, 0, &affilter);

    //set AF Sq
    CusAFFilterSq_t sq =
    {
        .bSobelYSatEn = 0,
        .u16SobelYThd = 1023,
        .bIIRSquareAccEn = 1,
        .bSobelSquareAccEn = 0,
        .u16IIR1Thd = 0,
        .u16IIR2Thd = 0,
        .u16SobelHThd = 0,
        .u16SobelVThd = 0,
        .u8AFTbl1X = {6,7,7,6,6,6,7,6,6,7,6,6,},
        .u16AFTbl1Y = {0,32,288,800,1152,1568,2048,3200,3872,4607,6271,7199,8191},
        .u8AFTbl2X = {6,7,7,6,6,6,7,6,6,7,6,6,},
        .u16AFTbl2Y = {0,32,288,800,1152,1568,2048,3200,3872,4607,6271,7199,8191},
    };

    MI_ISP_CUS3A_SetAFFilterSq(dev, 0, &sq);

#if MOTOR_TEST
    mod1_isp_af_motor_init();
#endif

    AMIGOS_INFO("**** af_init done ****\n");

    return 0;
}

static void mod1_isp_af_release(void *pdata)
{
     AMIGOS_INFO("****[%s] af_release ****\n", __FUNCTION__);
}

static void mod1_isp_af_run(void *pdata, const ISP_AF_INFO *af_info, ISP_AF_RESULT *result)
{

}

static int mod1_isp_af_ctrl(void *pdata, ISP_AF_CTRL_CMD cmd, void *param)
{
    return 0;
}


AmigosModuleIq::AmigosModuleIq(const std::string & strInSection)
    : AmigosSurfaceIq(strInSection), AmigosModuleMiBase(this)
{
}
AmigosModuleIq::~AmigosModuleIq()
{
}
unsigned int AmigosModuleIq::GetModId() const
{
    return E_MI_MODULE_ID_ISP;
    return 0;
}
unsigned int AmigosModuleIq::GetInputType(unsigned int port) const
{
    return 0;
}
unsigned int AmigosModuleIq::GetOutputType(unsigned int port) const
{
    return 0;
}
void AmigosModuleIq::_ResourceInit()
{
    if(stIqInfo.intIqServer)
    {
        auto itMapIqCreateDev = mapIqCreateDev.find(stModInfo.devId);
        if (itMapIqCreateDev == mapIqCreateDev.end())
        {
            mapIqCreateDev[stModInfo.devId] = 0;
        }
        mapIqCreateDev[stModInfo.devId]++;
    }
}
void AmigosModuleIq::_ResourceDeinit()
{
    if(stIqInfo.intIqServer)
    {
        auto itMapIqCreateDev = mapIqCreateDev.find(stModInfo.devId);
        if (itMapIqCreateDev != mapIqCreateDev.end())
        {
            itMapIqCreateDev->second--;
            if(!itMapIqCreateDev->second)
            {
                mapIqCreateDev.erase(itMapIqCreateDev);
            }
        }
    }
}

void AmigosModuleIq::_Init()
{

    auto itMapIqCreateDev = mapIqCreateDev.find(stModInfo.devId);
    if (itMapIqCreateDev == mapIqCreateDev.end())
    {
        if(stIqInfo.intIqServer)
        {
            MI_IQSERVER_Open();
            AMIGOS_INFO("Iq Server Open \n");
        }
        mapIqCreateDev[stModInfo.devId] = 0;
    }
    mapIqCreateDev[stModInfo.devId]++;
    MI_ISP_IQ_ParamInitInfoType_t status;
    memset(&status, 0x0, sizeof(MI_ISP_IQ_ParamInitInfoType_t));
    MI_ISP_IQ_GetParaInitStatus(stModInfo.devId, stModInfo.chnId, &status);
    if (1 != status.stParaAPI.bFlag)
    {
        AMIGOS_ERR("check isp & cus3a status failed! not load iq file!\n");
        return;
    }
    for (auto &it : stIqInfo.vectCaliCfg)
    {
        AMIGOS_INFO("isp(%d, %d) item: %s load califile(%s)\n", stModInfo.devId, stModInfo.chnId, it.caliItem.c_str(), it.caliFileName.c_str());
        MI_ISP_ApiCmdLoadCaliData(stModInfo.devId, stModInfo.chnId, ss_enum_cast<MI_ISP_IQ_CaliItem_e>::from_str(it.caliItem.c_str()),
                                  (char *)it.caliFileName.c_str());
    }
    if (0 == stIqInfo.intCus3a)
    {
        AMIGOS_INFO("Cus3a not open\n");
        return;
    }
    AMIGOS_INFO("Cus3a open\n");

    if(stIqInfo.Cus3aType == "sigma") //use sstar 3A
    {
        AMIGOS_INFO("== MIXER CUS3A DISABLE ==\n");

        CUS3A_SetAlgoAdaptor((CUS3A_ISP_DEV_e)stModInfo.devId, (CUS3A_ISP_CH_e)stModInfo.chnId, E_ALGO_ADAPTOR_NATIVE, E_ALGO_TYPE_AE);
        CUS3A_SetAlgoAdaptor((CUS3A_ISP_DEV_e)stModInfo.devId, (CUS3A_ISP_CH_e)stModInfo.chnId, E_ALGO_ADAPTOR_NATIVE, E_ALGO_TYPE_AWB);
        CUS3A_SetAlgoAdaptor((CUS3A_ISP_DEV_e)stModInfo.devId, (CUS3A_ISP_CH_e)stModInfo.chnId, E_ALGO_ADAPTOR_NATIVE, E_ALGO_TYPE_AF);

        CUS3A_RegInterfaceEX((CUS3A_ISP_DEV_e)stModInfo.devId, (CUS3A_ISP_CH_e)stModInfo.chnId, E_ALGO_ADAPTOR_1, E_ALGO_TYPE_AE, NULL);
        CUS3A_RegInterfaceEX((CUS3A_ISP_DEV_e)stModInfo.devId, (CUS3A_ISP_CH_e)stModInfo.chnId, E_ALGO_ADAPTOR_1, E_ALGO_TYPE_AWB, NULL);
        CUS3A_RegInterfaceEX((CUS3A_ISP_DEV_e)stModInfo.devId, (CUS3A_ISP_CH_e)stModInfo.chnId, E_ALGO_ADAPTOR_1, E_ALGO_TYPE_AF, NULL);  //if no need ,set algo to NULL

        return;
    }
    if(stIqInfo.Cus3aType == "cus")//use customer 3A
    {
        ISP_AE_INTERFACE tAeIf;
        ISP_AWB_INTERFACE tAwbIf;
        ISP_AF_INTERFACE tAfIf;
        ISP_AE_INTERFACE *pAeIf = NULL;
        ISP_AWB_INTERFACE *pAwbIf = NULL;
        ISP_AF_INTERFACE *pAfIf = NULL;
        MI_BOOL bAEenable = stIqInfo.intCus3aAe;
        MI_BOOL bAFenable = stIqInfo.intCus3aAf;
        MI_BOOL bAWBenable = stIqInfo.intCus3aAwb;
        static int IqModInfo[2];

        AMIGOS_INFO("== MIXER CUS3A ENABLE == , %d %d %d\n",bAEenable,bAWBenable,bAFenable);

        //CUS3A_Init((CUS3A_ISP_DEV_e)stModInfo.devId); //create isp chn already init

        if (bAEenable)
        {
            /*AE*/
            IqModInfo[0] = stModInfo.devId;
            IqModInfo[1] = stModInfo.chnId;
            tAeIf.ctrl = NULL;
            tAeIf.pdata = IqModInfo;
            tAeIf.init = mod1_isp_ae_init;
            tAeIf.release = mod1_isp_ae_release;
            tAeIf.run = mod1_isp_ae_run;
            pAeIf = &tAeIf;

            CUS3A_RegInterfaceEX((CUS3A_ISP_DEV_e)stModInfo.devId, (CUS3A_ISP_CH_e)stModInfo.chnId, E_ALGO_ADAPTOR_1, E_ALGO_TYPE_AE, pAeIf);
            CUS3A_SetAlgoAdaptor((CUS3A_ISP_DEV_e)stModInfo.devId, (CUS3A_ISP_CH_e)stModInfo.chnId, E_ALGO_ADAPTOR_1, E_ALGO_TYPE_AE);
        }
        if (bAWBenable)
        {
            /*AWB*/
            IqModInfo[0] = stModInfo.devId;
            IqModInfo[1] = stModInfo.chnId;
            tAwbIf.ctrl = NULL;
            tAwbIf.pdata = IqModInfo;
            tAwbIf.init = mod1_isp_awb_init;
            tAwbIf.release = mod1_isp_awb_release;
            tAwbIf.run = mod1_isp_awb_run;
            pAwbIf = &tAwbIf;

            CUS3A_RegInterfaceEX((CUS3A_ISP_DEV_e)stModInfo.devId, (CUS3A_ISP_CH_e)stModInfo.chnId, E_ALGO_ADAPTOR_1, E_ALGO_TYPE_AWB, pAwbIf);
            CUS3A_SetAlgoAdaptor((CUS3A_ISP_DEV_e)stModInfo.devId, (CUS3A_ISP_CH_e)stModInfo.chnId, E_ALGO_ADAPTOR_1, E_ALGO_TYPE_AWB);
        }
        if (bAFenable)
        {
            /*AF*/
            IqModInfo[0] = stModInfo.devId;
            IqModInfo[1] = stModInfo.chnId;
            tAfIf.pdata = IqModInfo;
            tAfIf.init = mod1_isp_af_init;
            tAfIf.release = mod1_isp_af_release;
            tAfIf.run = mod1_isp_af_run;
            tAfIf.ctrl = mod1_isp_af_ctrl;
            pAfIf = &tAfIf;

            CUS3A_RegInterfaceEX((CUS3A_ISP_DEV_e)stModInfo.devId, (CUS3A_ISP_CH_e)stModInfo.chnId, E_ALGO_ADAPTOR_1, E_ALGO_TYPE_AF, pAfIf);  //if no need ,set algo to NULL
            CUS3A_SetAlgoAdaptor((CUS3A_ISP_DEV_e)stModInfo.devId, (CUS3A_ISP_CH_e)stModInfo.chnId, E_ALGO_ADAPTOR_1, E_ALGO_TYPE_AF);
        }
    }
}
void AmigosModuleIq::_Deinit()
{
    std::map<unsigned int, unsigned int>::iterator itMapIqCreateDev;

    if(stIqInfo.intIqServer)
    {
        itMapIqCreateDev = mapIqCreateDev.find(stModInfo.devId);
        if (itMapIqCreateDev != mapIqCreateDev.end())
        {
            itMapIqCreateDev->second--;
            if(!itMapIqCreateDev->second)
            {
                mapIqCreateDev.erase(itMapIqCreateDev);
                MI_IQSERVER_Close();
                AMIGOS_INFO("Iq Server Close \n");
            }
        }
    }
}

void AmigosModuleIq::_Start()
{

}
int AmigosModuleIq::IspWaitReadyTimeout(int time_ms)
{
    return 0;
}

void AmigosModuleIq::_Stop()
{

}
AMIGOS_MODULE_INIT("IQ", AmigosModuleIq);
