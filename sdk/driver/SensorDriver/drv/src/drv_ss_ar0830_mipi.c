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
/* Sensor Porting on Master V4
   Porting owner : paul-pc.wang
   Date : 2023/10/24
   Build on : Master V4  I6C
   Verified on : mixer preview ok (linear/hdr),hdr frame mode only, realtime mode need to set ISP DoL mode but VIF VC mode
               AE gain/shutter ok , IQ not verify, hdr ae hard to converge.
   Remark : NA
*/

#ifdef __cplusplus
extern "C"
{
#endif

#include <drv_sensor_common.h>
#include <sensor_i2c_api.h>
#include <drv_sensor.h>
#include <drv_sensor_init_table.h> //TODO: move this header to drv_sensor_common.h
#include <AR0830_MIPI_init_table.h>
#ifdef __cplusplus
}
#endif

SENSOR_DRV_ENTRY_IMPL_BEGIN_EX(AR0830_HDR);

#ifndef ARRAY_SIZE
#define ARRAY_SIZE CAM_OS_ARRAY_SIZE
#endif

//#define DUMP_SEN_I2C_DATA

#define SENSOR_CHANNEL_MODE            CUS_SENSOR_CHANNEL_MODE_REALTIME_NORMAL
#define SENSOR_CHANNEL_MODE_SONY_DOL   CUS_SENSOR_CHANNEL_MODE_RAW_STORE_HDR

//============================================
#define ENABLE            1
#define DISABLE           0
#undef SENSOR_DBG
#define SENSOR_DBG        0

#define DEBUG_INFO        0

#if SENSOR_DBG == 1
//#define SENSOR_DMSG(args...) LOGD(args)
//#define SENSOR_DMSG(args...) LOGE(args)
//#define SENSOR_DMSG(args...) printf(args)
#elif SENSOR_DBG == 0
//#define SENSOR_DMSG(args...)
#endif
///////////////////////////////////////////////////////////////
//          @@@                                              //
//         @ @@      ==  S t a r t * H e r e  ==             //
//           @@      ==  S t a r t * H e r e  ==             //
//           @@      ==  S t a r t * H e r e  ==             //
//          @@@@                                             //
//                                                           //
//      Start Step 1 --  show preview on LCM                 //
//                                                           //
//  Fill these #define value and table with correct settings //
//      camera can work and show preview on LCM              //
//                                                           //
///////////////////////////////////////////////////////////////

////////////////////////////////////
// Sensor-If Info                 //
////////////////////////////////////
//MIPI config begin.
#define SENSOR_MIPI_LANE_NUM        (4)
#define SENSOR_MIPI_LANE_NUM_DOL    (4)


#define SENSOR_IFBUS_TYPE           CUS_SENIF_BUS_MIPI  //CFG //CUS_SENIF_BUS_PARL, CUS_SENIF_BUS_MIPI
#define SENSOR_DATAPREC             CUS_DATAPRECISION_12 // CUS_DATAPRECISION_10, xDR, CUS_DATAPRECISION_10, linear
#define SENSOR_DATAPREC_DOL         CUS_DATAPRECISION_10
#define SENSOR_BAYERID              CUS_BAYER_GR         //0h: CUS_BAYER_RG, 1h: CUS_BAYER_GR, 2h: CUS_BAYER_BG, 3h: CUS_BAYER_GB
#define SENSOR_BAYERID_HDR_DOL      CUS_BAYER_GR
#define SENSOR_RGBIRID              CUS_RGBIR_NONE
#define SENSOR_ORIT                 CUS_ORIT_M0F0        //CUS_ORIT_M0F0, CUS_ORIT_M1F0, CUS_ORIT_M0F1, CUS_ORIT_M1F1,
#define SENSOR_SEF_TH               3
#define SENSOR_LIMIT_LINE           ((8+4))
#define Preview_MAX_FPS     60                     //fastest preview FPS
#define Preview_MIN_FPS     5                      //slowest preview FPS

////////////////////////////////////
// MCLK Info                      //
////////////////////////////////////
#define Preview_MCLK_SPEED          CUS_CMU_CLK_24MHZ    //CUS_CMU_CLK_24MHZ //CUS_CMU_CLK_37P125MHZ//CUS_CMU_CLK_27MHZ
#define Preview_MCLK_SPEED_HDR_DOL  CUS_CMU_CLK_24MHZ

////////////////////////////////////
// I2C Info                       //
////////////////////////////////////
#define SENSOR_I2C_ADDR              0x6C                //I2C slave address
#define SENSOR_I2C_SPEED             200000              //200000 //300000 //240000                  //I2C speed, 60000~320000
#define SENSOR_I2C_LEGACY            I2C_NORMAL_MODE     //usally set CUS_I2C_NORMAL_MODE,  if use old OVT I2C protocol=> set CUS_I2C_LEGACY_MODE
#define SENSOR_I2C_FMT               I2C_FMT_A16D16       //CUS_I2C_FMT_A8D8, CUS_I2C_FMT_A8D16, CUS_I2C_FMT_A16D8, CUS_I2C_FMT_A16D16

////////////////////////////////////
// Sensor Signal                  //
////////////////////////////////////
#define SENSOR_PWDN_POL              CUS_CLK_POL_NEG     // if PWDN pin High can makes sensor in power down, set CUS_CLK_POL_POS
#define SENSOR_RST_POL               CUS_CLK_POL_NEG     // if RESET pin High can makes sensor in reset state, set CUS_CLK_POL_NEG
                                                         // VSYNC/HSYNC POL can be found in data sheet timing diagram,
                                                         // Notice: the initial setting may contain VSYNC/HSYNC POL inverse settings so that condition is different.
#define SENSOR_VSYNC_POL             CUS_CLK_POL_POS     // if VSYNC pin High and data bus have data, set CUS_CLK_POL_POS
#define SENSOR_HSYNC_POL             CUS_CLK_POL_POS     // if HSYNC pin High and data bus have data, set CUS_CLK_POL_POS
#define SENSOR_PCLK_POL              CUS_CLK_POL_POS     // depend on sensor setting, sometimes need to try CUS_CLK_POL_POS or CUS_CLK_POL_NEG

////////////////////////////////////
// Sensor ID                      //
////////////////////////////////////
//define SENSOR_ID

#undef SENSOR_NAME
#define SENSOR_NAME     AR0830

////////////////////////////////////
// Image Info                     //
////////////////////////////////////
static struct {     // LINEAR
    // Modify it based on number of support resolution
    enum {LINEAR_RES_1 = 0, LINEAR_RES_END}mode;
    // Sensor Output Image info
    struct _senout{
        s32 width, height, min_fps, max_fps;
    }senout;
    // VIF Get Image Info
    struct _sensif{
        s32 crop_start_X, crop_start_y, preview_w, preview_h;
    }senif;
    // Show resolution string
    struct _senstr{
        const char* strResDesc;
    }senstr;
}AR0830_mipi_linear[] = {
    {LINEAR_RES_1, {3864, 2192, 3, 30}, {0, 0, 3840, 2160}, {"3840x2160@30fps"}},
};
static struct {     // HDR
    // Modify it based on number of support resolution
    enum {HDR_RES_1 = 0, HDR_RES_END}mode;
    // Sensor Output Image info
    struct _hsenout{
        s32 width, height, min_fps, max_fps;
    }senout;
    // VIF Get Image Info
    struct _hsensif{
        s32 crop_start_X, crop_start_y, preview_w, preview_h;
    }senif;
    // Show resolution string
    struct _hsenstr{
        const char* strResDesc;
    }senstr;
}AR0830_mipi_hdr[] = {
    {HDR_RES_1, {3864, 2192, 3, 30}, {0, 0, 3840, 2160}, {"3840x2160@30fps"}},
};

#define AR0830_HDR_BRL                              2228

#define vts_30fps_xDR  (2216)
#define vts_30fps  (2334)
#define vts_30fps_HDR_DOL_4lane  (AR0830_HDR_VTS)

#define Preview_line_period  (15042) // 14814 for linear mode, 30084 = 1/15(fps)/vts_30fps_xDR @ 15fps
// 15042 = 1/30(fps)/vts_30fps_xDR @ 30fps
#define Preview_line_period_HDR_DOL_4LANE  (14988)

////////////////////////////////////
// AE Info                        //
////////////////////////////////////
#define SENSOR_MAX_GAIN                             (470652)        // max sensor again, a-gain * conversion-gain*d-gain
#define SENSOR_MAX_GAIN_HDR                         (246224)        // max sensor again, a-gain * conversion-gain*d-gain
#define SENSOR_MIN_GAIN                             (1 * 1024)
#define SENSOR_GAIN_DELAY_FRAME_COUNT               (2)
#define SENSOR_GAIN_DELAY_FRAME_COUNT_HDR_DOL       (2)
#define SENSOR_SHUTTER_DELAY_FRAME_COUNT            (2)
#define SENSOR_SHUTTER_DELAY_FRAME_COUNT_HDR_DOL    (2)
#define SENSOR_GAIN_CTRL_NUM                  (2)
#define SENSOR_SHUTTER_CTRL_NUM               (2)

////////////////////////////////////
// Mirror-Flip Info               //
////////////////////////////////////
#define MIRROR_FLIP                                 0x3030
#define SENSOR_NOR                                  0x0
#define SENSOR_MIRROR_EN                            0x1 << (0)
#define SENSOR_FLIP_EN                              0x1 << (1)
#define SENSOR_MIRROR_FLIP_EN                       0x3

