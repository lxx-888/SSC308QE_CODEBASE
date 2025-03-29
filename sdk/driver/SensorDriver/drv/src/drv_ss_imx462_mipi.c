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
   Porting owner : ming-en.lu
   Date          : 2023/09/18
   Build on      : Master_V4 I6C
   Verified on   : Not yet
   Remark        : NA
*/

#ifdef __cplusplus
extern "C"
{
#endif

#include <drv_sensor_common.h>
#include <sensor_i2c_api.h>
#include <drv_sensor.h>
#include <drv_sensor_init_table.h> //TODO: move this header to drv_sensor_common.h
#ifdef __cplusplus
}
#endif

SENSOR_DRV_ENTRY_IMPL_BEGIN_EX(IMX462_HDR);

#ifndef ARRAY_SIZE
#define ARRAY_SIZE CAM_OS_ARRAY_SIZE
#endif

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
//#define SENSOR_MIPI_HDR_MODE        (0) //0: Non-HDR mode. 1:Sony DOL mode

//#define SENSOR_ISP_TYPE             ISP_EXT             //ISP_EXT, ISP_SOC (Non-used)
#define SENSOR_IFBUS_TYPE           CUS_SENIF_BUS_MIPI  //CFG //CUS_SENIF_BUS_PARL, CUS_SENIF_BUS_MIPI
//#define SENSOR_MIPI_HSYNC_MODE      PACKET_HEADER_EDGE1
//#define SENSOR_MIPI_HSYNC_MODE_HDR_DOL PACKET_FOOTER_EDGE
#define SENSOR_DATAPREC             CUS_DATAPRECISION_10
#define SENSOR_DATAPREC_DOL         CUS_DATAPRECISION_10
//#define SENSOR_DATAMODE             CUS_SEN_10TO12_9000  //CFG
#define SENSOR_BAYERID              CUS_BAYER_RG         //0h: CUS_BAYER_RG, 1h: CUS_BAYER_GR, 2h: CUS_BAYER_BG, 3h: CUS_BAYER_GB
#define SENSOR_BAYERID_HDR_DOL      CUS_BAYER_RG
#define SENSOR_RGBIRID              CUS_RGBIR_NONE
#define SENSOR_ORIT                 CUS_ORIT_M0F0        //CUS_ORIT_M0F0, CUS_ORIT_M1F0, CUS_ORIT_M0F1, CUS_ORIT_M1F1,

////////////////////////////////////
// MCLK Info                      //
////////////////////////////////////
#define Preview_MCLK_SPEED          CUS_CMU_CLK_37P125MHZ    //CUS_CMU_CLK_24MHZ //CUS_CMU_CLK_37P125MHZ//CUS_CMU_CLK_27MHZ
#define Preview_MCLK_SPEED_HDR_DOL  CUS_CMU_CLK_37P125MHZ

////////////////////////////////////
// I2C Info                       //
////////////////////////////////////
#define SENSOR_I2C_ADDR              0x34                //I2C slave address
#define SENSOR_I2C_SPEED             300000              //200000 //300000 //240000                  //I2C speed, 60000~320000
#define SENSOR_I2C_LEGACY            I2C_NORMAL_MODE     //usally set CUS_I2C_NORMAL_MODE,  if use old OVT I2C protocol=> set CUS_I2C_LEGACY_MODE
#define SENSOR_I2C_FMT               I2C_FMT_A16D8       //CUS_I2C_FMT_A8D8, CUS_I2C_FMT_A8D16, CUS_I2C_FMT_A16D8, CUS_I2C_FMT_A16D16

////////////////////////////////////
// Sensor Signal                  //
////////////////////////////////////
#define SENSOR_PWDN_POL              CUS_CLK_POL_NEG     // if PWDN pin High can makes sensor in power down, set CUS_CLK_POL_POS
#define SENSOR_RST_POL               CUS_CLK_POL_NEG     // if RESET pin High can makes sensor in reset state, set CUS_CLK_POL_NEG

// VSYNC/HSYNC POL can be found in data sheet timing diagram,
// Notice: the initial setting may contain VSYNC/HSYNC POL inverse settings so that condition is different.
#define SENSOR_VSYNC_POL             CUS_CLK_POL_NEG     // if VSYNC pin High and data bus have data, set CUS_CLK_POL_POS
#define SENSOR_HSYNC_POL             CUS_CLK_POL_NEG     // if HSYNC pin High and data bus have data, set CUS_CLK_POL_POS
#define SENSOR_PCLK_POL              CUS_CLK_POL_NEG     // depend on sensor setting, sometimes need to try CUS_CLK_POL_POS or CUS_CLK_POL_NEG

////////////////////////////////////
// Sensor ID                      //
////////////////////////////////////
//define SENSOR_ID

#undef SENSOR_NAME
#define SENSOR_NAME     IMX462

#define CHIP_ID_r3F12   0x3F12
#define CHIP_ID_r3F13   0x3F13
#define CHIP_ID         0x0462

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
}imx462_mipi_linear[] = {
    {LINEAR_RES_1, {1920, 1080, 3, 60}, {0, 0, 1920, 1080}, {"1920x1080@60fps"}},
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
}imx462_mipi_hdr[] = {
    {HDR_RES_1, {1920, 1080, 3, 30}, {0, 0, 1920, 1080}, {"1920x1080@30fps_HDR"}}, // Modify it
};

#define vts_30fps                                   1125
#define vts_30fps_HDR_DOL_4lane                     1375//1125
#define Preview_line_period                         14814
#define Preview_line_period_HDR_DOL_4LANE           24242/2 //29629

////////////////////////////////////
// AE Info                        //
////////////////////////////////////
#define SENSOR_MAX_GAIN                             (3715 * 1024)        // max sensor again, a-gain * conversion-gain*d-gain
#define SENSOR_MIN_GAIN                             (1 * 1024)
#define SENSOR_GAIN_DELAY_FRAME_COUNT               (2)
#define SENSOR_SHUTTER_DELAY_FRAME_COUNT            (2)
#define SENSOR_SHUTTER_DELAY_FRAME_COUNT_HDR_DOL    (2)

////////////////////////////////////
// Mirror-Flip Info               //
////////////////////////////////////
#define MIRROR_FLIP                                 0x3007
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
        u32 final_gain;
        u32 fps;
        u32 preview_fps;
        u32 expo_lines;
        u32 expo_lef_us;
        u32 expo_sef_us;
    } expo;
    u32 y_outsize;
    u32 max_rhs1;
    u32 SEF_SHS1;
    u32 LEF_SHS2;
    u32 expo_lines_sef;
    I2C_ARRAY tVts_reg[3];
    I2C_ARRAY tExpo_reg[3];
    I2C_ARRAY tExpo_reg_LEF[3];
    I2C_ARRAY tExpo_reg_SEF[3];
    I2C_ARRAY tRHS1_reg[5];
    I2C_ARRAY tGain_reg[1];
    I2C_ARRAY tGain_hdr_dol_lef_reg[1];
    I2C_ARRAY tGain_hdr_dol_sef_reg[1];
    bool dirty;
    bool orien_dirty;
    CUS_CLK_POL pwdn_POLARITY;
    CUS_CLK_POL reset_POLARITY;
} imx462_params;

static int pCus_SetAEUSecs(ss_cus_sensor *handle, u32 us);
static int pCus_SetAEUSecsHDR_DOL_SEF(ss_cus_sensor *handle, u32 us);
static int pCus_SetAEUSecsHDR_DOL_LEF(ss_cus_sensor *handle, u32 us);
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

const static I2C_ARRAY Sensor_id_table[] = {
    {0x3004, 0x10},      // {address of ID, ID },
    {0x3008, 0xA0},      // {address of ID, ID },
};

