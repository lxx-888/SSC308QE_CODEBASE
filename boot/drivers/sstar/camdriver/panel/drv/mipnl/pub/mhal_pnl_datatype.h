/*
 * mhal_pnl_datatype.h- Sigmastar
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

#ifndef __MHAL_PNL_DATATYPE_H__
#define __MHAL_PNL_DATATYPE_H__

//-------------------------------------------------------------------------------------------------
//  structure
//-------------------------------------------------------------------------------------------------
/// @brief Definition of panel Signaling Type
typedef enum
{
    E_MHAL_PNL_LINK_TTL,              ///< TTL  type
    E_MHAL_PNL_LINK_LVDS,             ///< LVDS type
    E_MHAL_PNL_LINK_RSDS,             ///< RSDS type
    E_MHAL_PNL_LINK_MINILVDS,         ///< TCON
    E_MHAL_PNL_LINK_ANALOG_MINILVDS,  ///< Analog TCON
    E_MHAL_PNL_LINK_DIGITAL_MINILVDS, ///< Digital TCON
    E_MHAL_PNL_LINK_MFC,              ///< Ursa (TTL output to Ursa)
    E_MHAL_PNL_LINK_DAC_I,            ///< DAC output
    E_MHAL_PNL_LINK_DAC_P,            ///< DAC output
    E_MHAL_PNL_LINK_PDPLVDS,          ///< For PDP(Vsync use Manually MODE)
    E_MHAL_PNL_LINK_EXT,              ///< EXT LPLL TYPE
    E_MHAL_PNL_LINK_MIPI_DSI,         ///< Mipi DSI
    E_MHAL_PNL_LINK_BT656,            ///< BT656
    E_MHAL_PNL_LINK_BT601,            ///< BT601
    E_MHAL_PNL_LINK_BT1120,           ///< BT1120
    E_MHAL_PNL_LINK_MCU_TYPE,         ///< MCU Type
    E_MHAL_PNL_LINK_SRGB,             ///< sRGB
    E_MHAL_PNL_LINK_TTL_SPI_IF,       ///< TTL with SPI init interface
    E_MHAL_PNL_LINK_BT1120_DDR,       ///< BT1120 DDR
    E_MHAL_PNL_LINK_DUAL_LVDS,        ///< LVDS DUAL type
} MhalPnlLinkType_e;

/// @brief Define of aspct ratio
typedef enum
{
    E_MHAL_PNL_ASPECT_RATIO_4_3 = 0, ///< set aspect ratio to 4 : 3
    E_MHAL_PNL_ASPECT_RATIO_WIDE,    ///< set aspect ratio to 16 : 9
    E_MHAL_PNL_ASPECT_RATIO_OTHER,   ///< resvered for other aspect ratio other than 4:3/ 16:9
} MhalPnlAspectRatio_e;

/// @brief  Define TI bit mode
typedef enum
{
    E_MHAL_PNL_TI_10BIT_MODE = 0, ///< 10 bits
    E_MHAL_PNL_TI_8BIT_MODE  = 2, ///<  8 bits
    E_MHAL_PNL_TI_6BIT_MODE  = 3, ///<  6 bits
} MhalPnlTiBitMode_e;

/// @brief Define which panel output timing change mode is used to change VFreq for same panel
typedef enum
{
    E_MHAL_PNL_CHG_DCLK   = 0, ///< change output DClk to change Vfreq.
    E_MHAL_PNL_CHG_HTOTAL = 1, ///< change H total to change Vfreq.
    E_MHAL_PNL_CHG_VTOTAL = 2, ///< change V total to change Vfreq.
} MhalPnlOutputTimingMode_e;

/// @brief Define panel output format bit mode
typedef enum
{
    E_MHAL_PNL_OUTPUT_10BIT_MODE  = 0, // default is 10bit, becasue 8bit panel can use 10bit config and 8bit config.
    E_MHAL_PNL_OUTPUT_6BIT_MODE   = 1, // but 10bit panel(like PDP panel) can only use 10bit config.
    E_MHAL_PNL_OUTPUT_8BIT_MODE   = 2, // and some PDA panel is 6bit.
    E_MHAL_PNL_OUTPUT_565BIT_MODE = 3, // 565
} MhalPnlOutputFormatBitMode_e;

/// @brief Define RGB data type and LCD data bus
typedef enum
{
    E_MHAL_PNL_RGB_DTYPE_RGB888,        ///< DTYPE is RGB888
    E_MHAL_PNL_RGB_DTYPE_RGB666,        ///< DTYPE is RGB666
    E_MHAL_PNL_RGB_DTYPE_RGB565,        ///< DTYPE is RGB565
    E_MHAL_PNL_RGB_DTYPE_RGB444,        ///< DTYPE is RGB444
    E_MHAL_PNL_RGB_DTYPE_RGB333,        ///< DTYPE is RGB333
    E_MHAL_PNL_RGB_DTYPE_RGB332,        ///< DTYPE is RGB332
    E_MHAL_PNL_RGB_DTYPE_BGR888,        ///< DTYPE is RGB888
    E_MHAL_PNL_RGB_DTYPE_BGR666,        ///< DTYPE is RGB666
    E_MHAL_PNL_RGB_DTYPE_BGR565,        ///< DTYPE is RGB565
    E_MHAL_PNL_RGB_DTYPE_BGR444,        ///< DTYPE is RGB444
    E_MHAL_PNL_RGB_DTYPE_BGR333,        ///< DTYPE is RGB333
    E_MHAL_PNL_RGB_DTYPE_BGR332,        ///< DTYPE is RGB332
    E_MHAL_PNL_RGB_DTYPE_YUV422_UY0VY1, ///< DTYPE is YUV422(UY0VY1)
    E_MHAL_PNL_RGB_DTYPE_YUV422_VY0UY1, ///< DTYPE is YUV422(VY0UY1)
    E_MHAL_PNL_RGB_DTYPE_YUV422_UY1VY0, ///< DTYPE is YUV422(UY1VY0)
    E_MHAL_PNL_RGB_DTYPE_YUV422_VY1UY0, ///< DTYPE is YUV422(VY1UY0)
    E_MHAL_PNL_RGB_DTYPE_YUV422_Y0UY1V, ///< DTYPE is YUV422(Y0UY1V)
    E_MHAL_PNL_RGB_DTYPE_YUV422_Y1UY0V, ///< DTYPE is YUV422(Y1UY0V)
    E_MHAL_PNL_RGB_DTYPE_YUV422_Y0VY1U, ///< DTYPE is YUV422(Y0VY1U)
    E_MHAL_PNL_RGB_DTYPE_YUV422_Y1VY0U, ///< DTYPE is YUV422(Y1VY0U)
} MhalPnlRgbDataType_e;

/// @brief Define RGB delta mode
typedef enum
{
    E_MHAL_PNL_RGB_Delta_RGB_MODE, ///< RGB mode
    E_MHAL_PNL_RGB_Delta_RBG_MODE, ///< RBG mode
    E_MHAL_PNL_RGB_Delta_GRB_MODE, ///< GRB mode
    E_MHAL_PNL_RGB_Delta_GBR_MODE, ///< GBR mode
    E_MHAL_PNL_RGB_Delta_BRG_MODE, ///< BRG mode
    E_MHAL_PNL_RGB_Delta_BGR_MODE, ///< BGR mode
} MhalPnlRgbDeltaMode_e;

/// @brief Define MipiDsi lane order
typedef enum
{
    E_MHAL_PNL_CH_SWAP_0, ///< mipdis lane 0
    E_MHAL_PNL_CH_SWAP_1, ///< mipdis lane 1
    E_MHAL_PNL_CH_SWAP_2, ///< mipdis lane 2
    E_MHAL_PNL_CH_SWAP_3, ///< mipdis lane 3
    E_MHAL_PNL_CH_SWAP_4, ///< mipdis lane 4
} MhalPnlChannelSwapType_e;

/// @brief Define lane order of RGB
typedef enum
{
    E_MHAL_PNL_REG_SWAP_NONE = 0, ///< HW Default
    E_MHAL_PNL_RGB_SWAP_R    = 1, ///< Swap to R channel
    E_MHAL_PNL_RGB_SWAP_G    = 2, ///< Swap to G channel
    E_MHAL_PNL_RGB_SWAP_B    = 3, ///< Swap to B channel
} MhalPnlRgbSwapType_e;

/// @brief Define control mode of MipiDsi
typedef enum
{
    E_MHAL_PNL_MIPI_DSI_CMD_MODE   = 0, ///< MipiDsi command mode
    E_MHAL_PNL_MIPI_DSI_SYNC_PULSE = 1, ///< MipiDsi sync pluse mode
    E_MHAL_PNL_MIPI_DSI_SYNC_EVENT = 2, ///< MipiDsi Sync event mode
    E_MHAL_PNL_MIPI_DSI_BURST_MODE = 3, ///< MipiDsi burst mode
} MhalPnlMipiDsiCtrlMode_e;

/// @brief Define output color format of MipiDsi
typedef enum
{
    E_MHAL_PNL_MIPI_DSI_RGB565         = 0, ///< MipiDsi output RGB565
    E_MHAL_PNL_MIPI_DSI_RGB666         = 1, ///< MipiDsi output RGB666
    E_MHAL_PNL_MIPI_DSI_LOOSELY_RGB666 = 2, ///< MipiDsi output lossely RGB666
    E_MHAL_PNL_MIPI_DSI_RGB888         = 3, ///< MipiDsi output RGB888
} MhalPnlMipiDsiFormat_e;

/// @brief Define number of lane of MipiDsi
typedef enum
{
    E_MHAL_PNL_MIPI_DSI_LANE_NONE = 0, ///< NO defined
    E_MHAL_PNL_MIPI_DSI_LANE_1    = 1, ///< 1 lane mode
    E_MHAL_PNL_MIPI_DSI_LANE_2    = 2, ///< 2 lane mode
    E_MHAL_PNL_MIPI_DSI_LANE_3    = 3, ///< 3 lane mode
    E_MHAL_PNL_MIPI_DSI_LANE_4    = 4, ///< 4 lane mode
} MhalPnlMipiDsiLaneMode_e;

/// @brief Define LvdsTx format
typedef enum
{
    E_MHAL_PNL_LVDS_VESA  = 0, ///< LVDS format vesa
    E_MHAL_PNL_LVDS_JEIDA = 1, ///< LVDS format jeida
} MhalPnlLvdsFormat_e;

/// @brief Define number of lane of MipiDsi
typedef enum
{
    E_MHAL_PNL_LVDS_LANE_NONE = 0, ///< NO defined
    E_MHAL_PNL_LVDS_LANE_3    = 3, ///< 3 lane mode
    E_MHAL_PNL_LVDS_LANE_4    = 4, ///< 4 lane mode
} MhalPnlLvdsLaneMode_e;

/// @brief Define packet type of MipiDsi command
typedef enum
{
    E_MHAL_PNL_MIPI_DSI_PACKET_TYPE_DCS     = 0, ///< DCS mode
    E_MHAL_PNL_MIPI_DSI_PACKET_TYPE_GENERIC = 1, ///< generic mode
} MhalPnlMipiDsiPacketType_e;

/// @brief Define MCU I/F system
typedef enum
{
    E_MHAL_LCD_MCU_68SYS, ///< MCU 68 system
    E_MHAL_LCD_MCU_80SYS  ///< MCU 80 system
} MhalPnlMcuType;

/// @brief Define MCU data bus configuraiton
typedef enum
{
    // 16bit bus
    E_MHAL_MCU_RGB565_BUS16_CFG,       ///< RGB565_BUS16 0x0000
    E_MHAL_MCU_RGB444_BUS16_CFG,       ///< RGB444_BUS16 0x0001
    E_MHAL_MCU_RGB666_BUS16_CFG,       ///< RGB666_BUS16 0x0002
                                       // 8 bit bus
    E_MHAL_MCU_RGB888_BUS8_CFG,        ///< RGB888_BUS8 0x0003
    E_MHAL_MCU_RGB332_BUS8_CFG,        ///< RGB332_BUS8 0x0004
    E_MHAL_MCU_RGB444_BUS8_CFG,        ///< RGB444_BUS8 0x0005
    E_MHAL_MCU_RGB666_BUS8_CFG,        ///< RGB666_BUS8 0x0006
    E_MHAL_MCU_RGB565_BUS8_CFG,        ///< RGB565_BUS8 0x0007
                                       // 18 bit bus
    E_MHAL_MCU_RGB666_BUS18_CFG,       ///< RGB666_BUS18      0x0008
    E_MHAL_MCU_RGB666_BUS18_9_9_CFG,   ///< RGB666_BUS18_9_9  0x0009
    E_MHAL_MCU_RGB666_BUS18_2_16_CFG,  ///< RGB666_BUS18_2_16 0x000A
    E_MHAL_MCU_RGB666_BUS18_16_2_CFG,  ///< RGB666_BUS18_16_2 0x000B
    E_MHAL_MCU_RGB24_BUS18_16_8_CFG,   ///< RGB24_BUS18_16_8  0x000C
    E_MHAL_MCU_RGB24_BUS18_8_16_CFG,   ///< RGB24_BUS18_8_16  0x000D
                                       // 8 bit
    E_MHAL_MCU_RGB18_BUS8_2_8_8_CFG,   ///< RGB18_BUS_8_2_8_8 0x000E
                                       // TBD
    E_MHAL_MCU_RGB666_BUS8_2_7_CFG,    ///< RGB666_BUS8_2_7 0x000F
    E_MHAL_MCU_RGB444_B12_EXT_B16_CFG, ///< RGB444_B12_EXT_B16  0x0010
    E_MHAL_MCU_RGB444_B15_4_CFG,       ///< RGB444_B15_4        0x0011
    E_MHAL_MCU_RGBB9_9_17_CFG,         ///< RGBB9_17            0x0012
    E_MHAL_MCU_CFG_NOT_SUPPORT         ///< NOTSUPPORT CFG
} MhalPnlMcuDataBusCfg;

/// @brief Define panel configuration
/// @note  This sturcure is invalid.
typedef struct
{
    const char *pPanelName; ///<  PanelName

#if !defined(__aarch64__)
    u16 u32AlignmentDummy0;
#endif

    u8                u8Dither;  ///<  Diether On?off
    MhalPnlLinkType_e eLinkType; ///<  Panel LinkType

    //==========================================
    // Board related setting
    //==========================================
    u8 u8DualPort;    ///<  DualPort on/off
    u8 u8SwapPort;    ///<  Swap Port on/off
    u8 u8SwapOdd_ML;  ///<  Swap Odd ML
    u8 u8SwapEven_ML; ///<  Swap Even ML
    u8 u8SwapOdd_RB;  ///<  Swap Odd RB
    u8 u8SwapEven_RB; ///<  Swap Even RB

    u8 u8SwapLVDS_POL; ///<  Swap LVDS Channel Polarity
    u8 u8SwapLVDS_CH;  ///<  Swap LVDS channel
    u8 u8PDP10BIT;     ///<  PDP 10bits on/off
    u8 u8LVDS_TI_MODE; ///<  Ti Mode On/Off

    //==========================================
    // For TTL Only
    //==========================================
    u8 u8DCLKDelay; ///<  DCLK Delay
    u8 u8InvDCLK;   ///<  CLK Invert
    u8 u8InvDE;     ///<  DE Invert
    u8 u8InvHSync;  ///<  HSync Invert
    u8 u8InvVSync;  ///<  VSync Invert

    //==========================================
    // Output driving current setting
    //==========================================
    // driving current setting (0x00=4mA, 0x01=6mA, 0x02=8mA, 0x03=12mA)
    u8 u8DCKLCurrent;     ///< PANEL_DCLK_CURRENT
    u8 u8DECurrent;       ///< PANEL_DE_CURRENT
    u8 u8ODDDataCurrent;  ///< PANEL_ODD_DATA_CURRENT
    u8 u8EvenDataCurrent; ///< PANEL_EVEN_DATA_CURRENT

    //==========================================
    // panel on/off timing
    //==========================================
    u16 u16OnTiming1;  ///<  time between panel & data while turn on power
    u16 u16OnTiming2;  ///<  time between data & back light while turn on power
    u16 u16OffTiming1; ///<  time between back light & data while turn off power
    u16 u16OffTiming2; ///<  time between data & panel while turn off power

    //==========================================
    // panel timing spec.
    //==========================================
    // sync related
    u16 u16HSyncWidth;     ///<  Hsync Width
    u16 u16HSyncBackPorch; ///<  Hsync back porch

    u16 u16VSyncWidth;     ///<  Vsync width
    u16 u16VSyncBackPorch; ///<  Vsync back porch

    // DE related
    u16 u16HStart; ///<  HDe start
    u16 u16VStart; ///<  VDe start
    u16 u16Width;  ///<  Panel Width
    u16 u16Height; ///<  Panel Height

    // DClk related
    u16 u16MaxHTotal; ///<  Max H Total
    u16 u16HTotal;    ///<  H Total
    u16 u16MinHTotal; ///<  Min H Total

    u16 u16MaxVTotal; ///<  Max V Total
    u16 u16VTotal;    ///<  V Total
    u16 u16MinVTotal; ///<  Min V Total

    u16 u16MaxDCLK; ///<  Max DCLK
    u16 u16DCLK;    ///<  DCLK ( Htt * Vtt * Fps)
    u16 u16MinDCLK; ///<  Min DCLK

    ///<  SSC
    u16 u16SpreadSpectrumStep; ///<  Step of SSC
    u16 u16SpreadSpectrumSpan; ///<  Span of SSC

    ///< PWM
    u8 u8PwmPeriodL; ///<
    u8 u8PwmPeriodH; ///<  Period of Pwm, 2 ~ 262143 Hz
    u8 u8PwmDuty;    ///<  Duty of Pwm 0 ~ 100

    u8                   u8DeinterMode;     ///<  DeInter Mode
    MhalPnlAspectRatio_e ePanelAspectRatio; ///<  Aspec Ratio

    /*
     *
     * Board related params
     *
     *  If a board ( like BD_MST064C_D01A_S ) swap LVDS TX polarity
     *    : This polarity swap value =
     *      (LVDS_PN_SWAP_H<<8) | LVDS_PN_SWAP_L from board define,
     *  Otherwise
     *    : The value shall set to 0.
     */
    u16                          u16LVDSTxSwapValue; // LVDS Swap Value
    MhalPnlTiBitMode_e           eTiBitMode;         // Ti Bit Mode
    MhalPnlOutputFormatBitMode_e eOutputFormatBitMode;

    u8 u8SwapOdd_RG;  ///<  Swap Channel R
    u8 u8SwapEven_RG; ///<  Swap Channel G
    u8 u8SwapOdd_GB;  ///<  Swap Channel B
    u8 u8SwapEven_GB; ///<  Swap Rgb MSB/LSB

    /**
     *  Others
     */
    u8  u8DoubleClk; ///<  Double CLK On/off
    u32 u32MaxSET;   ///<  Max Lpll Set
    u32 u32MinSET;   ///<  Min Lpll Set
    MhalPnlOutputTimingMode_e
        eOutTimingMode; ///<  Define which panel output timing change mode is used to change VFreq for same panel
    u8  u8NoiseDith;    ///<  Noise Dither On/Off
    MhalPnlChannelSwapType_e eCh0; ///<  Channel swap for CH0
    MhalPnlChannelSwapType_e eCh1; ///<  Channel swap for CH1
    MhalPnlChannelSwapType_e eCh2; ///<  Channel swap for CH2
    MhalPnlChannelSwapType_e eCh3; ///<  Channel swap for CH3
    MhalPnlChannelSwapType_e eCh4; ///<  Channel swap for CH4

    MhalPnlMipiDsiPacketType_e enPacketType; ///<  Packet Type
} MhalPnlParamConfig_t;

