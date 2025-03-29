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

SENSOR_DRV_ENTRY_IMPL_BEGIN_EX(OS02N10);

#ifndef ARRAY_SIZE
#define ARRAY_SIZE CAM_OS_ARRAY_SIZE
#endif

#define SENSOR_PAD_GROUP_SET CUS_SENSOR_PAD_GROUP_A
#define SENSOR_CHANNEL_NUM (0)
#define SENSOR_CHANNEL_MODE_LINEAR CUS_SENSOR_CHANNEL_MODE_REALTIME_NORMAL

//============================================
//MIPI config begin.
#define SENSOR_MIPI_LANE_NUM (2)
//#define SENSOR_MIPI_HDR_MODE (1) //0: Non-HDR mode. 1:Sony DOL mode
//MIPI config end.
//============================================

#define R_GAIN_REG 1
#define G_GAIN_REG 2
#define B_GAIN_REG 3


//#undef SENSOR_DBG
#define SENSOR_DBG 0

///////////////////////////////////////////////////////////////
//          @@@                                                                                       //
//       @   @@      ==  S t a r t * H e r e ==                                            //
//            @@      ==  S t a r t * H e r e  ==                                            //
//            @@      ==  S t a r t * H e r e  ==                                           //
//         @@@@                                                                                  //
//                                                                                                     //
//      Start Step 1 --  show preview on LCM                                         //
//                                                                                                    �@//
//  Fill these #define value and table with correct settings                        //
//      camera can work and show preview on LCM                                 //
//                                                                                                       //
///////////////////////////////////////////////////////////////

#define SENSOR_ISP_TYPE     ISP_EXT                   //ISP_EXT, ISP_SOC
#define SENSOR_IFBUS_TYPE   CUS_SENIF_BUS_MIPI      //CFG //CUS_SENIF_BUS_PARL, CUS_SENIF_BUS_MIPI
#define SENSOR_MIPI_HSYNC_MODE PACKET_HEADER_EDGE1
#define SENSOR_DATAPREC     CUS_DATAPRECISION_10    //CFG //CUS_DATAPRECISION_8, CUS_DATAPRECISION_10
#define SENSOR_DATAMODE     CUS_SEN_10TO12_9000     //CFG
#define SENSOR_BAYERID      CUS_BAYER_BG            //CFG //CUS_BAYER_GB, CUS_BAYER_GR, CUS_BAYER_BG, CUS_BAYER_RG
#define SENSOR_RGBIRID      CUS_RGBIR_NONE
#define SENSOR_ORIT         CUS_ORIT_M0F0           //CUS_ORIT_M0F0, CUS_ORIT_M1F0, CUS_ORIT_M0F1, CUS_ORIT_M1F1,

#define SENSOR_MAX_GAIN     496//15.5 * 32//(15.5*32)                  // max sensor again, a-gain * conversion-gain*d-gain
#define MAX_A_GAIN 15872

#define Preview_MCLK_SPEED  CUS_CMU_CLK_24MHZ        //CFG //CUS_CMU_CLK_12M, CUS_CMU_CLK_16M, CUS_CMU_CLK_24M, CUS_CMU_CLK_27M
#define Preview_line_period 30222//16801//17814                           // MCLK=21.6 HTS/PCLK=3080 pixels/97.2MHZ=31.687us                              // 3126 for 25fps
#define vts_30fps           1109                                     // VTS for 30fps

#define Preview_WIDTH       1920//2688                    //resolution Width when preview
#define Preview_HEIGHT      1080//1520                    //resolution Height when preview
#define Preview_MAX_FPS     30                     //fastest preview FPS
#define Preview_MIN_FPS     3                      //slowest preview FPS

#define SENSOR_I2C_ADDR    0x78                   //I2C slave address
#define SENSOR_I2C_SPEED   200000 //300000// 240000                  //I2C speed, 60000~320000

#define SENSOR_I2C_LEGACY  I2C_NORMAL_MODE     //usally set CUS_I2C_NORMAL_MODE,  if use old OVT I2C protocol=> set CUS_I2C_LEGACY_MODE
#define SENSOR_I2C_FMT     I2C_FMT_A8D8        //CUS_I2C_FMT_A8D8, CUS_I2C_FMT_A8D16, CUS_I2C_FMT_A16D8, CUS_I2C_FMT_A16D16

#define SENSOR_PWDN_POL     CUS_CLK_POL_NEG        // if PWDN pin High can makes sensor in power down, set CUS_CLK_POL_POS
#define SENSOR_RST_POL      CUS_CLK_POL_NEG        // if RESET pin High can makes sensor in reset state, set CUS_CLK_POL_NEG

// VSYNC/HSYNC POL can be found in data sheet timing diagram,
// Notice: the initial setting may contain VSYNC/HSYNC POL inverse settings so that condition is different.

//#define SENSOR_VSYNC_POL    CUS_CLK_POL_NEG        // if VSYNC pin High and data bus have data, set CUS_CLK_POL_POS
//#define SENSOR_HSYNC_POL    CUS_CLK_POL_NEG        // if HSYNC pin High and data bus have data, set CUS_CLK_POL_POS
//#define SENSOR_PCLK_POL     CUS_CLK_POL_POS        // depend on sensor setting, sometimes need to try CUS_CLK_POL_POS or CUS_CLK_POL_NEG