#if defined (SENSOR_MODULE_VERSION)
#define TO_STR_NATIVE(e) #e
#define TO_STR_PROXY(m, e) m(e)
#define MACRO_TO_STRING(e) TO_STR_PROXY(TO_STR_NATIVE, e)
static char *sensor_module_version = MACRO_TO_STRING(SENSOR_MODULE_VERSION);
module_param(sensor_module_version, charp, S_IRUGO);
#endif

typedef struct {
    struct {
        u16 pre_div0;
        u16 div124;
        u16 div_cnt7b;
        u16 sdiv0;
        u16 mipi_div0;
        u16 r_divp;
        u16 sdiv1;
        u16 r_seld5;
        u16 r_sclk_dac;
        u16 sys_sel;
        u16 pdac_sel;
        u16 adac_sel;
        u16 pre_div_sp;
        u16 r_div_sp;
        u16 div_cnt5b;
        u16 sdiv_sp;
        u16 div12_sp;
        u16 mipi_lane_sel;
        u16 div_dac;
    } clk_tree;
    struct {
        bool bVideoMode;
        u16 res_idx;
        CUS_CAMSENSOR_ORIT  orit;
    } res;
    struct {
        float sclk;
        u32 hts;
        u32 vts;
        u32 vts_lef;
        u32 vts_sef;
        u32 ho;
        u32 xinc;
        u32 line_freq;
        u32 us_per_line;
        u32 final_us;
        u32 final_gain;
        u32 final_lef_gain;
        u32 final_sef_gain;
        u32 back_pv_us;
        u32 fps;
        u32 preview_fps;
        u32 expo_lines;
        u32 expo_lines_sef;
        u32 expo_lef_us;
        u32 expo_sef_us;
        u32 max_sef;
    } expo;

    u32 min_shr1;
    u32 min_rhs1;
    u32 min_shr0;
    u32 max_shr0;
    u32 fsc;
    I2C_ARRAY tVts_reg[3];
    I2C_ARRAY tExpo_reg[3];
    I2C_ARRAY tExpo_hdr_dol_lef_reg[3];
    I2C_ARRAY tExpo_hdr_dol_sef_reg[3];
    I2C_ARRAY tSHR0_reg[3];
    I2C_ARRAY tSHR1_reg[3];
    I2C_ARRAY tRHS1_reg[3];
    I2C_ARRAY tGain_reg[2];
    u32 Default_Res_line_period;
    u32 Default_Res_vts;
    bool dirty;
    bool orien_dirty;
} AR0830_params;

typedef struct {
    unsigned int total_gain;
    unsigned short reg_val;
} Gain_ARRAY;

static Gain_ARRAY gain_table[]={
    {1024, 0   },
    {1116, 1   },
    {1217, 2   },
    {1327, 3   },
    {1446, 4   },
    {1577, 5   },
    {1719, 6   },
    {1874, 7   },
    {2036, 8   },
    {2220, 9   },
    {2420, 10  },
    {2638, 11  },
    {2876, 12  },
    {3135, 13  },
    {3418, 14  },
    {3726, 15  },
    {4064, 16  },
    {4431, 17  },
    {4831, 18  },
    {5266, 19  },
    {5741, 20  },
    {6259, 21  },
    {6823, 22  },
    {7439, 23  },
    {8123, 24  },
    {8855, 25  },
    {9654, 26  },
    {10524, 27  },
    {11474, 28  },
    {12508, 29  },
    {13636, 30  },
    {14866, 31  },
    {16226, 32  },
    {17689, 33  },
    {19284, 34  },
    {21023, 35  },
    {22919, 36  },
    {24986, 37  },
    {27240, 38  },
    {29696, 39  },
    {32352, 40  },
    {35270, 41  },
    {38450, 42  },
    {41918, 43  },
    {45698, 44  },
    {49820, 45  },
    {54313, 46  },
    {59211, 47  },
    {64580, 48  },
    {70404, 49  },
    {76754, 50  },
    {83676, 51  },
    {91222, 52  },
    {99449, 53  },
    {108418, 54  },
    {118195, 55  },
    {128884, 56  },
    {140508, 57  },
    {153179, 58  },
    {166994, 59  },
    {182054, 60  },
    {198472, 61  },
    {216371, 62  },
    {235885, 63  },
    {257158, 64  },
    {280350, 65  },
    {305633, 66  },
    {333196, 67  },
    {363245, 68  },
    {396004, 69  },
    {431718, 70  },
    {470652, 71  },
};
static Gain_ARRAY gain_table_hdr[]={
    {1024    , 0   },
    {1069    , 1   },
    {1116    , 2   },
    {1166    , 3   },
    {1217    , 4   },
    {1271    , 5   },
    {1327    , 6   },
    {1385    , 7   },
    {1442    , 8   },
    {1505    , 9   },
    {1572    , 10  },
    {1641    , 11  },
    {1713    , 12  },
    {1789    , 13  },
    {1868    , 14  },
    {1950    , 15  },
    {2037    , 16  },
    {2126    , 17  },
    {2220    , 18  },
    {2318    , 19  },
    {2420    , 20  },
    {2527    , 21  },
    {2639    , 22  },
    {2755    , 23  },
    {2878    , 24  },
    {3005    , 25  },
    {3138    , 26  },
    {3276    , 27  },
    {3421    , 28  },
    {3572    , 29  },
    {3729    , 30  },
    {3894    , 31  },
    {4073    , 32  },
    {4253    , 33  },
    {4440    , 34  },
    {4636    , 35  },
    {4841    , 36  },
    {5054    , 37  },
    {5277    , 38  },
    {5510    , 39  },
    {5754    , 40  },
    {6008    , 41  },
    {6273    , 42  },
    {6550    , 43  },
    {6839    , 44  },
    {7140    , 45  },
    {7455    , 46  },
    {7784    , 47  },
    {8126    , 48  },
    {8484    , 49  },
    {8858    , 50  },
    {9249    , 51  },
    {9657    , 52  },
    {10083   , 53  },
    {10528   , 54  },
    {10993   , 55  },
    {11462   , 56  },
    {11967   , 57  },
    {12495   , 58  },
    {13047   , 59  },
    {13622   , 60  },
    {14223   , 61  },
    {14851   , 62  },
    {15506   , 63  },
    {16187   , 64  },
    {16901   , 65  },
    {17647   , 66  },
    {18426   , 67  },
    {19239   , 68  },
    {20087   , 69  },
    {20974   , 70  },
    {21899   , 71  },
    {22858   , 72  },
    {23867   , 73  },
    {24920   , 74  },
    {26019   , 75  },
    {27167   , 76  },
    {28366   , 77  },
    {29617   , 78  },
    {30924   , 79  },
    {32369   , 80  },
    {33797   , 81  },
    {35288   , 82  },
    {36845   , 83  },
    {38470   , 84  },
    {40167   , 85  },
    {41940   , 86  },
    {43790   , 87  },
    {45686   , 88  },
    {47702   , 89  },
    {49806   , 90  },
    {52004   , 91  },
    {54298   , 92  },
    {56693   , 93  },
    {59195   , 94  },
    {61806   , 95  },
    {64552   , 96  },
    {67400   , 97  },
    {70374   , 98  },
    {73479   , 99  },
    {76721   , 100 },
    {80105   , 101 },
    {83640   , 102 },
    {87330   , 103 },
    {91187   , 104 },
    {95210   , 105 },
    {99411   , 106 },
    {103797  , 107 },
    {108376  , 108 },
    {113157  , 109 },
    {118150  , 110 },
    {123362  , 111 },
    {128827  , 112 },
    {134511  , 113 },
    {140445  , 114 },
    {146642  , 115 },
    {153111  , 116 },
    {159866  , 117 },
    {166920  , 118 },
    {174284  , 119 },
    {182004  , 120 },
    {190034  , 121 },
    {198418  , 122 },
    {207172  , 123 },
    {216312  , 124 },
    {225856  , 125 },
    {235820  , 126 },
    {246224  , 127 },
};

static int pCus_SetAEUSecs(ss_cus_sensor *handle, u32 us);
static int pCus_SetAEUSecsHdrSef(ss_cus_sensor *handle, u32 us);
static int pCus_SetAEUSecsHdrLef(ss_cus_sensor *handle, u32 us);
///////////////////////////////////////////////////////////////
//          @@@                                              //
//         @  @@                                             //
//           @@                                              //
//          @@                                               //
//         @@@@@                                             //
//                                                           //
//      Start Step 2 --  set Sensor initial and              //
//                       adjust parameter                    //
//  Fill these register table with resolution settings       //
//      camera can work and show preview on LCM              //
//                                                           //
///////////////////////////////////////////////////////////////


#if 0
static I2C_ARRAY PatternTbl[] = {
    {0x0000,0x00},       // colorbar pattern , bit 0 to enable
};
#endif

const static I2C_ARRAY expo_reg[] = {
    {0x0202, 0x100},  // xDR
};
const static I2C_ARRAY expo_sef_reg[] = {
    {0x0224, 0x064},  // HDR
};
const static I2C_ARRAY expo_lef_reg[] = {
    {0x0202, 0x300},  // HDR
};


const static I2C_ARRAY vts_reg[] = {       //VMAX
    {0x340, 0x8A8},  // xDR
};

const static I2C_ARRAY gain_reg[] = {
    {0x3062, 0x4040},
};

/////////////////////////////////////////////////////////////////
//       @@@@@@@                                               //
//           @@                                                //
//          @@                                                 //
//          @@@                                                //
//       @   @@                                                //
//        @@@@                                                 //
//                                                             //
//      Step 3 --  complete camera features                    //
//                                                             //
//  camera set EV, MWB, orientation, contrast, sharpness       //
//   , saturation, and Denoise can work correctly.             //
//                                                             //
/////////////////////////////////////////////////////////////////