/// @brief Define MipiDsi configuration
/// @note  This sturcure is invalid.
typedef struct
{
    u8 u8HsTrail;   ///< HsTrail
    u8 u8HsPrpr;    ///< HsPrpr
    u8 u8HsZero;    ///< HsZero
    u8 u8ClkHsPrpr; ///< ClkHsPrpr
    u8 u8ClkHsExit; ///< ClkHsExit
    u8 u8ClkTrail;  ///< ClkTrail
    u8 u8ClkZero;   ///< ClkZero
    u8 u8ClkHsPost; ///< ClkHsPost
    u8 u8DaHsExit;  ///< DaHsExit
    u8 u8ContDet;   ///< ConDet

    u8 u8Lpx;    ///< Lpx
    u8 u8TaGet;  ///< TaGet
    u8 u8TaSure; ///< TaSure
    u8 u8TaGo;   ///< TaGo

    u16 u16Hactive; ///< Horizontal active size
    u16 u16Hpw;     ///< Hsync width
    u16 u16Hbp;     ///< Horizontal back porch
    u16 u16Hfp;     ///< Horizontal front porch

    u16 u16Vactive; ///< Vertical acitve size
    u16 u16Vpw;     ///< Vsync width
    u16 u16Vbp;     ///< Vertical back porch
    u16 u16Vfp;     ///< Vertical front porch

    u16 u16Bllp; ///< Bllp
    u16 u16Fps;  ///< Frame Rate

    MhalPnlMipiDsiLaneMode_e enLaneNum; ///< lane number
    MhalPnlMipiDsiFormat_e   enFormat;  ///< ouptput color format
    MhalPnlMipiDsiCtrlMode_e enCtrl;    ///< control mode

    u8 *pu8CmdBuf;     ///< command data
    u32 u32CmdBufSize; ///< command size

    u8  u8SyncCalibrate;
    u16 u16VirHsyncSt;
    u16 u16VirHsyncEnd;
    u16 u16VsyncRef;
    u16 u16DataClkSkew;

    u8 u8PolCh0; ///< Channel 0 polarity, 0:HW default, 1:positive, 2:negative
    u8 u8PolCh1; ///< Channel 1 polarity, 0:HW default, 1:positive, 2:negative
    u8 u8PolCh2; ///< Channel 2 polarity, 0:HW default, 1:positive, 2:negative
    u8 u8PolCh3; ///< Channel 3 polarity, 0:HW default, 1:positive, 2:negative
    u8 u8PolCh4; ///< Channel 4 polarity, 0:HW default, 1:positive, 2:negative

    MhalPnlMipiDsiPacketType_e enPacketType; ///< Packet type
} MhalPnlMipiDsiConfig_t;