#if defined (SENSOR_MODULE_VERSION)
#define TO_STR_NATIVE(e) #e
#define TO_STR_PROXY(m, e) m(e)
#define MACRO_TO_STRING(e) TO_STR_PROXY(TO_STR_NATIVE, e)
static char *sensor_module_version = MACRO_TO_STRING(SENSOR_MODULE_VERSION);
module_param(sensor_module_version, charp, S_IRUGO);
#endif
static int cus_camsensor_release_handle(ss_cus_sensor *handle);
static int OS02N10_SetAEGain(ss_cus_sensor *handle, u32 gain);
static int OS02N10_SetAEUSecs(ss_cus_sensor *handle, u32 us);

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
        u32 sclk;
        u32 hts;
        u32 vts;
        u32 ho;
        u32 xinc;
        u32 line_freq;
        u32 us_per_line;
        u32 final_us;
        u32 final_gain;
        u32 back_pv_us;
        u32 fps;
        u32 preview_fps;
        u32 lines;
    } expo;
    int sen_init;
    int still_min_fps;
    int video_min_fps;
    u32 gain;
    bool reg_dirty;
    bool ori_dirty;
    I2C_ARRAY tVts_reg[2];
    I2C_ARRAY tGain_reg[3];
    I2C_ARRAY tExpo_reg[2];
    I2C_ARRAY tMirror_reg[3];
} OS02N10_params;

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
}OS02N10_mipi_linear[] = {
    {LINEAR_RES_1, {1920, 1080, 3, 30}, {0, 0, 1920, 1080}, {"1920x1080@30fps"}},
};

// set sensor ID address and data,
const static I2C_ARRAY Sensor_id_table[] =
{//P0
    {0xfd, 0x00},
    {0x02, 0x53},      // {address of ID, ID },
    {0x03, 0x02},      // {address of ID, ID },
    {0x04, 0x4e},      // {address of ID, ID },
    {0x05, 0x10},      // {address of ID, ID },
};

const static I2C_ARRAY Sensor_init_table[] =
{
	{0xfc, 0x01},//
	{0xfd, 0x00},//
	{0xba, 0x02},//
	{0xfd, 0x00},//
	{0xb1, 0x14},// ;dpll 480m
	{0xba, 0x00},//
	{0x1a, 0x00},// ;disable dvp
	{0x1b, 0x13},//
	{0xfd, 0x01},//
	{0x0e, 0x00},// ;exp_h
	{0x0f, 0x02},// ;exp_l
	{0x24, 0xff},// ;again
	{0x2f, 0x30},//
	{0xfe, 0x02},//
	{0x2b, 0xff},// ;col_cap
	{0x30, 0x00},//
	{0x31, 0x16},//;rowsg_toggle
	{0x32, 0x25},//
	{0x33, 0xfb},// ;[5]cms
	{0xfd, 0x01},// ;timing
	{0x50, 0x03},//
	{0x51, 0x07},//
	{0x52, 0x04},//
	{0x53, 0x05},//
	{0x57, 0x40},//
	{0x66, 0x04},//;0c
	{0x6d, 0x58},// ;sig_clamp start
	{0x77, 0x01},//
	{0x79, 0x32},//
	{0x7c, 0x01},//;04
	{0x90, 0x3b},//
	{0x91, 0x0b},//;08
	{0x92, 0x18},//
	{0x95, 0x40},//
	{0x99, 0x05},//;dac-samp
	{0xaa, 0x0e},//;0c
	{0xab, 0x0c},//
	{0xac, 0x10},//;0e
	{0xad, 0x10},//
	{0xae, 0x20},//;24
	{0xb0, 0x0e},//;0d
	{0xb1, 0x0f},//
	{0xb2, 0x1a},//;1a
	{0xb3, 0x1c},//
	{0xfd, 0x00},// ;fix reg bug
	{0xb0, 0x00},//
	{0xb1, 0x14},//
	{0xb2, 0x00},//
	{0xb3, 0x10},//
	{0xfd, 0x03},//
	{0x08, 0x00},//;01
	{0x09, 0x20},//;cc ob select col
	{0x0a, 0x02},//
	{0x0b, 0x80},// ;n2p
	{0x11, 0x41},// ;target
	{0x12, 0x41},//
	{0x13, 0x41},//
	{0x14, 0x41},//
	{0x17, 0x72},// ;68;60 ;blc exp coe
	{0x18, 0x6f},// ;68;60
	{0x19, 0x70},// ;68;60
	{0x1a, 0x6f},// ;68;60
	{0x1b, 0xc0},//;blc dc limit lsb
	{0x1d, 0x01},//;blc_dc_limit en
	{0x1f, 0x80},//
	{0x20, 0x40},//
	{0x21, 0x80},//
	{0x22, 0x40},//
	{0x23, 0x88},//
	{0x4b, 0x06},//
	{0x0e, 0x03},//
	{0x58, 0x7b},// ;79;59 ;blc en
	{0x59, 0x17},// ;08 ;auto
	{0x5a, 0x32},// ;blc dc limit msb

	{0xfd, 0x03},//
	{0x4c, 0x01},// ;03
	{0x4d, 0x01},//
	{0x4e, 0x01},// ;03
	{0x4f, 0x02},//

	{0xfd, 0x00},//
	{0x13, 0xbe},// ;icomp1 5.64u
	{0x14, 0x02},//;bitline current
	{0x4c, 0x24},// ;bit3 osc off
	{0xb6, 0x00},// ;reg psnc on
	{0xb7, 0x08},// ;ncp lp op en
	{0xb9, 0xd6},//;d5 ;[7]dac 8bit, [6]dac 2x current
	{0xc6, 0x95},//;a5;c5 ;vlow 1.3
	{0xc7, 0x77},//;aa;bsun sig 2x 1x
	{0xc9, 0x22},//;66 ;bsun rst 2x, 1x 1.92V
	{0xca, 0x32},//;22;66 ;bsun rst 2x, 1x 1.92V
	{0xd7, 0xaa},//;6a
	{0xbc, 0x1f},// ;bc~d1 psnc
	{0xbd, 0x60},//
	{0xbe, 0x78},//
	{0xbf, 0xa5},//
	{0xcb, 0x00},//
	{0xcc, 0x00},//
	{0xce, 0x20},//
	{0xcf, 0x3f},//
	{0xd0, 0x76},//
	{0xd1, 0xec},//

	{0xfd, 0x04},//
	{0x1b, 0x01},// ;00 ;[0]=0 dpc off, =1 dpc on

	{0xfd, 0x03},//
	{0x01, 0x04},//
	{0x02, 0x07},//
	{0x03, 0x80},//
	{0x05, 0x04},//
	{0x06, 0x04},//
	{0x07, 0x38},//
	{0xfd, 0x00},//
	{0x1e, 0x0f},//
	{0x1d, 0xa1},// ;[5]mipi pwdn sel
	{0x21, 0x04},//
	{0x24, 0x02},//
	{0x27, 0x07},// ;mipi output size
	{0x28, 0x80},// //88
	{0x29, 0x04},//
	{0x2a, 0x38},// //40
	{0x2d, 0x04},//
	{0x2e, 0x03},//
	{0x2f, 0x0c},//
	{0x31, 0x04},//
	{0x32, 0x1a},//
	{0x33, 0x04},//
	{0x34, 0x05},//;02
	{0x3f, 0x40},//
	{0x40, 0x94},//
	{0x23, 0x01},// ;mipi en

	{0xfd, 0x03},//
	{0x26, 0x00},// ;01

	{0x28, 0x0a},// ;0c ;L
	{0x29, 0x0a},// ;0d ;0c ;R
	{0x2a, 0x52},// ;37 ;U
	{0x2b, 0x5a},// ;52 ;37 ;D

	{0x2c, 0x0a},// ;0c
	{0x2d, 0x0a},//; 0d ;0c
	{0x2e, 0x52},// ;37
	{0x2f, 0x5a},// ;52 ;37

	{0x31, 0x0a},//; 0c
	{0x32, 0x0a},// ;0d ;0c
	{0x33, 0x52},// ;37
	{0x34, 0x5a},// ;52 ;37

	{0x35, 0x0c},// ;0a ;LU
	{0x36, 0x10},// ;17 ;15 ;LD
	{0x37, 0x07},// ;05 ;RU
	{0x38, 0x0a},// ;10 ;RD

	{0x39, 0x0c},// ;0a
	{0x3a, 0x10},// ;17 ;15
	{0x3b, 0x07},// ;05
	{0x3c, 0x0a},// ;10

	{0x3d, 0x0c},// ;0a
	{0x3e, 0x10},// ;17 ;15
	{0x3f, 0x07},// ;05
	{0x40, 0x0a},// ;10

	{0x41, 0x07},//
	{0x42, 0x07},//
	{0x43, 0x07},//
	{0x44, 0x07},// ;00

	{0x46, 0x10},//
	{0x47, 0xe2},//
	{0x45, 0x50},//

	{0xfb, 0x03},// ;fast exp mode
};

