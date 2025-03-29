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
   Date : 2023/10/27
   Build on : Master V4  I6C
   Verified on : mixer preview ok (linear/hdr),
               AE gain/shutter ok , IQ not verify.
   Remark : NA
*/

#ifdef __cplusplus
extern "C"
{
#endif

#include <drv_sensor_common.h>
#include <sensor_i2c_api.h>
#include <drv_sensor.h>

#ifdef __cplusplus
}
#endif

SENSOR_DRV_ENTRY_IMPL_BEGIN_EX(HM4030);
// SENSOR_UPDATE_SEF_STEP : because of sensor limitation, one frame interval only can update sef < (vts - height -20)
// SENSOR_UPDATE_SEF_DELAY: update one shot can workaround update issue.

#define SENSOR_UPDATE_SEF_STEP //#define SENSOR_UPDATE_SEF_DELAY
#define SENSOR_MIPI_LANE_NUM (4)
#define SENSOR_MODEL_ID     "HM4030_MIPI"
#define SENSOR_IFBUS_TYPE   CUS_SENIF_BUS_MIPI      //CUS_SENIF_BUS_PARL, CUS_SENIF_BUS_MIPI
#define SENSOR_MIPI_DELAY   0x1212                  //CFG
#define SENSOR_DATAPREC     CUS_DATAPRECISION_10
#define SENSOR_BAYERID      CUS_BAYER_BG            //CUS_BAYER_GB, CUS_BAYER_GR, CUS_BAYER_BG, CUS_BAYER_RG
//HDR
//#define SENSOR_MIPI_HSYNC_MODE PACKET_FOOTER_EDGE//PACKET_HEADER_EDGE1
#define SENSOR_BAYERID_HDR      CUS_BAYER_BG//CUS_BAYER_GR
#define SENSOR_DATAPREC_HDR            CUS_DATAPRECISION_10


#define SENSOR_RGBIRID      CUS_RGBIR_NONE
#define SENSOR_ORIT         CUS_ORIT_M0F0           //CUS_ORIT_M0F0, CUS_ORIT_M1F0, CUS_ORIT_M0F1, CUS_ORIT_M1F1,
#define SENSOR_MAX_GAIN     261120//(255*1024)                  // max sensor gain, again 64x, dgain 3.984375x  //dgain 6bit float: 0x3F / 2^6 = float
#define SENSOR_MIN_GAIN      (1 * 1024)


#define Preview_MCLK_SPEED  CUS_CMU_CLK_24MHZ         //CUS_CMU_CLK_12M, CUS_CMU_CLK_16M, CUS_CMU_CLK_27M
#define Preview_MAX_FPS     60                      //fastest preview FPS
#define Preview_MIN_FPS     5                       //slowest preview FPS

#define hts_60fps_Linear_2064           (2200)
#define vts_60fps_Linear_2064           (2141)                    //for 60fps 2124
#define PREVIEW_LINE_PERIOD(vts, fps)  (1000000000 / (vts) / (fps))                   //2124

#define Preview_WIDTH       2064                    //resolution Width when preview
#define Preview_HEIGHT      2064                   //resolution Height when preview
//HDR
#define Preview_MAX_FPS_HDR     30                      //fastest preview FPS
#define Preview_MIN_FPS_HDR     5                       //slowest preview FPS

#define Preview_WIDTH_HDR       2064                  //resolution Width when preview
#define Preview_HEIGHT_HDR      2064                   //resolution Height when preview
#define vts_30fps_HDR_2064      (2134)//(2124
#define hts_30fps_HDR_2064      ((2243*2))//(2288

#define SENSOR_MAX_GAIN_HDR     (255*1024)                  //
#define SENSOR_MIN_GAIN_HDR      (1 * 1024)
#define SENSOR_GAIN_DELAY_FRAME_COUNT               (2)
#define SENSOR_GAIN_DELAY_FRAME_COUNT_HDR_DOL       (2)
#define SENSOR_SHUTTER_DELAY_FRAME_COUNT            (2)
#define SENSOR_SHUTTER_DELAY_FRAME_COUNT_HDR_DOL    (2)
#define SENSOR_GAIN_CTRL_NUM                  (2)
#define SENSOR_SHUTTER_CTRL_NUM               (2)
#define SENSOR_SEF_TH               8
#define SENSOR_DARK_ROW             20
#define SENSOR_LS_OFFSET            ((Preview_HEIGHT_HDR/11) + SENSOR_SEF_TH) //(136) //(Preview_HEIGHT_HDR/11)
//#define Preview_HEIGHT_HDR_DOL        2080


#define Preview_CROP_START_X     0                      //CROP_START_X
#define Preview_CROP_START_Y     0                      //CROP_START_Y

#define SENSOR_I2C_ADDR     0x50
#define SENSOR_I2C_FMT      I2C_FMT_A16D8
#define SENSOR_I2C_SPEED    200000
#define SENSOR_I2C_LEGACY  I2C_NORMAL_MODE     //usally set CUS_I2C_NORMAL_MODE,  if use old OVT I2C protocol=> set CUS_I2C_LEGACY_MODE

#define SENSOR_PWDN_POL     CUS_CLK_POL_NEG
#define SENSOR_RST_POL      CUS_CLK_POL_NEG
#define SENSOR_PCLK_POL     CUS_CLK_POL_NEG        // depend on sensor setting, sometimes need to try CUS_CLK_POL_POS or CUS_CLK_POL_NEG

typedef enum
{
    E_CAMERA_UPDATE_SEF_HW,
    E_CAMERA_UPDATE_SEF_SET,
    E_CAMERA_UPDATE_SEF_WAIT,
} UpdateSef_e;

typedef struct {
    struct {
        float sclk;
        u32 hts;
        u32 preview_vts;
        u32 ho;
        u32 xinc;
        u32 line_freq;
        u32 us_per_line;
        u32 final_us;
        u32 final_us_sef;
        u32 final_gain;
        u32 final_gain_sef;
        u32 Default_Res_fps;
        u32 preview_fps;
        u32 expo_lines_sef;
        u32 expo_lines_sef_pre;
        u32 expo_lines_sef_update;
        u32 expo_lines;
        u32 vts_sef;
        u32 vts_lef;
        u32 max_sef;
    } expo;

    int sen_init;
    int still_min_fps;
    int video_min_fps;
    UpdateSef_e sef_dirty;
    bool reg_dirty;
    bool gain_dirty;
    bool orien_dirty;
    I2C_ARRAY tctx_reg[2];
    I2C_ARRAY tmirror_reg[2];
    I2C_ARRAY tVts_reg[4];
    I2C_ARRAY tGain_reg[5];
    I2C_ARRAY tExpo_reg[4];
    I2C_ARRAY tGain_hdr_lef_reg[5];
    I2C_ARRAY tGain_hdr_sef_reg[5];
    I2C_ARRAY tExpo_hdr_lef_reg[4];
    I2C_ARRAY tExpo_hdr_sef_reg[4];
    u32 Default_Res_line_period;
    u32 Default_Res_vts;
    u32 max_sef_step;
} hm4030_params;
typedef struct {
    float total_gain;
    unsigned short reg_val;
} Gain_ARRAY;

typedef struct {
    u32 u32AgainMain;
    u32 u32AgainFloat;
    u32 u32DgainMain;
    u32 u32DgainFloat;
} Gain_Param;


I2C_ARRAY ctx_reg[] = {
    {0x3024, 0x20},    // 0x20 -> A ,0x21 -> B
};

I2C_ARRAY mirror_reg[] = {
    {0x0101, 0x00},    // HV mirror/flip
};

const I2C_ARRAY gain_reg[] = {
    {0x208, 0x00},    // again,   1-15.5x
    {0x20A, 0x01},    //dgain_h,
    {0x20B, 0x00},    //dgain_l,
};

const I2C_ARRAY expo_reg[] = {
    {0x0202, 0x01},    // shutter[15:8]
    {0x0203, 0xf6},    // shutter[7:0]
};

const I2C_ARRAY vts_reg[] = {
    {0x340, 0x00},    // Frame_Line[15:8]
    {0x341, 0x00},    // Frame_Line[7:0]
};
//HDR
const static I2C_ARRAY gain_HDR_LEF_reg[] =
{
    {0x208, 0x00},    // again,   1-15.5x
    {0x20A, 0x01},    //dgain_h,
    {0x20B, 0x00},    //dgain_l,
};

const static I2C_ARRAY gain_HDR_SEF1_reg[] =
{
    {0x209, 0x00},    // again,   1-15.5x
    {0x20C, 0x01},    //dgain_h, 1-3x
    {0x20D, 0x00},    //dgain_l,
};


const I2C_ARRAY expo_HDR_LEF_reg[] = {
    {0x202, 0x01},    // shutter[15:8]
    {0x203, 0xcf},    // shutter[7:0]
};

const I2C_ARRAY expo_HDR_SEF_reg[] = {
    {0x205, 0x00},    // shutter[15:8]
    {0x206, 0x02},    // shutter[7:0]
};