/// @brief Define SSC configuration
typedef struct
{
    bool bEn;           ///< 1 : enable, 0 : disalbe
    u16  u16Step;       ///< step of SSC
    u16  u16Span;       ///< span of SSC
    u16  u16Modulation; ///< Modulation of SSC, unit: Khz
    u16  u16Deviation;  ///< Deviation of SSC, unit: 1/1000
} MhalPnlSscConfig_t;

/// @brief Define timing configuration
typedef struct
{
    u16 u16HSyncWidth;      ///< Hsync width
    u16 u16HSyncBackPorch;  ///< Horizotnal back porch
    u16 u16HSyncFrontPorch; ///< Horizontal front porch

    u16 u16VSyncWidth;      ///< Vsync width
    u16 u16VSyncBackPorch;  ///< Vertical back porch
    u16 u16VSyncFrontPorch; ///< Vertical front porch

    u16 u16HStart; ///< Horizontal DE start
    u16 u16VStart; ///< Vertical DE start

    u16 u16HActive; ///< Horizontal active size
    u16 u16VActive; ///< Vertical active size

    u16 u16HTotal; ///< Horizontal total size
    u16 u16VTotal; ///< Vertical total size

    u16 u16Dclk;
} MhalPnlTimingConfig_t;

/// $brief Define power configuration
typedef struct
{
    bool bEn; ///< 1: enable, 0:disalbe
} MhalPnlPowerConfig_t;