const static I2C_ARRAY mirror_reg[] = {
    {0xfd, 0x01}, //0
    {0x12, 0x00}, //Mirror/Flip
    {0xfe, 0x02}, //2
};

/////////////////////////////////////////////////////////////////
//       @@@@@@                                                                                    //
//                 @@                                                                                    //
//             @@@                                                                                      //
//       @       @@                                                                                    //
//         @@@@                                                                                        //
//                                                                                                          //
//      Step 3 --  complete camera features                                              //
//                                                                                                         //
//                                                                                                         //
//  camera set EV, MWB, orientation, contrast, sharpness                          //
//   , saturation, and Denoise can work correctly.                                     //
//                                                                                                          //
/////////////////////////////////////////////////////////////////

typedef struct {
    short reg;
    char startbit;
    char stopbit;
} COLLECT_REG_SET;

const static I2C_ARRAY gain_reg[] = {
    //{0xfd, 0x01},
    {0x24, 0xf8},//again 1x:0x10, 15.5x:0xf8
	{0x1f, 0x00},
	{0x20, 0x40},//dgain 1x:0x40, 32x:0x07,0xff
	//{0xfe, 0x02},
};

const static I2C_ARRAY expo_reg[] = {
    //{0xfd, 0x01},//
    {0x0e, 0x01},
    {0x0f, 0x54},
	//{0xfe, 0x02},
};

const static I2C_ARRAY vts_reg[] = {
    //{0xfd, 0x01},//
    {0x14, 0x04},//MSB
    {0x15, 0x55},//LSB
	//{0xfe, 0x02},

};