const I2C_ARRAY Sensor_init_table_4lane[] =
{
    {0x0103, 0x00},    //software reset
    {0xffff, 100 },    //delay(100ms)
    {0x3101, 0x01},
    {0x3108, 0x02},
    {0x310F, 0x03},
    {0x3116, 0x04},
    {0x311D, 0x24},
    {0x3124, 0x24},
    {0x3129, 0x01},
    {0x3102, 0x01},
    {0x3109, 0x01},
    {0x3110, 0x01},
    {0x3117, 0x01},
    {0x311E, 0x01},
    {0x3125, 0x01},
    {0x3103, 0x9F},
    {0x310A, 0x1F},
    {0x3111, 0x1F},
    {0x3118, 0x3F},
    {0x311F, 0x3F},
    {0x3126, 0x3F},
    {0x3104, 0x24},
    {0x310B, 0x14},
    {0x3112, 0x13},
    {0x3119, 0x13},
    {0x3120, 0x13},
    {0x3127, 0x13},
    {0x3121, 0x94},
    {0x3139, 0x37},
    {0x313C, 0x52},
    {0x313E, 0x68},
    {0x3144, 0x0E},
    {0x3145, 0x24},
    {0x3146, 0x25},
    {0x3147, 0x97},
    {0x3148, 0x11},
    {0x3149, 0x27},
    {0x3150, 0x73},
    {0x3151, 0x0C},
    {0x31AF, 0x00},
    {0x31AC, 0x81},
    {0x314A, 0xA3},
    {0x314B, 0x03},
    {0x314C, 0x0C},
    {0x314D, 0x0E},
    {0x314E, 0x11},
    {0x3158, 0x00},
    {0x3181, 0x18},
    {0x3182, 0x88},
    {0x3161, 0x11},
    {0x316E, 0x10},
    {0x3171, 0x03},
    {0x3179, 0x50},
    {0x317A, 0x21},
    {0x317B, 0x10},
    {0x317C, 0x18},
    {0x317D, 0x00},
    {0x3183, 0x14},
    {0x3184, 0x3F},
    {0x318E, 0x80},
    {0x318F, 0x03},
    {0x3190, 0x00},
    {0x30D0, 0x02},
    {0x30D7, 0x03},
    {0x30DE, 0x04},
    {0x30E5, 0x24},
    {0x30EC, 0x2C},
    {0x30F3, 0x2C},
    {0x30F8, 0x01},
    {0x30D1, 0x03},
    {0x30D8, 0x03},
    {0x30DF, 0x03},
    {0x30E6, 0x03},
    {0x30ED, 0x03},
    {0x30F4, 0x03},
    {0x30D2, 0x3F},
    {0x30D9, 0x3F},
    {0x30E0, 0x3F},
    {0x30E7, 0x3F},
    {0x30EE, 0x3F},
    {0x30F5, 0x3F},
    {0x30D3, 0x15},
    {0x30DA, 0x15},
    {0x30E1, 0x15},
    {0x30E8, 0x15},
    {0x30EF, 0x15},
    {0x30F6, 0x15},
    {0x30C3, 0x00},
    {0x30C4, 0xF0},
    {0x3148, 0x12},
    {0x481E, 0x12},
    {0x495E, 0x12},
    {0x4C1E, 0x12},
    {0x3149, 0xAB},
    {0x4822, 0xAB},
    {0x4962, 0xAB},
    {0x4C22, 0xAB},
    {0x314B, 0xB2},
    {0x4823, 0xB2},
    {0x4963, 0xB2},
    {0x4C23, 0xB2},
    {0x314C, 0x03},
    {0x4824, 0x03},
    {0x4964, 0x03},
    {0x4C24, 0x03},
    {0x31B2, 0x24},
    {0x3070, 0x00},
    {0x31AA, 0x0C},
    {0x31AD, 0x08},
    {0x313B, 0x6C},
    {0x313D, 0x20},
    {0x3140, 0xBD},
    {0x313F, 0x85},
    {0x3152, 0x0B},
    {0x3155, 0x11},
    {0x3156, 0xC0},
    {0x0300, 0x6B},
    {0x0302, 0x0B},
    {0x0303, 0x42},
    {0x0304, 0xD1},
    {0x030E, 0x41},
    {0x030F, 0x18},
    {0x2849, 0x03},
    {0x283E, 0x00},
    {0x2832, 0x03},
    {0x281B, 0x02},
    {0x4003, 0x02},
    {0x4004, 0x02},
    {0x3001, 0x00},
    {0x3002, 0x88},
    {0x3004, 0x02},
    {0x3030, 0x68},
    {0x1000, 0xC3},
    {0x1001, 0xC0},
    {0x3070, 0x01},
    {0x307F, 0x01},
    {0x302D, 0x02},
    {0x3194, 0x98},
    {0x319E, 0x80},
    {0x0350, 0x61},
    {0x31B2, 0x24},
    {0x4800, 0x06},
    {0x4801, 0x10},
    {0x4802, 0x00},
    {0x4803, 0x00},
    {0x4804, 0x7B},
    {0x4805, 0x7F},
    {0x4806, 0x3F},
    {0x4807, 0x1F},
    {0x481F, 0x7F},
    {0x4820, 0x0E},
    {0x4821, 0x7F},
    {0x4809, ((vts_60fps_Linear_2064>>8)&0xff)},
    {0x480A, ((vts_60fps_Linear_2064)&0xff)},
    {0x480B, ((hts_60fps_Linear_2064>>8)&0xff)},
    {0x480C, ((hts_60fps_Linear_2064)&0xff)},
    {0x480D, 0x00},
    {0x480E, 0x00},
    {0x480F, 0x08},
    {0x4810, 0x0F},
    {0x4811, 0x00},
    {0x4812, 0x00},
    {0x4813, 0x00},
    {0x4814, 0x00},
    {0x481A, 0xC0},
    {0x481B, 0x80},
    {0x4815, 0x00},
    {0x4816, 0x00},
    {0x4817, 0x00},
    {0x4818, 0x00},
    {0x4819, 0x03},
    {0x48A0, 0x00},
    {0x48A1, 0x04},
    {0x48A2, 0x01},
    {0x48A3, 0xDD},
    {0x48A4, 0x04},
    {0x48A5, 0x0F},
    {0x48A6, 0x20},
    {0x48A7, 0x20},
    {0x48A8, 0x20},
    {0x48A9, 0x20},
    {0x48AA, 0x00},
    {0x48C0, 0x3F},
    {0x48C1, 0x29},
    {0x48C3, 0x14},
    {0x48C4, 0x00},
    {0x48C5, 0x08},
    {0x48C6, 0x10},
    {0x48C7, 0x08},
    {0x48C8, 0x10},
    {0x48C9, 0x00},
    {0x48CA, 0x00},
    {0x48CB, 0x00},
    {0x48CC, 0x00},
    {0x4840, 0x00},
    {0x4844, 0x00},
    {0x4845, 0x00},
    {0x4846, 0x00},
    {0x4847, 0x00},
    {0x4848, 0x01},
    {0x4849, 0x02},
    {0x484A, 0x01},
    {0x484B, 0x02},
    {0x484C, 0x01},
    {0x484D, 0x04},
    {0x484E, 0x64},
    {0x484F, 0x50},
    {0x4850, 0x04},
    {0x4851, 0x00},
    {0x4852, 0x01},
    {0x4853, 0x19},
    {0x4854, 0x50},
    {0x4855, 0x04},
    {0x4856, 0x00},
    {0x4863, 0x01},
    {0x4864, 0x1F},
    {0x4865, 0x01},
    {0x4866, 0x58},
    {0x4880, 0x00},
    {0x4931, 0x2B},
    {0x4934, 0x00},
    {0x4935, 0x3F},
    {0x48F0, 0x01},
    {0x48F1, 0x00},
    {0x48F2, 0x06},
    {0x48FB, 0x01},
    {0x48F3, 0x01},
    {0x48F4, 0x00},
    {0x48F5, 0x01},
    {0x48F6, 0x00},
    {0x48F7, 0x00},
    {0x48F8, 0x00},
    {0x48F9, 0x00},
    {0x48FA, 0x00},
    {0x4940, 0x06},
    {0x4941, 0x10},
    {0x4942, 0x01},
    {0x4943, 0x00},
    {0x4944, 0x7B},
    {0x4945, 0x7F},
    {0x4946, 0x3F},
    {0x4947, 0x1F},
    {0x495F, 0x7F},
    {0x4960, 0x0E},
    {0x4961, 0x7F},
    {0x4949, ((vts_60fps_Linear_2064>>8)&0xff)},
    {0x494A, ((vts_60fps_Linear_2064)&0xff)},
    {0x494B, 0x04},
    {0x494C, 0x68},
    {0x494D, 0x00},
    {0x494E, 0x00},
    {0x494F, 0x08},
    {0x4950, 0x0F},
    {0x4951, 0x01},
    {0x4952, 0x01},
    {0x4953, 0x03},
    {0x4954, 0x02},
    {0x495A, 0xC0},
    {0x495B, 0x80},
    {0x4955, 0x00},
    {0x4956, 0x00},
    {0x4957, 0x00},
    {0x4958, 0x00},
    {0x4959, 0x03},
    {0x49E0, 0x00},
    {0x49E1, 0x04},
    {0x49E2, 0x01},
    {0x49E3, 0xDD},
    {0x49E4, 0x04},
    {0x49E5, 0x07},
    {0x49E6, 0x10},
    {0x49E7, 0x10},
    {0x49E8, 0x10},
    {0x49E9, 0x10},
    {0x49EA, 0x00},
    {0x4A00, 0x3F},
    {0x4A01, 0x29},
    {0x4A03, 0x14},
    {0x4A04, 0x00},
    {0x4A05, 0x04},
    {0x4A06, 0x08},
    {0x4A07, 0x04},
    {0x4A08, 0x08},
    {0x4A09, 0x00},
    {0x4A0A, 0x00},
    {0x4A0B, 0x00},
    {0x4A0C, 0x00},
    {0x4980, 0x00},
    {0x4984, 0x00},
    {0x4985, 0x00},
    {0x4986, 0x00},
    {0x4987, 0x00},
    {0x4988, 0x00},
    {0x4989, 0x80},
    {0x498A, 0x00},
    {0x498B, 0x80},
    {0x498C, 0x01},
    {0x498D, 0x02},
    {0x498E, 0x32},
    {0x498F, 0x50},
    {0x4990, 0x04},
    {0x4991, 0x00},
    {0x4992, 0x00},
    {0x4993, 0x8C},
    {0x4994, 0x50},
    {0x4995, 0x04},
    {0x4996, 0x00},
    {0x49A3, 0x02},
    {0x49A4, 0x3C},
    {0x49A5, 0x02},
    {0x49A6, 0xAE},
    {0x49C0, 0x00},
    {0x4A71, 0x2B},
    {0x4A74, 0x01},
    {0x4A75, 0x87},
    {0x4A30, 0x01},
    {0x4A31, 0x00},
    {0x4A32, 0x06},
    {0x4A3B, 0x41},
    {0x4A33, 0x00},
    {0x4A34, 0x80},
    {0x4A35, 0x00},
    {0x4A36, 0x80},
    {0x4A37, 0x00},
    {0x4A38, 0x00},
    {0x4A39, 0x00},
    {0x4A3A, 0x00},
    {0x4C00, 0x26},
    {0x4C01, 0x10},
    {0x4C02, 0x18},
    {0x4C03, 0x02},
    {0x4C04, 0x7F},
    {0x4C05, 0x3F},
    {0x4C06, 0x3F},
    {0x4C07, 0x00},
    {0x4C1F, 0x0F},
    {0x4C20, 0x0E},
    {0x4C21, 0x0E},
    {0x4C09, 0x04},
    {0x4C0A, 0x26},
    {0x4C0B, 0x02},
    {0x4C0C, 0x34},
    {0x4C0D, 0x00},
    {0x4C0E, 0x00},
    {0x4C0F, 0x08},
    {0x4C10, 0x0F},
    {0x4C11, 0x04},
    {0x4C12, 0x04},
    {0x4C13, 0x03},
    {0x4C14, 0x02},
    {0x4C1A, 0xC0},
    {0x4C1B, 0x80},
    {0x4C15, 0x00},
    {0x4C16, 0x00},
    {0x4C17, 0x00},
    {0x4C18, 0x00},
    {0x4C19, 0x00},
    {0x4C80, 0x00},
    {0x4C81, 0x04},
    {0x4C82, 0x01},
    {0x4C83, 0xDD},
    {0x4C84, 0x04},
    {0x4C85, 0x0F},
    {0x4C86, 0x00},
    {0x4C87, 0x00},
    {0x4C88, 0x00},
    {0x4C89, 0x00},
    {0x4C8A, 0x00},
    {0x4C90, 0x70},
    {0x4C91, 0x00},
    {0x4C93, 0x04},
    {0x4C94, 0x00},
    {0x4C95, 0x00},
    {0x4C96, 0x80},
    {0x4C97, 0x00},
    {0x4C98, 0x80},
    {0x4C99, 0x00},
    {0x4C9A, 0x00},
    {0x4C9B, 0x00},
    {0x4C9C, 0x00},
    {0x4C30, 0x25},
    {0x4C34, 0x00},
    {0x4C35, 0x00},
    {0x4C36, 0x00},
    {0x4C37, 0x00},
    {0x4C38, 0x00},
    {0x4C39, 0x10},
    {0x4C3A, 0x00},
    {0x4C3B, 0x10},
    {0x4C3C, 0x00},
    {0x4C3D, 0x00},
    {0x4C3E, 0x4A},
    {0x4C3F, 0x50},
    {0x4C40, 0x0A},
    {0x4C41, 0x00},
    {0x4C42, 0x00},
    {0x4C43, 0x4A},
    {0x4C44, 0x50},
    {0x4C45, 0x1F},
    {0x4C46, 0xFF},
    {0x4C53, 0x00},
    {0x4C54, 0x01},
    {0x4C55, 0x00},
    {0x4C56, 0x01},
    {0x4CF1, 0x2B},
    {0x4CF4, 0x01},
    {0x4CF5, 0x1E},
    {0x4CB0, 0x00},
    {0x4CB1, 0x00},
    {0x4CB2, 0x00},
    {0x4CBB, 0x00},
    {0x4CB3, 0x00},
    {0x4CB4, 0x20},
    {0x4CB5, 0x00},
    {0x4CB6, 0x20},
    {0x4CB7, 0x00},
    {0x4CB8, 0x00},
    {0x4CB9, 0x00},
    {0x4CBA, 0x00},
    {0x4D00, 0x03},
    {0x4D01, 0x02},
    {0x4D08, 0x80},
    {0x4D02, 0x10},
    {0x4D03, 0x18},
    {0x4D04, 0x02},
    {0x4D05, 0x7F},
    {0x4D06, 0x02},
    {0x4D07, 0x34},
    {0x4D60, 0x00},
    {0x4D61, 0x04},
    {0x4D62, 0x01},
    {0x4D63, 0xDD},
    {0x4D64, 0x04},
    {0x4D65, 0x0F},
    {0x4D66, 0x00},
    {0x4D67, 0x00},
    {0x4D68, 0x00},
    {0x4D69, 0x00},
    {0x4D6A, 0x00},
    {0x4DA0, 0x01},
    {0x4DA1, 0x1E},
    {0x4DB0, 0x03},
    {0x4DB1, 0x02},
    {0x4DB8, 0x80},
    {0x4DB2, 0x10},
    {0x4DB3, 0x18},
    {0x4DB4, 0x02},
    {0x4DB5, 0x7F},
    {0x4DB6, 0x02},
    {0x4DB7, 0x34},
    {0x4E10, 0x00},
    {0x4E11, 0x04},
    {0x4E12, 0x01},
    {0x4E13, 0xDD},
    {0x4E14, 0x04},
    {0x4E15, 0x0F},
    {0x4E16, 0x00},
    {0x4E17, 0x00},
    {0x4E18, 0x00},
    {0x4E19, 0x00},
    {0x4E1A, 0x00},
    {0x4E50, 0x01},
    {0x4E51, 0x1E},
    {0x3001, 0x00},
    {0x3002, 0x88},
    {0x3004, 0x02},
    {0x3024, 0x20},
    {0x3025, 0x12},
    {0x3026, 0x00},
    {0x3027, 0x81},
    {0x3028, 0x01},
    {0x3029, 0x00},
    {0x302A, 0x30},
    {0x2000, 0x00},
    {0x4840, 0x00},
    {0x4980, 0x00},
    {0x4C30, 0x00},
    {0x3042, 0x00},
    {0x2088, 0x01},
    {0x2089, 0x00},
    {0x208A, 0xC8},
    {0x2700, 0x01},
    {0x272F, 0x01},
    {0x2711, 0x01},
    {0x2713, 0x04},
    {0x2821, 0x9E},
    {0x4932, 0x01},
    {0x4A72, 0x01},
    {0x4CF2, 0x01},
    {0x2800, 0x01},
    {0x2839, 0x6A},
    {0x283A, 0x0B},
    {0x2842, 0x0C},
    {0x2823, 0x03},
    {0x4933, 0x03},
    {0x4A73, 0x03},
    {0x4CF3, 0x03},
    {0x0202, 0x01},
    {0x0203, 0xCF},
    {0x0202, 0x07},
    {0x0203, 0xCF},
    {0x0208, 0x20},
    {0x4805, 0x0F},
    {0x48C0, 0x00},
    {0x48C1, 0x00},
    {0x48C2, 0x01},
    {0x4945, 0x0F},
    {0x4A00, 0x00},
    {0x4A01, 0x00},
    {0x4A02, 0x01},
    {0x4C05, 0x0F},
    {0x4C90, 0x00},
    {0x4C91, 0x00},
    {0x4C92, 0x01},
    {0x0300, 0x69},
    {0x2839, 0x69},
    {0x0104, 0x01},
    {0x0100, 0x01},
};
const I2C_ARRAY Sensor_init_table_HDR_4lane_2048[] =
{
    {0x0103, 0x00},    //software reset
    {0xffff, 100 },    //delay(100ms)
    {0x3101, 0x01},
    {0x3108, 0x02},
    {0x310F, 0x03},
    {0x3116, 0x04},
    {0x311D, 0x24},
    {0x3124, 0x24},
    {0x3129, 0x01},
    {0x3102, 0x01},
    {0x3109, 0x01},
    {0x3110, 0x01},
    {0x3117, 0x01},
    {0x311E, 0x01},
    {0x3125, 0x01},
    {0x3103, 0x9F},
    {0x310A, 0x1F},
    {0x3111, 0x1F},
    {0x3118, 0x3F},
    {0x311F, 0x3F},
    {0x3126, 0x3F},
    {0x3104, 0x24},
    {0x310B, 0x14},
    {0x3112, 0x13},
    {0x3119, 0x13},
    {0x3120, 0x13},
    {0x3127, 0x13},
    {0x3121, 0x94},
    {0x3139, 0x37},
    {0x313C, 0x52},
    {0x313E, 0x68},
    {0x3144, 0x0E},
    {0x3145, 0x24},
    {0x3146, 0x25},
    {0x3147, 0x97},
    {0x3150, 0x73},
    {0x3151, 0x0C},
    {0x31AF, 0x00},
    {0x31AC, 0x81},
    {0x314A, 0xA3},
    {0x314D, 0x0E},
    {0x314E, 0x11},
    {0x3158, 0x00},
    {0x3181, 0x18},
    {0x3182, 0x88},
    {0x3161, 0x11},
    {0x316E, 0x10},
    {0x3171, 0x03},
    {0x3179, 0x50},
    {0x317A, 0x21},
    {0x317B, 0x10},
    {0x317C, 0x18},
    {0x317D, 0x00},
    {0x3183, 0x14},
    {0x3184, 0x3F},
    {0x318E, 0x80},
    {0x318F, 0x03},
    {0x3190, 0x00},
    {0x30D0, 0x02},
    {0x30D7, 0x03},
    {0x30DE, 0x04},
    {0x30E5, 0x24},
    {0x30EC, 0x2C},
    {0x30F3, 0x2C},
    {0x30F8, 0x01},
    {0x30D1, 0x03},
    {0x30D8, 0x03},
    {0x30DF, 0x03},
    {0x30E6, 0x03},
    {0x30ED, 0x03},
    {0x30F4, 0x03},
    {0x30D2, 0x3F},
    {0x30D9, 0x3F},
    {0x30E0, 0x3F},
    {0x30E7, 0x3F},
    {0x30EE, 0x3F},
    {0x30F5, 0x3F},
    {0x30D3, 0x15},
    {0x30DA, 0x15},
    {0x30E1, 0x15},
    {0x30E8, 0x15},
    {0x30EF, 0x15},
    {0x30F6, 0x15},
    {0x30C3, 0x00},
    {0x30C4, 0xF0},
    {0x3148, 0x12},
    {0x481E, 0x12},
    {0x495E, 0x12},
    {0x4C1E, 0x12},
    {0x3149, 0xAB},
    {0x4822, 0xAB},
    {0x4962, 0xAB},
    {0x4C22, 0xAB},
    {0x314B, 0xB2},
    {0x4823, 0xB2},
    {0x4963, 0xB2},
    {0x4C23, 0xB2},
    {0x314C, 0x03},
    {0x4824, 0x03},
    {0x4964, 0x03},
    {0x4C24, 0x03},
    {0x31AA, 0x0C},
    {0x31AD, 0x08},
    {0x313B, 0x6C},
    {0x313D, 0x20},
    {0x3140, 0xBD},
    {0x313F, 0x85},
    {0x3152, 0x0B},
    {0x3155, 0x11},
    {0x3156, 0xC0},
    {0x0302, 0x0B},
    {0x0303, 0x42},
    {0x0304, 0xD1},
    {0x030E, 0x41},
    {0x030F, 0x18},
    {0x2849, 0x03},
    {0x283E, 0x00},
    {0x2832, 0x03},
    {0x4003, 0x02},
    {0x4004, 0x02},
    {0x3030, 0x68},
    {0x1000, 0xC3},
    {0x1001, 0xC0},
    {0x3070, 0x01},
    {0x307F, 0x01},
    {0x302D, 0x02},
    {0x3194, 0x98},
    {0x319E, 0x80},
    {0x0350, 0x61},
    {0x31B2, 0x24},
    {0x4800, 0x06},
    {0x4801, 0x10},
    {0x4802, 0x00},
    {0x4803, 0x00},
    {0x4804, 0x7B},
    {0x4806, 0x3F},
    {0x4807, 0x1F},
    {0x481F, 0x7F},
    {0x4820, 0x0E},
    {0x4821, 0x7F},
    {0x4809, ((vts_30fps_HDR_2064>>8)&0xff)},
    {0x480A, ((vts_30fps_HDR_2064)&0xff)},
    {0x480D, 0x00},
    {0x480E, 0x00},
    {0x480F, 0x08},
    {0x4810, 0x0F},
    {0x4811, 0x00},
    {0x4812, 0x00},
    {0x4813, 0x00},
    {0x4814, 0x00},
    {0x481A, 0xC0},
    {0x481B, 0x80},
    {0x4815, 0x11},
    {0x4816, 0x00},
    {0x4817, 0x00},
    {0x4818, 0x00},
    {0x4819, 0x03},
    {0x48A0, 0x00},
    {0x48A1, 0x04},
    {0x48A2, 0x01},
    {0x48A3, 0xDD},
    {0x48A4, 0x04},
    {0x48A5, 0x0F},
    {0x48A6, 0x20},
    {0x48A7, 0x20},
    {0x48A8, 0x20},
    {0x48A9, 0x20},
    {0x48AA, 0x00},
    {0x48C3, 0x12},
    {0x48C4, 0x00},
    {0x48C5, 0x08},   //digital window(X)
    {0x48C6, 0x00},
    {0x48C7, 0x08},   //digital window(Y)
    {0x48C8, 0x00},
    {0x48C9, 0x00},
    {0x48CA, 0x00},
    {0x48CB, 0x00},
    {0x48CC, 0x00},
    {0x4844, 0x00},
    {0x4845, 0x00},
    {0x4846, 0x00},
    {0x4847, 0x00},
    {0x4848, 0x01},
    {0x4849, 0x02},
    {0x484A, 0x01},
    {0x484B, 0x02},
    {0x484C, 0x01},
    {0x484D, 0x04},
    {0x484E, 0x64},
    {0x484F, 0x50},
    {0x4850, 0x04},
    {0x4851, 0x00},
    {0x4852, 0x00},
    {0x4853, 0x8C},
    {0x4854, 0x50},
    {0x4855, 0x04},
    {0x4856, 0x00},
    {0x4863, 0x01},
    {0x4864, 0x1F},
    {0x4865, 0x01},
    {0x4866, 0x58},
    {0x4880, 0x00},
    {0x4931, 0x2B},
    {0x48F0, 0x01},
    {0x48F1, 0x00},
    {0x48F2, 0x06},
    {0x48FB, 0x01},
    {0x48F3, 0x01},
    {0x48F4, 0x00},
    {0x48F5, 0x01},
    {0x48F6, 0x00},
    {0x48F7, 0x00},
    {0x48F8, 0x00},
    {0x48F9, 0x00},
    {0x48FA, 0x00},
    {0x4940, 0x06},
    {0x4941, 0x10},
    {0x4942, 0x01},
    {0x4943, 0x00},
    {0x4944, 0x7B},
    {0x4946, 0x3F},
    {0x4947, 0x1F},
    {0x495F, 0x7F},
    {0x4960, 0x0E},
    {0x4961, 0x7F},
    {0x4949, 0x04},
    {0x494A, 0x26},
    {0x494B, 0x08},
    {0x494C, 0xD0},
    {0x494D, 0x00},
    {0x494E, 0x00},
    {0x494F, 0x08},
    {0x4950, 0x0F},
    {0x4951, 0x01},
    {0x4952, 0x01},
    {0x4953, 0x03},
    {0x4954, 0x02},
    {0x495A, 0xC0},
    {0x495B, 0x80},
    {0x4955, 0x11},
    {0x4956, 0x00},
    {0x4957, 0x00},
    {0x4958, 0x00},
    {0x4959, 0x03},
    {0x49E0, 0x00},
    {0x49E1, 0x04},
    {0x49E2, 0x01},
    {0x49E3, 0xDD},
    {0x49E4, 0x04},
    {0x49E5, 0x0F},
    {0x49E6, 0x10},
    {0x49E7, 0x10},
    {0x49E8, 0x10},
    {0x49E9, 0x10},
    {0x49EA, 0x00},
    {0x4A03, 0x12},
    {0x4A04, 0x00},
    {0x4A05, 0x04},
    {0x4A06, 0x08},
    {0x4A07, 0x04},
    {0x4A08, 0x08},
    {0x4A09, 0x00},
    {0x4A0A, 0x00},
    {0x4A0B, 0x00},
    {0x4A0C, 0x00},
    {0x4984, 0x00},
    {0x4985, 0x00},
    {0x4986, 0x00},
    {0x4987, 0x00},
    {0x4988, 0x00},
    {0x4989, 0x80},
    {0x498A, 0x00},
    {0x498B, 0x80},
    {0x498C, 0x01},
    {0x498D, 0x02},
    {0x498E, 0x32},
    {0x498F, 0x50},
    {0x4990, 0x04},
    {0x4991, 0x00},
    {0x4992, 0x00},
    {0x4993, 0x8C},
    {0x4994, 0x50},
    {0x4995, 0x04},
    {0x4996, 0x00},
    {0x49A3, 0x02},
    {0x49A4, 0x3C},
    {0x49A5, 0x02},
    {0x49A6, 0xAE},
    {0x49C0, 0x00},
    {0x4A71, 0x2B},
    {0x4A74, 0x02},
    {0x4A75, 0x17},
    {0x4A30, 0x01},
    {0x4A31, 0x00},
    {0x4A32, 0x06},
    {0x4A3B, 0x41},
    {0x4A33, 0x00},
    {0x4A34, 0x80},
    {0x4A35, 0x00},
    {0x4A36, 0x80},
    {0x4A37, 0x00},
    {0x4A38, 0x00},
    {0x4A39, 0x00},
    {0x4A3A, 0x00},
    {0x4C00, 0x26},
    {0x4C01, 0x10},
    {0x4C02, 0x18},
    {0x4C03, 0x02},
    {0x4C04, 0x7F},
    {0x4C06, 0x3F},
    {0x4C07, 0x00},
    {0x4C1F, 0x0F},
    {0x4C20, 0x0E},
    {0x4C21, 0x0E},
    {0x4C09, 0x04},
    {0x4C0A, 0x26},
    {0x4C0B, 0x02},
    {0x4C0C, 0x34},
    {0x4C0D, 0x00},
    {0x4C0E, 0x00},
    {0x4C0F, 0x08},
    {0x4C10, 0x0F},
    {0x4C11, 0x04},
    {0x4C12, 0x04},
    {0x4C13, 0x03},
    {0x4C14, 0x02},
    {0x4C1A, 0xC0},
    {0x4C1B, 0x80},
    {0x4C15, 0x00},
    {0x4C16, 0x00},
    {0x4C17, 0x00},
    {0x4C18, 0x00},
    {0x4C19, 0x00},
    {0x4C80, 0x00},
    {0x4C81, 0x04},
    {0x4C82, 0x01},
    {0x4C83, 0xDD},
    {0x4C84, 0x04},
    {0x4C85, 0x0F},
    {0x4C86, 0x00},
    {0x4C87, 0x00},
    {0x4C88, 0x00},
    {0x4C89, 0x00},
    {0x4C8A, 0x00},
    {0x4C93, 0x04},
    {0x4C94, 0x00},
    {0x4C95, 0x00},
    {0x4C96, 0x80},
    {0x4C97, 0x00},
    {0x4C98, 0x80},
    {0x4C99, 0x00},
    {0x4C9A, 0x00},
    {0x4C9B, 0x00},
    {0x4C9C, 0x00},
    {0x4C34, 0x00},
    {0x4C35, 0x00},
    {0x4C36, 0x00},
    {0x4C37, 0x00},
    {0x4C38, 0x00},
    {0x4C39, 0x10},
    {0x4C3A, 0x00},
    {0x4C3B, 0x10},
    {0x4C3C, 0x00},
    {0x4C3D, 0x00},
    {0x4C3E, 0x4A},
    {0x4C3F, 0x50},
    {0x4C40, 0x0A},
    {0x4C41, 0x00},
    {0x4C42, 0x00},
    {0x4C43, 0x4A},
    {0x4C44, 0x50},
    {0x4C45, 0x1F},
    {0x4C46, 0xFF},
    {0x4C53, 0x00},
    {0x4C54, 0x01},
    {0x4C55, 0x00},
    {0x4C56, 0x01},
    {0x4CF1, 0x2B},
    {0x4CF4, 0x01},
    {0x4CF5, 0x1E},
    {0x4CB0, 0x00},
    {0x4CB1, 0x00},
    {0x4CB2, 0x00},
    {0x4CBB, 0x00},
    {0x4CB3, 0x00},
    {0x4CB4, 0x20},
    {0x4CB5, 0x00},
    {0x4CB6, 0x20},
    {0x4CB7, 0x00},
    {0x4CB8, 0x00},
    {0x4CB9, 0x00},
    {0x4CBA, 0x00},
    {0x4D00, 0x03},
    {0x4D01, 0x02},
    {0x4D08, 0x80},
    {0x4D02, 0x10},
    {0x4D03, 0x18},
    {0x4D04, 0x02},
    {0x4D05, 0x7F},
    {0x4D06, 0x02},
    {0x4D07, 0x34},
    {0x4D60, 0x00},
    {0x4D61, 0x04},
    {0x4D62, 0x01},
    {0x4D63, 0xDD},
    {0x4D64, 0x04},
    {0x4D65, 0x0F},
    {0x4D66, 0x00},
    {0x4D67, 0x00},
    {0x4D68, 0x00},
    {0x4D69, 0x00},
    {0x4D6A, 0x00},
    {0x4DA0, 0x01},
    {0x4DA1, 0x1E},
    {0x4DB0, 0x03},
    {0x4DB1, 0x02},
    {0x4DB8, 0x80},
    {0x4DB2, 0x10},
    {0x4DB3, 0x18},
    {0x4DB4, 0x02},
    {0x4DB5, 0x7F},
    {0x4DB6, 0x02},
    {0x4DB7, 0x34},
    {0x4E10, 0x00},
    {0x4E11, 0x04},
    {0x4E12, 0x01},
    {0x4E13, 0xDD},
    {0x4E14, 0x04},
    {0x4E15, 0x0F},
    {0x4E16, 0x00},
    {0x4E17, 0x00},
    {0x4E18, 0x00},
    {0x4E19, 0x00},
    {0x4E1A, 0x00},
    {0x4E50, 0x01},
    {0x4E51, 0x1E},
    {0x3001, (SENSOR_LS_OFFSET>>8)&0xff},  //HDR_long_short_distance (H)
    {0x3002, (SENSOR_LS_OFFSET)&0xff},
    {0x3004, 0x02},
    {0x3024, 0x20},
    {0x3025, 0x12},
    {0x3026, 0x00},
    {0x3027, 0x81},
    {0x3028, 0x01},
    {0x3029, 0x00},
    {0x302A, 0x30},
    {0x2000, 0x00},
    {0x4840, 0x00},
    {0x4980, 0x00},
    {0x4C30, 0x00},
    {0x3042, 0x00},
    {0x2088, 0x01},
    {0x2089, 0x00},
    {0x208A, 0xC8},
    {0x2700, 0x01},
    {0x272F, 0x01},
    {0x2711, 0x01},
    {0x2713, 0x04},
    {0x2821, 0x9E},
    {0x4932, 0x01},
    {0x4A72, 0x01},
    {0x4CF2, 0x01},
    {0x2800, 0x01},
    {0x2839, 0x77},
    {0x283A, 0x0B},
    {0x2842, 0x0C},
    {0x2823, 0x03},
    {0x4933, 0x03},
    {0x4A73, 0x03},
    {0x4CF3, 0x03},
    {0x0202, 0x01},  //Long exp
    {0x0203, 0xCF},  //Short exp
    {0x0208, 0x30},  //Analog gain(8x)
    {0x4805, 0x0F},
    {0x48C0, 0x00},
    {0x48C1, 0x00},
    {0x48C2, 0x01},
    {0x4945, 0x0F},
    {0x4A00, 0x00},
    {0x4A01, 0x00},
    {0x4A02, 0x01},
    {0x4C05, 0x0F},
    {0x4C90, 0x00},
    {0x4C91, 0x00},
    {0x4C92, 0x01},
    {0x0300, 0x6B},
    {0x281B, 0x02},
    {0x4934, 0x00},
    {0x4935, 0x9D},
    {0x480B, ((hts_30fps_HDR_2064>>8)&0xff)},
    {0x480C, ((hts_30fps_HDR_2064)&0xff)},
    {0x0104, 0x01},
    {0x0100, 0x01},
};