/////////////////// I2C function definition ///////////////////
#define SensorReg_Read(_reg,_data)     (handle->i2c_bus->i2c_rx(handle->i2c_bus, &(handle->i2c_cfg),_reg,_data))
#define SensorReg_Write(_reg,_data)    (handle->i2c_bus->i2c_tx(handle->i2c_bus, &(handle->i2c_cfg),_reg,_data))
#define SensorRegArrayW(_reg,_len)  (handle->i2c_bus->i2c_array_tx(handle->i2c_bus, &(handle->i2c_cfg),(_reg),(_len)))
#define SensorRegArrayR(_reg,_len)  (handle->i2c_bus->i2c_array_rx(handle->i2c_bus, &(handle->i2c_cfg),(_reg),(_len)))

/////////////////// sensor hardware dependent ///////////////////
static int pCus_sensor_GetAEInfo(ss_cus_sensor *handle, CUS_SENSOR_AE_INFO_t *info)
{
    AR0830_params *params = (AR0830_params *)handle->private_data;

    info->u8AEGainDelay                      = handle->sensor_ae_info_cfg.u8AEGainDelay;
    info->u8AEShutterDelay                   = handle->sensor_ae_info_cfg.u8AEShutterDelay;
    info->u8AEGainCtrlNum                    = handle->sensor_ae_info_cfg.u8AEGainCtrlNum;
    info->u8AEShutterCtrlNum                 = handle->sensor_ae_info_cfg.u8AEShutterCtrlNum;
    info->u32AEGain_min                      = handle->sensor_ae_info_cfg.u32AEGain_min;
    info->u32AEGain_max                      = handle->sensor_ae_info_cfg.u32AEGain_max;
    info->u32AEShutter_min                   = (params->Default_Res_line_period * 1);
    info->u32AEShutter_step                  = params->Default_Res_line_period;
    info->u32AEShutter_max                   = handle->sensor_ae_info_cfg.u32AEShutter_max;
    if(params->expo.max_sef)
    {
        params->expo.max_sef = (params->expo.vts -
            (2 * handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16height) - SENSOR_LIMIT_LINE) - SENSOR_SEF_TH;
        if(params->expo.max_sef < 2)
        {
            params->expo.max_sef = 2;
        }
        handle->sensor_ae_info_cfg.u32AEShutter_max                   = (params->Default_Res_line_period * params->expo.max_sef);
    }
    return SUCCESS;
}
static int pCus_poweronhdr(ss_cus_sensor *handle, u32 idx)
{
    return SUCCESS;
}
static int pCus_poweron(ss_cus_sensor *handle, u32 idx)
{
    int res = 0;
    //Sensor power on sequence
    //Sensor board PWDN/RST Pull Low
    ISensorIfAPI *sensor_if = handle->sensor_if_api;
    SENSOR_DMSG("[%s] ", __FUNCTION__);
    //Sensor power on sequence
    //Sensor board PWDN/RST Pull Low
#if 0
    sensor_if->PowerOff(idx, handle->pwdn_POLARITY);
#endif
    res = sensor_if->Reset(idx, CUS_CLK_POL_NEG);
    if(res>=0)//if success, it is normal case
    {
        SENSOR_UDELAY(1000);
    }

    sensor_if->SetIOPad(idx, handle->sif_bus, handle->interface_attr.attr_mipi.mipi_lane_num);
    sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_288M);
    sensor_if->SetCSI_Lane(idx, handle->interface_attr.attr_mipi.mipi_lane_num, 1);
    sensor_if->SetCSI_LongPacketType(idx, 0, 0x1C00, 0);
    //sensor_if->SetCSI_LongPacketType(idx, 0x6, 0x7C00, 0xFF);
    if (handle->interface_attr.attr_mipi.mipi_hdr_mode) {
        sensor_if->SetCSI_hdr_mode(idx, handle->interface_attr.attr_mipi.mipi_hdr_mode, 1);
    }

#if 0
    if (handle->interface_attr.attr_mipi.mipi_hdr_mode == CUS_HDR_MODE_SONY_DOL) {
        sensor_if->SetCSI_hdr_mode(idx, handle->interface_attr.attr_mipi.mipi_hdr_mode, 1);
    }
    SENSOR_UDELAY(100);
    sensor_if->PowerOff(idx, !handle->pwdn_POLARITY);
    //Sensor board RST Pull High after PWDN Enable
    SENSOR_UDELAY(1);
#endif

    res = sensor_if->MCLK(idx, 1, handle->mclk);
    if(res>=0)//if success, it is normal case
    {
        SENSOR_UDELAY(10);
    }

    res = sensor_if->Reset(idx, CUS_CLK_POL_POS);
    if(res>=0)//if success, it is normal case
    {
        SENSOR_UDELAY(3000);
    }

    //SENSOR_DMSG("Sensor Power On finished\n");

    return SUCCESS;
}

static int pCus_poweroff(ss_cus_sensor *handle, u32 idx)
{
  // power/reset low
    ISensorIfAPI *sensor_if = handle->sensor_if_api;
  AR0830_params *params = (AR0830_params *)handle->private_data;

    //SENSOR_DMSG("[%s] reset low\n", __FUNCTION__);
    sensor_if->Reset(idx, CUS_CLK_POL_NEG);
    sensor_if->MCLK(idx, 0, handle->mclk);

    sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_DISABLE);
    /*if (handle->interface_attr.attr_mipi.mipi_hdr_mode == CUS_HDR_MODE_SONY_DOL) {
        sensor_if->SetCSI_hdr_mode(idx, handle->interface_attr.attr_mipi.mipi_hdr_mode, 0);
    }*/
    params->res.orit = SENSOR_ORIT;

    return SUCCESS;
}

//Get and check sensor ID
//if i2c error or sensor id does not match then return FAIL
static int AR0830_SetPatternMode(ss_cus_sensor *handle,u32 mode)
{
    return SUCCESS;
}
//#define DUMP_SEN_I2C_DATA

static int pCus_init_linear(ss_cus_sensor *handle)
{
    //AR0830_params *params = (AR0830_params *)handle->private_data;
    int i,cnt=0;
    #ifdef DUMP_SEN_I2C_DATA
    u16 sen_data=0;
    #endif
    for(i=0;i< ARRAY_SIZE(Sensor_init_table);i++)
    {
        if(Sensor_init_table[i].reg==0xffff)
        {
            SENSOR_MSLEEP(Sensor_init_table[i].data);
        }
        else
        {
            cnt = 0;
            #ifdef DUMP_SEN_I2C_DATA
            sen_data = 0;
            #endif
            while(SensorReg_Write(Sensor_init_table[i].reg, Sensor_init_table[i].data) != SUCCESS)
            {
                cnt++;
                if(cnt>=10)
                {
                    SENSOR_EMSG("[%s:%d]Sensor init fail!!\n", __FUNCTION__, __LINE__);
                    return FAIL;
                }
                //SENSOR_UDELAY(1);
            }
            #ifdef DUMP_SEN_I2C_DATA
            SensorReg_Read(Sensor_init_table[i].reg, &sen_data );
            CamOsPrintf("reg 0x%x, 0x%x, read back: 0x%x\n",Sensor_init_table[i].reg, Sensor_init_table[i].data, sen_data);
            if(Sensor_init_table[i].data!=sen_data)
            {
                CamOsPrintf("reg 0x%x, 0x%x, read back: 0x%x, different!!\n\n\n",
                            Sensor_init_table[i].reg, Sensor_init_table[i].data, sen_data);
            }
            #endif
        }
    }

    return SUCCESS;
}
int pCus_init_hdr_3840x2160(ss_cus_sensor *handle)
{
    //AR0830_params *params = (AR0830_params *)handle->private_data;
    int i,cnt=0;
    #ifdef DUMP_SEN_I2C_DATA
    u16 sen_data=0;
    #endif
    for(i=0;i< ARRAY_SIZE(Sensor_init_table_hdr);i++)
    {
        if(Sensor_init_table_hdr[i].reg==0xffff)
        {
            SENSOR_MSLEEP(Sensor_init_table_hdr[i].data);
        }
        else
        {
            cnt = 0;
            #ifdef DUMP_SEN_I2C_DATA
            sen_data = 0;
            #endif
            while(SensorReg_Write(Sensor_init_table_hdr[i].reg, Sensor_init_table_hdr[i].data) != SUCCESS)
            {
                cnt++;
                if(cnt>=10)
                {
                    SENSOR_EMSG("[%s:%d]Sensor init fail!!\n", __FUNCTION__, __LINE__);
                    return FAIL;
                }
                //SENSOR_UDELAY(1);
            }
            #ifdef DUMP_SEN_I2C_DATA
            SensorReg_Read(Sensor_init_table_hdr[i].reg, &sen_data );
            CamOsPrintf("reg 0x%x, 0x%x, read back: 0x%x\n",Sensor_init_table_hdr[i].reg, Sensor_init_table_hdr[i].data, sen_data);
            if(Sensor_init_table_hdr[i].data!=sen_data)
            {
                CamOsPrintf("reg 0x%x, 0x%x, read back: 0x%x, different!!\n\n\n",
                            Sensor_init_table_hdr[i].reg, Sensor_init_table_hdr[i].data, sen_data);
            }
            #endif
        }
    }

    return SUCCESS;
}
int pCus_init_hdrlef(ss_cus_sensor *handle)
{

    return SUCCESS;
}
static int pCus_GetVideoResNum( ss_cus_sensor *handle, u32 *ulres_num)
{
    *ulres_num = handle->video_res_supported.num_res;

    return SUCCESS;
}