/// @brief Define backlight configuration
typedef struct
{
    bool bEn; ///< 1: enable, 0:disalbe
} MhalPnlBackLightOnOffConfig_t;

/// @brief Define level of backlight configuration
typedef struct
{
    u16 u16Duty;   ///< Duty of backlight
    u16 u16Period; ///< Period of backlight
} MhalPnlBackLightLevelConfig_t;

/// @brief Define driving currnet configuration
typedef struct
{
    u16 u16Val; ///< The value of driving current
} MhalPnlDrvCurrentConfig_t;

/// @brief Define test pattern color configuration
typedef struct
{
    bool bEn;  ///< 1: enable, 0:disalb
    u16  u16R; ///< The value of color R
    u16  u16G; ///< The value of color G
    u16  u16B; ///< The value of color B
} MhalPnlTestPatternConfig_t;

/// @brief Define unified timing generator configuation
typedef struct
{
    //==========================================
    // panel timing spec.
    //==========================================
    // sync related
    u16 u16HSyncWidth;     ///<  Hsync Width
    u16 u16HSyncBackPorch; ///<  Hsync back porch

    u16 u16VSyncWidth;     ///<  Vsync width
    u16 u16VSyncBackPorch; ///<  Vsync back porch

    // DE related
    u16 u16HStart;  ///<  HDe start
    u16 u16VStart;  ///<  VDe start
    u16 u16HActive; ///<  H Active = Panel Width
    u16 u16VActive; ///<  V Active = Panel Height

    // DClk related
    u16 u16HTotal; ///<  H Total
    u16 u16VTotal; ///<  V Total
    u32 u32Dclk;   ///<  DCLK(Htt * Vtt * Fps)
    u16 u16Dclk;   ///<  DCLK(Htt * Vtt * Fps)
} MhalPnlUnifiedTgnTimingConfig_t;