const static I2C_ARRAY Sensor_init_table_2m_60fps_4lane_linear[] =
{
/*
IMX462LQR All-pixel scan(Full HD 1080p) CSI-2_4lane 37.125MHz AD:10bit Output:10bit 445.5Mbps Master Mode LCG Mode 60fps Integration Time 16.637ms
Tool ver : Ver6.0
*/
    {0x3000, 0x01}, /* standby */
    {0x3002, 0x01}, /* XTMSTA */
    {0x3005, 0x00}, // ADBIT
    {0x3009, 0x01}, // FRSEL [1:0]
    {0x300A, 0x3C}, // BLKLEVEL [8:0]
    {0x300F, 0x00}, // -
    {0x3010, 0x21}, // FPGC [7:0]
    {0x3011, 0x02}, // -
    {0x3012, 0x64}, // -
    {0x3016, 0x09}, // -
    {0x301C, 0x98}, // HMAX [15:0]
    {0x301D, 0x08}, //
    {0x3020, 0x02}, // SHS1 [19:0]
    {0x3046, 0x00}, // ODBIT[1:0]
    {0x305C, 0x18}, // INCKSEL1
    {0x305D, 0x03}, // INCKSEL2
    {0x305E, 0x20}, // INCKSEL3
    {0x3070, 0x02},
    {0x3071, 0x11},
    {0x309B, 0x10},
    {0x309C, 0x22},
    {0x30A2, 0x02},
    {0x30A6, 0x20},
    {0x30A8, 0x20},
    {0x30AA, 0x20},
    {0x30AC, 0x20},
    {0x30B0, 0x43}, // GRSET1
    {0x3119, 0x9E},
    {0x311C, 0x1E},
    {0x311E, 0x08},
    {0x3128, 0x05},
    {0x3129, 0x1D}, // ADBIT1
    {0x313D, 0x83}, // -
    {0x3150, 0x03}, // -
    {0x315E, 0x1A}, // INCKSEL5
    {0x3164, 0x1A}, // INCKSEL6
    {0x317C, 0x12}, // ADBIT2
    {0x317E, 0x00}, // -
    {0x31EC, 0x37}, // ADBIT3
    {0x3257, 0x03},
    {0x3264, 0x1A},
    {0x3265, 0xB0},
    {0x3266, 0x02},
    {0x326B, 0x10},
    {0x3274, 0x1B},
    {0x3275, 0xA0},
    {0x3276, 0x02},
    {0x32B8, 0x50},
    {0x32B9, 0x10},
    {0x32BA, 0x00},
    {0x32BB, 0x04},
    {0x32C8, 0x50},
    {0x32C9, 0x10},
    {0x32CA, 0x00},
    {0x32CB, 0x04},
    {0x332C, 0xD3},
    {0x332D, 0x10},
    {0x332E, 0x0D},
    {0x3358, 0x06},
    {0x3359, 0xE1},
    {0x335A, 0x11},
    {0x3360, 0x1E},
    {0x3361, 0x61},
    {0x3362, 0x10},
    {0x33B0, 0x50},
    {0x33B2, 0x1A},
    {0x33B3, 0x04},
    {0x3405, 0x10},
    {0x3441, 0x0A}, // CSI_DT_FMT[15:0]
    {0x3442, 0x0A}, //
    {0x3444, 0x20}, // EXTCK_FREQ[15:0]
    {0x3445, 0x25}, //
    {0x3446, 0x57}, // TCLKPOST[8:0]
    {0x3448, 0x37}, // THSZERO[8:0]
    {0x344A, 0x1F}, // THSPREPARE[8:0]
    {0x344C, 0x1F}, // TCLKTRAIL[8:0]
    {0x344E, 0x1F}, // THSTRAIL[8:0]
    {0x3450, 0x77}, // TCLKZERO[8:0]
    {0x3452, 0x1F}, // TCLKPREPARE [8:0]
    {0x3454, 0x17}, // TLPX[8:0]
    {0x3480, 0x49}, // INCKSEL7

    {0xffff, 0x10},
    {0x3000, 0x00}, /* standby */
    {0xffff, 0x14},
    {0x3002, 0x00}, /* master mode start */
};

const static I2C_ARRAY Sensor_2m_30fps_init_table_4lane_HDR_DOL[] =
{
/*
IMX462LQR All-pixel scan(Full HD 1080p) CSI-2_4lane 37.125MHz AD:10bit Output:10bit 891Mbps Master Mode LCG Mode DOL HDR 2frame 30fps Integration Time LEF:3.2ms SEF:0.089ms
Tool ver : Ver6.0
*/
    {0x3000, 0x01}, /* standby */
    {0x3002, 0x01}, /* XTMSTA */

    {0x3005, 0x00}, // ADBIT
    {0x3009, 0x00}, // FRSEL [1:0]
    {0x300A, 0x3C}, // BLKLEVEL [8:0]
    {0x300C, 0x11}, // WDMODE
    {0x300F, 0x00}, // -
    {0x3010, 0x21}, // FPGC [7:0]
    {0x3011, 0x02}, // -
    {0x3012, 0x64}, // -
    {0x3016, 0x09}, // -
    {0x3018, 0x5f}, // VMAX [15:0]
    {0x3019, 0x05}, //
    {0x301C, 0x08}, // HMAX [15:0]
    {0x301D, 0x07}, //
    {0x3020, 0x0a}, // SHS1 [19:0]
    {0x3024, 0xF2}, // SHS2 [19:0]
    {0x3025, 0x02}, //
    {0x3030, 0xff}, // RHS1 [19:0]
    {0x3031, 0x00}, //
    {0x3045, 0x05}, // DOLSCDEN
    {0x3046, 0x00}, // ODBIT[1:0]
    {0x305C, 0x18}, // INCKSEL1
    {0x305D, 0x03}, // INCKSEL2
    {0x305E, 0x20}, // INCKSEL3
    {0x3070, 0x02},
    {0x3071, 0x11},
    {0x309B, 0x10},
    {0x309C, 0x22},
    {0x30A2, 0x02},
    {0x30A6, 0x20},
    {0x30A8, 0x20},
    {0x30AA, 0x20},
    {0x30AC, 0x20},
    {0x30B0, 0x43}, // GRSET1
    {0x3106, 0x11}, // XVSMSKCNT [1:0]
    {0x3119, 0x9E},
    {0x311C, 0x1E},
    {0x311E, 0x08},
    {0x3128, 0x05},
    {0x3129, 0x1D}, // ADBIT1
    {0x313D, 0x83}, // -
    {0x3150, 0x03}, // -
    {0x315E, 0x1A}, // INCKSEL5
    {0x3164, 0x1A}, // INCKSEL6
    {0x31A0, 0xB4}, // HBLANK1 [11:0]
    {0x31A1, 0x02}, //
    {0x317C, 0x12}, // ADBIT2
    {0x317E, 0x00}, // -
    {0x31EC, 0x37}, // ADBIT3
    {0x3257, 0x03},
    {0x3264, 0x1A},
    {0x3265, 0xB0},
    {0x3266, 0x02},
    {0x326B, 0x10},
    {0x3274, 0x1B},
    {0x3275, 0xA0},
    {0x3276, 0x02},
    {0x32B8, 0x50},
    {0x32B9, 0x10},
    {0x32BA, 0x00},
    {0x32BB, 0x04},
    {0x32C8, 0x50},
    {0x32C9, 0x10},
    {0x32CA, 0x00},
    {0x32CB, 0x04},
    {0x332C, 0xD3},
    {0x332D, 0x10},
    {0x332E, 0x0D},
    {0x3358, 0x06},
    {0x3359, 0xE1},
    {0x335A, 0x11},
    {0x3360, 0x1E},
    {0x3361, 0x61},
    {0x3362, 0x10},
    {0x33B0, 0x50},
    {0x33B2, 0x1A},
    {0x33B3, 0x04},
    {0x3405, 0x00},
    {0x3415, 0x00}, // NULL0_SIZE_V [5:0]
    {0x3418, 0xa8}, // Y_OUT_SIZE[12:0] 0x8ce
    {0x3419, 0x09}, //
    {0x3441, 0x0A}, // CSI_DT_FMT[15:0]
    {0x3442, 0x0A}, //
    {0x3444, 0x20}, // EXTCK_FREQ[15:0]
    {0x3445, 0x25}, // TCLKPOST[8:0]
    {0x3446, 0x77}, // TCLKPOST[8:0]
    {0x3448, 0x67}, // THSZERO[8:0]
    {0x344A, 0x47}, // THSPREPARE[8:0]
    {0x344C, 0x37}, // TCLKTRAIL[8:0]
    {0x344E, 0x3F}, // THSTRAIL[8:0]
    {0x3450, 0xFF}, // TCLKZERO[8:0]
    {0x3452, 0x3F}, // TCLKPREPARE [8:0]
    {0x3454, 0x37}, // TLPX[8:0]
    {0x347B, 0x23}, // MIF_SYNC_TIM0
    {0x3480, 0x49}, // INCKSEL7

    {0xffff, 0x10},
    {0x3000, 0x00}, /* standby */
    {0xffff, 0x14},
    {0x3002, 0x00}, /* master mode start */
};

const static I2C_ARRAY expo_SHS2_reg[] = { // SHS2 (HDR For LEF)
    {0x3026, 0x00},
    {0x3025, 0x00},
    {0x3024, 0xc4},
};

const static I2C_ARRAY expo_SHS1_reg[] = { // SHS1 (Linear mode; HDR For SEF)
//decreasing exposure ratio version.
    {0x3022, 0x00},
    {0x3021, 0x00},
    {0x3020, 0x09},
};

const static I2C_ARRAY vts_reg[] = {       //VMAX
    {0x301a, 0x00}, //bit0-3-->MSB
    {0x3019, 0x04},
    {0x3018, 0x65},
};