const I2C_ARRAY Sensor_init_table_HDR_4lane[] =
{
    {0x0103, 0x00}, // software reset
    {0xffff, 100 },    //delay(100ms)
    {0x3101, 0x01},
    {0x3108, 0x02},
    {0x310F, 0x03},
    {0x3116, 0x04},
    {0x311D, 0x24},
    {0x3124, 0x24},
    {0x3129, 0x01},
    {0x3102, 0x01},
    {0x3109, 0x01},
    {0x3110, 0x01},
    {0x3117, 0x01},
    {0x311E, 0x01},
    {0x3125, 0x01},
    {0x3103, 0x9F},
    {0x310A, 0x1F},
    {0x3111, 0x1F},
    {0x3118, 0x3F},
    {0x311F, 0x3F},
    {0x3126, 0x3F},
    {0x3104, 0x24},
    {0x310B, 0x14},
    {0x3112, 0x13},
    {0x3119, 0x13},
    {0x3120, 0x13},
    {0x3127, 0x13},
    {0x3121, 0x94},
    {0x3139, 0x37},
    {0x313C, 0x52},
    {0x313E, 0x68},
    {0x3144, 0x0E},
    {0x3145, 0x24},
    {0x3146, 0x25},
    {0x3147, 0x97},
    {0x3148, 0x11},
    {0x3149, 0x27},
    {0x3150, 0x73},
    {0x3151, 0x0C},
    {0x31AF, 0x00},
    {0x31AC, 0x81},
    {0x314A, 0xA3},
    {0x314B, 0x03},
    {0x314C, 0x0C},
    {0x314D, 0x0E},
    {0x314E, 0x11},
    {0x3158, 0x00},
    {0x3181, 0x18},
    {0x3182, 0x88},
    {0x3161, 0x11},
    {0x316E, 0x10},
    {0x3171, 0x03},
    {0x3179, 0x50},
    {0x317A, 0x21},
    {0x317B, 0x10},
    {0x317C, 0x18},
    {0x317D, 0x00},
    {0x3183, 0x14},
    {0x3184, 0x3F},
    {0x318E, 0x80},
    {0x318F, 0x03},
    {0x3190, 0x00},
    {0x30D0, 0x02},
    {0x30D7, 0x03},
    {0x30DE, 0x04},
    {0x30E5, 0x24},
    {0x30EC, 0x2C},
    {0x30F3, 0x2C},
    {0x30F8, 0x01},
    {0x30D1, 0x03},
    {0x30D8, 0x03},
    {0x30DF, 0x03},
    {0x30E6, 0x03},
    {0x30ED, 0x03},
    {0x30F4, 0x03},
    {0x30D2, 0x3F},
    {0x30D9, 0x3F},
    {0x30E0, 0x3F},
    {0x30E7, 0x3F},
    {0x30EE, 0x3F},
    {0x30F5, 0x3F},
    {0x30D3, 0x15},
    {0x30DA, 0x15},
    {0x30E1, 0x15},
    {0x30E8, 0x15},
    {0x30EF, 0x15},
    {0x30F6, 0x15},
    {0x30C3, 0x00},
    {0x30C4, 0xF0},
    {0x3148, 0x12},
    {0x481E, 0x12},
    {0x495E, 0x12},
    {0x4C1E, 0x12},
    {0x3149, 0xAB},
    {0x4822, 0xAB},
    {0x4962, 0xAB},
    {0x4C22, 0xAB},
    {0x314B, 0xB2},
    {0x4823, 0xB2},
    {0x4963, 0xB2},
    {0x4C23, 0xB2},
    {0x314C, 0x03},
    {0x4824, 0x03},
    {0x4964, 0x03},
    {0x4C24, 0x03},
    {0x31B2, 0x24},
    {0x3070, 0x00},
    {0x31AA, 0x0C},
    {0x31AD, 0x08},
    {0x313B, 0x6C},
    {0x313D, 0x20},
    {0x3140, 0xBD},
    {0x313F, 0x85},
    {0x3152, 0x0B},
    {0x3155, 0x11},
    {0x3156, 0xC0},
    {0x0300, 0x6B},
    {0x0302, 0x0B},
    {0x0303, 0x42},
    {0x0304, 0xD1},
    {0x0305, 0x06},
    {0x0306, 0x10},
    {0x0307, 0x00},
    {0x0308, 0x00},
    {0x0309, 0x7B},
    {0x030E, 0x41},
    {0x030F, 0x18},
    {0x2849, 0x03},
    {0x283E, 0x00},
    {0x2832, 0x03},
    {0x281B, 0x02},
    {0x4003, 0x02},
    {0x4004, 0x02},
    {0x3001, (SENSOR_LS_OFFSET>>8)&0xff},    // HDR_long_short_distance (H)
    {0x3002, (SENSOR_LS_OFFSET)&0xff},    // HDR_long_short_distance (L)
    //{0x3003, 0x02}, ; // HDR_adjustment
    {0x3004, 0x02},   // HDR_ratio
    {0x3030, 0x68},
    {0x1000, 0xC3},
    {0x1001, 0xC0},
    {0x3070, 0x01},
    {0x307F, 0x01},
    {0x302D, 0x02},
    {0x3194, 0x98},
    {0x319E, 0x80},
    {0x0350, 0x61},
    {0x31B2, 0x24},
    //*************************************************************************
    //   setA_MIPI_2064x2064_Serial_256x256_pkt100_HDR (CXA)
    //*************************************************************************
    {0x4800, 0x06},
    {0x4801, 0x10},
    {0x4802, 0x00},
    {0x4803, 0x00},
    {0x4804, 0x7B},
    {0x4805, 0x7F},
    {0x4806, 0x3F},
    {0x4807, 0x1F},
    {0x481F, 0x7F},
    {0x4820, 0x0E},
    {0x4821, 0x7F},
    {0x4809, ((vts_30fps_HDR_2064>>8)&0xff)},  // Frame Length H
    {0x480A, ((vts_30fps_HDR_2064)&0xff)},  // Frame Length L
    {0x480B, ((hts_30fps_HDR_2064>>8)&0xff)},  // Line Length H
    {0x480C, ((hts_30fps_HDR_2064)&0xff)},  // Line Length L
    {0x480D, 0x00},  // Y start H
    {0x480E, 0x00},  // Y start L
    {0x480F, 0x08},  // Y end H
    {0x4810, 0x0F},  // Y end L
    {0x4811, 0x00},
    {0x4812, 0x00},
    {0x4813, 0x00},
    {0x4814, 0x00},
    {0x481A, 0xC0},
    {0x481B, 0x80},
    {0x4815, 0x11},  // HDR mode
    {0x4816, 0x00},
    {0x4817, 0x00},
    {0x4818, 0x00},
    {0x4819, 0x03},
    {0x48A0, 0x00},
    {0x48A1, 0x04},
    {0x48A2, 0x01},
    {0x48A3, 0xDD},
    {0x48A4, 0x04},
    {0x48A5, 0x0F},
    {0x48A6, 0x20},
    {0x48A7, 0x20},
    {0x48A8, 0x20},
    {0x48A9, 0x20},
    {0x48AA, 0x00},
    {0x48C0, 0x3F},
    {0x48C1, 0x15},
    {0x48C3, 0x12},
    {0x48C4, 0x00},
    {0x48C5, 0x08},
    {0x48C6, 0x10},
    {0x48C7, 0x08},
    {0x48C8, 0x10},
    {0x48C9, 0x00},
    {0x48CA, 0x00},
    {0x48CB, 0x00},
    {0x48CC, 0x00},
    {0x4840, 0x00},
    {0x4844, 0x00},
    {0x4845, 0x00},
    {0x4846, 0x00},
    {0x4847, 0x00},
    {0x4848, 0x01},
    {0x4849, 0x02},
    {0x484A, 0x01},
    {0x484B, 0x02},
    {0x484C, 0x01},
    {0x484D, 0x04},
    {0x484E, 0x64},
    {0x484F, 0x50},
    {0x4850, 0x04},
    {0x4851, 0x00},
    {0x4852, 0x00},
    {0x4853, 0x8C},
    {0x4854, 0x50},
    {0x4855, 0x04},
    {0x4856, 0x00},
    {0x4863, 0x01},
    {0x4864, 0x1F},
    {0x4865, 0x01},
    {0x4866, 0x58},
    {0x4880, 0x00},
    {0x4931, 0x2B},
    {0x4934, 0x00},
    {0x4935, 0xCD},
    {0x48F0, 0x01},
    {0x48F1, 0x00},
    {0x48F2, 0x06},
    {0x48FB, 0x01},
    {0x48F3, 0x01},
    {0x48F4, 0x00},
    {0x48F5, 0x01},
    {0x48F6, 0x00},
    {0x48F7, 0x00},
    {0x48F8, 0x00},
    {0x48F9, 0x00},
    {0x48FA, 0x00},
    //*************************************************************************
    //   setB_MIPI_1032x1032_Serial_128x128_pkt100_HDR (CXB)
    //*************************************************************************
    {0x4940, 0x06},
    {0x4941, 0x10},
    {0x4942, 0x01},
    {0x4943, 0x00},
    {0x4944, 0x7B},
    {0x4945, 0x7F},
    {0x4946, 0x3F},
    {0x4947, 0x1F},
    {0x495F, 0x7F},
    {0x4960, 0x0E},
    {0x4961, 0x7F},
    {0x4949, ((vts_30fps_HDR_2064>>8)&0xff)},  // Frame Length H
    {0x494A, ((vts_30fps_HDR_2064)&0xff)},  // Frame Length L
    {0x494B, ((hts_30fps_HDR_2064>>8)&0xff)},  // Line Length H
    {0x494C, ((hts_30fps_HDR_2064)&0xff)},  // Line Length L
    {0x494D, 0x00},
    {0x494E, 0x00},
    {0x494F, 0x08},
    {0x4950, 0x0F},
    {0x4951, 0x01},
    {0x4952, 0x01},
    {0x4953, 0x03},
    {0x4954, 0x02},
    {0x495A, 0xC0},
    {0x495B, 0x80},
    {0x4955, 0x11},  // HDR mode
    {0x4956, 0x00},
    {0x4957, 0x00},
    {0x4958, 0x00},
    {0x4959, 0x03},
    {0x49E0, 0x00},
    {0x49E1, 0x04},
    {0x49E2, 0x01},
    {0x49E3, 0xDD},
    {0x49E4, 0x04},
    {0x49E5, 0x0F},
    {0x49E6, 0x10},
    {0x49E7, 0x10},
    {0x49E8, 0x10},
    {0x49E9, 0x10},
    {0x49EA, 0x00},
    {0x4A00, 0x3F},
    {0x4A01, 0x15},
    {0x4A03, 0x12},
    {0x4A04, 0x00},
    {0x4A05, 0x04},
    {0x4A06, 0x08},
    {0x4A07, 0x04},
    {0x4A08, 0x08},
    {0x4A09, 0x00},
    {0x4A0A, 0x00},
    {0x4A0B, 0x00},
    {0x4A0C, 0x00},
    {0x4980, 0x00},
    {0x4984, 0x00},
    {0x4985, 0x00},
    {0x4986, 0x00},
    {0x4987, 0x00},
    {0x4988, 0x00},
    {0x4989, 0x80},
    {0x498A, 0x00},
    {0x498B, 0x80},
    {0x498C, 0x01},
    {0x498D, 0x02},
    {0x498E, 0x32},
    {0x498F, 0x50},
    {0x4990, 0x04},
    {0x4991, 0x00},
    {0x4992, 0x00},
    {0x4993, 0x8C},
    {0x4994, 0x50},
    {0x4995, 0x04},
    {0x4996, 0x00},
    {0x49A3, 0x02},
    {0x49A4, 0x3C},
    {0x49A5, 0x02},
    {0x49A6, 0xAE},
    {0x49C0, 0x00},
    {0x4A71, 0x2B},
    {0x4A74, 0x02},
    {0x4A75, 0x17},
    {0x4A30, 0x01},
    {0x4A31, 0x00},
    {0x4A32, 0x06},
    {0x4A3B, 0x41},
    {0x4A33, 0x00},
    {0x4A34, 0x80},
    {0x4A35, 0x00},
    {0x4A36, 0x80},
    {0x4A37, 0x00},
    {0x4A38, 0x00},
    {0x4A39, 0x00},
    {0x4A3A, 0x00},
    //*************************************************
    //   setI_pkt100.txt
    //*************************************************
    {0x4C00, 0x26},
    {0x4C01, 0x10},
    {0x4C02, 0x18},
    {0x4C03, 0x02},
    {0x4C04, 0x7F},
    {0x4C05, 0x3f},
    {0x4C06, 0x3f},
    {0x4C07, 0x00},
    {0x4C1F, 0x0F},
    {0x4C20, 0x0E},
    {0x4C21, 0x0E},
    {0x4C09, 0x04},  // Frame Length H
    {0x4C0A, 0x26},  // Frame Length L
    {0x4C0B, 0x02},  // Line Length H
    {0x4C0C, 0x34},  // Line Length L
    {0x4C0D, 0x00},
    {0x4C0E, 0x00},
    {0x4C0F, 0x08},
    {0x4C10, 0x0F},
    {0x4C11, 0x04},
    {0x4C12, 0x04},
    {0x4C13, 0x03},
    {0x4C14, 0x02},
    {0x4C1A, 0xC0},
    {0x4C1B, 0x80},
    {0x4C15, 0x00},
    {0x4C16, 0x00},
    {0x4C17, 0x00},
    {0x4C18, 0x00},
    {0x4C19, 0x00},
    {0x4C80, 0x00},
    {0x4C81, 0x04},
    {0x4C82, 0x01},
    {0x4C83, 0xDD},
    {0x4C84, 0x04},
    {0x4C85, 0x0F},
    {0x4C86, 0x00},
    {0x4C87, 0x00},
    {0x4C88, 0x00},
    {0x4C89, 0x00},
    {0x4C8A, 0x00},
    {0x4C90, 0x70},
    {0x4C91, 0x00},
    {0x4C93, 0x04},
    {0x4C94, 0x00},
    {0x4C95, 0x00},
    {0x4C96, 0x80},
    {0x4C97, 0x00},
    {0x4C98, 0x80},
    {0x4C99, 0x00},
    {0x4C9A, 0x00},
    {0x4C9B, 0x00},
    {0x4C9C, 0x00},
    {0x4C30, 0x25},
    {0x4C34, 0x00},
    {0x4C35, 0x00},
    {0x4C36, 0x00},
    {0x4C37, 0x00},
    {0x4C38, 0x00},
    {0x4C39, 0x10},
    {0x4C3A, 0x00},
    {0x4C3B, 0x10},
    {0x4C3C, 0x00},
    {0x4C3D, 0x00},
    {0x4C3E, 0x4A},
    {0x4C3F, 0x50},
    {0x4C40, 0x0A},
    {0x4C41, 0x00},
    {0x4C42, 0x00},
    {0x4C43, 0x4A},
    {0x4C44, 0x50},
    {0x4C45, 0x1F},
    {0x4C46, 0xFF},
    {0x4C53, 0x00},
    {0x4C54, 0x01},
    {0x4C55, 0x00},
    {0x4C56, 0x01},
    {0x4CF1, 0x2B},
    {0x4CF4, 0x01},
    {0x4CF5, 0x1E},
    {0x4CB0, 0x00},
    {0x4CB1, 0x00},
    {0x4CB2, 0x00},
    {0x4CBB, 0x00},
    {0x4CB3, 0x00},
    {0x4CB4, 0x20},
    {0x4CB5, 0x00},
    {0x4CB6, 0x20},
    {0x4CB7, 0x00},
    {0x4CB8, 0x00},
    {0x4CB9, 0x00},
    {0x4CBA, 0x00},
    {0x4D00, 0x03},  //CXIA
    {0x4D01, 0x02},
    {0x4D08, 0x80},
    {0x4D02, 0x10},
    {0x4D03, 0x18},
    {0x4D04, 0x02},
    {0x4D05, 0x7F},
    {0x4D06, 0x02},
    {0x4D07, 0x34},
    {0x4D60, 0x00},
    {0x4D61, 0x04},
    {0x4D62, 0x01},
    {0x4D63, 0xDD},
    {0x4D64, 0x04},
    {0x4D65, 0x0F},
    {0x4D66, 0x00},
    {0x4D67, 0x00},
    {0x4D68, 0x00},
    {0x4D69, 0x00},
    {0x4D6A, 0x00},
    {0x4DA0, 0x01},
    {0x4DA1, 0x1E},
    {0x4DB0, 0x03},  //CXIB
    {0x4DB1, 0x02},
    {0x4DB8, 0x80},
    {0x4DB2, 0x10},
    {0x4DB3, 0x18},
    {0x4DB4, 0x02},
    {0x4DB5, 0x7F},
    {0x4DB6, 0x02},
    {0x4DB7, 0x34},
    {0x4E10, 0x00},
    {0x4E11, 0x04},
    {0x4E12, 0x01},
    {0x4E13, 0xDD},
    {0x4E14, 0x04},
    {0x4E15, 0x0F},
    {0x4E16, 0x00},
    {0x4E17, 0x00},
    {0x4E18, 0x00},
    {0x4E19, 0x00},
    {0x4E1A, 0x00},
    {0x4E50, 0x01},
    {0x4E51, 0x1E},
    {0x3001, (SENSOR_LS_OFFSET>>8)&0xff}, // HDR_long_short_distance (H)
    {0x3002, SENSOR_LS_OFFSET &0xff}, // HDR_long_short_distance (L)
    {0x3004, 0x02}, // HDR_ratio
    {0x3024, 0x20},  //PMU
    {0x3025, 0x12},
    {0x3026, 0x00},
    {0x3027, 0x81},
    {0x3028, 0x01},
    {0x3029, 0x00},
    {0x302A, 0x30},
    {0x2000, 0x00},
    {0x4840, 0x00},
    {0x4980, 0x00},
    {0x4C30, 0x00},
    {0x3042, 0x00},
    {0x2088, 0x01},
    {0x2089, 0x00},
    {0x208A, 0xC8},
    {0x2700, 0x01},
    {0x272F, 0x01},
    {0x2711, 0x01},
    {0x2713, 0x04},
    {0x2821, 0x8E},
    {0x4932, 0x01},
    {0x4A72, 0x01},
    {0x4CF2, 0x01},
    {0x2800, 0x01},
    {0x2839, 0x77},
    {0x283A, 0x0B},
    {0x2842, 0x0C},
    {0x2823, 0x03},
    {0x4933, 0x03},
    {0x4A73, 0x03},
    {0x4CF3, 0x03},
    {0x0202, 0x01},   //Exposure time(MSB)
    {0x0203, 0xCF},   //Exposure time(LSB)
    {0x4805, 0x0F},
    {0x48C0, 0x00},
    {0x48C1, 0x00},
    {0x48C2, 0x01},
    {0x4945, 0x0F},
    {0x4A00, 0x00},
    {0x4A01, 0x00},
    {0x4A02, 0x01},
    {0x4C05, 0x0F},
    {0x4C90, 0x00},
    {0x4C91, 0x00},
    {0x4C92, 0x01},
    {0x0104, 0x01},
    {0x0100, 0x01}, // mode_select -> 00 : SW standby   01 : SWcstream   02 : SWastream   03 : SWbstream
};