/// @brief Define polarity of unified timing configuation
typedef struct
{
    //==========================================
    // ttl polarity
    //==========================================
    u8 u8InvDCLK;  ///<  CLK Invert
    u8 u8InvDE;    ///<  DE Invert
    u8 u8InvHSync; ///<  HSync Invert
    u8 u8InvVSync; ///<  VSync Invert
} MhalPnlUnifiedTgnPolarityConfig_t;

/// @brief Define output RGB order of unified timing configuation
typedef struct
{
    //==========================================
    // ttl rgb output swap spec.
    //==========================================
    u8 u8SwapChnR;  ///<  Swap Channel R
    u8 u8SwapChnG;  ///<  Swap Channel G
    u8 u8SwapChnB;  ///<  Swap Channel B
    u8 u8SwapRgbML; ///<  Swap Rgb MSB/LSB
} MhalPnlUnifiedTgnRgbOutputSwapConfig_t;

/// @brief Define SSC of unified timing configuation
typedef struct
{
    //==========================================
    // ttl spread spectrum spec.
    //==========================================
    u16 u16SpreadSpectrumStep; ///<  Swap Channel R
    u16 u16SpreadSpectrumSpan; ///<  Swap Channel G
} MhalPnlUnifiedTgnSpreadSpectrumConfig_t;