const static I2C_ARRAY vts_reg_hdr[] = {
    {0x301a, 0x00}, //bit0-3-->MSB
    {0x3019, 0x09},
    {0x3018, 0x89},
};

const I2C_ARRAY expo_RHS1_reg[] = {
    //decreasing exposure ratio version.
    {0x3032, 0x00},
    {0x3031, 0x00},
    {0x3030, 0xff},
    {0x3419, 0x09}, // y_outsize: (1109 +(RHS1-1)/2)*2
    {0x3418, 0xa8},
};

const static I2C_ARRAY gain_reg[] = {
    {0x3014, 0x2A},
};

const static I2C_ARRAY gain_HDR_DOL_LEF_reg[] = {
    {0x3014, 0x2A},
};

const static I2C_ARRAY gain_HDR_DOL_SEF_reg[] = {
    {0x30f2, 0x2A},
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
    imx462_params *params = (imx462_params *)handle->private_data;

    info->u8AEGainDelay                      = handle->sensor_ae_info_cfg.u8AEGainDelay;
    info->u8AEShutterDelay                   = handle->sensor_ae_info_cfg.u8AEShutterDelay;
    info->u8AEGainCtrlNum                    = handle->sensor_ae_info_cfg.u8AEGainCtrlNum;
    info->u8AEShutterCtrlNum                 = handle->sensor_ae_info_cfg.u8AEShutterCtrlNum;
    info->u32AEGain_min                      = handle->sensor_ae_info_cfg.u32AEGain_min;
    info->u32AEGain_max                      = handle->sensor_ae_info_cfg.u32AEGain_max;
    if(handle->interface_attr.attr_mipi.mipi_hdr_mode==CUS_HDR_MODE_NONE)
    {
        info->u32AEShutter_min                   = Preview_line_period;
        info->u32AEShutter_step                  = Preview_line_period;
        info->u32AEShutter_max                   = handle->sensor_ae_info_cfg.u32AEShutter_max;
    }
    else
    {
        info->u32AEShutter_step                  = Preview_line_period_HDR_DOL_4LANE;
        if (handle->interface_attr.attr_mipi.mipi_hdr_virtual_channel_num == 1)
        {
            info->u32AEShutter_min                   = (Preview_line_period_HDR_DOL_4LANE * 2);
            info->u32AEShutter_max                   = Preview_line_period_HDR_DOL_4LANE * params->max_rhs1;
        }
        else
        {
            info->u32AEShutter_min                   = (Preview_line_period_HDR_DOL_4LANE * 5);
            info->u32AEShutter_max                   = handle->sensor_ae_info_cfg.u32AEShutter_max;
        }
    }
    return SUCCESS;
}
static int pCus_poweron(ss_cus_sensor *handle, u32 idx)
{
    ISensorIfAPI *sensor_if = handle->sensor_if_api;
    imx462_params *params = (imx462_params *)handle->private_data;

    //Sensor power on sequence
    sensor_if->PowerOff(idx, params->pwdn_POLARITY);   // Powerdn Pull Low
    sensor_if->Reset(idx, params->reset_POLARITY);     // Rst Pull Low
    sensor_if->SetIOPad(idx, handle->sif_bus, handle->interface_attr.attr_mipi.mipi_lane_num);
    sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_216M);
    sensor_if->SetCSI_Lane(idx, handle->interface_attr.attr_mipi.mipi_lane_num, ENABLE);
    sensor_if->SetCSI_LongPacketType(idx, 0, 0x1C00, 0);

    if (handle->interface_attr.attr_mipi.mipi_hdr_mode == CUS_HDR_MODE_SONY_DOL) {
        sensor_if->SetCSI_hdr_mode(idx, handle->interface_attr.attr_mipi.mipi_hdr_mode, 1);
    }

    sensor_if->PowerOff(idx, !params->pwdn_POLARITY);
    //Sensor board PWDN Enable, 1.8V & 2.9V need 30ms then Pull High
    SENSOR_MSLEEP(31);
    sensor_if->Reset(idx, !params->reset_POLARITY);
    SENSOR_UDELAY(1);
    sensor_if->MCLK(idx, 1, handle->mclk);
    SENSOR_DMSG("Sensor Power On finished\n");
    return SUCCESS;
}

static int pCus_poweroff(ss_cus_sensor *handle, u32 idx)
{
    // power/reset low
    ISensorIfAPI *sensor_if = handle->sensor_if_api;
    imx462_params *params = (imx462_params *)handle->private_data;

    SENSOR_DMSG("[%s] reset low\n", __FUNCTION__);
    sensor_if->PowerOff(idx, params->pwdn_POLARITY);   // Powerdn Pull Low
    sensor_if->Reset(idx, params->reset_POLARITY);     // Rst Pull Low
    sensor_if->MCLK(idx, 0, handle->mclk);

    sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_DISABLE);
    if (handle->interface_attr.attr_mipi.mipi_hdr_mode == CUS_HDR_MODE_SONY_DOL) {
        sensor_if->SetCSI_hdr_mode(idx, handle->interface_attr.attr_mipi.mipi_hdr_mode, 0);
    }

    params->res.orit = SENSOR_ORIT;

    return SUCCESS;
}

/////////////////// Check Sensor Product ID /////////////////////////
static int pCus_CheckSensorProductID(ss_cus_sensor *handle)
{
    u16 sen_id_msb, sen_id_lsb;

    /* Read Product ID */
    SensorReg_Read(0x3f12, (void*)&sen_id_lsb);
    SensorReg_Read(0x3f13, (void*)&sen_id_msb);//CHIP_ID_r3F13
#if 0
    if (sen_data != CHIP_ID) {
        printk("[***ERROR***]Check Product ID Fail: 0x%x\n", sen_data);
        return FAIL;
    }
#endif
    return SUCCESS;
}

static int imx462_SetPatternMode(ss_cus_sensor *handle,u32 mode)
{

    return SUCCESS;
}

static int pCus_init_2m_60fps_mipi4lane_linear(ss_cus_sensor *handle)
{
    //imx462_params *params = (imx462_params *)handle->private_data;
    int i,cnt=0;

    if (pCus_CheckSensorProductID(handle) == FAIL) {
        return FAIL;
    }

    for(i=0;i< ARRAY_SIZE(Sensor_init_table_2m_60fps_4lane_linear);i++)
    {
        if(Sensor_init_table_2m_60fps_4lane_linear[i].reg==0xffff)
        {
            SENSOR_MSLEEP(Sensor_init_table_2m_60fps_4lane_linear[i].data);
        }else
        {
            cnt = 0;
            while(SensorReg_Write(Sensor_init_table_2m_60fps_4lane_linear[i].reg, Sensor_init_table_2m_60fps_4lane_linear[i].data) != SUCCESS)
            {
                cnt++;
                if(cnt>=10)
                {
                    SENSOR_EMSG("[%s:%d]Sensor init fail!!\n", __FUNCTION__, __LINE__);
                    return FAIL;
                }
                //SENSOR_UDELAY(1);
            }
        }
    }

    return SUCCESS;
}

static int pCus_init_2m_30fps_mipi4lane_HDR_DOL(ss_cus_sensor *handle)
{
    int i,cnt=0;

    if (pCus_CheckSensorProductID(handle)) {
        return FAIL;
    }
    for(i=0;i< ARRAY_SIZE(Sensor_2m_30fps_init_table_4lane_HDR_DOL);i++)
    {
        if(Sensor_2m_30fps_init_table_4lane_HDR_DOL[i].reg==0xffff)
        {
            SENSOR_MSLEEP(Sensor_2m_30fps_init_table_4lane_HDR_DOL[i].data);
        }else
        {
            cnt = 0;
            while(SensorReg_Write(Sensor_2m_30fps_init_table_4lane_HDR_DOL[i].reg,Sensor_2m_30fps_init_table_4lane_HDR_DOL[i].data) != SUCCESS)
            {
                cnt++;
                if(cnt>=10)
                {
                    SENSOR_EMSG("[%s:%d]Sensor init fail!!\n", __FUNCTION__, __LINE__);
                    return FAIL;
                }
                //SENSOR_UDELAY(1);
            }
        }
    }
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
    imx462_params *params = (imx462_params *)handle->private_data;
    u32 num_res = handle->video_res_supported.num_res;

    if (res_idx >= num_res) {
        SENSOR_EMSG("[%s] Please check the number of resolutions supported by the sensor!\n", __FUNCTION__);
        return FAIL;
    }

    handle->video_res_supported.ulcur_res = res_idx;

    switch (res_idx) {
        case 0:
            handle->video_res_supported.ulcur_res = 0;
            handle->pCus_sensor_init = pCus_init_2m_60fps_mipi4lane_linear;
            params->expo.vts = vts_30fps;
            params->expo.fps = 30;
            break;
        default:
            break;
    }
    return SUCCESS;
}