static struct {     // LINEAR
    // Modify it based on number of support resolution
    enum {LINEAR_RES_1 = 0, LINEAR_RES_2 = 1, LINEAR_RES_END}mode;
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
}HM4030_mipi_linear[] = {
    {LINEAR_RES_1, {2064, 2064, 3, 60}, {0, 0, 2064, 2064}, {"2064x2064@60fps"}},
    {LINEAR_RES_2, {1032, 1032, 3, 60}, {0, 0, 1032, 1032}, {"1032x1032@60fps"}},
};
static struct {     // HDR
    // Modify it based on number of support resolution
    enum {HDR_RES_1 = 0, HDR_RES_2 = 1, HDR_RES_END}mode;
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
}HM4030_mipi_hdr[] = {
    {HDR_RES_1, {2064, 2064, 3, 30}, {0, 0, 2064, 2064}, {"2064x2064@30fps"}},
    {HDR_RES_2, {2064, 2064, 3, 30}, {0, 0, 2064, 2064}, {"2064x2064@30fps"}},
};


#define SensorReg_Read(_reg,_data)     (handle->i2c_bus->i2c_rx(handle->i2c_bus, &(handle->i2c_cfg),_reg,_data))
#define SensorReg_Write(_reg,_data)    (handle->i2c_bus->i2c_tx(handle->i2c_bus, &(handle->i2c_cfg),_reg,_data))
#define SensorRegArrayW(_reg,_len)  (handle->i2c_bus->i2c_array_tx(handle->i2c_bus, &(handle->i2c_cfg),(_reg),(_len)))
#define SensorRegArrayR(_reg,_len)  (handle->i2c_bus->i2c_array_rx(handle->i2c_bus, &(handle->i2c_cfg),(_reg),(_len)))