/// @brief Define MCU auto command configuration
typedef struct
{
    u32 *pu32CmdBuf;
    u32  u32CmdBufSize;
} MhalPnlUnifiedMcuAutoCmdConfig_t;

/// @brief Define MCU initilization command configuration
typedef struct
{
    u32 *pu32CmdBuf;
    u32  u32CmdBufSize;
} MhalPnlUnifiedMcuInitCmdConfig_t;

/// @brief Define MCU unified configuration
typedef struct
{
    u32                              u32HActive;         ///< Horizotnal active size
    u32                              u32VActive;         ///< Vertical active size
    u8                               u8WRCycleCnt;       ///< W/R cycle count
    u8                               u8CSLeadWRCycleCnt; ///< CS lead W/R cycle count
    u8                               u8RSLeadCSCycleCnt; ///< RS Lead CS cycle count
    MhalPnlMcuType                   enMcuType;          ///<  McuType
    MhalPnlMcuDataBusCfg             enMcuDataBusCfg;    ///<  McuDataBusCfg
    MhalPnlUnifiedMcuInitCmdConfig_t stMcuInitCmd;       ///< Initialization MCU command
    MhalPnlUnifiedMcuAutoCmdConfig_t stMcuAutoCmd;       ///< Auto MCU command
} MhalPnlUnifiedMcuConfig_t;