static int pCus_SetVideoRes_HDR_DOL_LEF(ss_cus_sensor *handle, u32 res_idx)
{
    u32 num_res = handle->video_res_supported.num_res;

    if (res_idx >= num_res) {
        SENSOR_EMSG("[%s] Please check the number of resolutions supported by the sensor!\n", __FUNCTION__);
        return FAIL;
    }

    handle->video_res_supported.ulcur_res = res_idx;
    return SUCCESS;
}

static int pCus_SetVideoRes_HDR_DOL_SEF(ss_cus_sensor *handle, u32 res_idx)
{
    imx462_params *params = (imx462_params *)handle->private_data;
    u32 num_res = handle->video_res_supported.num_res;

    if (res_idx >= num_res) {
        SENSOR_EMSG("[%s] Please check the number of resolutions supported by the sensor!\n", __FUNCTION__);
        return FAIL;
    }

    handle->video_res_supported.ulcur_res = res_idx;
    switch (res_idx) {
        case HDR_RES_1:
            handle->pCus_sensor_init = pCus_init_2m_30fps_mipi4lane_HDR_DOL;
            params->expo.vts = vts_30fps_HDR_DOL_4lane;
            params->expo.fps = 30;
            params->max_rhs1 = 255;//MAX
            break;
        default:
            break;
    }

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
    imx462_params *params = (imx462_params *)handle->private_data;

    params->res.orit = orit;
    params->orien_dirty = true;

    return SUCCESS;
}

static int DoOrien(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT orit)
{
    imx462_params *params = (imx462_params *)handle->private_data;
    s16 sen_data;
    //Read SENSOR MIRROR-FLIP STATUS
    SensorReg_Read(MIRROR_FLIP, (void*)&sen_data);
    sen_data &= ~(SENSOR_MIRROR_FLIP_EN);

    switch(orit)
    {
        case CUS_ORIT_M0F0:
            //sen_data |= SENSOR_NOR;
            params->res.orit = CUS_ORIT_M0F0;
            break;
        case CUS_ORIT_M1F0:
            sen_data |= SENSOR_MIRROR_EN;
            params->res.orit = CUS_ORIT_M1F0;
            break;
        case CUS_ORIT_M0F1:
            sen_data |= SENSOR_FLIP_EN;
            params->res.orit = CUS_ORIT_M0F1;
            break;
        case CUS_ORIT_M1F1:
            sen_data |= SENSOR_MIRROR_FLIP_EN;
            params->res.orit = CUS_ORIT_M1F1;
            break;
        default :
            params->res.orit = CUS_ORIT_M0F0;
            break;
    }
    //Write SENSOR MIRROR-FLIP STATUS
    SensorReg_Write(MIRROR_FLIP, sen_data);

    return SUCCESS;
}

static int pCus_GetFPS(ss_cus_sensor *handle)
{
    imx462_params *params = (imx462_params *)handle->private_data;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 tVts = (params->tVts_reg[0].data << 16) | (params->tVts_reg[1].data << 8) | (params->tVts_reg[2].data << 0);

    if (params->expo.fps >= 1000)
        params->expo.preview_fps = (vts_30fps*max_fps*1000)/tVts;
    else
        params->expo.preview_fps = (vts_30fps*max_fps)/tVts;

    return params->expo.preview_fps;
}

static int pCus_SetFPS(ss_cus_sensor *handle, u32 fps)
{
    u32 vts = 0;
    imx462_params *params = (imx462_params *)handle->private_data;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 min_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16min_fps;
    if(fps>=min_fps && fps <= max_fps){
        params->expo.vts=  (vts_30fps*(max_fps*1000) + fps * 500)/(fps * 1000);
    }else if(fps>=(min_fps*1000) && fps <= (max_fps*1000)){
        params->expo.vts=  (vts_30fps*(max_fps*1000) + (fps>>1))/fps;
    }else{
      SENSOR_DMSG("[%s] FPS %d out of range.\n",__FUNCTION__,fps);
      return FAIL;
    }

    if(params->expo.expo_lines > params->expo.vts - 2){
        vts = params->expo.expo_lines + 2;
    }else{
        vts = params->expo.vts;
    }

    params->expo.fps = fps;
    params->dirty = true;

    pCus_SetAEUSecs(handle, params->expo.expo_lef_us);
    return SUCCESS;
}

static int pCus_GetFPS_HDR_DOL_SEF(ss_cus_sensor *handle)
{
    imx462_params *params = (imx462_params *)handle->private_data;
    u32 cur_vts_30fps = vts_30fps_HDR_DOL_4lane;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 tVts = (params->tVts_reg[0].data << 16) | (params->tVts_reg[1].data << 8) | (params->tVts_reg[2].data << 0);

    if (params->expo.fps >= 1000)
        params->expo.preview_fps = (cur_vts_30fps*max_fps*1000)/tVts;
    else
        params->expo.preview_fps = (cur_vts_30fps*max_fps)/tVts;

    return params->expo.preview_fps;
}

static int pCus_SetFPS_HDR_DOL_SEF(ss_cus_sensor *handle, u32 fps)
{
    u32 cur_vts_30fps = 0;
    imx462_params *params = (imx462_params *)handle->private_data;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 min_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16min_fps;
    cur_vts_30fps=vts_30fps_HDR_DOL_4lane;

    if(fps>=min_fps && fps <= max_fps){
        params->expo.vts= (cur_vts_30fps*(max_fps*1000) + fps * 500 )/ (fps * 1000);
    }else if(fps>=(min_fps*1000) && fps <= (max_fps*1000)){
        params->expo.vts= (cur_vts_30fps*(max_fps*1000) + (fps >> 1))/fps;
    }else{
        SENSOR_DMSG("[%s] FPS %d out of range.\n",__FUNCTION__,fps);
        return FAIL;
    }

    params->expo.fps = fps;
    params->dirty = true;

    pCus_SetAEUSecsHDR_DOL_SEF(handle, params->expo.expo_sef_us);
    return SUCCESS;
}

///////////////////////////////////////////////////////////////////////
// auto exposure
///////////////////////////////////////////////////////////////////////
// unit: micro seconds
//AE status notification
static int pCus_AEStatusNotify(ss_cus_sensor *handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status)
{
    imx462_params *params = (imx462_params *)handle->private_data;

    switch(status)
    {
        case CUS_FRAME_INACTIVE:
             break;
        case CUS_FRAME_ACTIVE:
            if(params->dirty || params->orien_dirty) {
                SensorReg_Write(0x3001,1); // Global hold on
                if(params->dirty) {
                    SensorRegArrayW((I2C_ARRAY*)params->tExpo_reg, ARRAY_SIZE(expo_SHS1_reg));
                    SensorRegArrayW((I2C_ARRAY*)params->tGain_reg, ARRAY_SIZE(gain_reg));
                    SensorRegArrayW((I2C_ARRAY*)params->tVts_reg, ARRAY_SIZE(vts_reg));
                    params->dirty = false;
                }
                if(params->orien_dirty) {
                    DoOrien(handle, params->res.orit);
                    params->orien_dirty = false;
                }
                SensorReg_Write(0x3001,0); // Global hold off */
            }
            break;
        default :
             break;
    }
    return SUCCESS;
}

static int pCus_AEStatusNotifyHDR_DOL_SEF(ss_cus_sensor *handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status)
{
    //imx462_params *params = (imx462_params *)handle->private_data;
    switch(status)
    {
        case CUS_FRAME_INACTIVE:
            break;
        case CUS_FRAME_ACTIVE:
            break;
        default :
            break;
    }
    return SUCCESS;
}

static int pCus_AEStatusNotifyHDR_DOL_LEF(ss_cus_sensor *handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status)
{
    imx462_params *params = (imx462_params *)handle->private_data;

    switch(status)
    {
        case CUS_FRAME_INACTIVE:

             break;
        case CUS_FRAME_ACTIVE:

            if(params->dirty || params->orien_dirty) {
                SensorReg_Write(0x3001,1);
                if(params->dirty) {
                    SensorRegArrayW((I2C_ARRAY*)params->tVts_reg, ARRAY_SIZE(vts_reg_hdr));
                    SensorRegArrayW((I2C_ARRAY*)params->tExpo_reg_LEF, ARRAY_SIZE(expo_SHS2_reg));
                    SensorRegArrayW((I2C_ARRAY*)params->tExpo_reg_SEF, ARRAY_SIZE(expo_SHS1_reg));
                    SensorRegArrayW((I2C_ARRAY*)params->tRHS1_reg, ARRAY_SIZE(expo_RHS1_reg));
                    SensorRegArrayW((I2C_ARRAY*)params->tGain_hdr_dol_lef_reg, ARRAY_SIZE(gain_HDR_DOL_LEF_reg));
                    SensorRegArrayW((I2C_ARRAY*)params->tGain_hdr_dol_sef_reg, ARRAY_SIZE(gain_HDR_DOL_SEF_reg));
                    params->dirty = false;
                }
                if(params->orien_dirty) {
                    DoOrien(handle, params->res.orit);
                    params->orien_dirty = false;
                }
                SensorReg_Write(0x3001,0);
            }
            break;
        default :
             break;
    }
    return SUCCESS;
}