/////////////////// sensor hardware dependent //////////////
static int pCus_sensor_GetAEInfo(ss_cus_sensor *handle, CUS_SENSOR_AE_INFO_t *info)
{
    hm4030_params *params = (hm4030_params *)handle->private_data;

    info->u8AEGainDelay                      = handle->sensor_ae_info_cfg.u8AEGainDelay;
    info->u8AEShutterDelay                   = handle->sensor_ae_info_cfg.u8AEShutterDelay;
    info->u8AEGainCtrlNum                    = handle->sensor_ae_info_cfg.u8AEGainCtrlNum;
    info->u8AEShutterCtrlNum                 = handle->sensor_ae_info_cfg.u8AEShutterCtrlNum;
    info->u32AEGain_min                      = handle->sensor_ae_info_cfg.u32AEGain_min;
    info->u32AEGain_max                      = handle->sensor_ae_info_cfg.u32AEGain_max;
    info->u32AEShutter_min                   = (params->Default_Res_line_period * 2);
    info->u32AEShutter_step                  = (params->Default_Res_line_period);
    if(handle->interface_attr.attr_mipi.mipi_hdr_mode == CUS_HDR_MODE_NONE)
    {
        info->u32AEShutter_max                   = handle->sensor_ae_info_cfg.u32AEShutter_max;
    }
    else
    {
        if (handle->interface_attr.attr_mipi.mipi_hdr_virtual_channel_num == 1)
        {
            info->u32AEShutter_max                   = (params->Default_Res_line_period * params->expo.max_sef);
        }
        else
        {
            info->u32AEShutter_max                   = handle->sensor_ae_info_cfg.u32AEShutter_max;
        }
    }
    return SUCCESS;
}
static int pCus_poweron(ss_cus_sensor *handle, u32 idx)
{
    ISensorIfAPI *sensor_if = handle->sensor_if_api;
    SENSOR_DMSG("[%s] ", __FUNCTION__);
    //Sensor power on sequence: XSD_L&MCLK -> XSD H -> I2C
    sensor_if->PowerOff(idx, !SENSOR_PWDN_POL);
    sensor_if->Reset(idx, !SENSOR_RST_POL);
    sensor_if->SetIOPad(idx, handle->sif_bus, handle->interface_attr.attr_mipi.mipi_lane_num);
    sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_216M);
    sensor_if->SetCSI_Lane(idx, handle->interface_attr.attr_mipi.mipi_lane_num, 1);
    sensor_if->SetCSI_LongPacketType(idx, 0, 0x1C00, 0);
    if (handle->interface_attr.attr_mipi.mipi_hdr_mode == CUS_HDR_MODE_VC) {
        sensor_if->SetCSI_hdr_mode(idx, handle->interface_attr.attr_mipi.mipi_hdr_mode, 1);
    }
    sensor_if->MCLK(idx, 1, handle->mclk);
    sensor_if->Reset(idx, SENSOR_RST_POL );
    SENSOR_USLEEP(1000);

    sensor_if->PowerOff(idx, SENSOR_PWDN_POL);
    SENSOR_USLEEP(1000);
    sensor_if->PowerOff(idx, !SENSOR_PWDN_POL);
    SENSOR_USLEEP(1000);

    sensor_if->Reset(idx, !SENSOR_RST_POL );
    SENSOR_USLEEP(1000);

    return SUCCESS;
}

static int pCus_poweroff(ss_cus_sensor *handle, u32 idx)
{
    ISensorIfAPI *sensor_if = handle->sensor_if_api;

    SENSOR_DMSG("[%s] reset low\n", __FUNCTION__);
    sensor_if->Reset(idx, SENSOR_RST_POL);
    SENSOR_USLEEP(1000);

    SENSOR_DMSG("[%s] power low\n", __FUNCTION__);
    sensor_if->PowerOff(idx, SENSOR_PWDN_POL);
    SENSOR_USLEEP(1000);

    sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_DISABLE);
    sensor_if->MCLK(idx, 0, handle->mclk);

    if (handle->interface_attr.attr_mipi.mipi_hdr_mode == CUS_HDR_MODE_VC) {
        sensor_if->SetCSI_hdr_mode(idx, handle->interface_attr.attr_mipi.mipi_hdr_mode, 0);
    }
    return SUCCESS;
}

static int pCus_init(ss_cus_sensor *handle)
{
    hm4030_params *params = (hm4030_params *)handle->private_data;
    int i,cnt = 0;
    u32 size = 0;
    const I2C_ARRAY *sensor_init_table = NULL;

    size = ARRAY_SIZE(Sensor_init_table_4lane);
    sensor_init_table = Sensor_init_table_4lane;
    SENSOR_DMSG(KERN_DEBUG "[%s] %px\n", __FUNCTION__,handle);
    //UartSendTrace("hm4030 Sensor_init_table_4lane\n");
    for(i=0; i< size; i++)
    {
        if(sensor_init_table[i].reg == 0xffff)
        {
            SENSOR_MSLEEP(sensor_init_table[i].data);
        }
        else
        {
            cnt = 0;
            while(SensorReg_Write(sensor_init_table[i].reg, sensor_init_table[i].data) != SUCCESS)
            {
                cnt++;
                //printf("Sensor_init_table_4lane -> Retry %d...\n",cnt);
                if(cnt >= 10)
                {
                    //printf("[%s:%d]Sensor init fail!!\n", __FUNCTION__, __LINE__);
                    return FAIL;
                }
                //usleep(10*1000);
            }
            //SensorReg_Read( Sensor_init_table_4lane[i].reg, &sen_data );
            //UartSendTrace("hm4030 reg: 0x%x, data: 0x%x, read: 0x%x.\n",Sensor_init_table_4lane[i].reg, Sensor_init_table_4lane[i].data, sen_data);
        }
    }
    SensorRegArrayW((I2C_ARRAY*)params->tctx_reg, sizeof(ctx_reg)/sizeof(I2C_ARRAY));
    params->tVts_reg[0].data = (params->expo.preview_vts >> 8) & 0x00ff;
    params->tVts_reg[1].data = (params->expo.preview_vts >> 0) & 0x00ff;
    params->expo.final_gain = 1024;
    params->expo.final_gain_sef = 1024;
    params->expo.expo_lines = 0x1f6;
    params->expo.expo_lines_sef = 2;
    memcpy(params->tGain_reg, gain_reg, sizeof(gain_reg));
    memcpy(params->tExpo_reg, expo_reg, sizeof(expo_reg));
    memcpy(params->tmirror_reg, mirror_reg, sizeof(mirror_reg));
    params->reg_dirty = true;
    params->gain_dirty = true;
    params->orien_dirty = true;
    return SUCCESS;
}
int pCus_init_mipi4lane_hdr(ss_cus_sensor *handle)
{
    hm4030_params *params = (hm4030_params *)handle->private_data;
    int i,cnt = 0;
    u32 size = 0;
    const I2C_ARRAY *sensor_init_table = NULL;

    if(handle->video_res_supported.ulcur_res == HDR_RES_2)
    {
        size = ARRAY_SIZE(Sensor_init_table_HDR_4lane);
        sensor_init_table = Sensor_init_table_HDR_4lane;
    }
    else
    {
        size = ARRAY_SIZE(Sensor_init_table_HDR_4lane);
        sensor_init_table = Sensor_init_table_HDR_4lane;
    }
    SENSOR_DMSG(KERN_DEBUG "[%s] %px\n", __FUNCTION__,handle);
    for(i=0; i< size; i++)
    {
        if(sensor_init_table[i].reg == 0xffff)
        {
            SENSOR_MSLEEP(sensor_init_table[i].data);
        }
        else
        {
            cnt = 0;
            while(SensorReg_Write(sensor_init_table[i].reg, sensor_init_table[i].data) != SUCCESS)
            {
                cnt++;
                //printf("Sensor_init_table_4lane -> Retry %d...\n",cnt);
                if(cnt >= 10)
                {
                    //printf("[%s:%d]Sensor init fail!!\n", __FUNCTION__, __LINE__);
                    return FAIL;
                }
                //usleep(10*1000);
            }
            //SensorReg_Read( Sensor_init_table_4lane[i].reg, &sen_data );
            //UartSendTrace("hm4030 reg: 0x%x, data: 0x%x, read: 0x%x.\n",Sensor_init_table_4lane[i].reg, Sensor_init_table_4lane[i].data, sen_data);
        }
    }
    SensorRegArrayW((I2C_ARRAY*)params->tctx_reg, sizeof(ctx_reg)/sizeof(I2C_ARRAY));
    params->tVts_reg[0].data = (params->expo.preview_vts >> 8) & 0x00ff;
    params->tVts_reg[1].data = (params->expo.preview_vts >> 0) & 0x00ff;
    params->expo.final_gain = 1024;
    params->expo.final_gain_sef = 1024;
    params->expo.expo_lines_sef = 2;
    params->expo.expo_lines_sef_pre = 2;
    params->expo.expo_lines_sef_update = 2;
    params->expo.expo_lines = 0x1cf;
    memcpy(params->tGain_hdr_lef_reg, gain_HDR_LEF_reg, sizeof(gain_HDR_LEF_reg));
    memcpy(params->tGain_hdr_sef_reg, gain_HDR_SEF1_reg, sizeof(gain_HDR_SEF1_reg));
    memcpy(params->tExpo_hdr_lef_reg, expo_HDR_LEF_reg, sizeof(expo_HDR_LEF_reg));
    memcpy(params->tExpo_hdr_sef_reg, expo_HDR_SEF_reg, sizeof(expo_HDR_SEF_reg));
    memcpy(params->tmirror_reg, mirror_reg, sizeof(mirror_reg));
    params->reg_dirty = true;
    params->gain_dirty = true;
    params->orien_dirty = true;
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
        return FAIL;
    }

    *res = &handle->video_res_supported.res[*cur_idx];

    return SUCCESS;
}

static int pCus_SetVideoRes(ss_cus_sensor *handle, u32 res_idx)
{
    u32 num_res = handle->video_res_supported.num_res;
    hm4030_params *params = (hm4030_params *)handle->private_data;

    if (res_idx >= num_res) {
        return FAIL;
    }
    switch (res_idx) {
        case 0:
        case 1:
            handle->video_res_supported.ulcur_res = res_idx;
            handle->pCus_sensor_init = pCus_init;
            params->expo.preview_vts = vts_60fps_Linear_2064;
            params->expo.vts_lef = vts_60fps_Linear_2064;
            params->expo.vts_sef = vts_60fps_Linear_2064;
            params->Default_Res_line_period = PREVIEW_LINE_PERIOD((params->expo.preview_vts), Preview_MAX_FPS);
            params->expo.us_per_line = params->Default_Res_line_period / 1000;
            params->expo.Default_Res_fps = Preview_MAX_FPS;
            params->Default_Res_vts = params->expo.preview_vts;
            params->expo.preview_fps = Preview_MAX_FPS;
            params->tctx_reg[0].data = (0x20 | (res_idx));
            params->tVts_reg[0].data = (params->expo.preview_vts >> 8) & 0x00ff;
            params->tVts_reg[1].data = (params->expo.preview_vts >> 0) & 0x00ff;
            break;

        default:
            break;
    }
    pCus_sensor_GetAEInfo(handle, &handle->sensor_ae_info_cfg);
    return SUCCESS;
}
static int pCus_SetVideoRes_hdr(ss_cus_sensor *handle, u32 res_idx)
{
    u32 num_res = handle->video_res_supported.num_res;
    hm4030_params *params = (hm4030_params *)handle->private_data;

    if (res_idx >= num_res) {
        return FAIL;
    }
    switch (res_idx) {
        case 0:
        case 1:
            handle->video_res_supported.ulcur_res = res_idx;
            params->expo.preview_vts = vts_30fps_HDR_2064;
            params->expo.vts_lef = vts_30fps_HDR_2064;
            params->expo.vts_sef = vts_30fps_HDR_2064;
            params->expo.Default_Res_fps = Preview_MAX_FPS_HDR;
            params->Default_Res_line_period = PREVIEW_LINE_PERIOD((params->expo.preview_vts-SENSOR_SEF_TH), Preview_MAX_FPS_HDR);
            params->expo.us_per_line = params->Default_Res_line_period / 1000;
            params->Default_Res_vts = params->expo.preview_vts;
            params->expo.preview_fps = Preview_MAX_FPS_HDR;
            if (handle->interface_attr.attr_mipi.mipi_hdr_virtual_channel_num == 1)
            {
                handle->pCus_sensor_init = pCus_init_mipi4lane_hdr;
            }
            params->tctx_reg[0].data = (0x20);
            params->tVts_reg[0].data = (params->expo.preview_vts >> 8) & 0x00ff;
            params->tVts_reg[1].data = (params->expo.preview_vts >> 0) & 0x00ff;
            break;

        default:
            break;
    }
    pCus_sensor_GetAEInfo(handle, &handle->sensor_ae_info_cfg);
    return SUCCESS;
}

static int pCus_GetOrien(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT *orit)
{
    char sen_data;
    hm4030_params *params = (hm4030_params *)handle->private_data;

    sen_data = params->tmirror_reg[0].data;
    SENSOR_DMSG("mirror:%x\r\n", sen_data & 0x03);
    switch(sen_data & 0x03)
    {
        case 0x00:
            *orit = CUS_ORIT_M0F0;
        break;
        case 0x01:
            *orit = CUS_ORIT_M1F0;
        break;
        case 0x02:
            *orit = CUS_ORIT_M0F1;
        break;
        case 0x03:
            *orit = CUS_ORIT_M1F1;
        break;
    }

    return SUCCESS;
}

static int pCus_SetOrien(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT orit)
{
    hm4030_params *params = (hm4030_params *)handle->private_data;

    switch(orit) {
        case CUS_ORIT_M0F0:
            params->tmirror_reg[0].data = 0;
            handle->bayer_id=    CUS_BAYER_BG;
        break;
        case CUS_ORIT_M1F0:
            params->tmirror_reg[0].data = 1;
            handle->bayer_id=    CUS_BAYER_GB;
        break;
        case CUS_ORIT_M0F1:
            params->tmirror_reg[0].data = 2;
            handle->bayer_id=    CUS_BAYER_GR;
        break;
        case CUS_ORIT_M1F1:
            params->tmirror_reg[0].data = 3;
            handle->bayer_id=    CUS_BAYER_RG;
        break;
    }
    params->orien_dirty = true;

    return SUCCESS;
}

