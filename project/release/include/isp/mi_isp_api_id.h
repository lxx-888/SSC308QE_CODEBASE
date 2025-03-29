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

/*
*   mi_isp_api_id.h
*
*   Created on: June 27, 2018
*       Author: Jeffrey Chou
*/

#ifndef _TUNINGSERVER_API_ID_H_
#define _TUNINGSERVER_API_ID_H_

#ifdef __cplusplus
extern "C"
{
#endif
#define ID_API_COMMON_BASE              (0x0000)
#define ID_API_COMMON_END               (0x0FFF)

#if 0
#define ID_API_ISP_BASE                 (0x1000)
#define ID_API_ISP_END                  (0x1FFF)

#define ID_API_CALIBRATION_20_BASE      (0x2000)
#define ID_API_CALIBRATION_20_END       (0x22FF)

#define ID_API_MAINTOOL_BASE            (0x2300)
#define ID_API_MAINTOOL_END             (0x25FF)

#define ID_API_PLUGIN_BASE              (0x2600)
#define ID_API_PLUGIN_END               (0x28FF)
#else
#define ID_API_ISP_IQ_BASE              (0x1000)
#define ID_API_ISP_IQ_END               (0x13FF)

#define ID_API_ISP_AE_BASE              (0x1400)
#define ID_API_ISP_AE_END               (0x17FF)

#define ID_API_ISP_AWB_BASE             (0x1800)
#define ID_API_ISP_AWB_END              (0x1BFF)

#define ID_API_ISP_AF_BASE              (0x1C00)
#define ID_API_ISP_AF_END               (0x1FFF)

#define ID_API_ISP_OTHER_BASE           (0x2000)
#define ID_API_ISP_OTHER_END            (0x23FF)

#define ID_API_CALIBRATION_20_BASE      (0x2400)
#define ID_API_CALIBRATION_20_END       (0x27FF)

#define ID_API_MAINTOOL_BASE            (0x2800)
#define ID_API_MAINTOOL_END             (0x2BFF)

#define ID_API_PLUGIN_BASE              (0x2C00)
#define ID_API_PLUGIN_END               (0x2DFF)

#define ID_API_CUS3A_BASE               (0x2E00)
#define ID_API_CUS3A_END                (0x2FFF)

#define ID_API_DEBUG_LEVEL_BASE         (0x2FFF)
#define ID_API_DEBUG_LEVEL_END          (0x3100)

#define ID_API_VIDEO_ENCODE_BASE        (0x3101)
#define ID_API_VIDEO_ENCODE_END         (0x31FF)

#define ID_API_RAW_DATA_BASE            (0x3200)
#define ID_API_RAW_DATA_END             (0x32FF)

#define ID_API_RESERVED_BASE            (0x3300)
#define ID_API_RESERVED_END             (0x3FFF)

#endif

#define ID_CHIP_PRETZEL                 (0x6d65031E)
#define ID_CHIP_MACARON                 (0x6d650320)
#define ID_CHIP_PUDDING                 (0x6d650321)
#define ID_CHIP_ISPAHAN                 (0x6d650322)
#define ID_CHIP_TIRAMISU                (0x6d650323)
#define ID_CHIP_IKAYAKI                 (0x6d650324)
#define ID_CHIP_MUFFIN                  (0x6d650325)
#define ID_CHIP_MOCHI                   (0x6d650326)
#define ID_CHIP_MARUKO                  (0x6d650327)
#define ID_CHIP_OPERA                   (0x6d650328)
#define ID_CHIP_SOUFFLE                 (0x6d650329)
#define ID_CHIP_IFORD                   (0x6d65032a)
#define ID_CHIP_PCUPID                  (0x6d65032b)



//================================================================
//  ID Defined : Common  API
//================================================================
#define ID_API_COMMON_I2C_ACCESS                    ( ID_API_COMMON_BASE +  32 ) //SET_API_ID_MI_ISP_WriteI2C
//#define ID_API_COMMON_QuerySensorInfo               ( ID_API_COMMON_BASE +  33 ) //GET_API_ID_MI_ISP_QuerySensorInfo
#define ID_API_COMMON_MIRROR                        ( ID_API_COMMON_BASE +  34 )
#define ID_API_COMMON_ROTATE                        ( ID_API_COMMON_BASE +  35 )
#define ID_API_COMMON_SENSOR_ID                     ( ID_API_COMMON_BASE +  36 )

#define ID_API_COMMON_FileID                        ( ID_API_COMMON_BASE + 100 ) //I1 or I3 or I2 or I5 ...
#define ID_API_COMMON_CHANNEL_ID                    ( ID_API_COMMON_BASE + 101 )
#define ID_API_COMMON_ISPROOT                       ( ID_API_COMMON_BASE + 102 )
#define ID_API_COMMON_AVAILABLE_MEMINFO             ( ID_API_COMMON_BASE + 103 )
#define ID_API_COMMON_DEVICE_ID                     ( ID_API_COMMON_BASE + 104 )
#define ID_API_COMMON_REOPEN                        ( ID_API_COMMON_BASE + 105 )//Ispmid reopen
#define ID_API_COMMON_IMAGE_RESOLUTION_ID           ( ID_API_COMMON_BASE + 106 )

//================================================================
//  ID Defined : ISP  API
//================================================================
#define ID_API_ISP_IQ_VERSION_INFO                  ( ID_API_ISP_IQ_BASE  +  1 ) //  1. 4097
#define ID_API_ISP_IQ_PARAM_INIT_STATUS             ( ID_API_ISP_IQ_BASE  +  2 ) //  2. 4098
#define ID_API_ISP_IQ_FAST_MODE                     ( ID_API_ISP_IQ_BASE  +  3 ) //  3. 4099
#define ID_API_ISP_IQ_COLORTOGRAY                   ( ID_API_ISP_IQ_BASE  +  4 ) //  4. 4100
#define ID_API_ISP_IQ_CONTRAST                      ( ID_API_ISP_IQ_BASE  +  5 ) //  5. 4101
#define ID_API_ISP_IQ_BRIGHTNESS                    ( ID_API_ISP_IQ_BASE  +  6 ) //  6. 4102
#define ID_API_ISP_IQ_LIGHTNESS                     ( ID_API_ISP_IQ_BASE  +  7 ) //  7. 4103
#define ID_API_ISP_IQ_RGBGAMMA                      ( ID_API_ISP_IQ_BASE  +  8 ) //  8. 4104
#define ID_API_ISP_IQ_YUVGAMMA                      ( ID_API_ISP_IQ_BASE  +  9 ) //  9. 4105
#define ID_API_ISP_IQ_SATURATION                    ( ID_API_ISP_IQ_BASE  + 10 ) // 10. 4106
#define ID_API_ISP_IQ_DEFOG                         ( ID_API_ISP_IQ_BASE  + 11 ) // 11. 4107
#define ID_API_ISP_IQ_CCM                           ( ID_API_ISP_IQ_BASE  + 12 ) // 12. 4108
#define ID_API_ISP_IQ_ANTI_FALSE_COLOR              ( ID_API_ISP_IQ_BASE  + 13 ) // 13. 4109
#define ID_API_ISP_IQ_NR3D                          ( ID_API_ISP_IQ_BASE  + 14 ) // 14. 4110
#define ID_API_ISP_IQ_NR2D_DESPIKE                  ( ID_API_ISP_IQ_BASE  + 15 ) // 15. 4111
#define ID_API_ISP_IQ_NR2D_LUMA                     ( ID_API_ISP_IQ_BASE  + 16 ) // 16. 4112
#define ID_API_ISP_IQ_NR2D_CHROMA                   ( ID_API_ISP_IQ_BASE  + 17 ) // 17. 4113
#define ID_API_ISP_IQ_SHARPNESS                     ( ID_API_ISP_IQ_BASE  + 18 ) // 18. 4114
#define ID_API_ISP_IQ_CROSSTALK                     ( ID_API_ISP_IQ_BASE  + 19 ) // 19. 4115
#define ID_API_ISP_IQ_BLACK_LEVEL                   ( ID_API_ISP_IQ_BASE  + 20 ) // 20. 4116
#define ID_API_ISP_IQ_BLACK_LEVEL_P1                ( ID_API_ISP_IQ_BASE  + 21 ) // 21. 4117
#define ID_API_ISP_IQ_WDR                           ( ID_API_ISP_IQ_BASE  + 22 ) // 22. 4118
#define ID_API_ISP_IQ_DEFECT_PIXEL                  ( ID_API_ISP_IQ_BASE  + 23 ) // 23. 4119
#define ID_API_ISP_IQ_HSV                           ( ID_API_ISP_IQ_BASE  + 24 ) // 24. 4120
#define ID_API_ISP_IQ_RGBIR                         ( ID_API_ISP_IQ_BASE  + 25 ) // 25. 4121
#define ID_API_ISP_IQ_FPN                           ( ID_API_ISP_IQ_BASE  + 26 ) // 26. 4122
#define ID_API_ISP_IQ_PFC                           ( ID_API_ISP_IQ_BASE  + 27 ) // 27. 4123
#define ID_API_ISP_IQ_DM                            ( ID_API_ISP_IQ_BASE  + 28 ) // 28. 4124
#define ID_API_ISP_IQ_COLOR_TRANSFORM               ( ID_API_ISP_IQ_BASE  + 29 ) // 29. 4125
#define ID_API_ISP_IQ_HDR                           ( ID_API_ISP_IQ_BASE  + 30 ) // 30. 4126
#define ID_API_ISP_IQ_EFFECT                        ( ID_API_ISP_IQ_BASE  + 31 ) // 31. 4127
#define ID_API_ISP_IQ_SYS_MCNR_MEMORY               ( ID_API_ISP_IQ_BASE  + 32 ) // 32. 4128
#define ID_API_ISP_IQ_LSC                           ( ID_API_ISP_IQ_BASE  + 33 ) // 33. 4129
#define ID_API_ISP_IQ_PARAM_MODE                    ( ID_API_ISP_IQ_BASE  + 34 ) // 34. 4130
#define ID_API_ISP_IQ_LinearityLUT                  ( ID_API_ISP_IQ_BASE  + 35 ) // 35. 4131 //Get Gamma data for CCM calibration
#define ID_API_ISP_IQ_OBCCALIB                      ( ID_API_ISP_IQ_BASE  + 36 ) // 36. 4132 //Get OB data for Gamma or CCM calibration
#define ID_API_ISP_IQ_NR3D_P1                       ( ID_API_ISP_IQ_BASE  + 37 ) // 37. 4133
#define ID_API_ISP_IQ_WDR_FC_CURVE                  ( ID_API_ISP_IQ_BASE  + 38 ) // 38. 4134
#define ID_API_ISP_IQ_R2Y                           ( ID_API_ISP_IQ_BASE  + 39 ) // 39. 4135
#define ID_API_ISP_IQ_ALSC                          ( ID_API_ISP_IQ_BASE  + 40 ) // 40. 4136
#define ID_API_ISP_IQ_LSC_CTRL                      ( ID_API_ISP_IQ_BASE  + 41 ) // 41. 4137
#define ID_API_ISP_IQ_ALSC_CTRL                     ( ID_API_ISP_IQ_BASE  + 42 ) // 42. 4138
#define ID_API_ISP_IQ_DEFECT_PIXEL_CLUSTER          ( ID_API_ISP_IQ_BASE  + 43 ) // 43. 4139
#define ID_API_ISP_IQ_QUERY_CCM_INFO                ( ID_API_ISP_IQ_BASE  + 44 ) // 44. 4140
#define ID_API_ISP_IQ_NR2D_LUMA_ADV                 ( ID_API_ISP_IQ_BASE  + 45 ) // 45. 4141
#define ID_API_ISP_IQ_NR2D_CHROMA_ADV               ( ID_API_ISP_IQ_BASE  + 46 ) // 46. 4142
#define ID_API_ISP_IQ_PFC_EX                        ( ID_API_ISP_IQ_BASE  + 47 ) // 47. 4143
#define ID_API_ISP_IQ_HDR_EX                        ( ID_API_ISP_IQ_BASE  + 48 ) // 48. 4144
#define ID_API_ISP_IQ_SHP_EX                        ( ID_API_ISP_IQ_BASE  + 49 ) // 49. 4145
#define ID_API_ISP_IQ_NR_3D_EX                      ( ID_API_ISP_IQ_BASE  + 50 ) // 50. 4146
#define ID_API_ISP_IQ_DUMMY                         ( ID_API_ISP_IQ_BASE  + 51 ) // 51. 4147
#define ID_API_ISP_IQ_DUMMY_EX                      ( ID_API_ISP_IQ_BASE  + 52 ) // 52. 4148
#define ID_API_ISP_IQ_WDR_FC_CURVE_FULL             ( ID_API_ISP_IQ_BASE  + 53 ) // 53. 4149
#define ID_API_ISP_IQ_ADAPTIVE_GAMMA                ( ID_API_ISP_IQ_BASE  + 54 ) // 54. 4150
#define ID_API_ISP_IQ_NR2D_CHROMA_PRE               ( ID_API_ISP_IQ_BASE  + 55 ) // 55. 4151
#define ID_API_ISP_IQ_TEMP                          ( ID_API_ISP_IQ_BASE  + 56 ) // 56. 4152
#define ID_API_ISP_IQ_TEMP_INFO                     ( ID_API_ISP_IQ_BASE  + 57 ) // 57. 4153
#define ID_API_ISP_IQ_DAYNIGHT_DETECTION            ( ID_API_ISP_IQ_BASE  + 58 ) // 58. 4154
#define ID_API_ISP_IQ_QUERY_DAYNIGHT_INFO           ( ID_API_ISP_IQ_BASE  + 59 ) // 59. 4155
#define ID_API_ISP_IQ_CSA                           ( ID_API_ISP_IQ_BASE  + 60 ) // 60. 4156
#define ID_API_ISP_IQ_WDR_LCE                       ( ID_API_ISP_IQ_BASE  + 61 ) // 61. 4157
#define ID_API_ISP_IQ_WDR_NR                        ( ID_API_ISP_IQ_BASE  + 62 ) // 62. 4158
#define ID_API_ISP_IQ_YCLPF                         ( ID_API_ISP_IQ_BASE  + 63 ) // 63. 4159
#define ID_API_ISP_IQ_DARKSHADING                   ( ID_API_ISP_IQ_BASE  + 64 ) // 64. 4160
#define ID_API_ISP_IQ_WDR_LTM                       ( ID_API_ISP_IQ_BASE  + 65 ) // 65. 4161
#define ID_API_ISP_IQ_ALSC_ADJ                      ( ID_API_ISP_IQ_BASE  + 66 ) // 66. 4162
#define ID_API_ISP_IQ_CT_EX                         ( ID_API_ISP_IQ_BASE  + 67 ) // 67. 4163
#define ID_API_ISP_IQ_BLACK_LEVEL_P2                ( ID_API_ISP_IQ_BASE  + 68 ) // 68. 4164
#define ID_API_ISP_IQ_THERMAL_IR                    ( ID_API_ISP_IQ_BASE  + 69 ) // 69. 4165
#define ID_API_ISP_IQ_DARKFRAME                     ( ID_API_ISP_IQ_BASE  + 70 ) // 70. 4166
#define ID_API_ISP_IQ_DYNAMIC_BLACK_LEVEL           ( ID_API_ISP_IQ_BASE  + 71 ) // 71. 4167
#define ID_API_ISP_IQ_WDR_CURVE_ALIGN_LDR_TO_HDR    ( ID_API_ISP_IQ_BASE  + 72 ) // 72. 4168
#define ID_API_ISP_IQ_WDR_CURVE_ALIGN_HDR_TO_LDR    ( ID_API_ISP_IQ_BASE  + 73 ) // 73. 4169
#define ID_API_ISP_IQ_STITCH_LPF                    ( ID_API_ISP_IQ_BASE  + 74 ) // 74. 4170
#define ID_API_ISP_IQ_WDR_CURVE_PARAM               ( ID_API_ISP_IQ_BASE  + 75 ) // 75. 4171
#define ID_API_ISP_IQ_ALSC_REL                      ( ID_API_ISP_IQ_BASE  + 76 ) // 76. 4172
#define ID_API_ISP_IQ_FPN_CTRL                      ( ID_API_ISP_IQ_BASE  + 77 ) // 77. 4173
#define ID_API_ISP_IQ_AWBALIGN                      ( ID_API_ISP_IQ_BASE  + 78 ) // 78. 4174
#define ID_API_ISP_IQ_GROUP_HOLD                    ( ID_API_ISP_IQ_BASE  + 79 ) // 79. 4175
#define ID_API_ISP_IQ_HUE                           ( ID_API_ISP_IQ_BASE  + 80 ) // 80. 4176
#define ID_API_ISP_IQ_DECOMPRESS                    ( ID_API_ISP_IQ_BASE  + 81 ) // 81. 4177
#define ID_API_ISP_IQ_ROI                           ( ID_API_ISP_IQ_BASE  + 99 ) // 99. 4195
#define ID_API_ISP_IQ_SCENE_DECISION                ( ID_API_ISP_IQ_BASE  + 100 ) // 100. 4196
#define ID_API_ISP_IQ_SCENE_STATIS                  ( ID_API_ISP_IQ_BASE  + 101 ) // 101. 4197
#define ID_API_ISP_IQ_SCENE_INDEX                   ( ID_API_ISP_IQ_BASE  + 102 ) // 102. 4198
#define ID_API_ISP_IQ_SCENE_ADJ                     ( ID_API_ISP_IQ_BASE  + 103 ) // 103. 4199
#define ID_API_ISP_IQ_QUERY_SCENE_CURRENT_LEVEL     ( ID_API_ISP_IQ_BASE  + 104 ) // 104. 4200
#define ID_API_ISP_IQ_QUERY_SCENE_INFO              ( ID_API_ISP_IQ_BASE  + 105 ) // 105. 4201
#define ID_API_ISP_IQ_AIBNR_INFO                    ( ID_API_ISP_IQ_BASE  + 106 ) // 106. 4202
#define ID_API_ISP_IQ_AIBNR_CTRL                    ( ID_API_ISP_IQ_BASE  + 107 ) // 107. 4203

#define ID_API_ISP_OTHER_WDR_CURVE_ALIGN_LDR_TO_HDR ( ID_API_ISP_OTHER_BASE  + 1 ) // 1. 8193
#define ID_API_ISP_OTHER_WDR_CURVE_ALIGN_HDR_TO_LDR ( ID_API_ISP_OTHER_BASE  + 2 ) // 2. 8194

#define ID_API_ISP_AE_HIST_WEIGHT_Y                 ( ID_API_ISP_AE_BASE  +  1 ) //  1. 5121
#define ID_API_ISP_AE_QUERY_EXPOSURE_INFO           ( ID_API_ISP_AE_BASE  +  2 ) //  2. 5122
#define ID_API_ISP_AE_EV_COMP                       ( ID_API_ISP_AE_BASE  +  3 ) //  3. 5123
#define ID_API_ISP_AE_EXPO_MODE                     ( ID_API_ISP_AE_BASE  +  4 ) //  4. 5124
#define ID_API_ISP_AE_MANUAL_EXPO                   ( ID_API_ISP_AE_BASE  +  5 ) //  5. 5125
#define ID_API_ISP_AE_STATE                         ( ID_API_ISP_AE_BASE  +  6 ) //  6. 5126
#define ID_API_ISP_AE_TARGET                        ( ID_API_ISP_AE_BASE  +  7 ) //  7. 5127
#define ID_API_ISP_AE_CONVERGE                      ( ID_API_ISP_AE_BASE  +  8 ) //  8. 5128
#define ID_API_ISP_AE_EXPOSURE_LIMIT                ( ID_API_ISP_AE_BASE  +  9 ) //  9. 5129
#define ID_API_ISP_AE_PLAIN_LONG_EXPO_TABLE         ( ID_API_ISP_AE_BASE  + 10 ) // 10. 5130
#define ID_API_ISP_AE_PLAIN_SHORT_EXPO_TABLE        ( ID_API_ISP_AE_BASE  + 11 ) // 11. 5131
#define ID_API_ISP_AE_WINDOW_WGT_MODE               ( ID_API_ISP_AE_BASE  + 12 ) // 12. 5132
#define ID_API_ISP_AE_WINDOW_WGT                    ( ID_API_ISP_AE_BASE  + 13 ) // 13. 5133
#define ID_API_ISP_AE_FLICKER                       ( ID_API_ISP_AE_BASE  + 14 ) // 14. 5134
#define ID_API_ISP_AE_STRATEGY                      ( ID_API_ISP_AE_BASE  + 15 ) // 15. 5135
#define ID_API_ISP_AE_RGBIRExposureAttr             ( ID_API_ISP_AE_BASE  + 16 ) // 16. 5136
#define ID_API_ISP_AE_HDR                           ( ID_API_ISP_AE_BASE  + 17 ) // 17. 5137
#define ID_API_ISP_AE_MANUAL_EXPO_SHORT             ( ID_API_ISP_AE_BASE  + 18 ) // 18. 5138
#define ID_API_ISP_AE_FLICKER_EX                    ( ID_API_ISP_AE_BASE  + 19 ) // 19. 5139
#define ID_API_ISP_AE_QUERY_FLICKER_EX_INFO         ( ID_API_ISP_AE_BASE  + 20 ) // 20. 5140
#define ID_API_ISP_AE_STABILIZER                    ( ID_API_ISP_AE_BASE  + 21 ) // 21. 5141
#define ID_API_ISP_AE_STRATEGY_EX                   ( ID_API_ISP_AE_BASE  + 22 ) // 22. 5142
#define ID_API_ISP_AE_QUERY_STRATEGY_EX_INFO        ( ID_API_ISP_AE_BASE  + 23 ) // 23. 5143
#define ID_API_ISP_AE_DAYNIGHT_DETECTION            ( ID_API_ISP_AE_BASE  + 24 ) // 24. 5144
#define ID_API_ISP_AE_QUERY_DAYNIGHT_INFO           ( ID_API_ISP_AE_BASE  + 25 ) // 25. 5145
#define ID_API_ISP_AE_STRATEGY_EX_ADV               ( ID_API_ISP_AE_BASE  + 26 ) // 26. 5146
#define ID_API_ISP_AE_HDR_ML_DYNAMIC_RATIO          ( ID_API_ISP_AE_BASE  + 27 ) // 27. 5147
#define ID_API_ISP_AE_HDR_SM_DYNAMIC_RATIO          ( ID_API_ISP_AE_BASE  + 28 ) // 28. 5148
#define ID_API_ISP_AE_LUMA_WGT                      ( ID_API_ISP_AE_BASE  + 29 ) // 29. 5149
#define ID_API_ISP_AE_POWERLINE                     ( ID_API_ISP_AE_BASE  + 30 ) // 30. 5150
#define ID_API_ISP_AE_QUERY_POWERLINE_INFO          ( ID_API_ISP_AE_BASE  + 31 ) // 31. 5151
#define ID_API_ISP_AE_QUERY_HDR_DYNAMIC_RATIO_INFO  ( ID_API_ISP_AE_BASE  + 32 ) // 32. 5152
#define ID_API_ISP_AE_PLAIN_VERY_SHORT_EXPO_TABLE   ( ID_API_ISP_AE_BASE  + 33 ) // 33. 5153
#define ID_API_ISP_AE_MANUAL_EXPO_VERY_SHORT        ( ID_API_ISP_AE_BASE  + 34 ) // 34. 5154
#define ID_API_ISP_AE_QUERY_VERY_SHORT_EXPO_INFO    ( ID_API_ISP_AE_BASE  + 35 ) // 35. 5155
#define ID_API_ISP_AE_PLAIN_EXPO_TABLE_MODE         ( ID_API_ISP_AE_BASE  + 36 ) // 36. 5156
#define ID_API_ISP_AE_CONVERGE_SPEED_EX             ( ID_API_ISP_AE_BASE  + 37 ) // 37. 5157
#define ID_API_ISP_AE_STABILIZER_EX                 ( ID_API_ISP_AE_BASE  + 38 ) // 38. 5158
#define ID_API_ISP_AE_FD_PARAM                      ( ID_API_ISP_AE_BASE  + 39 ) // 39. 5159
#define ID_API_ISP_AE_SetFD_INFO                    ( ID_API_ISP_AE_BASE  + 40 ) // 40. 5160
#define ID_API_ISP_AE_FD_PARAM_EX                   ( ID_API_ISP_AE_BASE  + 41 ) // 41. 5161
//#define ID_API_ISP_AE_FD_PARAM                      ( ID_API_ISP_AE_BASE  + 51 ) // 51. 5171
//#define ID_API_ISP_AE_FD_COR_INFO                   ( ID_API_ISP_AE_BASE  + 52 ) // 52. 5172
#define ID_API_ISP_AE_FD_EX_PARAM                   ( ID_API_ISP_AE_BASE  + 53 ) // 53. 5173
#define ID_API_ISP_AE_FD_COR_EX_INFO                ( ID_API_ISP_AE_BASE  + 54 ) // 54. 5174
#define ID_API_ISP_AE_FAST_MODE                     ( ID_API_ISP_AE_BASE  + 98 ) // 98. 5218
#define ID_API_ISP_AE_VERSION_INFO                  ( ID_API_ISP_AE_BASE  + 99 ) // 99. 5219
#define ID_API_ISP_AE_SCENE_INDEX                   ( ID_API_ISP_AE_BASE  + 100 ) // 100. 5220
#define ID_API_ISP_AE_SCENE_ADJ                     ( ID_API_ISP_AE_BASE  + 101 ) // 101. 5221
#define ID_API_ISP_AE_QUERY_SCENE_CURRENT_LEVEL     ( ID_API_ISP_AE_BASE  + 102 ) // 102. 5222

#define ID_API_ISP_AWB_QUERY_WHITE_BALANCE_INFO     ( ID_API_ISP_AWB_BASE +  1 ) //  1. 6145
#define ID_API_ISP_AWB_ATTR                         ( ID_API_ISP_AWB_BASE +  2 ) //  2. 6146
#define ID_API_ISP_AWB_ATTR_EX                      ( ID_API_ISP_AWB_BASE +  3 ) //  3. 6147
#define ID_API_ISP_AWB_MULTI_LS_ATTR                ( ID_API_ISP_AWB_BASE +  4 ) //  4. 6148
#define ID_API_ISP_AWB_CT_WEIGHT                    ( ID_API_ISP_AWB_BASE +  5 ) //  5. 6149
#define ID_API_ISP_AWB_CTMWB                        ( ID_API_ISP_AWB_BASE +  6 ) //  6. 6150
#define ID_API_ISP_AWB_DAYNIGHT_DETECTION           ( ID_API_ISP_AWB_BASE +  7 ) //  7. 6151
#define ID_API_ISP_AWB_QUERY_DAYNIGHT_INFO          ( ID_API_ISP_AWB_BASE +  8 ) //  8. 6152
#define ID_API_ISP_AWB_STABILIZER                   ( ID_API_ISP_AWB_BASE +  9 ) //  9. 6153
#define ID_API_ISP_AWB_SPECIALCASE                  ( ID_API_ISP_AWB_BASE + 10 ) // 10. 6154
#define ID_API_ISP_AWB_QUERY_SPECIALCASE_INFO       ( ID_API_ISP_AWB_BASE + 11 ) // 11. 6155
#define ID_API_ISP_AWB_STATIS_FILTER                ( ID_API_ISP_AWB_BASE + 12 ) // 12. 6156
#define ID_API_ISP_AWB_STATIS_NR                    ( ID_API_ISP_AWB_BASE + 13 ) // 13. 6157
//#define ID_API_ISP_AWB_INCLUDE_AREA                 ( ID_API_ISP_AWB_BASE + 14 ) // 14. 6158
//#define ID_API_ISP_AWB_EXCLUDE_AREA                 ( ID_API_ISP_AWB_BASE + 15 ) // 15. 6159
#define ID_API_ISP_AWB_FD_INFO                      ( ID_API_ISP_AWB_BASE + 16 ) // 16. 6160
#define ID_API_ISP_AWB_FDAWB_PARAM                  ( ID_API_ISP_AWB_BASE + 17 ) // 17. 6161
#define ID_API_ISP_AWB_FDAWB_INFO                   ( ID_API_ISP_AWB_BASE + 18 ) // 18. 6162
#define ID_API_ISP_AWB_FWST_STRATEGY                ( ID_API_ISP_AWB_BASE + 19 ) // 19. 6163
#define ID_API_ISP_AWB_VERSION_INFO                 ( ID_API_ISP_AWB_BASE + 99 ) // 99. 6243
#define ID_API_ISP_AWB_UPDATE_AIAWB_STATIS          ( ID_API_ISP_AWB_BASE + 100 )// 100.6244
#define ID_API_ISP_AWB_AIAWB_ADJ                    ( ID_API_ISP_AWB_BASE + 101 )// 101.6245
#define ID_API_ISP_AWB_QUERY_AIAWB_INFO             ( ID_API_ISP_AWB_BASE + 102 )// 102.6246
#define ID_API_ISP_AWB_RUN_PERIOD                   ( ID_API_ISP_AWB_BASE + 103 )// 103.6247
#define ID_API_ISP_AWB_SCENE_INDEX                  ( ID_API_ISP_AWB_BASE + 104 )// 104. 6248
#define ID_API_ISP_AWB_SCENE_ADJ                    ( ID_API_ISP_AWB_BASE + 105 )// 105. 6249
#define ID_API_ISP_AWB_QUERY_SCENE_CURRENT_LEVEL    ( ID_API_ISP_AWB_BASE + 106 )// 106. 6250

#define ID_API_ISP_AF_HW_ROI_MODE                   ( ID_API_ISP_AF_BASE  +  1 ) //  1. 7169
#define ID_API_ISP_AF_HW_WIN                        ( ID_API_ISP_AF_BASE  +  2 ) //  2. 7170
#define ID_API_ISP_AF_HW_FILTER_ATTR                ( ID_API_ISP_AF_BASE  +  3 ) //  3. 7171
#define ID_API_ISP_AF_HW_FILTERSQ                   ( ID_API_ISP_AF_BASE  +  4 ) //  4. 7172
#define ID_API_ISP_AF_HW_YPARAM                     ( ID_API_ISP_AF_BASE  +  5 ) //  5. 7173
#define ID_API_ISP_AF_HW_SOURCE                     ( ID_API_ISP_AF_BASE  +  6 ) //  6. 7174
#define ID_API_ISP_AF_HW_PREFILTER                  ( ID_API_ISP_AF_BASE  +  7 ) //  7. 7175
#define ID_API_ISP_AF_HW_YMAP                       ( ID_API_ISP_AF_BASE  +  8 ) //  8. 7176
#define ID_API_ISP_AF_HW_LDG                        ( ID_API_ISP_AF_BASE  +  9 ) //  9. 7177
#define ID_API_ISP_AF_HW_BNR                        ( ID_API_ISP_AF_BASE  + 10 ) // 10. 7178
#define ID_API_ISP_AF_HW_PEAK_MODE                  ( ID_API_ISP_AF_BASE  + 11 ) // 11. 7179

#define ID_API_ISP_AF_ATTR                          ( ID_API_ISP_AF_BASE  + 41 ) // 41. 7209
#define ID_API_ISP_AF_MOTOR                         ( ID_API_ISP_AF_BASE  + 42 ) // 42. 7210
#define ID_API_ISP_AF_ACC_WEIGHT                    ( ID_API_ISP_AF_BASE  + 43 ) // 43. 7211
#define ID_API_ISP_AF_ONESHOT                       ( ID_API_ISP_AF_BASE  + 44 ) // 44. 7212
#define ID_API_ISP_AF_SCENE_CHANGE                  ( ID_API_ISP_AF_BASE  + 45 ) // 45. 7213
#define ID_API_ISP_AF_SEARCH_START                  ( ID_API_ISP_AF_BASE  + 46 ) // 46. 7214
#define ID_API_ISP_AF_SEARCH                        ( ID_API_ISP_AF_BASE  + 47 ) // 47. 7215
#define ID_API_ISP_AF_QUERY_FOCUS_INFO              ( ID_API_ISP_AF_BASE  + 48 ) // 48. 7216
#define ID_API_ISP_AF_ADJUST                        ( ID_API_ISP_AF_BASE  + 49 ) // 49. 7217
#define ID_API_ISP_AF_DETECTFLATZONE                ( ID_API_ISP_AF_BASE  + 50 ) // 50. 7218
#define ID_API_ISP_AF_FDAF                          ( ID_API_ISP_AF_BASE  + 51 ) // 51. 7219
#define ID_API_ISP_AF_FDAFINFO                      ( ID_API_ISP_AF_BASE  + 52 ) // 52. 7220
#define ID_API_ISP_AF_AFVERINFO                     ( ID_API_ISP_AF_BASE  + 53 ) // 53. 7221
#define ID_API_ISP_AF_STATUS                        ( ID_API_ISP_AF_BASE  + 54 ) // 54. 7222
#define ID_API_ISP_AF_STARTVCMPOS                   ( ID_API_ISP_AF_BASE  + 55 ) // 55. 7223
#define ID_API_ISP_AF_ADJUST_II                     ( ID_API_ISP_AF_BASE  + 56 ) // 56. 7224
#define ID_API_ISP_AF_OFFSET                        ( ID_API_ISP_AF_BASE  + 57 ) // 57. 7225
#define ID_API_ISP_AF_BACKUPPOSITION                ( ID_API_ISP_AF_BASE  + 58 ) // 58. 7226
#define ID_API_ISP_AF_AWBINFO                       ( ID_API_ISP_AF_BASE  + 59 ) // 59. 7227
#define ID_API_ISP_AF_FDAEINFO                      ( ID_API_ISP_AF_BASE  + 60 ) // 60. 7228
#define ID_API_ISP_AF_ADJUST_III                    ( ID_API_ISP_AF_BASE  + 61 ) // 61. 7229
#define ID_API_ISP_AF_VERSION_INFO                  ( ID_API_ISP_AF_BASE  + 99 ) // 99. 7267

//#define ID_API_VENC_RcParamEx                     ( ID_API_ISP_OTHER_BASE +  1 ) // 1. 8193 --> old ( ID_API_ISP_BASE + 35 ) //35. 4131
//#define ID_API_VI_SensorFrameRate                 ( ID_API_ISP_OTHER_BASE +  2 ) // 2. 8194 --> old ( ID_API_ISP_BASE + 36 ) //36. 4132
//#define ID_API_VENC_Resolution                    ( ID_API_ISP_OTHER_BASE +  3 ) // 3. 8195 --> old ( ID_API_ISP_BASE + 37 ) //37. 4133

//================================================================
//  ID Defined : Calibration 2.0 API
//================================================================
#define ID_API_CALIBRATION_20_CaliBVAV              ( ID_API_CALIBRATION_20_BASE +  1) //  1. 9217 //SET_API_ID_CALIBRATION_CaliBVAV = 61,
#define ID_API_CALIBRATION_20_Apply                 ( ID_API_CALIBRATION_20_BASE +  2) //  2. 9218 //SET_API_ID_CALIBRATION_ApplyBVAV = 62,
#define ID_API_CALIBRATION_20_CaliData              ( ID_API_CALIBRATION_20_BASE +  3) //  3. 9219 //SET_API_ID_CALIBRATION_SetCaliData = 63,
#define ID_API_CALIBRATION_20_CaliPath              ( ID_API_CALIBRATION_20_BASE +  4) //  4. 9220 //SET_API_ID_CALIBRATION_SetCaliPath = 64,
#define ID_API_CALIBRATION_20_CaliDBPath            ( ID_API_CALIBRATION_20_BASE +  5) //  5. 9221 //SET_API_ID_CALIBRATION_SetCaliDBPath
#define ID_API_CALIBRATION_20_CaliInfo              ( ID_API_CALIBRATION_20_BASE +  6) //  6. 9222 //GET_API_ID_CALIBRATION_GetCaliInfo

#define ID_API_CALIBRATION_20_CaliAWB               ( ID_API_CALIBRATION_20_BASE +  7) //  7. 9223
#define ID_API_CALIBRATION_20_CaliOB                ( ID_API_CALIBRATION_20_BASE +  8) //  8. 9224
#define ID_API_CALIBRATION_20_CaliMinGain           ( ID_API_CALIBRATION_20_BASE +  9) //  9. 9225
#define ID_API_CALIBRATION_20_CaliShutterLinearity  ( ID_API_CALIBRATION_20_BASE + 10) // 10. 9226
#define ID_API_CALIBRATION_20_CaliGainLinearity     ( ID_API_CALIBRATION_20_BASE + 11) // 11  9227
#define ID_API_CALIBRATION_20_CaliDPC               ( ID_API_CALIBRATION_20_BASE + 12) // 12. 9228
#define ID_API_CALIBRATION_20_CaliALSC              ( ID_API_CALIBRATION_20_BASE + 13) // 13. 9229
#define ID_API_CALIBRATION_20_CaliFPN               ( ID_API_CALIBRATION_20_BASE + 14) // 14. 9230

#define ID_API_CALIBRATION_30_ApplyAWB              ( ID_API_CALIBRATION_20_BASE + 15) // 15. 9231
#define ID_API_CALIBRATION_30_ApplyOBC              ( ID_API_CALIBRATION_20_BASE + 16) // 16. 9232
#define ID_API_CALIBRATION_30_ApplyMinGain          ( ID_API_CALIBRATION_20_BASE + 17) // 17. 9233
#define ID_API_CALIBRATION_30_ApplyShutterLinearity ( ID_API_CALIBRATION_20_BASE + 18) // 18. 9234
#define ID_API_CALIBRATION_30_ApplyGainLinearity    ( ID_API_CALIBRATION_20_BASE + 19) // 19  9235
#define ID_API_CALIBRATION_30_ApplySDC              ( ID_API_CALIBRATION_20_BASE + 20) // 20. 9236
#define ID_API_CALIBRATION_30_ApplyALSC             ( ID_API_CALIBRATION_20_BASE + 21) // 21. 9237
#define ID_API_CALIBRATION_30_ApplyFPN              ( ID_API_CALIBRATION_20_BASE + 22) // 22. 9238
#define ID_API_CALIBRATION_30_ApplyLSC              ( ID_API_CALIBRATION_20_BASE + 23) // 23. 9239

#define ID_API_CALIBRATION_40_ApplyAWB              ( ID_API_CALIBRATION_20_BASE + 24) // 24. 9240
#define ID_API_CALIBRATION_30_ApplyNE               ( ID_API_CALIBRATION_20_BASE + 25) // 24. 9241
#define ID_API_CALIBRATION_30_ApplyAIBNR            ( ID_API_CALIBRATION_20_BASE + 26) // 25. 9242

//================================================================
//  ID Defined : MainTool  API
//================================================================
#define ID_API_MAINTOOL_QUERY_SENSOR_INFO           ( ID_API_MAINTOOL_BASE + 1 ) //  1. 10241
#define ID_API_MAINTOOL_IQ_INDEX                    ( ID_API_MAINTOOL_BASE + 2 ) //  2. 10242
#define ID_API_MAINTOOL_BYPASS                      ( ID_API_MAINTOOL_BASE + 3 ) //  3. 10243

//================================================================
//  ID Defined : Plugin  API
//================================================================
#define ID_API_PLUGIN_I2C_ACCESS                    ( ID_API_PLUGIN_BASE + 1 ) //  1. 11265 //SET_API_ID_MI_ISP_WriteI2C
#define ID_API_PLUGIN_WBCT                          ( ID_API_PLUGIN_BASE + 3 ) //  3. 11267 //SetWBCTCaliAttr
#define ID_API_PLUGIN_AWBCTStats                    ( ID_API_PLUGIN_BASE + 4 ) //  4. 11268
#define ID_API_PLUGIN_AWBHWStats                    ( ID_API_PLUGIN_BASE + 5 ) //  5. 11269
#define ID_API_PLUGIN_CUR_WBCT                      ( ID_API_PLUGIN_BASE + 6 ) //  6. 11270 //GetCurCTCaliAttr

//================================================================
//  ID Defined : Cus3A  API
//================================================================
#define ID_API_CUS_AE_HW_AVG_STATS                  ( ID_API_CUS3A_BASE  + 1 ) // 1. 11777
#define ID_API_CUS_AE_HW_HISTO_0_STATS              ( ID_API_CUS3A_BASE  + 2 ) // 2. 11778
#define ID_API_CUS_AE_HW_HISTO_1_STATS              ( ID_API_CUS3A_BASE  + 3 ) // 3. 11779
#define ID_API_CUS_AE_GET_INIT_STATUS               ( ID_API_CUS3A_BASE  + 4 ) // 4. 11780
#define ID_API_CUS_AE_GET_CUR_STATUS                ( ID_API_CUS3A_BASE  + 5 ) // 5. 11781
#define ID_API_CUS_AE_SET_PARAM                     ( ID_API_CUS3A_BASE  + 6 ) // 6. 11782
#define ID_API_CUS_AWB_HW_AVG_STATS                 ( ID_API_CUS3A_BASE  + 7 ) // 7. 11783
#define ID_API_CUS_3A_ENABLE                        ( ID_API_CUS3A_BASE  + 8 ) // 8. 11784
#define ID_API_CUS_AWB_GET_CUR_STATUS               ( ID_API_CUS3A_BASE  + 9 ) // 9. 11785
#define ID_API_CUS_AWB_SET_PARAM                    ( ID_API_CUS3A_BASE  + 10) //10. 11786
#define ID_API_CUS_AE_WINDOW_BLOCK_NUMBER           ( ID_API_CUS3A_BASE  + 11 )//11. 11787
#define ID_API_CUS_AWB_SAMPLING                     ( ID_API_CUS3A_BASE  + 12 )//12. 11788
#define ID_API_CUS_AF_STATS                         ( ID_API_CUS3A_BASE  + 13 )//13. 11789
#define ID_API_CUS_AF_WINDOW                        ( ID_API_CUS3A_BASE  + 14 )//14. 11790
#define ID_API_CUS_AF_FILTER                        ( ID_API_CUS3A_BASE  + 15 )//15. 11791
#define ID_API_CUS_AF_FILTER_SQUARE                 ( ID_API_CUS3A_BASE  + 16 )//16. 11792
#define ID_API_CUS_AF_MODE                          ( ID_API_CUS3A_BASE  + 17 )//17. 11793
#define ID_API_CUS_AE_HISTOGRAM_WINDOW              ( ID_API_CUS3A_BASE  + 18 )//18. 11794
#define ID_API_CUS_GET_IMAGE_RESOLUTION             ( ID_API_CUS3A_BASE  + 19 )//19. 11795
#define ID_API_CUS_ENABLE_ISP_OUT_IMAGE             ( ID_API_CUS3A_BASE  + 20 )//20. 11796
#define ID_API_CUS_GET_ISP_OUT_IMAGE_COUNT          ( ID_API_CUS3A_BASE  + 21 )//21. 11797
#define ID_API_CUS_CAPTURE_HDR_RAW_IMAGE            ( ID_API_CUS3A_BASE  + 22 )//22. 11798
#define ID_API_CUS_FRAME_META_INFO                  ( ID_API_CUS3A_BASE  + 23 )//23. 11799
#define ID_API_CUS_I2C_ACCESS                       ( ID_API_CUS3A_BASE  + 24 )//24. 11800
#define ID_API_CUS_AWB_HW_AVG_STATS_SHORT           ( ID_API_CUS3A_BASE  + 25 )//25. 11801
#define ID_API_CUS_CAPTURE_RAW_IMAGE                ( ID_API_CUS3A_BASE  + 26 )//26. 11802
#define ID_API_CUS_CUSTOMER_AE_CTRL                 ( ID_API_CUS3A_BASE  + 27 )//27. 11803 , Customer defined AE control only
#define ID_API_CUS_CUSTOMER_AWB_CTRL                ( ID_API_CUS3A_BASE  + 28 )//28. 11804 , Customer defined AWB control only
#define ID_API_CUS_CUSTOMER_AF_CTRL                 ( ID_API_CUS3A_BASE  + 29 )//29. 11805 , Customer defined AF control only
#define ID_API_CUS_AE_HW_RGBIR_HISTO_STATS          ( ID_API_CUS3A_BASE  + 30 )//30. 11806
#ifdef ENABLE_EARLY_AE_DONE
#define ID_API_CUS_AE_DONE_RATIO                    ( ID_API_CUS3A_BASE  + 31 )//31. 11807
#endif
#define ID_API_CUS_AF_GET_CUR_STATUS                ( ID_API_CUS3A_BASE  + 32 )//32. 11808 , Customer defined AF control only
#define ID_API_CUS_AF_SET_PARAM                     ( ID_API_CUS3A_BASE  + 33 )//33. 11809 , Customer defined AF control only
#define ID_API_CUS_AE_CROP_SIZE                     ( ID_API_CUS3A_BASE  + 34 )//34. 11810
#define ID_API_CUS_AWB_CROP_SIZE                    ( ID_API_CUS3A_BASE  + 35 )//35. 11811
#define ID_API_CUS_AF_YPARAM                        ( ID_API_CUS3A_BASE  + 36 )//36. 11812
#define ID_API_CUS_AF_SOURCE                        ( ID_API_CUS3A_BASE  + 37 )//37. 11813
#define ID_API_CUS_AF_PRE_FILTER                    ( ID_API_CUS3A_BASE  + 38 )//38. 11814
#define ID_API_CUS_AF_YMAP                          ( ID_API_CUS3A_BASE  + 39 )//39. 11815
#define ID_API_CUS_AF_LDG                           ( ID_API_CUS3A_BASE  + 40 )//40. 11816
#define ID_API_CUS_AF_BNR                           ( ID_API_CUS3A_BASE  + 41 )//41. 11817
#define ID_API_CUS_AF_PEAK_MODE                     ( ID_API_CUS3A_BASE  + 42 )//42. 11818
#define ID_API_CUS_AE_HW_SHORT_HISTO_0_STATS        ( ID_API_CUS3A_BASE  + 43 )//43. 11819
#define ID_API_CUS_AE_HW_SHORT_HISTO_1_STATS        ( ID_API_CUS3A_BASE  + 44 )//44. 11820
#define ID_API_CUS_INJECT3A_ENABLE                  ( ID_API_CUS3A_BASE  + 45 )//45. 11821
#define ID_API_CUS_AE_HW_YUV_HISTO_STATS            ( ID_API_CUS3A_BASE  + 46 )//46. 11822
#define ID_API_CUS_AE_STATS_POST_PROCESSING         ( ID_API_CUS3A_BASE  + 47 )//47. 11823
#define ID_API_CUS_AWB_STATS_POST_PROCESSING        ( ID_API_CUS3A_BASE  + 48 )//48. 11824
#define ID_API_CUS_AF_WINDOW_PIEXL_COUNT            ( ID_API_CUS3A_BASE  + 49 )//49. 11825
#define ID_API_CUS_AF_G_MODE                        ( ID_API_CUS3A_BASE  + 50 )//50. 11826
#define ID_API_CUS_AF_FIR_FILTER                    ( ID_API_CUS3A_BASE  + 51 )//51. 11827
#define ID_API_CUS_GET_INPUT_SIZE                   ( ID_API_CUS3A_BASE  + 52 )//52. 11828
#define ID_API_CUS_AE_SOURCE                        ( ID_API_CUS3A_BASE  + 53 )//53. 11829
#define ID_API_CUS_AWB_SOURCE                       ( ID_API_CUS3A_BASE  + 54 )//54. 11830
#if 1//def ENABLE_ISP_PDAF
#define ID_API_CUS_PDAF_STATS                       ( ID_API_CUS3A_BASE  + 55 )//43. 11819
#endif
#define ID_API_CUS_HISTO_SOURCE                     ( ID_API_CUS3A_BASE  + 56 )//56. 11832

#if 1//def ENABLE_CUS3A_ULTRA_AWB
#define ID_API_CUS_ULTRA_AWB_MODE_ENABLE            ( ID_API_CUS3A_BASE  + 57 )//57. 11833
#define ID_API_CUS_ULTRA_AWB_INFO                   ( ID_API_CUS3A_BASE  + 58 )//58. 11834
#endif

#define ID_API_CUS_DO_AE_COUNT                      ( ID_API_CUS3A_BASE  + 59 )//59. 11835
#define ID_API_CUS_AE_OP_MODE                       ( ID_API_CUS3A_BASE  + 60 )//60. 11836

#ifdef ENABLE_EARLY_AWB_DONE
#define ID_API_CUS_ERALY_AWB_DONE                   ( ID_API_CUS3A_BASE  + 61 )//61. 11837
#endif

#define ID_API_CUS_GET_IQ_SIZE                      ( ID_API_CUS3A_BASE  + 62 )//61. 11838
#define ID_API_CUS_LOAD_IQ_DATA                     ( ID_API_CUS3A_BASE  + 63 )//62. 11839

//================================================================
//  ID Defined : Debug Level  API
//================================================================
#define ID_API_ISP_DeBugLevel_AE                    ( ID_API_DEBUG_LEVEL_BASE  + 1 ) // 1. 12288
#define ID_API_ISP_DeBugLevel_AWB                   ( ID_API_DEBUG_LEVEL_BASE  + 2 ) // 2. 12289
#define ID_API_ISP_DeBugLevel_AWB_PERIOD            ( ID_API_DEBUG_LEVEL_BASE  + 3 ) // 3. 12290
#define ID_API_ISP_DeBugLevel_AF                    ( ID_API_DEBUG_LEVEL_BASE  + 4 ) // 4. 12291


//================================================================
//  ID Defined : ENCODE API
//================================================================
#define ID_API_VIDEO_CHANNEL_ID                     ( ID_API_VIDEO_ENCODE_BASE  + 1 ) // 1. 12546
#define ID_API_VIDEO_RATE_CONTROL_ATTR              ( ID_API_VIDEO_ENCODE_BASE  + 2 ) // 2. 12547
#define ID_API_VIDEO_RATE_CONTROL_PARAM             ( ID_API_VIDEO_ENCODE_BASE  + 3 ) // 2. 12548


#define ID_API_RAW_DATA_INFO                        ( ID_API_RAW_DATA_BASE + 1 ) //12801
#define ID_API_RAW_DATA_INJECT_SWITCH               ( ID_API_RAW_DATA_BASE + 2 ) //12802
#define ID_API_RAW_DATA_COUNT                       ( ID_API_RAW_DATA_BASE + 3 ) //12803
#define ID_API_RAW_DATA_COMPRESS                    ( ID_API_RAW_DATA_BASE + 4 ) //12804
#define ID_API_RAW_SCL_INFO                         ( ID_API_RAW_DATA_BASE + 5 ) //12805
#define ID_API_RAW_DATA_LOCATION_INFO               ( ID_API_RAW_DATA_BASE + 6 ) //12806
#define ID_API_HDR_FUSION_TYPE                      ( ID_API_RAW_DATA_BASE + 7 ) //12807


#define ID_API_REGISTER_API_SET_CB                  ( ID_API_RESERVED_BASE + 1) //1. register API_SET callback
#define ID_API_REGISTER_API_GET_CB                  ( ID_API_RESERVED_BASE + 2) //2. register API_GET callback

#ifdef __cplusplus
}   //end of extern C
#endif

#endif  //_TUNINGSERVER_API_ID_H_