#if 0
static CUS_INT_TASK_ORDER def_order = {
	.RunLength = 9,
	.Orders = {
			CUS_INT_TASK_AE|CUS_INT_TASK_VDOS|CUS_INT_TASK_AF,
			CUS_INT_TASK_AWB|CUS_INT_TASK_VDOS|CUS_INT_TASK_AF,
			CUS_INT_TASK_VDOS|CUS_INT_TASK_AF,
			CUS_INT_TASK_AE|CUS_INT_TASK_VDOS|CUS_INT_TASK_AF,
			CUS_INT_TASK_AWB|CUS_INT_TASK_VDOS|CUS_INT_TASK_AF,
			CUS_INT_TASK_VDOS|CUS_INT_TASK_AF,
			CUS_INT_TASK_AE|CUS_INT_TASK_VDOS|CUS_INT_TASK_AF,
			CUS_INT_TASK_AWB|CUS_INT_TASK_VDOS|CUS_INT_TASK_AF,
			CUS_INT_TASK_VDOS|CUS_INT_TASK_AF,
	},
};
#endif

/////////// function definition ///////////////////
#if SENSOR_DBG == 1
//#define SENSOR_DMSG(args...) SENSOR_DMSG(args)
//#define SENSOR_DMSG(args...) LOGE(args)
#define SENSOR_DMSG(args...) SENSOR_DMSG(args)
#elif SENSOR_DBG == 0
//#define SENSOR_DMSG(args...)
#endif
#undef SENSOR_NAME
#define SENSOR_NAME OS02N10


#define SensorReg_Read(_reg,_data)     (handle->i2c_bus->i2c_rx(handle->i2c_bus,&(handle->i2c_cfg),_reg,_data))
#define SensorReg_Write(_reg,_data)    (handle->i2c_bus->i2c_tx(handle->i2c_bus,&(handle->i2c_cfg),_reg,_data))
#define SensorRegArrayW(_reg,_len)  (handle->i2c_bus->i2c_array_tx(handle->i2c_bus, &(handle->i2c_cfg),(_reg),(_len)))
#define SensorRegArrayR(_reg,_len)  (handle->i2c_bus->i2c_array_rx(handle->i2c_bus, &(handle->i2c_cfg),(_reg),(_len)))

/////////////////// sensor hardware dependent //////////////
static int OS02N10_poweron(ss_cus_sensor *handle, u32 idx)
{
    ISensorIfAPI *sensor_if = handle->sensor_if_api;
    SENSOR_DMSG("[%s] ", __FUNCTION__);

    //Sensor power on sequence
    sensor_if->MCLK(idx, 1, handle->mclk);

    sensor_if->SetIOPad(idx, handle->sif_bus, handle->interface_attr.attr_mipi.mipi_lane_num);
    sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_216M);
    sensor_if->SetCSI_Lane(idx, handle->interface_attr.attr_mipi.mipi_lane_num, 1);
    sensor_if->SetCSI_LongPacketType(idx, 0, 0x3C00, 0);

    SENSOR_DMSG("[%s] reset low\n", __FUNCTION__);
    sensor_if->Reset(idx, CUS_CLK_POL_NEG);
    SENSOR_USLEEP(1000);
    SENSOR_DMSG("[%s] power low\n", __FUNCTION__);
    sensor_if->PowerOff(idx, CUS_CLK_POL_POS);
    SENSOR_USLEEP(5000);

    // power -> high, reset -> high
    SENSOR_DMSG("[%s] power high\n", __FUNCTION__);
    sensor_if->PowerOff(idx, CUS_CLK_POL_NEG);
    SENSOR_USLEEP(5000);
    SENSOR_DMSG("[%s] reset high\n", __FUNCTION__);
    sensor_if->Reset(idx, CUS_CLK_POL_POS);
    SENSOR_USLEEP(5000);

    return SUCCESS;
}

static int OS02N10_poweroff(ss_cus_sensor *handle, u32 idx)
{
    // power/reset low
    ISensorIfAPI *sensor_if = handle->sensor_if_api;

    sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_DISABLE);
    sensor_if->MCLK(idx, 0, handle->mclk);
    SENSOR_DMSG("[%s] power low\n", __FUNCTION__);
    sensor_if->PowerOff(idx, CUS_CLK_POL_POS);
    sensor_if->Reset(idx, CUS_CLK_POL_NEG);
    SENSOR_USLEEP(5000);
    return SUCCESS;
}

/////////////////// image function /////////////////////////
//Get and check sensor ID
//if i2c error or sensor id does not match then return FAIL
static int OS02N10_GetSensorID(ss_cus_sensor *handle, u32 *id)
{
    int i,n;
    //u16 sen_data1,sen_data2;
    int table_length= ARRAY_SIZE(Sensor_id_table);
    I2C_ARRAY id_from_sensor[ARRAY_SIZE(Sensor_id_table)];

    for(n=0;n<table_length;++n)
    {
      id_from_sensor[n].reg = Sensor_id_table[n].reg;
      id_from_sensor[n].data = 0;
    }
    *id =0;
    if(table_length>8) table_length=8;

    SENSOR_DMSG("\n\n[%s]", __FUNCTION__);

    for(n=0;n<4;++n) //retry , until I2C success
    {
      if(n>2) return FAIL;

      if( SensorRegArrayR((I2C_ARRAY*)id_from_sensor,table_length) == SUCCESS) //read sensor ID from I2C
        break;
      else
        continue;
    }

    //convert sensor id to u32 format
    for(i=0;i<table_length;++i)
    {
      if( id_from_sensor[i].data != Sensor_id_table[i].data )
        return FAIL;
      *id = ((*id)+ id_from_sensor[i].data)<<8;

    *id >>= 8;
    SENSOR_DMSG("[%s]OS02N10 Read sensor id, get 0x%x Success\n", __FUNCTION__, (int)*id);
   // printf("OS02N10 Read sensor id, get 0x%x Success\n", (int)*id);

    }
    return SUCCESS;
}