static int pCus_GetAEUSecs(ss_cus_sensor *handle, u32 *us)
{
    u32 lines = 0;
    imx462_params *params = (imx462_params *)handle->private_data;

    lines |= (u32)(params->tExpo_reg[0].data & 0x03) << 16;
    lines |= (u32)(params->tExpo_reg[1].data & 0xff) << 8;
    lines |= (u32)(params->tExpo_reg[2].data & 0xff) << 0;

    *us = (lines * Preview_line_period) / 1000;

    SENSOR_DMSG("[%s] sensor expo lines/us %u,%u us\n", __FUNCTION__, lines, *us);

    return SUCCESS;
}

static int pCus_SetAEUSecs(ss_cus_sensor *handle, u32 us)
{
    u32 expo_lines = 0, vts = 0, SHS1 = 0;
    imx462_params *params = (imx462_params *)handle->private_data;

    params->expo.expo_lef_us = us;
    expo_lines = (1000*us)/Preview_line_period;

    if (expo_lines <= 1) expo_lines=1;
    if (expo_lines > (params->expo.vts -2)) {
        vts = expo_lines + 2;
    }
    else
        vts = params->expo.vts;
    SHS1 =  vts - expo_lines - 1;

    params->expo.expo_lines = expo_lines;

    SENSOR_DMSG("[%s] us %u, SHS1 %u, vts %u\n", __FUNCTION__,
                                                 us,  \
                                                 SHS1, \
                                                 vts
               );
    params->tExpo_reg[0].data = (SHS1>>16) & 0x0003;
    params->tExpo_reg[1].data = (SHS1>>8) & 0x00ff;
    params->tExpo_reg[2].data = (SHS1>>0) & 0x00ff;

    params->tVts_reg[0].data = (vts >> 16) & 0x0003;
    params->tVts_reg[1].data = (vts >> 8) & 0x00ff;
    params->tVts_reg[2].data = (vts >> 0) & 0x00ff;

    params->dirty = true;
    return SUCCESS;
}

static int pCus_SetAEUSecsHDR_DOL_SEF(ss_cus_sensor *handle, u32 us)
{
    u32 expo_lines_sef = 0;
    u32 cur_line_period = Preview_line_period_HDR_DOL_4LANE;
    imx462_params *params = (imx462_params *)handle->private_data;

    cur_line_period = Preview_line_period_HDR_DOL_4LANE;
    expo_lines_sef = (1000 * us + (cur_line_period >> 1)) / cur_line_period;
    params->expo.expo_sef_us = us;

    /* 1 <= lines_sef <= (RHS1-3); */
    if(expo_lines_sef <= 1) expo_lines_sef = 1;
    if(expo_lines_sef >= (params->max_rhs1 - 3))
        expo_lines_sef = params->max_rhs1 - 3;

    params->expo_lines_sef = expo_lines_sef;

    SENSOR_DMSG("[%s] us %u, expo_lines_sef %u rhs %u SHS1 %u\n", __FUNCTION__,
                                                                 us, \
                                                                 expo_lines_sef, \
                                                                 params->max_rhs1, \
                                                                 params->SEF_SHS1
               );

    pCus_SetAEUSecsHDR_DOL_LEF(handle, params->expo.expo_lef_us);
    params->dirty = true;
    return SUCCESS;
}

// Gain: 1x = 1024
static int pCus_GetAEGain(ss_cus_sensor *handle, u32* gain)
{
    imx462_params *params = (imx462_params *)handle->private_data;

    *gain = params->expo.final_gain;
    SENSOR_DMSG("[%s] get gain %u\n", __FUNCTION__, *gain);

    return SUCCESS;
}

static int pCus_SetAEGain(ss_cus_sensor *handle, u32 gain)
{
    imx462_params *params = (imx462_params *)handle->private_data;
    u64 gain_double;

    params->expo.final_gain = gain;
    if(gain < SENSOR_MIN_GAIN)
        gain = SENSOR_MIN_GAIN;
    else if(gain >= SENSOR_MAX_GAIN)
        gain = SENSOR_MAX_GAIN;

    gain_double = 20 * (intlog10(gain) - intlog10(1024));
    gain_double = (u16)((gain_double * 10) >> 24) / 3;

    params->tGain_reg[0].data = gain_double & 0xff;

    SENSOR_DMSG("[%s] set gain/reg=%u/0x%x 0x%x\n", __FUNCTION__, gain,params->tGain_reg[0].data, gain,params->tGain_reg[1].dat);
    params->dirty = true;
    return SUCCESS;
}

static void pCus_SetAEGainHDR_DOL_Calculate(u32 gain, u16 *gain_reg)
{
    u64 gain_double;

    if(gain < SENSOR_MIN_GAIN){
      gain = SENSOR_MIN_GAIN;
    }
    else if(gain >= SENSOR_MAX_GAIN){
      gain = SENSOR_MAX_GAIN;
    }
    gain_double = 20 * (intlog10(gain) - intlog10(1024));
    *gain_reg =(u16)((gain_double * 10) >> 24) / 3;
}

static int pCus_SetAEGainHDR_DOL_SEF1(ss_cus_sensor *handle, u32 gain)
{
    imx462_params *params = (imx462_params *)handle->private_data;
    u16 gain_reg = 0;

    pCus_SetAEGainHDR_DOL_Calculate(gain, &gain_reg);
    params->tGain_hdr_dol_sef_reg[0].data = gain_reg & 0xff;

    SENSOR_DMSG("[%s] set gain/reg=%u/0x%x 0x%x\n", __FUNCTION__, gain, params->tGain_hdr_dol_sef_reg[0].data, params->tGain_hdr_dol_sef_reg[1].data);

    params->dirty = true;
    return SUCCESS;
}

//lef functions
static int pCus_init_HDR_DOL_LEF(ss_cus_sensor *handle)
{
    return SUCCESS;
}

static int pCus_poweron_HDR_DOL_LEF(ss_cus_sensor *handle, u32 idx)
{
    return SUCCESS;
}

static int pCus_poweroff_HDR_DOL_LEF(ss_cus_sensor *handle, u32 idx)
{
    return SUCCESS;
}

static int pCus_GetOrien_HDR_DOL_LEF(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT *orit)
{
    return SUCCESS;
}

static int pCus_SetOrien_HDR_DOL_LEF(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT orit)
{
    imx462_params *params = (imx462_params *)handle->private_data;

    params->res.orit = orit;
    params->orien_dirty = true;

    return SUCCESS;
}

static int pCus_GetFPS_HDR_DOL_LEF(ss_cus_sensor *handle)
{
    imx462_params *params = (imx462_params *)handle->private_data;
    u32 cur_vts_30fps = vts_30fps_HDR_DOL_4lane;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 tVts = (params->tVts_reg[0].data << 16) | (params->tVts_reg[1].data << 8) | (params->tVts_reg[2].data << 0);

    if (params->expo.fps >= 1000)
        params->expo.preview_fps = (cur_vts_30fps*max_fps*1000)/tVts;
    else
        params->expo.preview_fps = (cur_vts_30fps*max_fps)/tVts;

    return params->expo.preview_fps;
}

static int pCus_SetFPS_HDR_DOL_LEF(ss_cus_sensor *handle, u32 fps)
{
    u32 cur_vts_30fps = 0;//vts = 0,
    imx462_params *params = (imx462_params *)handle->private_data;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 min_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16min_fps;

    cur_vts_30fps=vts_30fps_HDR_DOL_4lane;
    if(fps>=min_fps && fps <= max_fps){
        params->expo.vts=  (cur_vts_30fps*(max_fps*1000) + fps * 500 )/ (fps * 1000);
    }else if(fps>=(min_fps*1000) && fps <= (max_fps*1000)){
       params->expo.vts=  (cur_vts_30fps*(max_fps*1000) + (fps >> 1))/fps;
    }else {
        SENSOR_DMSG("[%s] FPS %d out of range.\n",__FUNCTION__,fps);
        return FAIL;
    }

    params->dirty = true;
    params->expo.fps = fps;
    pCus_SetAEUSecsHDR_DOL_LEF(handle, params->expo.expo_sef_us);

    return SUCCESS;
}