static int pCus_GetVideoRes(ss_cus_sensor *handle, u32 res_idx, cus_camsensor_res **res)
{
    u32 num_res = handle->video_res_supported.num_res;

    if (res_idx >= num_res) {
        SENSOR_EMSG("[%s] Please check the number of resolutions supported by the sensor!\n", __FUNCTION__);
        return FAIL;
    }

    *res = &handle->video_res_supported.res[res_idx];

    return SUCCESS;
}

static int pCus_GetCurVideoRes(ss_cus_sensor *handle, u32 *cur_idx, cus_camsensor_res **res)
{
    u32 num_res = handle->video_res_supported.num_res;

    *cur_idx = handle->video_res_supported.ulcur_res;

    if (*cur_idx >= num_res) {
        SENSOR_EMSG("[%s] Please check the number of resolutions supported by the sensor!\n", __FUNCTION__);
        return FAIL;
    }

    *res = &handle->video_res_supported.res[*cur_idx];

    return SUCCESS;
}

static int pCus_SetVideoRes(ss_cus_sensor *handle, u32 res_idx)
{
    AR0830_params *params = (AR0830_params *)handle->private_data;
    u32 num_res = handle->video_res_supported.num_res;

    if (res_idx >= num_res) {
        SENSOR_EMSG("[%s] Please check the number of resolutions supported by the sensor!\n", __FUNCTION__);
        return FAIL;
    }

    handle->video_res_supported.ulcur_res = res_idx;


    switch (res_idx) {
        case 0:
            handle->video_res_supported.ulcur_res = 0;
            handle->pCus_sensor_init = pCus_init_linear;
            params->expo.vts = vts_30fps_xDR;
            params->Default_Res_vts = vts_30fps_xDR;
            params->expo.fps = 30;
            params->Default_Res_line_period = Preview_line_period;
            handle->data_prec = CUS_DATAPRECISION_12;
            break;
        default:
            handle->video_res_supported.ulcur_res = 0;
            handle->pCus_sensor_init = pCus_init_linear;
            params->expo.vts = vts_30fps_xDR;
            params->Default_Res_vts = vts_30fps_xDR;
            params->expo.fps = 30;
            params->Default_Res_line_period = Preview_line_period;
            handle->data_prec = CUS_DATAPRECISION_12;
            break;
    }
    pCus_sensor_GetAEInfo(handle, &handle->sensor_ae_info_cfg);
    /*params->tVts_reg[0].data = (vts_30fps >> 16) & 0x0003;
    params->tVts_reg[1].data = (vts_30fps >> 8) & 0x00ff;
    params->tVts_reg[2].data = (vts_30fps >> 0) & 0x00ff;*/

    return SUCCESS;
}
static int pCus_SetVideoResHdr(ss_cus_sensor *handle, u32 res_idx)
{
    AR0830_params *params = (AR0830_params *)handle->private_data;
    u32 num_res = handle->video_res_supported.num_res;

    if (res_idx >= num_res) {
        SENSOR_EMSG("[%s] Please check the number of resolutions supported by the sensor!\n", __FUNCTION__);
        return FAIL;
    }
    handle->video_res_supported.ulcur_res = res_idx;


    switch (res_idx) {
        case 0:
            handle->video_res_supported.ulcur_res = 0;
            if (handle->interface_attr.attr_mipi.mipi_hdr_virtual_channel_num == 1) {
                handle->pCus_sensor_init = pCus_init_hdr_3840x2160;
                }
            params->expo.vts = vts_30fps_HDR_DOL_4lane;
            params->Default_Res_vts = vts_30fps_HDR_DOL_4lane;
            params->expo.fps = 30;
            params->Default_Res_line_period = Preview_line_period_HDR_DOL_4LANE;
            handle->data_prec = SENSOR_DATAPREC_DOL;
            break;
        default:
            handle->video_res_supported.ulcur_res = 0;
            if (handle->interface_attr.attr_mipi.mipi_hdr_virtual_channel_num == 1) {
                handle->pCus_sensor_init = pCus_init_hdr_3840x2160;
                }
            params->expo.vts = vts_30fps_HDR_DOL_4lane;
            params->Default_Res_vts = vts_30fps_HDR_DOL_4lane;
            params->expo.fps = 30;
            params->Default_Res_line_period = Preview_line_period_HDR_DOL_4LANE;
            handle->data_prec = SENSOR_DATAPREC_DOL;
            break;
    }

    /*params->tVts_reg[0].data = (vts_30fps >> 16) & 0x0003;
    params->tVts_reg[1].data = (vts_30fps >> 8) & 0x00ff;
    params->tVts_reg[2].data = (vts_30fps >> 0) & 0x00ff;*/

    return SUCCESS;
}

static int pCus_GetOrien(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT *orit)
{
    s16 sen_data;

    //Read SENSOR MIRROR-FLIP STATUS
    SensorReg_Read(MIRROR_FLIP, (void*)&sen_data);

    switch(sen_data & SENSOR_MIRROR_FLIP_EN)
    {
        case SENSOR_NOR:
            *orit = CUS_ORIT_M0F0;
            break;
        case SENSOR_FLIP_EN:
            *orit = CUS_ORIT_M0F1;
            break;
        case SENSOR_MIRROR_EN:
            *orit = CUS_ORIT_M1F0;
            break;
        case SENSOR_MIRROR_FLIP_EN:
            *orit = CUS_ORIT_M1F1;
            break;
    }
    return SUCCESS;
}

static int pCus_SetOrien(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT orit)
{
    AR0830_params *params = (AR0830_params *)handle->private_data;

    params->res.orit = orit;
    params->orien_dirty = true;

    return SUCCESS;
}

static int DoOrien(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT orit)
{
    s16 sen_data;
    AR0830_params *params = (AR0830_params *)handle->private_data;
    //Read SENSOR MIRROR-FLIP STATUS

    SensorReg_Read(MIRROR_FLIP, (void*)&sen_data);
    sen_data &= ~(SENSOR_MIRROR_FLIP_EN);
    params->res.orit = orit;

    switch(orit)
    {
        case CUS_ORIT_M0F0:
            sen_data = SENSOR_NOR;
            break;
        case CUS_ORIT_M1F0:
            sen_data = SENSOR_MIRROR_EN;
            break;
        case CUS_ORIT_M0F1:
            sen_data = SENSOR_FLIP_EN;
            break;
        case CUS_ORIT_M1F1:
            sen_data = SENSOR_MIRROR_FLIP_EN;
            break;
        default :
            sen_data = SENSOR_NOR;
            break;
    }
    //Write SENSOR MIRROR-FLIP STATUS
    SensorReg_Write(MIRROR_FLIP, sen_data);

    return SUCCESS;
}

static int pCus_GetFPS(ss_cus_sensor *handle)
{
    AR0830_params *params = (AR0830_params *)handle->private_data;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 tVts = params->tVts_reg[0].data;

    if (params->expo.fps >= 1000)
        params->expo.preview_fps = (params->Default_Res_vts*max_fps*1000)/tVts;
    else
        params->expo.preview_fps = (params->Default_Res_vts*max_fps)/tVts;

    return params->expo.preview_fps;
}

static int pCus_SetFPS(ss_cus_sensor *handle, u32 fps)
{
    AR0830_params *params = (AR0830_params *)handle->private_data;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 min_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16min_fps;

    if(fps>=min_fps && fps <= max_fps){
        params->expo.vts=  (params->Default_Res_vts*(max_fps*1000) + fps * 500)/(fps * 1000);
    }else if(fps>=(min_fps*1000) && fps <= (max_fps*1000)){
        params->expo.vts=  (params->Default_Res_vts*(max_fps*1000) + (fps>>1))/fps;
    }else{
      SENSOR_DMSG("[%s] FPS %d out of range.\n",__FUNCTION__,fps);
      return FAIL;
    }

    params->expo.fps = fps;
    params->dirty = true;

    pCus_SetAEUSecs(handle, params->expo.expo_lef_us);
    return SUCCESS;
}
static int pCus_GetFPSHdr(ss_cus_sensor *handle)
{
    AR0830_params *params = (AR0830_params *)handle->private_data;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 tVts = params->tVts_reg[0].data;

    if (params->expo.fps >= 1000)
        params->expo.preview_fps = (params->Default_Res_vts*max_fps*1000)/tVts;
    else
        params->expo.preview_fps = (params->Default_Res_vts*max_fps)/tVts;
    return params->expo.preview_fps;
}

static int pCus_SetFPSHdr(ss_cus_sensor *handle, u32 fps)
{
    AR0830_params *params = (AR0830_params *)handle->private_data;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 min_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16min_fps;

    if(fps>=min_fps && fps <= max_fps){
        params->expo.vts=  (params->Default_Res_vts*(max_fps*1000) + fps * 500)/(fps * 1000);
    }else if(fps>=(min_fps*1000) && fps <= (max_fps*1000)){
        params->expo.vts=  (params->Default_Res_vts*(max_fps*1000) + (fps>>1))/fps;
    }else{
      SENSOR_DMSG("[%s] FPS %d out of range.\n",__FUNCTION__,fps);
      return FAIL;
    }

    params->expo.fps = fps;
    params->dirty = true;
    pCus_sensor_GetAEInfo(handle, &handle->sensor_ae_info_cfg);
    pCus_SetAEUSecsHdrSef(handle, params->expo.expo_sef_us);
    pCus_SetAEUSecsHdrLef(handle, params->expo.expo_lef_us);
    return SUCCESS;
}