static int OS02N10_SetPatternMode(ss_cus_sensor *handle,u32 mode)
{
    SENSOR_DMSG("\n\n[%s], mode=%d \n", __FUNCTION__, mode);
    return SUCCESS;
}

static int OS02N10_init(ss_cus_sensor *handle){
    //SENSOR_DMSG("\n\n[%s]", __FUNCTION__);
    int i,cnt;

    for(i=0;i< ARRAY_SIZE(Sensor_init_table);i++)
    {

        if(Sensor_init_table[i].reg==0xff)
        {
            SENSOR_MSLEEP(Sensor_init_table[i].data);
        }
        else
        {
            cnt = 0;
            while(SensorReg_Write(Sensor_init_table[i].reg, Sensor_init_table[i].data) != SUCCESS)
            {
                cnt++;
                SENSOR_DMSG("Sensor_init_table -> Retry %d...\n",cnt);
                if(cnt>=10)
                {
                    SENSOR_DMSG("[%s:%d]Sensor init fail!!\n", __FUNCTION__, __LINE__);
                    return FAIL;
                }
                SENSOR_MSLEEP(10);
            }
        }
    }
    return SUCCESS;
}

static int OS02N10_GetVideoResNum( ss_cus_sensor *handle, u32 *ulres_num)
{
    *ulres_num = handle->video_res_supported.num_res;
    return SUCCESS;
}

static int OS02N10_GetVideoRes(ss_cus_sensor *handle, u32 res_idx, cus_camsensor_res **res)
{
    u32 num_res = handle->video_res_supported.num_res;

    if (res_idx >= num_res) {
        return FAIL;
    }
    *res = &handle->video_res_supported.res[res_idx];

    return SUCCESS;
}

static int OS02N10_GetCurVideoRes(ss_cus_sensor *handle, u32 *cur_idx, cus_camsensor_res **res)
{
    u32 num_res = handle->video_res_supported.num_res;

    *cur_idx = handle->video_res_supported.ulcur_res;

    if (*cur_idx >= num_res) {
        return FAIL;
    }

    *res = &handle->video_res_supported.res[*cur_idx];

    return SUCCESS;
}

static int OS02N10_SetVideoRes(ss_cus_sensor *handle, u32 res_idx)
{
    u32 num_res = handle->video_res_supported.num_res;
    if (res_idx >= num_res) {
        return FAIL;
    }
    switch (res_idx) {
        case 0:
            handle->video_res_supported.ulcur_res = 0;
            handle->pCus_sensor_init = OS02N10_init;
            break;
        default:
            break;
    }

    return SUCCESS;
}