static int pCus_GetAEUSecs_HDR_DOL_LEF(ss_cus_sensor *handle, u32 *us)
{
    *us = 0;
    return SUCCESS;
}

static int pCus_SetAEUSecsHDR_DOL_LEF(ss_cus_sensor *handle, u32 us)
{
    u32 expo_lines_lef = 0, vts = 0, fsc = 0, RHS1 = 0;
    u32 cur_line_period = Preview_line_period_HDR_DOL_4LANE;
    imx462_params *params = (imx462_params *)handle->private_data;

    cur_line_period = Preview_line_period_HDR_DOL_4LANE;
    expo_lines_lef = (1000 * us + (cur_line_period >> 1)) / cur_line_period;

    params->expo.expo_lef_us = us;
    fsc = params->expo.vts * 2;

    /* 2 <= (lines_lef + lines_sef) <= (FSC - 6) */
    if(expo_lines_lef >= (fsc - params->expo_lines_sef - 8))
        expo_lines_lef = fsc - params->expo_lines_sef - 8;

    /* 1 <= lines_lef <= (FSC-RHS1-3) */
    if(expo_lines_lef <= 1) expo_lines_lef = 1;
    if(expo_lines_lef >= (fsc - params->max_rhs1 - 3))
        RHS1 = (((fsc - expo_lines_lef - 2)>>1)<<1) -1;
    else
        RHS1 = params->max_rhs1;
    params->y_outsize = (((RHS1 -1)>>1) + 1109)<<1;

    /* lines_lef = FSC - (SHS2 + 1); */
    params->LEF_SHS2 = (fsc - 1) - expo_lines_lef;
    /* lines_sef = RHS1-(SHS1+1); */
    params->SEF_SHS1 = params->max_rhs1 - params->expo_lines_sef - 1;

    vts = params->expo.vts;
    SENSOR_DMSG("[%s] us %u, expo_lines_lef %u, vts %u, SHR0 %u \n", __FUNCTION__,
                                                                     us, \
                                                                     expo_lines_lef, \
                                                                     vts, \
                                                                     params->LEF_SHS2
                );
    params->tExpo_reg_LEF[0].data = (params->LEF_SHS2 >> 16) & 0x0003;
    params->tExpo_reg_LEF[1].data = (params->LEF_SHS2 >> 8) & 0x00ff;
    params->tExpo_reg_LEF[2].data = (params->LEF_SHS2 >> 0) & 0x00ff;

    params->tExpo_reg_SEF[0].data = (params->SEF_SHS1 >> 16) & 0x0003;
    params->tExpo_reg_SEF[1].data = (params->SEF_SHS1 >> 8) & 0x00ff;
    params->tExpo_reg_SEF[2].data = (params->SEF_SHS1 >> 0) & 0x00ff;

    params->tRHS1_reg[0].data = (RHS1 >>16) & 0x03;
    params->tRHS1_reg[1].data = (RHS1 >>8) & 0xff;
    params->tRHS1_reg[2].data = (RHS1 >>0) & 0xff;
    params->tRHS1_reg[3].data = (params->y_outsize >>8) & 0xff;
    params->tRHS1_reg[4].data = (params->y_outsize >>0) & 0xff;

    params->tVts_reg[0].data = (vts >> 16) & 0x0003;
    params->tVts_reg[1].data = (vts >> 8) & 0x00ff;
    params->tVts_reg[2].data = (vts >> 0) & 0x00ff;

    params->dirty = true;
    return SUCCESS;
}

static int pCus_GetAEGain_HDR_DOL_LEF(ss_cus_sensor *handle, u32* gain)
{
    *gain = 0;
    return SUCCESS;
}

static int pCus_SetAEGainHDR_DOL_LEF(ss_cus_sensor *handle, u32 gain)
{
    imx462_params *params = (imx462_params *)handle->private_data;
    u16 gain_reg = 0;

    pCus_SetAEGainHDR_DOL_Calculate(gain, &gain_reg);
    params->tGain_hdr_dol_lef_reg[0].data = gain_reg & 0xff;

    SENSOR_DMSG("[%s] set gain/reg=%u/0x%x\n", __FUNCTION__, gain, params->tGain_hdr_dol_lef_reg[0].data, params->tGain_hdr_dol_lef_reg[1].data);

    params->dirty = true;
    return SUCCESS;
}

int cus_camsensor_init_handle_linear(ss_cus_sensor* drv_handle)
{
    ss_cus_sensor *handle = drv_handle;
    imx462_params *params;
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

    params = (imx462_params *)handle->private_data;
    memcpy(params->tVts_reg, vts_reg, sizeof(vts_reg));
    memcpy(params->tGain_reg, gain_reg, sizeof(gain_reg));
    memcpy(params->tExpo_reg, expo_SHS1_reg, sizeof(expo_SHS1_reg));

    ////////////////////////////////////////////////////
    // Sensor Stream Name info
    ////////////////////////////////////////////////////

    sprintf(handle->strSensorStreamName,"IMX462_MIPI");

    ////////////////////////////////////////////////////
    // Sensor interface info
    ////////////////////////////////////////////////////
    handle->sif_bus                                               = SENSOR_IFBUS_TYPE;
    handle->data_prec                                             = SENSOR_DATAPREC;
    handle->bayer_id                                              = SENSOR_BAYERID;
    handle->RGBIR_id                                              = SENSOR_RGBIRID;

    handle->interface_attr.attr_mipi.mipi_lane_num                = SENSOR_MIPI_LANE_NUM;
    handle->interface_attr.attr_mipi.mipi_data_format             = CUS_SEN_INPUT_FORMAT_RGB;
    handle->interface_attr.attr_mipi.mipi_yuv_order               = 0; //don't care in RGB pattern.
    handle->interface_attr.attr_mipi.mipi_hdr_mode                = CUS_HDR_MODE_NONE;
    handle->interface_attr.attr_mipi.mipi_hdr_virtual_channel_num = 0;

    ////////////////////////////////////////////////////
    // Sensor i2c configuration
    ////////////////////////////////////////////////////
    handle->i2c_cfg.mode                                          = SENSOR_I2C_LEGACY;
    handle->i2c_cfg.fmt                                           = SENSOR_I2C_FMT;
    handle->i2c_cfg.address                                       = SENSOR_I2C_ADDR;
    handle->i2c_cfg.speed                                         = SENSOR_I2C_SPEED;

    ////////////////////////////////////////////////////
    // Sensor Power configuration
    ////////////////////////////////////////////////////
    handle->pCus_sensor_poweron                                   = pCus_poweron;
    handle->pCus_sensor_poweroff                                  = pCus_poweroff;

    ////////////////////////////////////
    // Sensor mclk
    ////////////////////////////////////
    handle->mclk                                                  = Preview_MCLK_SPEED;

    ////////////////////////////////////////////////////
    // Sensor resolution capability
    ////////////////////////////////////////////////////
    handle->video_res_supported.ulcur_res                         = 0;
    for (res = 0; res < LINEAR_RES_END; res++) {
        handle->video_res_supported.num_res = res+1;
        handle->video_res_supported.res[res].u16width             = imx462_mipi_linear[res].senif.preview_w;
        handle->video_res_supported.res[res].u16height            = imx462_mipi_linear[res].senif.preview_h;
        handle->video_res_supported.res[res].u16max_fps           = imx462_mipi_linear[res].senout.max_fps;
        handle->video_res_supported.res[res].u16min_fps           = imx462_mipi_linear[res].senout.min_fps;
        handle->video_res_supported.res[res].u16crop_start_x      = imx462_mipi_linear[res].senif.crop_start_X;
        handle->video_res_supported.res[res].u16crop_start_y      = imx462_mipi_linear[res].senif.crop_start_y;
        handle->video_res_supported.res[res].u16OutputWidth       = imx462_mipi_linear[res].senout.width;
        handle->video_res_supported.res[res].u16OutputHeight      = imx462_mipi_linear[res].senout.height;
        sprintf(handle->video_res_supported.res[res].strResDesc, imx462_mipi_linear[res].senstr.strResDesc);
    }

    ////////////////////////////////////////////////////
    // Sensor resolution counter status
    ////////////////////////////////////////////////////
    handle->pCus_sensor_init                                      = pCus_init_2m_60fps_mipi4lane_linear;
    handle->pCus_sensor_GetVideoResNum                            = pCus_GetVideoResNum;
    handle->pCus_sensor_GetVideoRes                               = pCus_GetVideoRes;
    handle->pCus_sensor_GetCurVideoRes                            = pCus_GetCurVideoRes;
    handle->pCus_sensor_SetVideoRes                               = pCus_SetVideoRes;

    handle->pCus_sensor_GetOrien                                  = pCus_GetOrien;
    handle->pCus_sensor_SetOrien                                  = pCus_SetOrien;
    handle->pCus_sensor_GetFPS                                    = pCus_GetFPS;
    handle->pCus_sensor_SetFPS                                    = pCus_SetFPS;
    handle->pCus_sensor_SetPatternMode                            = imx462_SetPatternMode;

    ///////////////////////////////////////////////////////
    // Sensor AE
    ///////////////////////////////////////////////////////
    handle->pCus_sensor_AEStatusNotify                            = pCus_AEStatusNotify;
    handle->pCus_sensor_GetAEUSecs                                = pCus_GetAEUSecs;
    handle->pCus_sensor_SetAEUSecs                                = pCus_SetAEUSecs;
    handle->pCus_sensor_GetAEGain                                 = pCus_GetAEGain;
    handle->pCus_sensor_SetAEGain                                 = pCus_SetAEGain;
    handle->pCus_sensor_GetAEInfo                                 = pCus_sensor_GetAEInfo;
    handle->sensor_ae_info_cfg.u8AEGainDelay                      = SENSOR_GAIN_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEShutterDelay                   = SENSOR_SHUTTER_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEGainCtrlNum                    = 1;
    handle->sensor_ae_info_cfg.u8AEShutterCtrlNum                 = 1;
    handle->sensor_ae_info_cfg.u32AEGain_min                      = SENSOR_MIN_GAIN;
    handle->sensor_ae_info_cfg.u32AEGain_max                      = SENSOR_MAX_GAIN;
    handle->sensor_ae_info_cfg.u32AEShutter_min                   = Preview_line_period;
    handle->sensor_ae_info_cfg.u32AEShutter_max                   = 1000000000/imx462_mipi_linear[0].senout.min_fps;
    handle->sensor_ae_info_cfg.u32AEShutter_step                  = Preview_line_period;

    ///////////////////////////////////////////////////////
    // Private area
    ///////////////////////////////////////////////////////
    params->res.orit                                              = SENSOR_ORIT;
    params->expo.vts                                              = vts_30fps;
    params->dirty                                                 = false;
    params->pwdn_POLARITY                                         = SENSOR_PWDN_POL;  //CUS_CLK_POL_NEG;
    params->reset_POLARITY                                        = SENSOR_RST_POL;   //CUS_CLK_POL_NEG;

    return SUCCESS;
}