static int pCus_GetFPS(ss_cus_sensor *handle)
{
    hm4030_params *params = (hm4030_params *)handle->private_data;

    return  params->expo.preview_fps;
}

static int pCus_SetFPS(ss_cus_sensor *handle, u32 fps)
{
    hm4030_params *params = (hm4030_params *)handle->private_data;
    u16 height;
    u16 u16max = 60;
    u16 u16min = 5;

    u16min = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16min_fps;
    u16max = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    if(fps>=u16min && fps <= u16max) {
        params->expo.preview_vts =  (params->Default_Res_vts * params->expo.Default_Res_fps)/fps;
        params->expo.vts_lef = params->expo.preview_vts;
        params->expo.vts_sef = params->expo.preview_vts;
        params->expo.preview_fps = fps;
        params->max_sef_step = params->expo.preview_vts - Preview_HEIGHT_HDR - SENSOR_DARK_ROW;
        params->tVts_reg[0].data = (params->expo.preview_vts >> 8) & 0xff;
        params->tVts_reg[1].data = (params->expo.preview_vts >> 0) & 0xff;
        params->reg_dirty = true;
        height = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16height;
        SENSOR_DMSG("\n\n[%s], fps=%d, lines=%d\n", __FUNCTION__, fps, params->expo.preview_vts);
        return SUCCESS;
    }else{
        SENSOR_DMSG("[%s] FPS %d out of range.\n",__FUNCTION__,fps);
        return FAIL;
    }
}

static int pCus_AEStatusNotify(ss_cus_sensor *handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status)
{
    hm4030_params *params = (hm4030_params *)handle->private_data;
    //ISensorIfAPI *sensor_if = handle->sensor_if_api;
    //ISensorIfAPI2 *sensor_if1 = handle->sensor_if_api2;

    switch(status) {
        case CUS_FRAME_ACTIVE:
            if(params->reg_dirty || params->orien_dirty || params->gain_dirty) {
                if(params->orien_dirty)
                {
                    SensorRegArrayW((I2C_ARRAY*)params->tmirror_reg, sizeof(mirror_reg)/sizeof(I2C_ARRAY));
                    params->orien_dirty = false;
                }
                if(params->gain_dirty)
                {
                    SensorRegArrayW((I2C_ARRAY*)params->tGain_reg, sizeof(gain_reg)/sizeof(I2C_ARRAY));
                    params->gain_dirty = false;
                }
                if(params->reg_dirty)
                {
                    SensorRegArrayW((I2C_ARRAY*)params->tctx_reg, sizeof(ctx_reg)/sizeof(I2C_ARRAY));
                    SensorRegArrayW((I2C_ARRAY*)params->tExpo_reg, sizeof(expo_reg)/sizeof(I2C_ARRAY));
                    SensorRegArrayW((I2C_ARRAY*)params->tVts_reg, sizeof(vts_reg)/sizeof(I2C_ARRAY));
                    params->reg_dirty = false;
                }
                SensorReg_Write(0x0104, 0x0001); // Global hold on, 0x3001
                SENSOR_DMSG(KERN_DEBUG "[%s] set  sh:(0x%x%x)gain:(%x,%x,%x)(%d) vts_reg value:(0x%x%x)\n", __FUNCTION__,
                    params->tExpo_reg[0].data,params->tExpo_reg[1].data, params->tGain_reg[0].data, params->tGain_reg[1].data,
                    params->tGain_reg[2].data, params->expo.final_gain, params->tVts_reg[0].data,params->tVts_reg[1].data);
            }
        break;
        case CUS_FRAME_INACTIVE:
        default :
        break;
    }

    return SUCCESS;
}
static int pCus_AEStatusNotify_hdr_sef(ss_cus_sensor *handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status)
{
    hm4030_params *params = (hm4030_params *)handle->private_data;
    u32 vts = 0;
    u32 sef = 0;

    switch(status)
    {
        case CUS_FRAME_INACTIVE:
        #ifdef SENSOR_UPDATE_SEF_DELAY
             if(params->sef_dirty == E_CAMERA_UPDATE_SEF_SET)
             {
                params->sef_dirty = E_CAMERA_UPDATE_SEF_WAIT;
             }
             else if (params->sef_dirty == E_CAMERA_UPDATE_SEF_WAIT)
             {
                params->sef_dirty = E_CAMERA_UPDATE_SEF_HW;
                params->reg_dirty = true;
             }
        #endif
        #ifdef SENSOR_UPDATE_SEF_STEP
            if(params->sef_dirty == E_CAMERA_UPDATE_SEF_WAIT)
            {
                if(params->expo.expo_lines_sef != params->expo.expo_lines_sef_update)
                {
                    params->sef_dirty = E_CAMERA_UPDATE_SEF_SET;
                }
                else
                {
                    params->sef_dirty = E_CAMERA_UPDATE_SEF_HW;
                }
                params->reg_dirty = true;
            }
        #endif
             break;
        case CUS_FRAME_ACTIVE:
            if(params->reg_dirty || params->orien_dirty || params->gain_dirty)
            {
                if(params->orien_dirty){
                     SensorRegArrayW((I2C_ARRAY*)params->tmirror_reg, sizeof(mirror_reg)/sizeof(I2C_ARRAY));
                     params->orien_dirty = false;
                }
                if(params->gain_dirty)
                {
                    SensorRegArrayW((I2C_ARRAY*)params->tGain_hdr_sef_reg, ARRAY_SIZE(gain_HDR_SEF1_reg));
                    SensorRegArrayW((I2C_ARRAY*)params->tGain_hdr_lef_reg, ARRAY_SIZE(gain_HDR_LEF_reg));
                    params->gain_dirty = false;
                }
                if(params->reg_dirty){
                    vts = params->expo.preview_vts;
                    if(params->expo.vts_lef != params->expo.preview_vts || params->expo.vts_sef != params->expo.preview_vts)
                    {
                        if (params->expo.expo_lines_sef + params->expo.expo_lines  >= params->expo.preview_vts - SENSOR_SEF_TH)
                        {
                            vts = (params->expo.expo_lines_sef + SENSOR_SEF_TH + params->expo.expo_lines + 1);
                        }
                    }
                    params->tVts_reg[0].data = (vts >> 8) & 0xff;
                    params->tVts_reg[1].data = (vts >> 0) & 0xff;
                    SensorRegArrayW((I2C_ARRAY*)params->tctx_reg, sizeof(ctx_reg)/sizeof(I2C_ARRAY));
                    SensorRegArrayW((I2C_ARRAY*)params->tVts_reg, sizeof(vts_reg)/sizeof(I2C_ARRAY));
                    SensorRegArrayW((I2C_ARRAY*)params->tExpo_hdr_lef_reg, ARRAY_SIZE(expo_HDR_LEF_reg));
#ifdef SENSOR_UPDATE_SEF_DELAY
                    if(params->sef_dirty == E_CAMERA_UPDATE_SEF_HW)
#endif
#ifdef SENSOR_UPDATE_SEF_STEP
                    if (params->sef_dirty != E_CAMERA_UPDATE_SEF_WAIT)
                    {
                        params->expo.expo_lines_sef_pre = params->expo.expo_lines_sef_update;
                    }
                    sef = params->expo.expo_lines_sef_pre;
                    if(params->sef_dirty == E_CAMERA_UPDATE_SEF_SET)
                    {
                        if((params->expo.expo_lines_sef > sef) &&
                            (params->expo.expo_lines_sef > sef + params->max_sef_step))
                        {
                            sef += params->max_sef_step;
                        }
                        else if((params->expo.expo_lines_sef < sef) &&
                            (sef >= 2 + params->max_sef_step) &&
                            (params->expo.expo_lines_sef < sef - params->max_sef_step))
                        {
                            sef -= params->max_sef_step;
                        }
                        else
                        {
                            sef = params->expo.expo_lines_sef;
                        }
                        params->tExpo_hdr_sef_reg[0].data = (sef>>8) & 0xff;
                        params->tExpo_hdr_sef_reg[1].data = (sef)    & 0xff;
                        params->sef_dirty = E_CAMERA_UPDATE_SEF_WAIT;
                        params->expo.expo_lines_sef_update = sef;
                    }
#endif
                    {
                        SensorRegArrayW((I2C_ARRAY*)params->tExpo_hdr_sef_reg, ARRAY_SIZE(expo_HDR_SEF_reg));
                    }
                    params->reg_dirty=false;
                }
                SensorReg_Write(0x0104, 0x0001); // Global hold on, 0x3001
                /*
                CamOsPrintf(KERN_DEBUG "[%s] set  lsh:(0x%x%x) ssh:(0x%x%x) lgain:(%x,%x,%x)(%d) sgain:(%x,%x,%x)(%d) vts_reg value:(0x%x%x) upsefstm:%d step:%d sef:%d pre:%d\n", __FUNCTION__,
                    params->tExpo_hdr_lef_reg[0].data,params->tExpo_hdr_lef_reg[1].data,
                    params->tExpo_hdr_sef_reg[0].data,params->tExpo_hdr_sef_reg[1].data,
                    params->tGain_hdr_lef_reg[0].data, params->tGain_hdr_lef_reg[1].data,
                    params->tGain_hdr_lef_reg[2].data, params->expo.final_gain,
                    params->tGain_hdr_sef_reg[0].data, params->tGain_hdr_sef_reg[1].data,
                    params->tGain_hdr_sef_reg[2].data, params->expo.final_gain_sef,
                    params->tVts_reg[0].data,params->tVts_reg[1].data,params->sef_dirty,params->max_sef_step,sef,params->expo.expo_lines_sef_pre);*/
            }
            break;
        default :
             break;
    }
    return SUCCESS;
}




// Gain: 1x = 1024
static int pCus_GetAEGain(ss_cus_sensor *handle, u32* gain)
{
    int rc = SUCCESS;
    hm4030_params *params = (hm4030_params *)handle->private_data;

    *gain = params->expo.final_gain;

  return rc;
}
static int pCus_GetAEGainSef(ss_cus_sensor *handle, u32* gain)
{
    int rc = SUCCESS;
    hm4030_params *params = (hm4030_params *)handle->private_data;

    *gain = params->expo.final_gain_sef;

  return rc;
}
int pCus_GetSensorGainParam(u32 gain, Gain_Param *pGain)
{
    u32 x,y,z,u;
    u32 X_array[6] = {1,2,4,8,16,32};
    u32 u32gainXTemp = 0;
    u32 u32ATemp = 0, u32ATempDiff = 0;
    u32 u32Jump = 0,u32BestX = 0,u32BestY = 0, u32BestTemp = 0;

    //gain = (16X + XY) * (64Z + U) = 1024XZ + 16XU +64XYZ + XYU
    // X = 2^(again[6:4]) = {1,2,4,8,16,32}
    // Y = again[3:0]    = {0~15}
    // Z = dgain[1:0]    = {1,2,3}
    // U = dgain[7:2]    = {0~63}
    for(z = 1 ;z < 4 ;z++)
    {
        if(gain < (X_array[5] * (16 + 15)) * (64 * z + 63))
        {
            break;
        }
    }
    for(x = 0 ;x <6 ;x++)
    {
        u32gainXTemp = X_array[x];
        for(y = 0 ;y <16 ;y++)
        {
            u32ATemp = (u32gainXTemp * (16 + y)) * (64 * z );
            if(u32ATemp <= gain)
            {
                u32BestX = x;
                u32BestY = y;
                u32BestTemp = u32ATemp;
            }
            else
            {
                u32Jump = 1;
                break;
            }
        }
        if(u32Jump)
        {
            break;
        }
    }
    u32ATemp = (X_array[u32BestX] * (16 + u32BestY));
    u32ATempDiff = gain - u32BestTemp;
    if(u32ATempDiff)
    {
        for(u = 1 ;u <64 ;u++)
        {
            if(u32ATempDiff < (u32ATemp * u))
            {
                if((u32ATemp * u) - u32ATempDiff > (u32ATemp /2))
                {
                    u--;
                }
                break;
            }
        }
    }
    else
    {
        u = 0;
    }
    pGain->u32AgainMain = u32BestX;
    pGain->u32AgainFloat = u32BestY;
    pGain->u32DgainMain = z;
    pGain->u32DgainFloat = u;
    SENSOR_DMSG(KERN_DEBUG "[%s] set gain %d, ATemp:%d ATempDiff:%d\n", __FUNCTION__, gain, u32ATemp, u32ATempDiff);
    SENSOR_DMSG(KERN_DEBUG "[%s] set gain %d, x:%d y:%d z:%d u:%d\n", __FUNCTION__, gain, u32BestX, u32BestY, z,u);
    return SUCCESS;
}
static int pCus_SetAEGainReg(Gain_Param *pGain,I2C_ARRAY *ptGain_reg)
{
    ptGain_reg[0].data = ((pGain->u32AgainMain << 4) | pGain->u32AgainFloat) &0xff; // A
    ptGain_reg[1].data = (pGain->u32DgainMain) &0x3; // DI
    ptGain_reg[2].data = (pGain->u32DgainFloat << 2) & 0xfc; // DF

    return SUCCESS;
}
static int pCus_GetAEGainFinal(Gain_Param *pGain,u32 *final_gain)
{
    u32 X_array[6] = {1,2,4,8,16,32};

    *final_gain = (X_array[pGain->u32AgainMain] * (16 + pGain->u32AgainFloat)) *
        ((64 * pGain->u32DgainMain) + pGain->u32DgainFloat);
    return SUCCESS;
}
// HII: 2^(gain[7:4])*(1+gain[3:0]/16)
static int pCus_SetAEGain(ss_cus_sensor *handle, u32 gain)
{
    Gain_Param stGain;
    hm4030_params *params = (hm4030_params *)handle->private_data;
    u32 u32Gain = params->expo.final_gain;

    pCus_GetSensorGainParam(gain, &stGain);
    pCus_SetAEGainReg(&stGain, params->tGain_reg);
    pCus_GetAEGainFinal(&stGain, &params->expo.final_gain);

    if(u32Gain != params->expo.final_gain)
    {
        params->gain_dirty = true;
        SENSOR_DMSG(KERN_DEBUG "[%s] set gain %d, reg=0x%x/0x%x.%x\n", __FUNCTION__, gain, params->tGain_reg[0].data, params->tGain_reg[1].data, params->tGain_reg[2].data);
    }
    return SUCCESS;
}
static int pCus_SetAEGain_hdr_lef(ss_cus_sensor *handle, u32 gain)
{
    Gain_Param stGain;
    hm4030_params *params = (hm4030_params *)handle->private_data;
    u32 u32Gain = params->expo.final_gain;

    pCus_GetSensorGainParam(gain, &stGain);
    pCus_SetAEGainReg(&stGain, params->tGain_hdr_lef_reg);
    pCus_GetAEGainFinal(&stGain, &params->expo.final_gain);


    if(u32Gain != params->expo.final_gain)
    {
        params->gain_dirty = true;
        SENSOR_DMSG(KERN_DEBUG "[%s] set gain %d, reg=0x%x/0x%x.%x\n", __FUNCTION__, gain, params->tGain_hdr_lef_reg[0].data, params->tGain_hdr_lef_reg[1].data, params->tGain_hdr_lef_reg[2].data);
    }
    return SUCCESS;
}
static int pCus_SetAEGain_hdr_sef(ss_cus_sensor *handle, u32 gain)
{
    Gain_Param stGain;
    hm4030_params *params = (hm4030_params *)handle->private_data;
    u32 u32Gain = params->expo.final_gain_sef;

    pCus_GetSensorGainParam(gain, &stGain);
    pCus_SetAEGainReg(&stGain, params->tGain_hdr_sef_reg);
    pCus_GetAEGainFinal(&stGain, &params->expo.final_gain_sef);


    if(u32Gain != params->expo.final_gain_sef)
    {
        params->gain_dirty = true;
        SENSOR_DMSG(KERN_DEBUG "[%s] set gain %d, reg=0x%x/0x%x.%x\n", __FUNCTION__, gain, params->tGain_hdr_sef_reg[0].data, params->tGain_hdr_sef_reg[1].data, params->tGain_hdr_sef_reg[2].data);
    }
    return SUCCESS;
}