static int OS02N10_GetOrien(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT *orit) {
    char sen_data;
    OS02N10_params *params = (OS02N10_params *)handle->private_data;

    sen_data = params->tMirror_reg[1].data & 0x03;
    SENSOR_DMSG("\n\n[%s]:mirror:%x\r\n\n\n\n",__FUNCTION__, sen_data);
    switch(sen_data) {
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

static int OS02N10_SetOrien(ss_cus_sensor *handle, CUS_CAMSENSOR_ORIT orit)
{
    OS02N10_params *params = (OS02N10_params *)handle->private_data;
    switch(orit) {
        case CUS_ORIT_M0F0:
            params->tMirror_reg[1].data = 0x00;
//            params->tMirror_reg[4].data = 0x22;
            params->ori_dirty = true;
        break;
        case CUS_ORIT_M1F0:
            params->tMirror_reg[1].data = 0x02;
 //           params->tMirror_reg[4].data = 0x32;
            params->ori_dirty = true;
        break;
        case CUS_ORIT_M0F1:
            params->tMirror_reg[1].data = 0x01;
//           params->tMirror_reg[4].data = 0x32;
            params->ori_dirty = true;
        break;
        case CUS_ORIT_M1F1:
            params->tMirror_reg[1].data = 0x03;
//            params->tMirror_reg[4].data = 0x32;
            params->ori_dirty = true;
        break;
    }
     return SUCCESS;
}

static int OS02N10_GetFPS(ss_cus_sensor *handle)
{
    OS02N10_params *params = (OS02N10_params *)handle->private_data;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 tVts = 0;
	u32 tVb = (params->tVts_reg[0].data << 8) | (params->tVts_reg[1].data << 0);
	tVts = tVb + vts_30fps;

    if (params->expo.fps >= 1000)
        params->expo.preview_fps = (vts_30fps*max_fps*1000)/tVts;
    else
        params->expo.preview_fps = (vts_30fps*max_fps)/tVts;

    return params->expo.preview_fps;
}

static int OS02N10_SetFPS(ss_cus_sensor *handle, u32 fps)
{
    u32 vts = 0, vb = 0;
    OS02N10_params *params = (OS02N10_params *)handle->private_data;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16max_fps;
    u32 min_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16min_fps;

    if(fps>=min_fps && fps <= max_fps){
        params->expo.fps = fps;
        params->expo.vts=  (vts_30fps*max_fps)/fps;
    }else if((fps >= (min_fps*1000)) && (fps <= (max_fps*1000))){
        params->expo.fps = fps;
        params->expo.vts=  (vts_30fps*(max_fps*1000))/fps;
    }else{
        SENSOR_DMSG("[%s] FPS %d out of range.\n",__FUNCTION__,fps);
        return FAIL;
    }

    if ((params->expo.lines) > (params->expo.vts - 8)){
        vts = params->expo.lines + 8;
    }else{
        vts = params->expo.vts;
	}
	vb = vts - vts_30fps;
	if(vb < 0){
		vb = 0;
	}
    params->tVts_reg[0].data = (vb >> 8) & 0x00ff;
    params->tVts_reg[1].data = (vb >> 0) & 0x00ff;
    params->reg_dirty = true;
    return SUCCESS;
}


///////////////////////////////////////////////////////////////////////
// auto exposure
///////////////////////////////////////////////////////////////////////
// unit: micro seconds
//AE status notification
static int OS02N10_AEStatusNotify(ss_cus_sensor *handle, u32 idx, CUS_CAMSENSOR_AE_STATUS_NOTIFY status)
{
    OS02N10_params *params = (OS02N10_params *)handle->private_data;
    switch(status)
    {
        case CUS_FRAME_INACTIVE:

        break;
        case CUS_FRAME_ACTIVE:
            if(params->ori_dirty){
                SensorRegArrayW((I2C_ARRAY*)params->tMirror_reg, ARRAY_SIZE(mirror_reg));
                params->ori_dirty = false;
            }
            if(params->reg_dirty)
            {
                SensorReg_Write(0xfd,0x01);//page 1
                //SensorReg_Write(0x01,0x00);//frame sync disable
                SensorRegArrayW((I2C_ARRAY*)params->tExpo_reg, ARRAY_SIZE(expo_reg));
                SensorRegArrayW((I2C_ARRAY*)params->tGain_reg, ARRAY_SIZE(gain_reg));
                SensorRegArrayW((I2C_ARRAY*)params->tVts_reg, ARRAY_SIZE(vts_reg));
                //SensorReg_Write(0x01,0x01);//frame sync enable
				SensorReg_Write(0xfe,0x02);
                params->reg_dirty = false;
            }
            break;
            default :
            break;
    }
    return SUCCESS;
}

static int OS02N10_GetAEUSecs(ss_cus_sensor *handle, u32 *us)
{
    OS02N10_params *params = (OS02N10_params *)handle->private_data;
    int rc = SUCCESS;
    u32 lines = 0;

    lines  =  (u32)(params->tExpo_reg[1].data&0xff);
    lines |= (u32)(params->tExpo_reg[0].data&0xff)<<8;
    *us = (lines*Preview_line_period)/1000;
    return rc;
}

static int OS02N10_SetAEUSecs(ss_cus_sensor *handle, u32 us)
{
    u32 lines = 0, vts = 0, vb = 0;
    OS02N10_params *params = (OS02N10_params *)handle->private_data;

    lines=(u32)((1000*us+(Preview_line_period>>1))/Preview_line_period);
    if (lines < 2) lines = 2;
    if (lines >params->expo.vts - 9){
        vts = lines + 9;
    }else{
        vts=params->expo.vts;
	}

	vb = vts - vts_30fps;
    SENSOR_DMSG("[%s] us %ld, lines %ld, vts %ld\n", __FUNCTION__,
                us,
                lines,
                params->expo.vts
                );
   // lines <<= 4;
    params->tExpo_reg[0].data = (lines>>8) & 0x00ff;
    params->tExpo_reg[1].data = (lines>>0) & 0x00ff;

    params->tVts_reg[0].data = (vb >> 8) & 0x00ff;
    params->tVts_reg[1].data = (vb >> 0) & 0x00ff;

    params->reg_dirty = true;
    return SUCCESS;
}

// Gain: 1x = 1024
static int OS02N10_GetAEGain(ss_cus_sensor *handle, u32* gain) {
    OS02N10_params *params = (OS02N10_params *)handle->private_data;

    *gain = params->expo.final_gain;
    return SUCCESS;
}

static int OS02N10_SetAEGain(ss_cus_sensor *handle, u32 gain)
{
    OS02N10_params *params = (OS02N10_params *)handle->private_data;
    u32 dgain = 0, again;
    gain = (gain * handle->sensor_ae_info_cfg.u32AEGain_min  + 512)>>10; // need to add min sat gain

    if(gain<1024)
        gain=1024;
    else if(gain>=SENSOR_MAX_GAIN*1024)
        gain=SENSOR_MAX_GAIN*1024;

    params->expo.final_gain = gain;
    /* A Gain */
    if (gain <= 1024) {
        again=1024;
    } else if (gain < 2048) {
        again = (gain>>6)<<6;
    } else if (gain < 4096) {
        again = (gain>>7)<<7;
    } else if (gain < 8192) {
        again = (gain>>8)<<8;
    } else if (gain < MAX_A_GAIN) {
        again = (gain>>9)<<9;
    } else {
        again = MAX_A_GAIN;
    }
    dgain = gain*64/again;

    again = again>>6;
    params->tGain_reg[0].data = again&0xff;//again
    params->tGain_reg[1].data = (dgain>>8)&0x07; //high byte
    params->tGain_reg[2].data = dgain&0xff; //low byte
    return SUCCESS;
}

static int pCus_sensor_GetAEInfo(ss_cus_sensor *handle, CUS_SENSOR_AE_INFO_t *info)
{
    //OS02N10_params *params;
    //params = (OS02N10_params *)handle->private_data;

    info->u8AEGainDelay                      = handle->sensor_ae_info_cfg.u8AEGainDelay;
    info->u8AEShutterDelay                   = handle->sensor_ae_info_cfg.u8AEShutterDelay;
    info->u8AEGainCtrlNum                    = handle->sensor_ae_info_cfg.u8AEGainCtrlNum;
    info->u8AEShutterCtrlNum                 = handle->sensor_ae_info_cfg.u8AEShutterCtrlNum;
    info->u32AEGain_min                      = handle->sensor_ae_info_cfg.u32AEGain_min;
    info->u32AEGain_max                      = handle->sensor_ae_info_cfg.u32AEGain_max;
    info->u32AEShutter_min                   = handle->sensor_ae_info_cfg.u32AEShutter_min;
    info->u32AEShutter_max                   = handle->sensor_ae_info_cfg.u32AEShutter_max;
    info->u32AEShutter_step                  = handle->sensor_ae_info_cfg.u32AEShutter_step;

	return 0;
}

int cus_os02n10_camsensor_init_handle(ss_cus_sensor* drv_handle) {
    ss_cus_sensor *handle = drv_handle;
    OS02N10_params *params;
    int res;
    if (!handle) {
        SENSOR_DMSG("[%s] not enough memory!\n", __FUNCTION__);
        return FAIL;
    }
    SENSOR_DMSG("[%s]", __FUNCTION__);
    //private data allocation & init
    handle->private_data = CamOsMemCalloc(1, sizeof(OS02N10_params));
    params = (OS02N10_params *)handle->private_data;
    memcpy(params->tVts_reg, vts_reg, sizeof(vts_reg));
    memcpy(params->tGain_reg, gain_reg, sizeof(gain_reg));
    memcpy(params->tExpo_reg, expo_reg, sizeof(expo_reg));
    memcpy(params->tMirror_reg, mirror_reg, sizeof(mirror_reg));

    ////////////////////////////////////
    //    sensor model ID                           //
    ////////////////////////////////////
    //sprintf(handle->strSensorStreamName,"OS02N10_MIPI");
    CamOsSnprintf(handle-> strSensorStreamName, sizeof(handle-> strSensorStreamName), "OS02N10_MIPI");
    ////////////////////////////////////
    //    sensor interface info       //
    ////////////////////////////////////
    //SENSOR_DMSG("[%s] entering function with id %d\n", __FUNCTION__, id);
    //handle->isp_type    = SENSOR_ISP_TYPE;  //ISP_SOC;
    //handle->data_fmt    = SENSOR_DATAFMT;   //CUS_DATAFMT_YUV;
    handle->sif_bus     = SENSOR_IFBUS_TYPE;//CUS_SENIF_BUS_PARL;
    handle->data_prec   = SENSOR_DATAPREC;  //CUS_DATAPRECISION_8;
    //handle->data_mode   = SENSOR_DATAMODE;
    handle->bayer_id    = SENSOR_BAYERID;   //CUS_BAYER_GB;
    handle->RGBIR_id    = SENSOR_RGBIRID;
    //handle->orient      = SENSOR_ORIT;      //CUS_ORIT_M1F1;
    //handle->YC_ODER     = SENSOR_YCORDER;   //CUS_SEN_YCODR_CY;
    handle->interface_attr.attr_mipi.mipi_lane_num = SENSOR_MIPI_LANE_NUM;
    handle->interface_attr.attr_mipi.mipi_data_format = CUS_SEN_INPUT_FORMAT_RGB; // RGB pattern.
    handle->interface_attr.attr_mipi.mipi_yuv_order = 0; //don't care in RGB pattern.
    //handle->interface_attr.attr_mipi.mipi_hsync_mode = SENSOR_MIPI_HSYNC_MODE;
    handle->interface_attr.attr_mipi.mipi_hdr_mode = CUS_HDR_MODE_NONE;
    handle->interface_attr.attr_mipi.mipi_hdr_virtual_channel_num = 0; //Short frame

    ////////////////////////////////////
    //    resolution capability       //
    ////////////////////////////////////
    handle->video_res_supported.ulcur_res = 0;
    for (res = 0; res < LINEAR_RES_END; res++) {
        handle->video_res_supported.num_res = res+1;
        handle->video_res_supported.res[res].u16width         = OS02N10_mipi_linear[res].senif.preview_w;
        handle->video_res_supported.res[res].u16height        = OS02N10_mipi_linear[res].senif.preview_h;
        handle->video_res_supported.res[res].u16max_fps       = OS02N10_mipi_linear[res].senout.max_fps;
        handle->video_res_supported.res[res].u16min_fps       = OS02N10_mipi_linear[res].senout.min_fps;
        handle->video_res_supported.res[res].u16crop_start_x  = OS02N10_mipi_linear[res].senif.crop_start_X;
        handle->video_res_supported.res[res].u16crop_start_y  = OS02N10_mipi_linear[res].senif.crop_start_y;
        handle->video_res_supported.res[res].u16OutputWidth  = OS02N10_mipi_linear[res].senout.width;
        handle->video_res_supported.res[res].u16OutputHeight = OS02N10_mipi_linear[res].senout.height;
        strcpy(handle->video_res_supported.res[res].strResDesc, OS02N10_mipi_linear[res].senstr.strResDesc);
		//CamOsStrncpy(handle->video_res_supported.res[i].strResDesc, "1920x1080@30fps", CamOsStrlen(1920x1080@30fps));
    }

    // i2c
    handle->i2c_cfg.mode                = SENSOR_I2C_LEGACY;    //(CUS_ISP_I2C_MODE) FALSE;
    handle->i2c_cfg.fmt                 = SENSOR_I2C_FMT;       //CUS_I2C_FMT_A16D16;
    handle->i2c_cfg.address             = SENSOR_I2C_ADDR;      //0x5a;
    handle->i2c_cfg.speed               = SENSOR_I2C_SPEED;     //320000;

    // mclk
    handle->mclk                        = Preview_MCLK_SPEED;

    //polarity
    /////////////////////////////////////////////////////
    //handle->pwdn_POLARITY               = SENSOR_PWDN_POL;  //CUS_CLK_POL_NEG;
    //handle->reset_POLARITY              = SENSOR_RST_POL;   //CUS_CLK_POL_NEG;
    //handle->VSYNC_POLARITY              = SENSOR_VSYNC_POL; //CUS_CLK_POL_POS;
    //handle->HSYNC_POLARITY              = SENSOR_HSYNC_POL; //CUS_CLK_POL_POS;
    //handle->PCLK_POLARITY               = SENSOR_PCLK_POL;  //CUS_CLK_POL_POS);    // use '!' to clear board latch error
    /////////////////////////////////////////////////////

    ////////////////////////////////////////////////////
    // AE parameters
    ////////////////////////////////////////////////////
    handle->sensor_ae_info_cfg.u8AEGainDelay      = 2;
    handle->sensor_ae_info_cfg.u8AEShutterDelay   = 2;
    handle->sensor_ae_info_cfg.u8AEGainCtrlNum    = 1;
    handle->sensor_ae_info_cfg.u8AEShutterCtrlNum = 1;
    handle->sensor_ae_info_cfg.u32AEGain_min                      = 1024;
    handle->sensor_ae_info_cfg.u32AEGain_max                      = SENSOR_MAX_GAIN*1024;
    handle->sensor_ae_info_cfg.u32AEShutter_min                   = Preview_line_period / 2;
    handle->sensor_ae_info_cfg.u32AEShutter_max                   = 1000000000/handle->video_res_supported.res[handle->video_res_supported.ulcur_res].u16min_fps;
    handle->sensor_ae_info_cfg.u32AEShutter_step                  = Preview_line_period / 2;
    //SENSOR_EMSG("[%s] AEShutter_min = %d  AEShutter_step = %d\n", __FUNCTION__,handle->sensor_ae_info_cfg.u32AEShutter_min,handle->sensor_ae_info_cfg.u32AEShutter_step);

    //LOGD("[%s:%d]\n", __FUNCTION__, __LINE__);
    handle->pCus_sensor_release     = cus_camsensor_release_handle;
    handle->pCus_sensor_init        = OS02N10_init    ;
    handle->pCus_sensor_poweron     = OS02N10_poweron ;
    handle->pCus_sensor_poweroff    = OS02N10_poweroff;

    // Normal
    handle->pCus_sensor_GetSensorID       = OS02N10_GetSensorID   ;
    handle->pCus_sensor_GetVideoResNum = OS02N10_GetVideoResNum;
    handle->pCus_sensor_GetVideoRes       = OS02N10_GetVideoRes;
    handle->pCus_sensor_GetCurVideoRes  = OS02N10_GetCurVideoRes;
    handle->pCus_sensor_SetVideoRes       = OS02N10_SetVideoRes;
    handle->pCus_sensor_GetOrien          = OS02N10_GetOrien      ;
    handle->pCus_sensor_SetOrien          = OS02N10_SetOrien      ;
    handle->pCus_sensor_GetFPS          = OS02N10_GetFPS      ;
    handle->pCus_sensor_SetFPS          = OS02N10_SetFPS      ;
    //handle->pCus_sensor_GetSensorCap    = pCus_GetSensorCap;
    handle->pCus_sensor_SetPatternMode = OS02N10_SetPatternMode;
    ///////////////////////////////////////////////////////
    // AE
    ///////////////////////////////////////////////////////
    // unit: micro seconds
    handle->pCus_sensor_AEStatusNotify = OS02N10_AEStatusNotify;
    handle->pCus_sensor_GetAEUSecs      = OS02N10_GetAEUSecs;
    handle->pCus_sensor_SetAEUSecs      = OS02N10_SetAEUSecs;
    handle->pCus_sensor_GetAEGain       = OS02N10_GetAEGain;
    handle->pCus_sensor_SetAEGain       = OS02N10_SetAEGain;

    //handle->pCus_sensor_GetAEMinMaxGain = OS02N10_GetAEMinMaxGain;
    //handle->pCus_sensor_GetAEMinMaxUSecs= OS02N10_GetAEMinMaxUSecs;

    //handle->pCus_sensor_GetShutterInfo = OS02N10_GetShutterInfo;
    handle->pCus_sensor_GetAEInfo          = pCus_sensor_GetAEInfo;

    params->expo.vts=vts_30fps;
    params->expo.fps = 30;
    params->expo.lines = 1000;
    params->gain = 1024;
    params->reg_dirty = false;
    params->ori_dirty = false;
    return SUCCESS;
}

static int cus_camsensor_release_handle(ss_cus_sensor *handle) {

    return SUCCESS;
}

SENSOR_DRV_ENTRY_IMPL_END_EX(  OS02N10,
                            cus_os02n10_camsensor_init_handle,
                            NULL,
                            NULL,
                            OS02N10_params
                         );