int cus_camsensor_init_handle_hdr_dol_sef(ss_cus_sensor* drv_handle)
{
    ss_cus_sensor *handle = drv_handle;
    imx462_params *params = NULL;
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

    params = (imx462_params *)handle->private_data;
    memcpy(params->tVts_reg, vts_reg_hdr, sizeof(vts_reg_hdr));
    memcpy(params->tRHS1_reg, expo_RHS1_reg, sizeof(expo_RHS1_reg));
    memcpy(params->tExpo_reg_SEF, expo_SHS1_reg, sizeof(expo_SHS1_reg));
    memcpy(params->tGain_hdr_dol_sef_reg, gain_HDR_DOL_SEF_reg, sizeof(gain_HDR_DOL_SEF_reg));

    ////////////////////////////////////////////////////
    // Sensor Stream Name info
    ////////////////////////////////////////////////////

    sprintf(handle->strSensorStreamName,"IMX462_MIPI_HDR_SEF");

    ////////////////////////////////////////////////////
    // Sensor interface info
    ////////////////////////////////////////////////////
    handle->sif_bus                                               = SENSOR_IFBUS_TYPE;
    handle->data_prec                                             = SENSOR_DATAPREC_DOL;
    handle->bayer_id                                              = SENSOR_BAYERID_HDR_DOL;
    handle->RGBIR_id                                              = SENSOR_RGBIRID;

    handle->interface_attr.attr_mipi.mipi_lane_num                = SENSOR_MIPI_LANE_NUM_DOL;
    handle->interface_attr.attr_mipi.mipi_data_format             = CUS_SEN_INPUT_FORMAT_RGB;
    handle->interface_attr.attr_mipi.mipi_hdr_mode                = CUS_HDR_MODE_SONY_DOL;
    handle->interface_attr.attr_mipi.mipi_hdr_virtual_channel_num = 1;

    ////////////////////////////////////////////////////
    // Sensor i2c configuration
    ////////////////////////////////////////////////////
    handle->i2c_cfg.mode                                          = SENSOR_I2C_LEGACY;
    handle->i2c_cfg.fmt                                           = SENSOR_I2C_FMT;
    handle->i2c_cfg.address                                       = SENSOR_I2C_ADDR;
    handle->i2c_cfg.speed                                         = SENSOR_I2C_SPEED;

    ////////////////////////////////////////////////////
    // Sensor Power configuration
    ////////////////////////////////////////////////////
    handle->pCus_sensor_poweron                                   = pCus_poweron;
    handle->pCus_sensor_poweroff                                  = pCus_poweroff;

    ////////////////////////////////////
    // Sensor mclk
    ////////////////////////////////////
    handle->mclk                                                  = Preview_MCLK_SPEED_HDR_DOL;

    ////////////////////////////////////////////////////
    // Sensor resolution capability
    ////////////////////////////////////////////////////
    handle->video_res_supported.ulcur_res                         = 0;
    for (res = 0; res < HDR_RES_END; res++) {
        handle->video_res_supported.num_res = res+1;
        handle->video_res_supported.res[res].u16width             = imx462_mipi_hdr[res].senif.preview_w;
        handle->video_res_supported.res[res].u16height            = imx462_mipi_hdr[res].senif.preview_h;
        handle->video_res_supported.res[res].u16max_fps           = imx462_mipi_hdr[res].senout.max_fps;
        handle->video_res_supported.res[res].u16min_fps           = imx462_mipi_hdr[res].senout.min_fps;
        handle->video_res_supported.res[res].u16crop_start_x      = imx462_mipi_hdr[res].senif.crop_start_X;
        handle->video_res_supported.res[res].u16crop_start_y      = imx462_mipi_hdr[res].senif.crop_start_y;
        handle->video_res_supported.res[res].u16OutputWidth       = imx462_mipi_hdr[res].senout.width;
        handle->video_res_supported.res[res].u16OutputHeight      = imx462_mipi_hdr[res].senout.height;
        sprintf(handle->video_res_supported.res[res].strResDesc, imx462_mipi_hdr[res].senstr.strResDesc);
    }

    ////////////////////////////////////////////////////
    // Sensor resolution counter status
    ////////////////////////////////////////////////////
    handle->pCus_sensor_init                                      = pCus_init_2m_30fps_mipi4lane_HDR_DOL;
    handle->pCus_sensor_GetVideoResNum                            = pCus_GetVideoResNum;
    handle->pCus_sensor_GetVideoRes                               = pCus_GetVideoRes;
    handle->pCus_sensor_GetCurVideoRes                            = pCus_GetCurVideoRes;
    handle->pCus_sensor_SetVideoRes                               = pCus_SetVideoRes_HDR_DOL_SEF;

    handle->pCus_sensor_GetOrien                                  = pCus_GetOrien;
    handle->pCus_sensor_SetOrien                                  = pCus_SetOrien;
    handle->pCus_sensor_GetFPS                                    = pCus_GetFPS_HDR_DOL_SEF;
    handle->pCus_sensor_SetFPS                                    = pCus_SetFPS_HDR_DOL_SEF;

    ///////////////////////////////////////////////////////
    // Sensor AE
    ///////////////////////////////////////////////////////
    handle->pCus_sensor_AEStatusNotify                            = pCus_AEStatusNotifyHDR_DOL_SEF;
    handle->pCus_sensor_GetAEUSecs                                = pCus_GetAEUSecs;
    handle->pCus_sensor_SetAEUSecs                                = pCus_SetAEUSecsHDR_DOL_SEF;
    handle->pCus_sensor_GetAEGain                                 = pCus_GetAEGain;
    handle->pCus_sensor_SetAEGain                                 = pCus_SetAEGainHDR_DOL_SEF1;
    handle->pCus_sensor_GetAEInfo                                 = pCus_sensor_GetAEInfo;
    handle->sensor_ae_info_cfg.u8AEGainDelay                      = SENSOR_GAIN_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEShutterDelay                   = SENSOR_SHUTTER_DELAY_FRAME_COUNT_HDR_DOL;
    handle->sensor_ae_info_cfg.u8AEGainCtrlNum                    = 1;
    handle->sensor_ae_info_cfg.u8AEShutterCtrlNum                 = 2;
    handle->sensor_ae_info_cfg.u32AEGain_min                      = SENSOR_MIN_GAIN;
    handle->sensor_ae_info_cfg.u32AEGain_max                      = SENSOR_MAX_GAIN;
    handle->sensor_ae_info_cfg.u32AEShutter_min                   = Preview_line_period_HDR_DOL_4LANE * 2;
    handle->sensor_ae_info_cfg.u32AEShutter_max                   = Preview_line_period_HDR_DOL_4LANE * params->max_rhs1;
    handle->sensor_ae_info_cfg.u32AEShutter_step                  = Preview_line_period_HDR_DOL_4LANE;

    ///////////////////////////////////////////////////////
    // Private area
    ///////////////////////////////////////////////////////
    params->expo.vts                                              = vts_30fps_HDR_DOL_4lane;
    params->pwdn_POLARITY                                         = SENSOR_PWDN_POL;  //CUS_CLK_POL_NEG;
    params->reset_POLARITY                                        = SENSOR_RST_POL;   //CUS_CLK_POL_NEG;

    return SUCCESS;
}