/// @brief Define RGB data type  configuraiton
typedef struct
{
    u8                   u8RgbDswap; ///< Dswap of RGB
    MhalPnlRgbDataType_e eRgbDtype;  ///< Dtype of RGB
} MhalPnlUnifiedRgbDataConfig_t;

/// @brief Define RGB delta configuraiton
typedef struct
{
    MhalPnlRgbDeltaMode_e eOddLine;  ///< Delta mode for Odd line
    MhalPnlRgbDeltaMode_e eEvenLine; ///< Delta mode for Even line
} MhalPnlUnifiedRgbDeltaConfig_t;

/// @brief unified MipiDsi configuraiton
typedef struct
{
    u8 u8HsTrail;   ///< HsTrail
    u8 u8HsPrpr;    ///< HsPrpr
    u8 u8HsZero;    ///< HsZero
    u8 u8ClkHsPrpr; ///< ClkHsPrpr
    u8 u8ClkHsExit; ///< ClkHsexit
    u8 u8ClkTrail;  ///< ClkTrail
    u8 u8ClkZero;   ///< ClkZero
    u8 u8ClkHsPost; ///< ClkHsPost
    u8 u8DaHsExit;  ///< DaHsexit
    u8 u8ContDet;   ///< ConDet

    u8 u8Lpx;    ///< Lpx
    u8 u8TaGet;  ///< TaGet
    u8 u8TaSure; ///< TaSure
    u8 u8TaGo;   ///< TaGo

    u16 u16Hactive; ///< Horizontal active size
    u16 u16Hpw;     ///< Hsycn width
    u16 u16Hbp;     ///< Horizontal back porch
    u16 u16Hfp;     ///< vertical back porch

    u16 u16Vactive; ///< vertical active size
    u16 u16Vpw;     ///< Vsync Width
    u16 u16Vbp;     ///< Vertical back porch
    u16 u16Vfp;     ///< Vertical front porch

    u16 u16Bllp; ///< Bllp
    u16 u16Fps;  ///< Frame Rate

    MhalPnlMipiDsiLaneMode_e enLaneNum; ///< Number of lane
    MhalPnlMipiDsiFormat_e   enFormat;  ///< Output color format
    MhalPnlMipiDsiCtrlMode_e enCtrl;    ///< cotnrol mode

    u8 *pu8CmdBuf;     ///< command Data
    u32 u32CmdBufSize; ///< command size

    u8  u8SyncCalibrate;
    u16 u16VirHsyncSt;
    u16 u16VirHsyncEnd;
    u16 u16VsyncRef;
    u16 u16DataClkSkew;

    u8 u8PolCh0; ///< Channel 0 polarity, 0:HW default, 1:positive, 2:negative
    u8 u8PolCh1; ///< Channel 1 polarity, 0:HW default, 1:positive, 2:negative
    u8 u8PolCh2; ///< Channel 2 polarity, 0:HW default, 1:positive, 2:negative
    u8 u8PolCh3; ///< Channel 3 polarity, 0:HW default, 1:positive, 2:negative
    u8 u8PolCh4; ///< Channel 4 polarity, 0:HW default, 1:positive, 2:negative

    u8 u8SwapCh0; ///< Lane nummber of Channel 0
    u8 u8SwapCh1; ///< Lane nummber of Channel 1
    u8 u8SwapCh2; ///< Lane nummber of Channel 2
    u8 u8SwapCh3; ///< Lane nummber of Channel 3
    u8 u8SwapCh4; ///< Lane nummber of Channel 4

    MhalPnlMipiDsiPacketType_e enPacketType; ///< Mipidsi command type
} MhalPnlUnifiedMipiDsiConfig_t;

/// @brief unified LvdsTx configuration
typedef struct
{
    MhalPnlLvdsFormat_e   enFormat;  ///< Lvds format
    MhalPnlLvdsLaneMode_e enLaneNum; ///< Number of lane

    u8 u8SwapOddEven;
    u8 u8SwapML;
    u8 u8PolHsync; ///< Hsync polarity
    u8 u8PolVsync; ///< Vsync polarity

    u8 u8ClkLane;

    u8 u8PolLane0; ///< Lane0 polarity
    u8 u8PolLane1; ///< Lane1 polarity
    u8 u8PolLane2; ///< Lane2 polarity
    u8 u8PolLane3; ///< Lane3 polarity
    u8 u8PolLane4; ///< Lane4 polarity

    u8 u8SwapLane0; ///< Lane0 selection
    u8 u8SwapLane1; ///< Lane1 selection
    u8 u8SwapLane2; ///< Lane2 selection
    u8 u8SwapLane3; ///< Lane3 selection
    u8 u8SwapLane4; ///< Lane4 selection
} MhalPnlUnifiedLvdsConfig_t;