///////////////////////////////////////////////////////////////////////
// auto exposure
///////////////////////////////////////////////////////////////////////
// unit: micro seconds
//AE status notification
static int pCus_AEStatusNotify(ss_cus_sensor *handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status)
{
    AR0830_params *params = (AR0830_params *)handle->private_data;
    //ISensorIfAPI *sensor_if = handle->sensor_if_api;

    switch(status)
    {
        case CUS_FRAME_INACTIVE:
             break;
        case CUS_FRAME_ACTIVE:
            if(params->dirty || params->orien_dirty) {
                SensorReg_Write(0x0104, 0x0100); // Global hold on, 0x3001
                if(params->dirty) {
                    SensorRegArrayW((I2C_ARRAY*)params->tExpo_reg, ARRAY_SIZE(expo_reg));
                    SensorRegArrayW((I2C_ARRAY*)params->tGain_reg, ARRAY_SIZE(gain_reg));
                    SensorRegArrayW((I2C_ARRAY*)params->tVts_reg, ARRAY_SIZE(vts_reg));
                    /*CamOsPrintf(KERN_DEBUG "%s gain reg value:0x%x, expo_reg value:0x%x, vts_reg value:0x%x\n",
                                 __func__, params->tGain_reg[0].data,
                                 params->tExpo_reg[0].data, params->tVts_reg[0].data);*/
                    params->dirty = false;
                }

                if(params->orien_dirty) {
                    DoOrien(handle, params->res.orit);
                    params->orien_dirty = false;
                }
                SensorReg_Write(0x0104, 0x0000); // Global hold off
            }

            break;
        default :
             break;
    }
    return SUCCESS;
}
static int pCus_AEStatusNotifyHdrLef(ss_cus_sensor *handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status)
{
    return SUCCESS;
}
static int pCus_AEStatusNotifyHdrSef(ss_cus_sensor *handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status)
{
    AR0830_params *params = (AR0830_params *)handle->private_data;
    //s16 sen_data;
    //ISensorIfAPI *sensor_if = handle->sensor_if_api;
    switch(status)
    {
        case CUS_FRAME_INACTIVE:
             break;
        case CUS_FRAME_ACTIVE:
            if(params->dirty || params->orien_dirty) {
                SensorReg_Write(0x0104, 0x0100); // Global hold on
                if(params->dirty) {
                    SensorRegArrayW((I2C_ARRAY*)params->tExpo_hdr_dol_lef_reg, ARRAY_SIZE(expo_lef_reg));
                    SensorRegArrayW((I2C_ARRAY*)params->tExpo_hdr_dol_sef_reg, ARRAY_SIZE(expo_sef_reg));
                    SensorRegArrayW((I2C_ARRAY*)params->tGain_reg, ARRAY_SIZE(gain_reg));
                    SensorRegArrayW((I2C_ARRAY*)params->tVts_reg, ARRAY_SIZE(vts_reg));
                    //SensorReg_Read(params->tExpo_hdr_dol_lef_reg[0].reg, &sen_data );
                    //CamOsPrintf(KERN_DEBUG"Expo_hdr_dol_lef:reg 0x%x, 0x%x, read back: 0x%x\n",params->tExpo_hdr_dol_lef_reg[0].reg, params->tExpo_hdr_dol_lef_reg[0].data, sen_data);
                    //SensorReg_Read(params->tExpo_hdr_dol_sef_reg[0].reg, &sen_data );
                    //CamOsPrintf(KERN_DEBUG"Expo_hdr_dol_sef:reg 0x%x, 0x%x, read back: 0x%x\n",params->tExpo_hdr_dol_sef_reg[0].reg, params->tExpo_hdr_dol_sef_reg[0].data, sen_data);
                    //SensorReg_Read(params->tGain_reg[0].reg, &sen_data );
                    //CamOsPrintf(KERN_DEBUG"tGain:reg 0x%x, 0x%x, read back: 0x%x\n",params->tGain_reg[0].reg, params->tGain_reg[0].data, sen_data);
                   // SensorReg_Read(params->tVts_reg[0].reg, &sen_data );
                   // CamOsPrintf(KERN_DEBUG"tVts:reg 0x%x, 0x%x, read back: 0x%x\n",params->tVts_reg[0].reg, params->tVts_reg[0].data, sen_data);
                    /*CamOsPrintf(KERN_DEBUG "%s gain reg value:0x%x, expo_reg value:0x%x, vts_reg value:0x%x\n",
                                 __func__, params->tGain_reg[0].data,
                                 params->tExpo_reg[0].data, params->tVts_reg[0].data);*/
                    params->dirty = false;
                    //CamOsPrintf("[%s] set lef=%x sef:%x gain:%x\n", __FUNCTION__,
                    //    params->tExpo_hdr_dol_lef_reg[0].data, params->tExpo_hdr_dol_sef_reg[0].data, params->tGain_reg[0].data);
                }

                if(params->orien_dirty) {
                    DoOrien(handle, params->res.orit);
                    params->orien_dirty = false;
                }
                SensorReg_Write(0x0104, 0x0000); // Global hold off
            }

            break;
        default :
             break;
    }
    return SUCCESS;
}

static int pCus_GetAEUSecs(ss_cus_sensor *handle, u32 *us)
{
    u16 lines = 0;
    AR0830_params *params = (AR0830_params *)handle->private_data;

    lines = params->tExpo_reg[0].data;

    *us = (lines * params->Default_Res_line_period) / 1000;

    SENSOR_DMSG("[%s] sensor expo lines/us %u,%u us\n", __FUNCTION__, lines, *us);

    return SUCCESS;
}

static int pCus_SetAEUSecs(ss_cus_sensor *handle, u32 us)
{
    u16 expo_lines = 0, vts = 0;
    AR0830_params *params = (AR0830_params *)handle->private_data;

    params->expo.expo_lef_us = us;
    expo_lines = (1000*us)/params->Default_Res_line_period;

    if (expo_lines > params->expo.vts) {
        vts = expo_lines + 8;
    }
    else
      vts = params->expo.vts;

    params->expo.expo_lines = expo_lines;
    //params->expo.vts = vts;

    SENSOR_DMSG("[%s] us %u, vts %u\n", __FUNCTION__,
                                                 us,  \
                                                 vts);
    //pr_info("[%s]  leslie_shutter,expo_lines,params_expo_lines : %d,%d,%d\n\n", __FUNCTION__,us,expo_lines,params->expo.expo_lines);
    //pr_info("[%s]  leslie_shutter_vts : %u,%u\n\n", __FUNCTION__,params->expo.vts,vts);
    params->tExpo_reg[0].data = expo_lines;

    params->tVts_reg[0].data = vts;

    params->dirty = true;
    return SUCCESS;
}
static int pCus_GetAEUSecsHdrLef(ss_cus_sensor *handle, u32 *us)
{
    u16 lines = 0;
    AR0830_params *params = (AR0830_params *)handle->private_data;

    lines = params->tExpo_hdr_dol_lef_reg[0].data;

    *us = (lines * params->Default_Res_line_period) / 1000;

    SENSOR_DMSG("[%s] sensor expo lines/us %u,%u us\n", __FUNCTION__, lines, *us);

    return SUCCESS;
}

static int pCus_SetAEUSecsHdrLef(ss_cus_sensor *handle, u32 us)
{
    u16 expo_lines = 0, vts = 0;
    AR0830_params *params = (AR0830_params *)handle->private_data;

    params->expo.expo_lef_us = us;
    expo_lines = (1000*us)/params->Default_Res_line_period;

    if (expo_lines > params->expo.vts) {
        vts = (expo_lines + SENSOR_LIMIT_LINE + params->expo.expo_lines_sef + 10)& 0xFFFC;
    }
    else
      vts = params->expo.vts;

    params->expo.expo_lines = expo_lines;
    params->expo.vts_lef = vts;

    SENSOR_DMSG("[%s] us %u, vts %u, expo_lines %u\n", __FUNCTION__,
                                                 us,  \
                                                 vts,expo_lines
               );
    //pr_info("[%s]  leslie_shutter,expo_lines,params_expo_lines : %d,%d,%d\n\n", __FUNCTION__,us,expo_lines,params->expo.expo_lines);
    //pr_info("[%s]  leslie_shutter_vts : %u,%u\n\n", __FUNCTION__,params->expo.vts,vts);
    params->tExpo_hdr_dol_lef_reg[0].data = expo_lines;

    if(vts != params->expo.vts)
    {
        if(params->tVts_reg[0].data < vts)
        {
            params->tVts_reg[0].data = vts;
        }
    }
    else
    {
        if(params->expo.vts_sef == params->expo.vts)
        {
            params->tVts_reg[0].data = params->expo.vts;
        }
    }
    params->dirty = true;
    return SUCCESS;
}
static int pCus_GetAEUSecsHdrSef(ss_cus_sensor *handle, u32 *us)
{
    u16 lines = 0;
    AR0830_params *params = (AR0830_params *)handle->private_data;

    lines = params->tExpo_hdr_dol_sef_reg[0].data;

    *us = (lines * params->Default_Res_line_period) / 1000;

    SENSOR_DMSG("[%s] sensor expo lines/us %u,%u us\n", __FUNCTION__, lines, *us);

    return SUCCESS;
}