static int cus_camsensor_init_handle_hdr_dol_lef(ss_cus_sensor* drv_handle)
{
    ss_cus_sensor *handle = drv_handle;
    imx462_params *params;
    s32 res;
    if (!handle) {
        SENSOR_DMSG("[%s] not enough memory!\n", __FUNCTION__);
        return FAIL;
    }

    //private data allocation & init
    if (handle->private_data == NULL) {
        SENSOR_EMSG("[%s] Private data is empty!\n", __FUNCTION__);
        return FAIL;
    }
    params = (imx462_params *)handle->private_data;
    memcpy(params->tVts_reg, vts_reg_hdr, sizeof(vts_reg_hdr));
    memcpy(params->tExpo_reg_LEF, expo_SHS2_reg, sizeof(expo_SHS2_reg));
    memcpy(params->tGain_hdr_dol_lef_reg, gain_HDR_DOL_LEF_reg, sizeof(gain_HDR_DOL_LEF_reg));

    ////////////////////////////////////////////////////
    // Sensor Stream Name info
    ////////////////////////////////////////////////////

    sprintf(handle->strSensorStreamName,"IMX462_MIPI_HDR_LEF");

    ////////////////////////////////////////////////////
    // Sensor interface info
    ////////////////////////////////////////////////////
    handle->sif_bus                                               = SENSOR_IFBUS_TYPE;
    handle->data_prec                                             = SENSOR_DATAPREC_DOL;
    handle->bayer_id                                              = SENSOR_BAYERID_HDR_DOL;
    handle->RGBIR_id                                              = SENSOR_RGBIRID;

    handle->interface_attr.attr_mipi.mipi_lane_num                = SENSOR_MIPI_LANE_NUM_DOL;
    handle->interface_attr.attr_mipi.mipi_data_format             = CUS_SEN_INPUT_FORMAT_RGB;
    handle->interface_attr.attr_mipi.mipi_yuv_order               = 0; //don't care in RGB pattern.
    handle->interface_attr.attr_mipi.mipi_hdr_mode                = CUS_HDR_MODE_SONY_DOL;
    handle->interface_attr.attr_mipi.mipi_hdr_virtual_channel_num = 0;

   ////////////////////////////////////////////////////
    // Sensor i2c configuration
    ////////////////////////////////////////////////////
    handle->i2c_cfg.mode                                          = SENSOR_I2C_LEGACY;
    handle->i2c_cfg.fmt                                           = SENSOR_I2C_FMT;
    handle->i2c_cfg.address                                       = SENSOR_I2C_ADDR;
    handle->i2c_cfg.speed                                         = SENSOR_I2C_SPEED;

    ////////////////////////////////////////////////////
    // Sensor Power configuration
    ////////////////////////////////////////////////////
    handle->pCus_sensor_poweron                                   = pCus_poweron_HDR_DOL_LEF;
    handle->pCus_sensor_poweroff                                  = pCus_poweroff_HDR_DOL_LEF;

    ////////////////////////////////////
    // Sensor mclk
    ////////////////////////////////////
    handle->mclk                                                  = Preview_MCLK_SPEED_HDR_DOL;

    ////////////////////////////////////////////////////
    // Sensor resolution capability
    ////////////////////////////////////////////////////
    handle->video_res_supported.ulcur_res                         = 0;
    for (res = 0; res < HDR_RES_END; res++) {
        handle->video_res_supported.num_res = res+1;
        handle->video_res_supported.res[res].u16width             = imx462_mipi_hdr[res].senif.preview_w;
        handle->video_res_supported.res[res].u16height            = imx462_mipi_hdr[res].senif.preview_h;
        handle->video_res_supported.res[res].u16max_fps           = imx462_mipi_hdr[res].senout.max_fps;
        handle->video_res_supported.res[res].u16min_fps           = imx462_mipi_hdr[res].senout.min_fps;
        handle->video_res_supported.res[res].u16crop_start_x      = imx462_mipi_hdr[res].senif.crop_start_X;
        handle->video_res_supported.res[res].u16crop_start_y      = imx462_mipi_hdr[res].senif.crop_start_y;
        handle->video_res_supported.res[res].u16OutputWidth       = imx462_mipi_hdr[res].senout.width;
        handle->video_res_supported.res[res].u16OutputHeight      = imx462_mipi_hdr[res].senout.height;
        sprintf(handle->video_res_supported.res[res].strResDesc, imx462_mipi_hdr[res].senstr.strResDesc);
    }

    ////////////////////////////////////////////////////
    // Sensor resolution counter status
    ////////////////////////////////////////////////////
    handle->pCus_sensor_init                                      = pCus_init_HDR_DOL_LEF;
    handle->pCus_sensor_GetVideoResNum                            = NULL;
    handle->pCus_sensor_GetVideoRes                               = NULL;
    handle->pCus_sensor_GetCurVideoRes                            = NULL;
    handle->pCus_sensor_SetVideoRes                               = pCus_SetVideoRes_HDR_DOL_LEF;

    handle->pCus_sensor_GetOrien                                  = pCus_GetOrien_HDR_DOL_LEF;
    handle->pCus_sensor_SetOrien                                  = pCus_SetOrien_HDR_DOL_LEF;
    handle->pCus_sensor_GetFPS                                    = pCus_GetFPS_HDR_DOL_LEF;
    handle->pCus_sensor_SetFPS                                    = pCus_SetFPS_HDR_DOL_LEF;

    ///////////////////////////////////////////////////////
    // Sensor AE
    ///////////////////////////////////////////////////////
    handle->pCus_sensor_AEStatusNotify                            = pCus_AEStatusNotifyHDR_DOL_LEF;
    handle->pCus_sensor_GetAEUSecs                                = pCus_GetAEUSecs_HDR_DOL_LEF;
    handle->pCus_sensor_SetAEUSecs                                = pCus_SetAEUSecsHDR_DOL_LEF;
    handle->pCus_sensor_GetAEGain                                 = pCus_GetAEGain_HDR_DOL_LEF;
    handle->pCus_sensor_SetAEGain                                 = pCus_SetAEGainHDR_DOL_LEF;
    handle->pCus_sensor_GetAEInfo                                 = pCus_sensor_GetAEInfo;
    handle->sensor_ae_info_cfg.u8AEGainDelay                      = SENSOR_GAIN_DELAY_FRAME_COUNT;
    handle->sensor_ae_info_cfg.u8AEShutterDelay                   = SENSOR_SHUTTER_DELAY_FRAME_COUNT_HDR_DOL;
    handle->sensor_ae_info_cfg.u8AEGainCtrlNum                    = 1;
    handle->sensor_ae_info_cfg.u8AEShutterCtrlNum                 = 2;
    handle->sensor_ae_info_cfg.u32AEGain_min                      = SENSOR_MIN_GAIN;
    handle->sensor_ae_info_cfg.u32AEGain_max                      = SENSOR_MAX_GAIN;
    handle->sensor_ae_info_cfg.u32AEShutter_min                   = Preview_line_period_HDR_DOL_4LANE * 5;
    handle->sensor_ae_info_cfg.u32AEShutter_max                   = 1000000000/imx462_mipi_hdr[0].senout.min_fps;
    handle->sensor_ae_info_cfg.u32AEShutter_step                  = Preview_line_period_HDR_DOL_4LANE;

    ///////////////////////////////////////////////////////
    // Private area
    ///////////////////////////////////////////////////////
    params->res.orit                                              = SENSOR_ORIT;
    params->expo.vts                                              = vts_30fps_HDR_DOL_4lane;
    params->dirty                                                 = false;
    params->pwdn_POLARITY                                         = SENSOR_PWDN_POL;  //CUS_CLK_POL_NEG;
    params->reset_POLARITY                                        = SENSOR_RST_POL;   //CUS_CLK_POL_NEG;

    return SUCCESS;
}


SENSOR_DRV_ENTRY_IMPL_END_EX(IMX462_HDR,
                            cus_camsensor_init_handle_linear,
                            cus_camsensor_init_handle_hdr_dol_sef,
                            cus_camsensor_init_handle_hdr_dol_lef,
                            imx462_params
                         );