/// @brief unified Pad Driving configuraiton
typedef struct
{
    u32 u32PadMode;    ///< pad mode
    u8  u8PadDrvngLvl; ///< pad driving level to set
} MhalPnlUnifiedPadDrvngConfig_t;

/// @brief unified parameter configuraiton
typedef struct
{
    const char *      pPanelName; ///<  PanelName
    MhalPnlLinkType_e eLinkType;  ///<  Panel LinkType

    //==========================================
    // Flags
    //==========================================
    // Tgen related
    u8 u8TgnTimingFlag;      ///<  1: stTgnTimingInfo is available;0 not available
    u8 u8TgnPolarityFlag;    ///<  1: stTgnPolarityInfo is available;0 not available
    u8 u8TgnRgbSwapFlag;     ///<  1: stTgnRgbSwapInfo is available;0 not available
    u8 u8TgnOutputBitMdFlag; ///<  1: eOutputFormatBitMode is available;0 not available
    u8 u8TgnFixedDclkFlag;   ///<  1: u16FDclk is available;0 not available
    u8 u8TgnSscFlag;         ///<  1: stTgnSscInfo is available;0 not available
    u8 u8TgnPadMuxFlag;      ///<  1: u16PadMux is available;0 not available

    // Mcu related
    u8 u8McuPhaseFlag;      ///<  1 u8McuPhase is available;0 not available
    u8 u8McuPolarityFlag;   ///<  1 u8McuPolarity is available;0 not available
    u8 u8McuRsPolarityFlag; ///<  1 u8RsPolarity is available;0 not available
    u8 u8McuConfigFlag;     ///<  1 stMcuInfo is available;0 not available

    u8 u8RgbDataFlag;    ///<  1: stRgbDataInfo is available;0 not available
    u8 u8RgbDeltaMdFlag; ///<  1: stRgbDeltaInfo is available;0 not available
    // mipi dsi related
    u8 u8MpdFlag; ///<  1: stMpdInfo is available;0 not available

    // bt1120 related
    u8 u8Bt1120PhaseFlag; ///<  1: u8Bt1120Phase is available;0 not available

    // pad driving related
    u8 u8PadDrvngFlag;      ///<  1: u8PadDrvngInfo is available;0 not available
    u8 u8PadDrvngPadMdFlag; ///<  1: u8PadDrvngInfo.u32PadMode is available;0 not available

    // lvds related
    u8 u8LvdsFlag; ///<  1: stLvdsInfo is available;0 not available
    //==========================================
    // Parameters
    //==========================================
    // Tgen related
    MhalPnlUnifiedTgnTimingConfig_t         stTgnTimingInfo;      ///< timing generator configuration
    MhalPnlUnifiedTgnPolarityConfig_t       stTgnPolarityInfo;    ///< polarity configuration
    MhalPnlUnifiedTgnRgbOutputSwapConfig_t  stTgnRgbSwapInfo;     ///< Output RGB order configuration
    MhalPnlOutputFormatBitMode_e            eOutputFormatBitMode; ///< output bits configuraiton
    u32                                     u32FDclk;             ///< Fixed DClk
    u16                                     u16FDclk;             ///< Fixed DClk
    MhalPnlUnifiedTgnSpreadSpectrumConfig_t stTgnSscInfo;         ///< SSC configuration
    u16                                     u16PadMux;            ///< Pad Mux

    // Mcu related
    u8                        u8McuPhase;      ///<  MCU Phase
    u8                        u8McuPolarity;   ///<  MCU Polarity
    u8                        u8McuRsPolarity; ///<  MCU RsPolarity
    MhalPnlUnifiedMcuConfig_t stMcuInfo;       ///<  MCU configuration

    MhalPnlUnifiedRgbDataConfig_t  stRgbDataInfo;  ///< RGBd Data type configuration
    MhalPnlUnifiedRgbDeltaConfig_t stRgbDeltaInfo; ///< RGB delta configuration

    // mipi dsi related
    MhalPnlUnifiedMipiDsiConfig_t stMpdInfo; ///< MipiDsi configuration

    // bt1120 related
    u8 u8Bt1120Phase; ///<  BT1120 Phase, means phase diff between data & clk. 0:0; 1:90; 2:180; 3:270

    // pad driving related
    MhalPnlUnifiedPadDrvngConfig_t stPadDrvngInfo; ///<  Pad Driving

    // lvds related
    MhalPnlUnifiedLvdsConfig_t stLvdsInfo; ///< LVDS configuration
} MhalPnlUnifiedParamConfig_t;

#endif //