static int pCus_SetAEUSecsHdrSef(ss_cus_sensor *handle, u32 us)
{
    u16 expo_lines = 0, vts = 0,height = 3840;
    AR0830_params *params = (AR0830_params *)handle->private_data;

    params->expo.expo_sef_us = us;
    height = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16height;
    expo_lines = (1000*us)/params->Default_Res_line_period;

    if (expo_lines > (params->expo.vts - SENSOR_LIMIT_LINE - height*2 - 10)) {
        vts = (expo_lines + 12 + height*2 + 4) & 0xFFFC;
    }
    else
    {
        vts = params->expo.vts;
    }
    params->expo.expo_lines_sef = expo_lines;
    params->expo.vts_sef = vts;

    SENSOR_DMSG("[%s] us %u, vts %u expo_lines %u\n", __FUNCTION__,
                                                 us,  \
                                                 vts,expo_lines
               );
    //pr_info("[%s]  leslie_shutter,expo_lines,params_expo_lines : %d,%d,%d\n\n", __FUNCTION__,us,expo_lines,params->expo.expo_lines);
    //pr_info("[%s]  leslie_shutter_vts : %u,%u\n\n", __FUNCTION__,params->expo.vts,vts);
    params->tExpo_hdr_dol_sef_reg[0].data = expo_lines;
    if(vts != params->expo.vts)
    {
        if(params->tVts_reg[0].data < vts)
        {
            params->tVts_reg[0].data = vts;
        }
    }
    else
    {
        if(params->expo.vts_lef == params->expo.vts)
        {
            params->tVts_reg[0].data = params->expo.vts;
        }
    }
    params->dirty = true;
    return SUCCESS;
}


// Gain: 1x = 1024
static int pCus_GetAEGain(ss_cus_sensor *handle, u32* gain)
{
    AR0830_params *params = (AR0830_params *)handle->private_data;

    *gain = params->expo.final_gain;
    SENSOR_DMSG("[%s] get gain %u\n", __FUNCTION__, *gain);

    return SUCCESS;
}

static int pCus_SetAEGain(ss_cus_sensor *handle, u32 gain)
{
    AR0830_params *params = (AR0830_params *)handle->private_data;
    u32 i;
    unsigned short index;
    index = 0;

    if(gain < SENSOR_MIN_GAIN)
    {
        index = 0;
    }
    else if(gain >= SENSOR_MAX_GAIN)
    {
        index = 71;
    }
    else
    {
        for(i = 0; i < sizeof(gain_table)/sizeof(gain_table[0])-1; i++){
            if((gain>=gain_table[i].total_gain) && (gain < gain_table[(i+1)].total_gain)){
                  index = i;
                  break;
            }
        }
    }

    params->tGain_reg[0].data = gain_table[index].reg_val & 0xff;
    params->expo.final_gain = gain_table[index].total_gain;

    SENSOR_DMSG("[%s] set gain/reg=%u/0x%x\n", __FUNCTION__, gain,params->tGain_reg[0].data);
    params->dirty = true;
    return SUCCESS;
}
static int pCus_GetAEGainHdrLef(ss_cus_sensor *handle, u32* gain)
{
    AR0830_params *params = (AR0830_params *)handle->private_data;

    *gain = params->expo.final_lef_gain;
    SENSOR_DMSG("[%s] get gain %u\n", __FUNCTION__, *gain);

    return SUCCESS;
}
u32 pCus_SetAEGainTryHdr(ss_cus_sensor *handle, u32 gain)
{
    u32 i;
    unsigned short index;

    index = 0;
    if(gain < SENSOR_MIN_GAIN)
    {
        index = 0;
    }
    else if(gain >= SENSOR_MAX_GAIN_HDR)
    {
        index = 127;
    }
    else
    {
        for(i = 0; i < sizeof(gain_table_hdr)/sizeof(gain_table_hdr[0])-1; i++){
            if((gain>=gain_table_hdr[i].total_gain) && (gain < gain_table_hdr[(i+1)].total_gain)){
                        index = i;
                    break;
            }
        }
    }
    return gain_table_hdr[index].total_gain;
}

static int pCus_SetAEGainHdrLef(ss_cus_sensor *handle, u32 gain)
{
    AR0830_params *params = (AR0830_params *)handle->private_data;
    u32 i;
    unsigned short index;

    index = 0;
    if(gain < SENSOR_MIN_GAIN)
    {
        index = 0;
    }
    else if(gain >= SENSOR_MAX_GAIN_HDR)
    {
        index = 127;
    }
    else
    {
        for(i = 0; i < sizeof(gain_table_hdr)/sizeof(gain_table_hdr[0])-1; i++){
            if((gain>=gain_table_hdr[i].total_gain) && (gain < gain_table_hdr[(i+1)].total_gain)){
                        index = i;
                    break;
            }
        }
    }

    params->tGain_reg[0].data = (gain_table_hdr[index].reg_val & 0xff) | (params->tGain_reg[0].data&0xff00);
    params->expo.final_lef_gain = gain_table_hdr[index].total_gain;

    SENSOR_DMSG("[%s] set gain/reg=%u/0x%x\n", __FUNCTION__, gain,params->tGain_reg[0].data);
    params->dirty = true;
    return SUCCESS;
}
static int pCus_GetAEGainHdrSef(ss_cus_sensor *handle, u32* gain)
{
    AR0830_params *params = (AR0830_params *)handle->private_data;

    *gain = params->expo.final_sef_gain ;
    SENSOR_DMSG("[%s] get gain %u\n", __FUNCTION__, *gain);

    return SUCCESS;
}

static int pCus_SetAEGainHdrSef(ss_cus_sensor *handle, u32 gain)
{
    AR0830_params *params = (AR0830_params *)handle->private_data;
    u32 i;
    unsigned short index;
    index = 0;

    if(gain < SENSOR_MIN_GAIN)
    {
        index = 0;
    }
    else if(gain >= SENSOR_MAX_GAIN_HDR)
    {
        index = 127;
    }
    else
    {
        for(i = 0; i < sizeof(gain_table_hdr)/sizeof(gain_table_hdr[0]); i++){
            if(i == sizeof(gain_table_hdr)/sizeof(gain_table_hdr[0])-1){
                index = i;
                break;
            }
            if((gain>=gain_table_hdr[i].total_gain) && (gain < gain_table_hdr[(i+1)].total_gain)){
                        index = i;
                    break;
            }
        }
    }

    params->tGain_reg[0].data = (gain_table_hdr[index].reg_val & 0xff)<< 8 | (params->tGain_reg[0].data&0xff);
    params->expo.final_sef_gain = gain_table_hdr[index].total_gain;

    SENSOR_DMSG("[%s] set gain/reg=%u/0x%x\n", __FUNCTION__, gain,params->tGain_reg[0].data);
    params->dirty = true;
    return SUCCESS;
}