//HDR


static int pCus_GetAEUSecs(ss_cus_sensor *handle, u32 *us)
{
    int rc=0;
    u32 lines = 0;
    hm4030_params *params = (hm4030_params *)handle->private_data;

    lines  = (u32)(params->tExpo_reg[1].data);
    lines |= (u32)(params->tExpo_reg[0].data)<<8;

    *us = params->expo.final_us;

    SENSOR_DMSG("[%s] sensor expo lines/us %d, %dus\n", __FUNCTION__, lines, *us);
    return rc;
}


static int pCus_GetAEUSecs_hdr_sef(ss_cus_sensor *handle, u32 *us)
{
    int rc=0;
    u32 lines = 0;
    hm4030_params *params = (hm4030_params *)handle->private_data;

    lines  = (u32)(params->tExpo_hdr_sef_reg[1].data);
    lines |= (u32)(params->tExpo_hdr_sef_reg[0].data)<<8;

    *us = params->expo.final_us_sef;

    SENSOR_DMSG("[%s] sensor expo lines/us %d, %dus\n", __FUNCTION__, lines, *us);
    return rc;

}

static int pCus_GetAEUSecs_hdr_lef(ss_cus_sensor *handle, u32 *us)
{
    int rc=0;
    u32 lines = 0;
    hm4030_params *params = (hm4030_params *)handle->private_data;

    lines  = (u32)(params->tExpo_hdr_lef_reg[1].data);
    lines |= (u32)(params->tExpo_hdr_lef_reg[0].data)<<8;

    *us = params->expo.final_us;

    SENSOR_DMSG("[%s] sensor expo lines/us %d, %dus\n", __FUNCTION__, lines, *us);
    return rc;

}

static int pCus_SetAEUSecs(ss_cus_sensor *handle, u32 us)
{
    u32 lines = 0, vts = 0;
    hm4030_params *params = (hm4030_params *)handle->private_data;
    u32 u32preexpoline = params->expo.expo_lines;

    params->expo.final_us = us;
    lines = (1000*us)/params->Default_Res_line_period;
    if(lines < 2)
    {
        lines = 2;
    }
    if (lines >params->expo.preview_vts-2) {
        vts = lines +2;
    }
    else
        vts=params->expo.preview_vts;


    params->expo.expo_lines = lines;
    params->tExpo_reg[0].data = (lines>>8) & 0xff;
    params->tExpo_reg[1].data = (lines)    & 0xff;

    params->tVts_reg[0].data = (vts >> 8) & 0xff;
    params->tVts_reg[1].data = (vts >> 0) & 0xff;
    if(params->expo.expo_lines != u32preexpoline)
    {
        params->reg_dirty = true;
        SENSOR_DMSG(KERN_DEBUG"[%s] us %ld, lines %ld, vts %ld\n", __FUNCTION__,
                    us,
                    lines,
                    vts
                    );
    }
  return SUCCESS;
}
static int pCus_SetAEUSecs_hdr_lef(ss_cus_sensor *handle, u32 us)
{
    u16 expolines = 0, vts = 0;
    hm4030_params *params = (hm4030_params *)handle->private_data;
    u32 u32preexpoline = params->expo.expo_lines;

    params->expo.final_us = us;
    expolines = (1000*us)/params->Default_Res_line_period;
    if(expolines < 2)
    {
        expolines = 2;
    }
    if (expolines + params->expo.expo_lines_sef >= params->expo.preview_vts - SENSOR_SEF_TH) {
        vts = (expolines + SENSOR_SEF_TH + params->expo.expo_lines_sef + 1);
    }
    else
      vts = params->expo.preview_vts;

    params->expo.expo_lines = expolines;
    params->expo.vts_lef = vts;

    //pr_info("[%s]  leslie_shutter,expolines,params_expo_lines : %d,%d,%d\n\n", __FUNCTION__,us,expolines,params->expo.expolines);
    //pr_info("[%s]  leslie_shutter_vts : %u,%u\n\n", __FUNCTION__,params->expo.vts,vts);
    params->tExpo_hdr_lef_reg[0].data = (expolines>>8) & 0xff;
    params->tExpo_hdr_lef_reg[1].data = (expolines)    & 0xff;
    if(params->expo.expo_lines != u32preexpoline)
    {
        params->reg_dirty = true;
        SENSOR_DMSG(KERN_DEBUG"[%s] us %u, vts %u, expolines %u period:%u\n", __FUNCTION__,
                                                     us,  \
                                                     vts,expolines, params->Default_Res_line_period
                   );
    }
    return SUCCESS;
}
static u32 pCus_TryAEShutters_hdr_sef(ss_cus_sensor *handle, u32 us)
{
    u16 expolines = 0;
    hm4030_params *params = (hm4030_params *)handle->private_data;
    u32 sef = 0;
    u32 ret_us = us;

    expolines = (1000*us)/params->Default_Res_line_period;
    if(expolines < 2)
    {
        expolines = 2;
    }
    else if(expolines > SENSOR_LS_OFFSET -5)
    {
        expolines = SENSOR_LS_OFFSET -5;
    }

    sef = params->expo.expo_lines_sef_pre;
    if((expolines > sef) &&
        (expolines > sef + params->max_sef_step))
    {
        sef += params->max_sef_step;
    }
    else if((expolines < sef) &&
        (sef >= 2 + params->max_sef_step) &&
        (expolines < sef - params->max_sef_step))
    {
        sef -= params->max_sef_step;
    }
    else
    {
        sef = expolines;
    }
    if(expolines != sef)
    {
        ret_us = ((params->Default_Res_line_period * sef) + 500) / 1000;
    }
    /*CamOsPrintf(KERN_DEBUG "[%s] us %u, ret_us:%d expolines %u sef %u pre:%d\n", __FUNCTION__,
                                                 us,  ret_us,\
                                                 expolines,sef,params->expo.expo_lines_sef_pre
               );*/
    return ret_us;
}
static int pCus_SetAEUSecs_hdr_sef(ss_cus_sensor *handle, u32 us)
{
    u16 expolines = 0, vts = 0,height = 3840;
    hm4030_params *params = (hm4030_params *)handle->private_data;
    u32 u32preexpoline = params->expo.expo_lines_sef;
    u32 u32Diff = 0;

    params->expo.final_us_sef = us;
    height = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16height;
    expolines = (1000*us)/params->Default_Res_line_period;
    if(expolines < 2)
    {
        expolines = 2;
    }
    else if(expolines > SENSOR_LS_OFFSET -5)
    {
        expolines = SENSOR_LS_OFFSET -5;
    }

    if (expolines + params->expo.expo_lines  >= params->expo.preview_vts - SENSOR_SEF_TH)
    {
        vts = (expolines + SENSOR_SEF_TH + params->expo.expo_lines + 1);
    }
    else
    {
        vts = params->expo.preview_vts;
    }
    params->expo.expo_lines_sef = expolines;
    params->expo.vts_sef = vts;

    //pr_info("[%s]  leslie_shutter,expolines,params_expo_lines : %d,%d,%d\n\n", __FUNCTION__,us,expolines,params->expo.expolines);
    //pr_info("[%s]  leslie_shutter_vts : %u,%u\n\n", __FUNCTION__,params->expo.vts,vts);
#ifndef SENSOR_UPDATE_SEF_STEP
    params->tExpo_hdr_sef_reg[0].data = (expolines>>8) & 0xff;
    params->tExpo_hdr_sef_reg[1].data = (expolines)    & 0xff;
    if(params->expo.expo_lines_sef != u32preexpoline)
    {
        params->reg_dirty = true;
        if(params->expo.expo_lines_sef > u32preexpoline)
        {
            u32Diff = params->expo.expo_lines_sef - u32preexpoline;
        }
        else
        {
            u32Diff = u32preexpoline - params->expo.expo_lines_sef;
        }
        if(u32Diff > params->max_sef_step)
        {
            params->sef_dirty = E_CAMERA_UPDATE_SEF_SET;
        }
        else
        {
            params->sef_dirty = E_CAMERA_UPDATE_SEF_HW;
        }
    }
#else
    if(params->sef_dirty != E_CAMERA_UPDATE_SEF_WAIT)
    {
        params->sef_dirty = E_CAMERA_UPDATE_SEF_SET;
        if(params->expo.expo_lines_sef != u32preexpoline)
        {
            params->reg_dirty = true;
        }
    }
    u32Diff = u32Diff;
    u32preexpoline = u32preexpoline;
#endif
    /*CamOsPrintf(KERN_DEBUG "[%s] us %u, vts %u expolines %u diff %u pre:%d\n", __FUNCTION__,
                                                 us,  \
                                                 vts,expolines,u32Diff,u32preexpoline
               );*/
    return SUCCESS;
}


static int pCus_poweron_hdr_lef(ss_cus_sensor *handle, u32 idx)
{
    return SUCCESS;
}

static int pCus_poweroff_hdr_lef(ss_cus_sensor *handle, u32 idx)
{
    return SUCCESS;
}



static int pCus_AEStatusNotify_hdr_lef(ss_cus_sensor *handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status)
{
    return SUCCESS;
}

static int pCus_SetPatternMode(ss_cus_sensor *handle,u32 mode)
{
    SENSOR_DMSG("\n\n[%s], mode=%d \n", __FUNCTION__, mode);

    return SUCCESS;
}
static int pCus_init_hdr_lef(ss_cus_sensor *handle)
{
    return SUCCESS;
}


int cus_camsensor_init_handle(ss_cus_sensor* drv_handle)
{
    ss_cus_sensor *handle = drv_handle;
    hm4030_params *params;
    int res;

    if (!handle) {
        SENSOR_DMSG("[%s] not enough memory!\n", __FUNCTION__);
        return FAIL;
    }
    SENSOR_DMSG("[%s]", __FUNCTION__);

    //private data allocation & init
    if (handle->private_data == NULL) {
        SENSOR_EMSG("[%s] Private data is empty!\n", __FUNCTION__);
        return FAIL;
    }
    params = (hm4030_params *)handle->private_data;
    memcpy(params->tGain_reg, gain_reg, sizeof(gain_reg));
    memcpy(params->tExpo_reg, expo_reg, sizeof(expo_reg));
    memcpy(params->tmirror_reg, mirror_reg, sizeof(mirror_reg));
    memcpy(params->tctx_reg, ctx_reg, sizeof(ctx_reg));
    memcpy(params->tVts_reg, vts_reg, sizeof(vts_reg));

    ////////////////////////////////////
    //    sensor model ID                           //
    ////////////////////////////////////
    sprintf(handle->strSensorStreamName ,"HM4030_MIPI");

    ////////////////////////////////////
    //    sensor interface info       //
    ////////////////////////////////////
    handle->sif_bus     = SENSOR_IFBUS_TYPE;//;
    handle->data_prec   = SENSOR_DATAPREC;  //;
    handle->bayer_id    = SENSOR_BAYERID;   //;
    handle->RGBIR_id    = SENSOR_RGBIRID;
    handle->interface_attr.attr_mipi.mipi_lane_num = SENSOR_MIPI_LANE_NUM;
    handle->interface_attr.attr_mipi.mipi_data_format = CUS_SEN_INPUT_FORMAT_RGB; // RGB pattern.
    handle->interface_attr.attr_mipi.mipi_yuv_order = 0; //don't care in RGB pattern.
    handle->interface_attr.attr_mipi.mipi_hdr_mode = CUS_HDR_MODE_NONE;
    handle->interface_attr.attr_mipi.mipi_hdr_virtual_channel_num = 0; //Short frame

    ////////////////////////////////////
    //    resolution capability       //
    ////////////////////////////////////

    handle->video_res_supported.ulcur_res = 0; //default resolution index is 0.
    for (res = 0; res < LINEAR_RES_END; res++) {
        handle->video_res_supported.num_res = res +1;
        handle->video_res_supported.res[res].u16width = HM4030_mipi_linear[res].senif.preview_w;
        handle->video_res_supported.res[res].u16height = HM4030_mipi_linear[res].senif.preview_h;
        handle->video_res_supported.res[res].u16max_fps= HM4030_mipi_linear[res].senout.max_fps;
        handle->video_res_supported.res[res].u16min_fps= HM4030_mipi_linear[res].senout.min_fps;
        handle->video_res_supported.res[res].u16crop_start_x= HM4030_mipi_linear[res].senif.crop_start_X;
        handle->video_res_supported.res[res].u16crop_start_y= HM4030_mipi_linear[res].senif.crop_start_y;
        handle->video_res_supported.res[res].u16OutputWidth = HM4030_mipi_linear[res].senout.width;
        handle->video_res_supported.res[res].u16OutputHeight = HM4030_mipi_linear[res].senout.height;
        sprintf(handle->video_res_supported.res[res].strResDesc, HM4030_mipi_linear[res].senstr.strResDesc);
    }
    // i2c
    handle->i2c_cfg.mode                = SENSOR_I2C_LEGACY;    //(
    handle->i2c_cfg.fmt                 = SENSOR_I2C_FMT;       //;
    handle->i2c_cfg.address             = SENSOR_I2C_ADDR;      //;
    handle->i2c_cfg.speed               = SENSOR_I2C_SPEED;     //;

    // mclk
    handle->mclk                        = Preview_MCLK_SPEED;

    //polarity
    /////////////////////////////////////////////////////
    /////////////////////////////////////////////////////


    handle->pCus_sensor_init        = pCus_init;
    handle->pCus_sensor_poweron     = pCus_poweron;
    handle->pCus_sensor_poweroff    = pCus_poweroff;

    handle->pCus_sensor_GetVideoResNum = pCus_GetVideoResNum;
    handle->pCus_sensor_GetVideoRes       = pCus_GetVideoRes;
    handle->pCus_sensor_GetCurVideoRes  = pCus_GetCurVideoRes;
    handle->pCus_sensor_SetVideoRes       = pCus_SetVideoRes;

    handle->pCus_sensor_GetOrien          = pCus_GetOrien;
    handle->pCus_sensor_SetOrien          = pCus_SetOrien;
    handle->pCus_sensor_GetFPS            = pCus_GetFPS;
    handle->pCus_sensor_SetFPS            = pCus_SetFPS;
    handle->pCus_sensor_SetPatternMode    = pCus_SetPatternMode;

    handle->pCus_sensor_AEStatusNotify  = pCus_AEStatusNotify;
    handle->pCus_sensor_GetAEUSecs      = pCus_GetAEUSecs;
    handle->pCus_sensor_SetAEUSecs      = pCus_SetAEUSecs;
    handle->pCus_sensor_GetAEGain       = pCus_GetAEGain;
    handle->pCus_sensor_SetAEGain       = pCus_SetAEGain;
    handle->pCus_sensor_GetAEInfo       = pCus_sensor_GetAEInfo;
    params->expo.preview_vts = vts_60fps_Linear_2064;
    params->expo.vts_lef = vts_60fps_Linear_2064;
    params->expo.vts_sef = vts_60fps_Linear_2064;
    params->expo.Default_Res_fps = Preview_MAX_FPS;
    params->reg_dirty = false;
    params->gain_dirty = false;
    params->orien_dirty = false;
    params->expo.preview_fps = params->expo.Default_Res_fps;
    params->expo.us_per_line = PREVIEW_LINE_PERIOD(vts_60fps_Linear_2064, Preview_MAX_FPS)/1000;
    params->Default_Res_vts = vts_60fps_Linear_2064;
    params->Default_Res_line_period = PREVIEW_LINE_PERIOD(vts_60fps_Linear_2064, Preview_MAX_FPS);
    ////////////////////////////////////////////////////
    // AE parameters
    ////////////////////////////////////////////////////
    handle->sensor_ae_info_cfg.u8AEGainDelay                      = SENSOR_GAIN_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEShutterDelay                   = SENSOR_SHUTTER_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u32AEGain_min                      = SENSOR_MIN_GAIN;
    handle->sensor_ae_info_cfg.u32AEGain_max                      = SENSOR_MAX_GAIN;
    handle->sensor_ae_info_cfg.u32AEShutter_min                   = (params->Default_Res_line_period * 2);
    handle->sensor_ae_info_cfg.u32AEShutter_max                   = 1000000000/Preview_MIN_FPS;
    handle->sensor_ae_info_cfg.u32AEShutter_step                  = params->Default_Res_line_period;
    handle->sensor_ae_info_cfg.u8AEGainCtrlNum                    = 1;
    handle->sensor_ae_info_cfg.u8AEShutterCtrlNum                 = 1;
    //SENSOR_DMSG("[%s] VTS=%d!\n", __FUNCTION__, params->expo.vts);
    return SUCCESS;
}