int cus_camsensor_init_handle_linear(ss_cus_sensor* drv_handle)
{
    ss_cus_sensor *handle = drv_handle;
    AR0830_params *params;
    int res;

    if (!handle) {
        SENSOR_DMSG("[%s] not enough memory!\n", __FUNCTION__);
        return FAIL;
    }
    SENSOR_DMSG("[%s]", __FUNCTION__);
    ////////////////////////////////////
    // private data allocation & init //
    ////////////////////////////////////
    if (handle->private_data == NULL) {
        SENSOR_EMSG("[%s] Private data is empty!\n", __FUNCTION__);
        return FAIL;
    }

    params = (AR0830_params *)handle->private_data;
    memcpy(params->tVts_reg, vts_reg, sizeof(vts_reg));
    memcpy(params->tGain_reg, gain_reg, sizeof(gain_reg));
    memcpy(params->tExpo_reg, expo_reg, sizeof(expo_reg));
    memcpy(params->tExpo_hdr_dol_sef_reg, expo_sef_reg, sizeof(expo_sef_reg));
    memcpy(params->tExpo_hdr_dol_lef_reg, expo_lef_reg, sizeof(expo_lef_reg));

    ////////////////////////////////////
    //    sensor model ID             //
    ////////////////////////////////////
    sprintf(handle->strSensorStreamName,"AR0830_MIPI");

    ////////////////////////////////////
    //    i2c config                  //
    ////////////////////////////////////
    handle->i2c_cfg.mode          = SENSOR_I2C_LEGACY;    //(CUS_ISP_I2C_MODE) FALSE;
    handle->i2c_cfg.fmt           = SENSOR_I2C_FMT;       //CUS_I2C_FMT_A16D8;
    handle->i2c_cfg.address       = SENSOR_I2C_ADDR;      //0x34;
    handle->i2c_cfg.speed         = SENSOR_I2C_SPEED;     //300000;

    ////////////////////////////////////
    //    mclk                        //
    ////////////////////////////////////
   // handle->mclk                  = UseParaMclk(SENSOR_DRV_PARAM_MCLK());
    handle->mclk                    =Preview_MCLK_SPEED;

    ////////////////////////////////////
    //    sensor interface info       //
    ////////////////////////////////////
    //handle->data_fmt              = SENSOR_DATAFMT;
    handle->sif_bus               = SENSOR_IFBUS_TYPE;
    handle->data_prec             = SENSOR_DATAPREC;
    handle->bayer_id              = SENSOR_BAYERID;
    handle->RGBIR_id              = SENSOR_RGBIRID;
    //handle->YC_ODER               = SENSOR_YCORDER;   //CUS_SEN_YCODR_CY;
    handle->interface_attr.attr_mipi.mipi_lane_num    = SENSOR_MIPI_LANE_NUM;
    handle->interface_attr.attr_mipi.mipi_data_format = CUS_SEN_INPUT_FORMAT_RGB; // RGB pattern.
    handle->interface_attr.attr_mipi.mipi_yuv_order   = 0; //don't care in RGB pattern.
    handle->interface_attr.attr_mipi.mipi_hdr_mode    = CUS_HDR_MODE_NONE;
    handle->interface_attr.attr_mipi.mipi_hdr_virtual_channel_num = 0; //Short frame

    ////////////////////////////////////
    //    resolution capability       //
    ////////////////////////////////////
    handle->video_res_supported.ulcur_res = 0; //default resolution index is 0.
   //handle->video_res_supported.num_res = LINEAR_RES_END;
    for (res = 0; res < LINEAR_RES_END; res++) {
        handle->video_res_supported.num_res = res+1;
        handle->video_res_supported.res[res].u16width         = AR0830_mipi_linear[res].senif.preview_w;
        handle->video_res_supported.res[res].u16height        = AR0830_mipi_linear[res].senif.preview_h;
        handle->video_res_supported.res[res].u16max_fps       = AR0830_mipi_linear[res].senout.max_fps;
        handle->video_res_supported.res[res].u16min_fps       = AR0830_mipi_linear[res].senout.min_fps;
        handle->video_res_supported.res[res].u16crop_start_x  = AR0830_mipi_linear[res].senif.crop_start_X;
        handle->video_res_supported.res[res].u16crop_start_y  = AR0830_mipi_linear[res].senif.crop_start_y;
        handle->video_res_supported.res[res].u16OutputWidth  = AR0830_mipi_linear[res].senout.width;
        handle->video_res_supported.res[res].u16OutputHeight = AR0830_mipi_linear[res].senout.height;
        sprintf(handle->video_res_supported.res[res].strResDesc, AR0830_mipi_linear[res].senstr.strResDesc);
    }

    ////////////////////////////////////////
    // Sensor Status Control and Get Info //
    ////////////////////////////////////////
    handle->pCus_sensor_init           = pCus_init_linear;
    //handle->pCus_sensor_powerupseq     = pCus_powerupseq;
    handle->pCus_sensor_poweron        = pCus_poweron;
    handle->pCus_sensor_poweroff       = pCus_poweroff;
    handle->pCus_sensor_GetVideoResNum = pCus_GetVideoResNum;
    handle->pCus_sensor_GetVideoRes    = pCus_GetVideoRes;
    handle->pCus_sensor_GetCurVideoRes = pCus_GetCurVideoRes;
    handle->pCus_sensor_SetVideoRes    = pCus_SetVideoRes;

    handle->pCus_sensor_GetOrien       = pCus_GetOrien;
    handle->pCus_sensor_SetOrien       = pCus_SetOrien;
    handle->pCus_sensor_GetFPS         = pCus_GetFPS;
    handle->pCus_sensor_SetFPS         = pCus_SetFPS;
    handle->pCus_sensor_SetPatternMode = AR0830_SetPatternMode; //NONE
    params->Default_Res_vts = vts_30fps_xDR;
    params->Default_Res_line_period = Preview_line_period;
    ////////////////////////////////////
    //    AE parameters               //
    ////////////////////////////////////
    params->expo.max_sef = 0;
    handle->sensor_ae_info_cfg.u8AEGainDelay                      = SENSOR_GAIN_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEShutterDelay                   = SENSOR_SHUTTER_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u32AEGain_min                      = SENSOR_MIN_GAIN;
    handle->sensor_ae_info_cfg.u32AEGain_max                      = SENSOR_MAX_GAIN;
    handle->sensor_ae_info_cfg.u32AEShutter_min                   = (params->Default_Res_line_period * 1);
    handle->sensor_ae_info_cfg.u32AEShutter_max                   = 1000000000/AR0830_mipi_linear[0].senout.min_fps;
    handle->sensor_ae_info_cfg.u32AEShutter_step                  = params->Default_Res_line_period;
    handle->sensor_ae_info_cfg.u8AEGainCtrlNum                    = 1;
    handle->sensor_ae_info_cfg.u8AEShutterCtrlNum                 = 1;
    //handle->dgain_remainder = 0;

    ////////////////////////////////////
    //  AE Control and Get Info       //
    ////////////////////////////////////
    // unit: micro seconds
    //handle->pCus_sensor_GetAETrigger_mode      = pCus_GetAETrigger_mode;
    //handle->pCus_sensor_SetAETrigger_mode      = pCus_SetAETrigger_mode;
    handle->pCus_sensor_AEStatusNotify  = pCus_AEStatusNotify;
    handle->pCus_sensor_GetAEUSecs      = pCus_GetAEUSecs;
    handle->pCus_sensor_SetAEUSecs      = pCus_SetAEUSecs;
    handle->pCus_sensor_GetAEGain       = pCus_GetAEGain;
    handle->pCus_sensor_SetAEGain       = pCus_SetAEGain;
    handle->pCus_sensor_GetAEInfo       = pCus_sensor_GetAEInfo;


    params->res.orit = SENSOR_ORIT;
    params->expo.vts = vts_30fps_xDR;

    return SUCCESS;
}
int cus_camsensor_init_handle_hdr_dcg_sef(ss_cus_sensor* drv_handle)
{
    ss_cus_sensor *handle = drv_handle;
    AR0830_params *params = NULL;
    int res;

    cus_camsensor_init_handle_linear(drv_handle);
    params = (AR0830_params *)handle->private_data;
    sprintf(handle->strSensorStreamName,"AR0830_MIPI_HDR_SEF");

    handle->data_prec   = SENSOR_DATAPREC_DOL;  //CUS_DATAPRECISION_8;
    handle->bayer_id    = SENSOR_BAYERID_HDR_DOL;   //CUS_BAYER_GB;
    handle->RGBIR_id    = SENSOR_RGBIRID;
    //handle->YC_ODER     = SENSOR_YCORDER;   //CUS_SEN_YCODR_CY;
    handle->interface_attr.attr_mipi.mipi_lane_num = SENSOR_MIPI_LANE_NUM_DOL;
    handle->interface_attr.attr_mipi.mipi_hdr_mode = CUS_HDR_MODE_VC;
    handle->interface_attr.attr_mipi.mipi_hdr_virtual_channel_num = 1; //Short frame

    ////////////////////////////////////
    //    resolution capability       //
    ////////////////////////////////////
    handle->video_res_supported.ulcur_res = 0;
    for (res = 0; res < HDR_RES_END; res++) {
        handle->video_res_supported.num_res = res+1;
        handle->video_res_supported.res[res].u16width         = AR0830_mipi_hdr[res].senif.preview_w;
        handle->video_res_supported.res[res].u16height        = AR0830_mipi_hdr[res].senif.preview_h;
        handle->video_res_supported.res[res].u16max_fps       = AR0830_mipi_hdr[res].senout.max_fps;
        handle->video_res_supported.res[res].u16min_fps       = AR0830_mipi_hdr[res].senout.min_fps;
        handle->video_res_supported.res[res].u16crop_start_x  = AR0830_mipi_hdr[res].senif.crop_start_X;
        handle->video_res_supported.res[res].u16crop_start_y  = AR0830_mipi_hdr[res].senif.crop_start_y;
        handle->video_res_supported.res[res].u16OutputWidth  = AR0830_mipi_hdr[res].senout.width;
        handle->video_res_supported.res[res].u16OutputHeight = AR0830_mipi_hdr[res].senout.height;
        sprintf(handle->video_res_supported.res[res].strResDesc, AR0830_mipi_hdr[res].senstr.strResDesc);
    }

    // mclk
    handle->mclk                        = Preview_MCLK_SPEED_HDR_DOL;
    params->Default_Res_vts = vts_30fps_HDR_DOL_4lane;
    params->Default_Res_line_period = Preview_line_period_HDR_DOL_4LANE;
    ////////////////////////////////////////////////////
    // AE parameters
    ////////////////////////////////////////////////////
    params->expo.max_sef = 216;
    handle->sensor_ae_info_cfg.u8AEGainDelay                      = SENSOR_GAIN_DELAY_FRAME_COUNT_HDR_DOL;
    handle->sensor_ae_info_cfg.u8AEShutterDelay                   = SENSOR_SHUTTER_DELAY_FRAME_COUNT_HDR_DOL;
    handle->sensor_ae_info_cfg.u32AEGain_min                      = SENSOR_MIN_GAIN;
    handle->sensor_ae_info_cfg.u32AEGain_max                      = SENSOR_MAX_GAIN_HDR;
    handle->sensor_ae_info_cfg.u32AEShutter_min                   = (params->Default_Res_line_period * 1);
    handle->sensor_ae_info_cfg.u32AEShutter_max                   = (params->Default_Res_line_period * params->expo.max_sef);
    handle->sensor_ae_info_cfg.u32AEShutter_step                  = params->Default_Res_line_period;
    handle->sensor_ae_info_cfg.u8AEGainCtrlNum                    = SENSOR_GAIN_CTRL_NUM;
    handle->sensor_ae_info_cfg.u8AEShutterCtrlNum                 = SENSOR_SHUTTER_CTRL_NUM;

    //LOGD("[%s:%d]\n", __FUNCTION__, __LINE__);
    handle->pCus_sensor_init        = pCus_init_hdr_3840x2160;

    // Normal
    handle->pCus_sensor_SetVideoRes       = pCus_SetVideoResHdr;
    handle->pCus_sensor_GetFPS          = pCus_GetFPSHdr;
    handle->pCus_sensor_SetFPS          = pCus_SetFPSHdr;

    handle->pCus_sensor_AEStatusNotify = pCus_AEStatusNotifyHdrSef;
    handle->pCus_sensor_GetAEUSecs      = pCus_GetAEUSecsHdrSef;
    handle->pCus_sensor_SetAEUSecs      = pCus_SetAEUSecsHdrSef;
    handle->pCus_sensor_GetAEGain       = pCus_GetAEGainHdrSef;
    handle->pCus_sensor_SetAEGain       = pCus_SetAEGainHdrSef;
    handle->pCus_sensor_TryAEGain       = pCus_SetAEGainTryHdr;
    handle->pCus_sensor_GetAEInfo       = pCus_sensor_GetAEInfo;

    params->res.orit = SENSOR_ORIT;
    params->expo.vts=vts_30fps_HDR_DOL_4lane;
    params->expo.fps = 30;
    params->expo.expo_sef_us = 1000;
    params->expo.vts_sef = vts_30fps_HDR_DOL_4lane;

    return SUCCESS;
}