static int cus_camsensor_init_handle_hdr_sef(ss_cus_sensor* drv_handle)
{
    int res;
    ss_cus_sensor *handle = drv_handle;
    hm4030_params *params = NULL;

    cus_camsensor_init_handle(drv_handle);
    params = (hm4030_params *)handle->private_data;
    //SENSOR_DMSG("[%s] HDR SEF INIT!\n", __FUNCTION__);
    sprintf(handle->strSensorStreamName,"HM4030_MIPI_HDR_SEF");

    handle->bayer_id    = SENSOR_BAYERID_HDR;
    handle->RGBIR_id    = SENSOR_RGBIRID;
    handle->data_prec   = SENSOR_DATAPREC_HDR;
    handle->interface_attr.attr_mipi.mipi_lane_num = SENSOR_MIPI_LANE_NUM;//hdr_lane_num;
    handle->interface_attr.attr_mipi.mipi_hdr_mode = CUS_HDR_MODE_VC;

    handle->video_res_supported.ulcur_res = 0; //default resolution index is 0.
    for (res = 0; res < HDR_RES_END; res++) {
        handle->video_res_supported.num_res = res+1;
        handle->video_res_supported.res[res].u16width         = HM4030_mipi_hdr[res].senif.preview_w;
        handle->video_res_supported.res[res].u16height        = HM4030_mipi_hdr[res].senif.preview_h;
        handle->video_res_supported.res[res].u16max_fps       = HM4030_mipi_hdr[res].senout.max_fps;
        handle->video_res_supported.res[res].u16min_fps       = HM4030_mipi_hdr[res].senout.min_fps;
        handle->video_res_supported.res[res].u16crop_start_x  = HM4030_mipi_hdr[res].senif.crop_start_X;
        handle->video_res_supported.res[res].u16crop_start_y  = HM4030_mipi_hdr[res].senif.crop_start_y;
        handle->video_res_supported.res[res].u16OutputWidth  = HM4030_mipi_hdr[res].senout.width;
        handle->video_res_supported.res[res].u16OutputHeight = HM4030_mipi_hdr[res].senout.height;
        sprintf(handle->video_res_supported.res[res].strResDesc, HM4030_mipi_hdr[res].senstr.strResDesc);
    }

    handle->pCus_sensor_SetVideoRes       = pCus_SetVideoRes_hdr;

    handle->pCus_sensor_init        = pCus_init_mipi4lane_hdr;

    handle->pCus_sensor_GetFPS          = pCus_GetFPS;
    handle->pCus_sensor_SetFPS          = pCus_SetFPS;

    handle->pCus_sensor_AEStatusNotify = pCus_AEStatusNotify_hdr_sef;
    handle->pCus_sensor_GetAEUSecs      = pCus_GetAEUSecs_hdr_sef;
    handle->pCus_sensor_SetAEUSecs      = pCus_SetAEUSecs_hdr_sef;
    handle->pCus_sensor_TryAEShutter    = pCus_TryAEShutters_hdr_sef;
    handle->pCus_sensor_GetAEGain       = pCus_GetAEGainSef;
    handle->pCus_sensor_SetAEGain       = pCus_SetAEGain_hdr_sef;
    handle->pCus_sensor_GetAEInfo       = pCus_sensor_GetAEInfo;
    params->expo.preview_vts = vts_30fps_HDR_2064;
    params->expo.vts_lef = vts_30fps_HDR_2064;
    params->expo.vts_sef = vts_30fps_HDR_2064;
    params->expo.Default_Res_fps = Preview_MAX_FPS_HDR;
    params->expo.preview_fps = params->expo.Default_Res_fps;
    params->expo.us_per_line = PREVIEW_LINE_PERIOD(vts_30fps_HDR_2064, Preview_MAX_FPS_HDR)/1000;
    params->Default_Res_vts = vts_30fps_HDR_2064;
    params->Default_Res_line_period = PREVIEW_LINE_PERIOD(vts_30fps_HDR_2064, Preview_MAX_FPS_HDR);
    params->expo.max_sef = SENSOR_LS_OFFSET - SENSOR_SEF_TH;
    params->max_sef_step = vts_30fps_HDR_2064 - Preview_HEIGHT_HDR - SENSOR_DARK_ROW;
    params->expo.expo_lines_sef_pre = 2;
    params->expo.expo_lines_sef = 2;
    params->expo.expo_lines_sef_update = 2;
    handle->interface_attr.attr_mipi.mipi_hdr_virtual_channel_num = 1; //Short frame
    ////////////////////////////////////////////////////
    // AE parameters
    ////////////////////////////////////////////////////
    handle->sensor_ae_info_cfg.u8AEGainDelay                      = SENSOR_GAIN_DELAY_FRAME_COUNT_HDR_DOL;
    handle->sensor_ae_info_cfg.u8AEShutterDelay                   = SENSOR_SHUTTER_DELAY_FRAME_COUNT_HDR_DOL;
    handle->sensor_ae_info_cfg.u32AEGain_min                      = SENSOR_MIN_GAIN_HDR;
    handle->sensor_ae_info_cfg.u32AEGain_max                      = SENSOR_MAX_GAIN_HDR;
    handle->sensor_ae_info_cfg.u32AEShutter_min                   = (params->Default_Res_line_period * 2);
    handle->sensor_ae_info_cfg.u32AEShutter_max                   = (params->Default_Res_line_period * params->expo.max_sef);
    handle->sensor_ae_info_cfg.u32AEShutter_step                  = params->Default_Res_line_period;
    handle->sensor_ae_info_cfg.u8AEGainCtrlNum                    = SENSOR_GAIN_CTRL_NUM;
    handle->sensor_ae_info_cfg.u8AEShutterCtrlNum                 = SENSOR_SHUTTER_CTRL_NUM;
    return SUCCESS;
}

static int cus_camsensor_init_handle_hdr_lef(ss_cus_sensor* drv_handle)
{
    ss_cus_sensor *handle = drv_handle;
    hm4030_params *params;
    int res;

    //SENSOR_DMSG("[%s] HDR LEF INIT!\n", __FUNCTION__);
    if (!handle) {
        SENSOR_DMSG("[%s] not enough memory!\n", __FUNCTION__);
        return FAIL;
    }
    SENSOR_DMSG("[%s]", __FUNCTION__);
    //private data allocation & init
    if (handle->private_data == NULL) {
        SENSOR_EMSG("[%s] Private data is empty!\n", __FUNCTION__);
        return FAIL;
    }
    params = (hm4030_params *)handle->private_data;
    memcpy(params->tGain_reg, gain_reg, sizeof(gain_reg));
    memcpy(params->tExpo_reg, expo_reg, sizeof(expo_reg));
    memcpy(params->tmirror_reg, mirror_reg, sizeof(mirror_reg));
    memcpy(params->tctx_reg, ctx_reg, sizeof(ctx_reg));
    memcpy(params->tVts_reg, vts_reg, sizeof(vts_reg));
    memcpy(params->tExpo_hdr_lef_reg, expo_HDR_LEF_reg, sizeof(expo_HDR_LEF_reg));
    memcpy(params->tExpo_hdr_sef_reg, expo_HDR_SEF_reg, sizeof(expo_HDR_SEF_reg));
    memcpy(params->tGain_hdr_lef_reg, gain_HDR_LEF_reg, sizeof(gain_HDR_LEF_reg));
    memcpy(params->tGain_hdr_sef_reg, gain_HDR_SEF1_reg, sizeof(gain_HDR_SEF1_reg));

    ////////////////////////////////////
    //    sensor model ID                           //
    ////////////////////////////////////
    sprintf(handle->strSensorStreamName,"HM4030_MIPI_HDR_LEF");

    ////////////////////////////////////
    //    sensor interface info       //
    ////////////////////////////////////
    handle->sif_bus     = SENSOR_IFBUS_TYPE;//CUS_SENIF_BUS_PARL;
    handle->data_prec   = SENSOR_DATAPREC_HDR;  //CUS_DATAPRECISION_8;
    handle->bayer_id    = SENSOR_BAYERID_HDR;   //CUS_BAYER_GB;
    handle->RGBIR_id    = SENSOR_RGBIRID;
    handle->interface_attr.attr_mipi.mipi_lane_num = SENSOR_MIPI_LANE_NUM;
    handle->interface_attr.attr_mipi.mipi_data_format = CUS_SEN_INPUT_FORMAT_RGB; // RGB pattern.
    handle->interface_attr.attr_mipi.mipi_yuv_order = 0; //don't care in RGB pattern.
    handle->interface_attr.attr_mipi.mipi_hdr_mode = CUS_HDR_MODE_VC;
    handle->interface_attr.attr_mipi.mipi_hdr_virtual_channel_num =  0; //Long frame

    ////////////////////////////////////
    //    resolution capability       //
    ////////////////////////////////////

    handle->video_res_supported.ulcur_res = 0; //default resolution index is 0.
    for (res = 0; res < HDR_RES_END; res++) {
        handle->video_res_supported.num_res = res+1;
        handle->video_res_supported.res[res].u16width         = HM4030_mipi_hdr[res].senif.preview_w;
        handle->video_res_supported.res[res].u16height        = HM4030_mipi_hdr[res].senif.preview_h;
        handle->video_res_supported.res[res].u16max_fps       = HM4030_mipi_hdr[res].senout.max_fps;
        handle->video_res_supported.res[res].u16min_fps       = HM4030_mipi_hdr[res].senout.min_fps;
        handle->video_res_supported.res[res].u16crop_start_x  = HM4030_mipi_hdr[res].senif.crop_start_X;
        handle->video_res_supported.res[res].u16crop_start_y  = HM4030_mipi_hdr[res].senif.crop_start_y;
        handle->video_res_supported.res[res].u16OutputWidth  = HM4030_mipi_hdr[res].senout.width;
        handle->video_res_supported.res[res].u16OutputHeight = HM4030_mipi_hdr[res].senout.height;
        sprintf(handle->video_res_supported.res[res].strResDesc, HM4030_mipi_hdr[res].senstr.strResDesc);
    }

    // i2c
    handle->i2c_cfg.mode                = SENSOR_I2C_LEGACY;    //(CUS_ISP_I2C_MODE) FALSE;
    handle->i2c_cfg.fmt                 = SENSOR_I2C_FMT;       //CUS_I2C_FMT_A16D16;
    handle->i2c_cfg.address             = SENSOR_I2C_ADDR;      //0x5a;
    handle->i2c_cfg.speed               = SENSOR_I2C_SPEED;     //320000;

    // mclk
    handle->mclk                        = Preview_MCLK_SPEED;//ParaMclk(SENSOR_DRV_PARAM_MCLK());


    handle->pCus_sensor_init        = pCus_init_hdr_lef;
    handle->pCus_sensor_poweron     = pCus_poweron_hdr_lef;
    handle->pCus_sensor_poweroff    = pCus_poweroff_hdr_lef;

    handle->pCus_sensor_GetVideoResNum = pCus_GetVideoResNum;
    handle->pCus_sensor_GetVideoRes       = pCus_GetVideoRes;
    handle->pCus_sensor_GetCurVideoRes  =  pCus_GetCurVideoRes;
    handle->pCus_sensor_SetVideoRes       = pCus_SetVideoRes_hdr;

    handle->pCus_sensor_GetOrien          = pCus_GetOrien;
    handle->pCus_sensor_SetOrien          = pCus_SetOrien;
    handle->pCus_sensor_GetFPS          = pCus_GetFPS;
    handle->pCus_sensor_SetFPS          = pCus_SetFPS;
    handle->pCus_sensor_SetPatternMode = pCus_SetPatternMode;//imx307_SetPatternMode_hdr_dol_lef;
    handle->pCus_sensor_AEStatusNotify = pCus_AEStatusNotify_hdr_lef;
    handle->pCus_sensor_GetAEUSecs      = pCus_GetAEUSecs_hdr_lef;
    handle->pCus_sensor_SetAEUSecs      = pCus_SetAEUSecs_hdr_lef;
    handle->pCus_sensor_GetAEGain       = pCus_GetAEGain;
    handle->pCus_sensor_SetAEGain       = pCus_SetAEGain_hdr_lef;
    handle->pCus_sensor_GetAEInfo       = pCus_sensor_GetAEInfo;

    params->expo.preview_vts = vts_30fps_HDR_2064;
    params->expo.vts_lef = vts_30fps_HDR_2064;
    params->expo.vts_sef = vts_30fps_HDR_2064;
    params->expo.Default_Res_fps = Preview_MAX_FPS_HDR;
    params->expo.preview_fps = params->expo.Default_Res_fps;
    params->expo.us_per_line = PREVIEW_LINE_PERIOD(vts_30fps_HDR_2064, Preview_MAX_FPS_HDR)/1000;
    params->Default_Res_vts = vts_30fps_HDR_2064;
    params->Default_Res_line_period = PREVIEW_LINE_PERIOD(vts_30fps_HDR_2064, Preview_MAX_FPS_HDR);
    params->reg_dirty = false;
    params->gain_dirty = false;
    params->orien_dirty = false;
    ////////////////////////////////////////////////////
    // AE parameters
    ////////////////////////////////////////////////////
    handle->sensor_ae_info_cfg.u8AEGainDelay                      = SENSOR_GAIN_DELAY_FRAME_COUNT_HDR_DOL;
    handle->sensor_ae_info_cfg.u8AEShutterDelay                   = SENSOR_SHUTTER_DELAY_FRAME_COUNT_HDR_DOL;
    handle->sensor_ae_info_cfg.u32AEGain_min                      = SENSOR_MIN_GAIN_HDR;
    handle->sensor_ae_info_cfg.u32AEGain_max                      = SENSOR_MAX_GAIN_HDR;
    handle->sensor_ae_info_cfg.u32AEShutter_min                   = (params->Default_Res_line_period * 2);
    handle->sensor_ae_info_cfg.u32AEShutter_max                   = 1000000000/Preview_MIN_FPS;
    handle->sensor_ae_info_cfg.u32AEShutter_step                  = params->Default_Res_line_period;
    handle->sensor_ae_info_cfg.u8AEGainCtrlNum                    = SENSOR_GAIN_CTRL_NUM;
    handle->sensor_ae_info_cfg.u8AEShutterCtrlNum                 = SENSOR_SHUTTER_CTRL_NUM;

    return SUCCESS;
}



SENSOR_DRV_ENTRY_IMPL_END_EX(  HM4030,
                            cus_camsensor_init_handle,
                            cus_camsensor_init_handle_hdr_sef,
                            cus_camsensor_init_handle_hdr_lef,
                            hm4030_params
                         );