int cus_camsensor_init_handle_hdr_dcg_lef(ss_cus_sensor* drv_handle)
{
    ss_cus_sensor *handle = drv_handle;
    AR0830_params *params;
    int res;

    if (!handle) {
        SENSOR_DMSG("[%s] not enough memory!\n", __FUNCTION__);
        return FAIL;
    }
    //private data allocation & init
    if (handle->private_data == NULL) {
        SENSOR_EMSG("[%s] Private data is empty!\n", __FUNCTION__);
        return FAIL;
    }
    params = (AR0830_params *)handle->private_data;
    memcpy(params->tVts_reg, vts_reg, sizeof(vts_reg));
    memcpy(params->tGain_reg, gain_reg, sizeof(gain_reg));
    memcpy(params->tExpo_reg, expo_reg, sizeof(expo_reg));
    memcpy(params->tExpo_hdr_dol_sef_reg, expo_sef_reg, sizeof(expo_sef_reg));
    memcpy(params->tExpo_hdr_dol_lef_reg, expo_lef_reg, sizeof(expo_lef_reg));

    ////////////////////////////////////
    //    sensor model ID                           //
    ////////////////////////////////////
    sprintf(handle->strSensorStreamName,"AR0830_MIPI_HDR_LEF");

    ////////////////////////////////////
    //    sensor interface info       //
    ////////////////////////////////////
    //SENSOR_DMSG1("[%s] entering function with id %d\n", __FUNCTION__, id);
    //handle->data_fmt    = SENSOR_DATAFMT;   //CUS_DATAFMT_YUV;
    handle->sif_bus     = SENSOR_IFBUS_TYPE;//CUS_SENIF_BUS_PARL;
    handle->data_prec   = SENSOR_DATAPREC_DOL;  //CUS_DATAPRECISION_8;
    handle->bayer_id    = SENSOR_BAYERID_HDR_DOL;   //CUS_BAYER_GB;
    handle->RGBIR_id    = SENSOR_RGBIRID;
    //handle->YC_ODER     = SENSOR_YCORDER;   //CUS_SEN_YCODR_CY;
    handle->interface_attr.attr_mipi.mipi_lane_num = SENSOR_MIPI_LANE_NUM_DOL;
    handle->interface_attr.attr_mipi.mipi_data_format = CUS_SEN_INPUT_FORMAT_RGB; // RGB pattern.
    handle->interface_attr.attr_mipi.mipi_yuv_order = 0; //don't care in RGB pattern.
    handle->interface_attr.attr_mipi.mipi_hdr_mode = CUS_HDR_MODE_VC;
    handle->interface_attr.attr_mipi.mipi_hdr_virtual_channel_num = 0; //Long frame

    ////////////////////////////////////
    //    resolution capability       //
    ////////////////////////////////////
    handle->video_res_supported.ulcur_res = 0;
    for (res = 0; res < HDR_RES_END; res++) {
        handle->video_res_supported.num_res = res+1;
        handle->video_res_supported.res[res].u16width         = AR0830_mipi_hdr[res].senif.preview_w;
        handle->video_res_supported.res[res].u16height        = AR0830_mipi_hdr[res].senif.preview_h;
        handle->video_res_supported.res[res].u16max_fps       = AR0830_mipi_hdr[res].senout.max_fps;
        handle->video_res_supported.res[res].u16min_fps       = AR0830_mipi_hdr[res].senout.min_fps;
        handle->video_res_supported.res[res].u16crop_start_x  = AR0830_mipi_hdr[res].senif.crop_start_X;
        handle->video_res_supported.res[res].u16crop_start_y  = AR0830_mipi_hdr[res].senif.crop_start_y;
        handle->video_res_supported.res[res].u16OutputWidth  = AR0830_mipi_hdr[res].senout.width;
        handle->video_res_supported.res[res].u16OutputHeight = AR0830_mipi_hdr[res].senout.height;
        sprintf(handle->video_res_supported.res[res].strResDesc, AR0830_mipi_hdr[res].senstr.strResDesc);
    }

    // i2c
    handle->i2c_cfg.mode                = SENSOR_I2C_LEGACY;    //(CUS_ISP_I2C_MODE) FALSE;
    handle->i2c_cfg.fmt                 = SENSOR_I2C_FMT;       //CUS_I2C_FMT_A16D16;
    handle->i2c_cfg.address             = SENSOR_I2C_ADDR;      //0x5a;
    handle->i2c_cfg.speed               = SENSOR_I2C_SPEED;     //320000;

    // mclk
    handle->mclk                        = Preview_MCLK_SPEED_HDR_DOL;

    params->Default_Res_line_period = Preview_line_period_HDR_DOL_4LANE;
    params->Default_Res_vts = vts_30fps_HDR_DOL_4lane;
    ////////////////////////////////////////////////////
    // AE parameters
    ////////////////////////////////////////////////////
    params->expo.max_sef = 0;
    handle->sensor_ae_info_cfg.u8AEGainDelay                      = SENSOR_GAIN_DELAY_FRAME_COUNT_HDR_DOL;
    handle->sensor_ae_info_cfg.u8AEShutterDelay                   = SENSOR_SHUTTER_DELAY_FRAME_COUNT_HDR_DOL;
    handle->sensor_ae_info_cfg.u32AEGain_min                      = SENSOR_MIN_GAIN;
    handle->sensor_ae_info_cfg.u32AEGain_max                      = SENSOR_MAX_GAIN_HDR;
    handle->sensor_ae_info_cfg.u32AEShutter_min                   = (params->Default_Res_line_period * 1);
    handle->sensor_ae_info_cfg.u32AEShutter_max                   = 1000000000/AR0830_mipi_hdr[0].senout.min_fps;
    handle->sensor_ae_info_cfg.u32AEShutter_step                  = params->Default_Res_line_period;
    handle->sensor_ae_info_cfg.u8AEGainCtrlNum                    = SENSOR_GAIN_CTRL_NUM;
    handle->sensor_ae_info_cfg.u8AEShutterCtrlNum                 = SENSOR_SHUTTER_CTRL_NUM;


    //LOGD("[%s:%d]\n", __FUNCTION__, __LINE__);
    handle->pCus_sensor_init        = pCus_init_hdrlef;
    handle->pCus_sensor_poweron     = pCus_poweronhdr;
    handle->pCus_sensor_poweroff    = pCus_poweronhdr;

    // Normal
    handle->pCus_sensor_GetVideoResNum = pCus_GetVideoResNum;
    handle->pCus_sensor_GetVideoRes       = pCus_GetVideoRes;
    handle->pCus_sensor_GetCurVideoRes  =  pCus_GetCurVideoRes;
    handle->pCus_sensor_SetVideoRes       = pCus_SetVideoResHdr;
    handle->pCus_sensor_GetOrien          =  pCus_GetOrien      ;
    handle->pCus_sensor_SetOrien          =  pCus_SetOrien   ;
    handle->pCus_sensor_GetFPS          = pCus_GetFPSHdr;
    handle->pCus_sensor_SetFPS          = pCus_SetFPSHdr;
    handle->pCus_sensor_SetPatternMode = AR0830_SetPatternMode;
    ///////////////////////////////////////////////////////
    // AE
    ///////////////////////////////////////////////////////
    // unit: micro seconds
    handle->pCus_sensor_AEStatusNotify = pCus_AEStatusNotifyHdrLef;
    handle->pCus_sensor_GetAEUSecs      = pCus_GetAEUSecsHdrLef;
    handle->pCus_sensor_SetAEUSecs      = pCus_SetAEUSecsHdrLef;
    handle->pCus_sensor_GetAEGain       = pCus_GetAEGainHdrLef;
    handle->pCus_sensor_SetAEGain       = pCus_SetAEGainHdrLef;
    handle->pCus_sensor_TryAEGain       = pCus_SetAEGainTryHdr;
    handle->pCus_sensor_GetAEInfo       = pCus_sensor_GetAEInfo;

    params->res.orit = SENSOR_ORIT;
    params->expo.vts=vts_30fps_HDR_DOL_4lane;
    params->expo.fps = 30;
    params->expo.expo_lines = 1000;
    params->expo.expo_lef_us = 33000;
    params->expo.vts_lef = vts_30fps_HDR_DOL_4lane;

    params->orien_dirty = false;
    params->dirty = false;

    return SUCCESS;
}

SENSOR_DRV_ENTRY_IMPL_END_EX( AR0830_HDR,
                            cus_camsensor_init_handle_linear,
                            cus_camsensor_init_handle_hdr_dcg_sef,
                            cus_camsensor_init_handle_hdr_dcg_lef,
                            AR0830_params
                         );
