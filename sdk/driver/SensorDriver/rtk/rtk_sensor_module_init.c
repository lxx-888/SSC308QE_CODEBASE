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

#include <cam_os_wrapper.h>
#include <initcall.h>

#if defined(_SENSOR_IMX307_) || defined(_SENSOR_IMX307MCLK36_)
extern int IMX307_HDR_init_driver(unsigned char chmap);
extern int IMX307_HDR_deinit_driver(unsigned char chmap);
#endif

#if defined(_SENSOR_IMX664_)
extern int IMX664_init_driver(unsigned char chmap);
extern int IMX664_deinit_driver(unsigned char chmap);
#endif

#if defined(_SENSOR_OS02H10_)
extern int OS02H10_init_driver(unsigned char chmap);
extern int OS02H10_deinit_driver(unsigned char chmap);
#endif

#if defined(_SENSOR_PS5250_)
extern int PS5250_init_driver(unsigned char chmap);
#endif

#if defined(_SENSOR_PS5520_)
extern int PS5520_init_driver(unsigned char chmap);
#endif

#if defined(_SENSOR_PS5268_)
extern int PS5268_init_driver(unsigned char chmap);
#endif

#if defined(_SENSOR_PS5416_)
extern int PS5416_init_driver(unsigned char chmap);
#endif

#if defined(_SENSOR_IMX291_)
extern int IMX291_init_driver(unsigned char chmap);
#endif

#if defined(_SENSOR_IMX323_)
extern int IMX323_init_driver(unsigned char chmap);
#endif

#if defined(_SENSOR_IMX327_)
extern int IMX327_HDR_init_driver(unsigned char chmap);
#endif

#if defined(_SENSOR_NVP6124B_)
extern int NVP6124B_init_driver(unsigned char chmap);
#endif

#if defined(_SENSOR_PS5270_)
extern int PS5270_init_driver(unsigned char chmap);
#endif

#if defined(_SENSOR_SC4238_)
extern int SC4238_HDR_init_driver(unsigned char chmap);
#endif

#if defined(_SENSOR_IMX415_)
extern int IMX415_HDR_init_driver(unsigned char chmap);
extern int IMX415_HDR_deinit_driver(unsigned char chmap);
#endif

#if defined(_SENSOR_IMX675_)
extern int IMX675_HDR_init_driver(unsigned char chmap);
extern int IMX675_HDR_deinit_driver(unsigned char chmap);
#endif

#if defined(_SENSOR_IMX678_)
extern int IMX678_HDR_init_driver(unsigned char chmap);
extern int IMX678_HDR_deinit_driver(unsigned char chmap);
#endif

#if defined(_SENSOR_F32_)
extern int F32_init_driver(unsigned char chmap);
#endif

#if defined(_SENSOR_PS5258_)
extern int PS5258_init_driver(unsigned char chmap);
#endif

#if defined(_SENSOR_PS5260_)
extern int PS5260_init_driver(unsigned char chmap);
#endif

#if defined(_SENSOR_SC210IOT_)
extern int  SC210iot_init_driver(unsigned char chmap);
#endif

#if defined(_SENSOR_H66_)
extern int  H66_init_driver(unsigned char chmap);
#endif

#if defined(_SENSOR_GC2053_)
extern int  gc2053_init_driver(unsigned char chmap);
#endif

#if defined(_SENSOR_GC4653_)
extern int  gc4653_init_driver(unsigned char chmap);
#endif

#if defined(_SENSOR_K351P_)
extern int  K351P_HDR_init_driver(unsigned char chmap);
extern int  K351P_HDR_deinit_driver(unsigned char chmap);

#endif

#if defined(_SENSOR_SC4336_)
extern int  SC4336_init_driver(unsigned char chmap);
extern int  SC4336_deinit_driver(unsigned char chmap);
#endif

#if defined(_SENSOR_SC4336P_)
extern int  SC4336P_init_driver(unsigned char chmap);
extern int  SC4336P_deinit_driver(unsigned char chmap);
#endif

#if defined(_SENSOR_SC4336PDUALSNRAOV_)
extern int  SC4336P_DUALSNR_AOV_init_driver(unsigned char chmap);
extern int  SC4336P_DUALSNR_AOV_deinit_driver(unsigned char chmap);
#endif

#if defined(_SENSOR_OS04D10_)
extern int  OS04D10_init_driver(unsigned char chmap);
extern int  OS04D10_deinit_driver(unsigned char chmap);
#endif

#if defined(_SENSOR_SC830AI_)
extern int  SC830AI_init_driver(unsigned char chmap);
extern int  SC830AI_deinit_driver(unsigned char chmap);
#endif
#if defined(_SENSOR_OS02N10_)
extern int  OS02N10_init_driver(unsigned char chmap);
extern int  OS02N10_deinit_driver(unsigned char chmap);
#endif
#if defined(_SENSOR_SC2336_)
extern int  SC2336_init_driver(unsigned char chmap);
extern int  SC2336_deinit_driver(unsigned char chmap);
#endif
#if defined(_SENSOR_SC431HAI_)
extern int  SC431HAI_HDR_init_driver(unsigned char chmap);
extern int  SC431HAI_HDR_deinit_driver(unsigned char chmap);
#endif
#if defined(_SENSOR_SC2336P_)
extern int  SC2336P_init_driver(unsigned char chmap);
extern int  SC2336P_deinit_driver(unsigned char chmap);
#endif

#if defined(_SENSOR_SC835HAI_)
extern int  SC835HAI_init_driver(unsigned char chmap);
extern int  SC835HAI_deinit_driver(unsigned char chmap);
#endif

#if defined(_SENSOR_OG0VA1B_)
extern int  OG0VA1B_init_driver(unsigned char chmap);
extern int  OG0VA1B_deinit_driver(unsigned char chmap);
#endif

#if defined(_SENSOR_SC535HAI_)
extern int  SC535HAI_init_driver(unsigned char chmap);
extern int  SC535HAI_deinit_driver(unsigned char chmap);
#endif

#if defined(_SENSOR_SC2356_)
extern int  SC2356_init_driver(unsigned char chmap);
extern int  SC2356_deinit_driver(unsigned char chmap);
#endif

#if defined(_SENSOR_EARLYINIT_)
void rtk_earlyinit_prepare(void)
{
#if defined(_SENSOR_IMX307_) || defined(_SENSOR_IMX307MCLK36_)
extern void IMX307_EarlyInitReg(unsigned int);
    #if (_SENSOR_IMX307_ == 0)
        IMX307_EarlyInitReg(_SENSOR0_CHMAP_);
    #elif (_SENSOR_IMX307_ == 1)
        IMX307_EarlyInitReg(_SENSOR1_CHMAP_);
    #endif
#endif

#if defined(_SENSOR_IMX664_)
extern void IMX664_EarlyInitReg(unsigned int);
    #if (_SENSOR_IMX664_ == 0)
        IMX664_EarlyInitReg(_SENSOR0_CHMAP_);
    #elif (_SENSOR_IMX664_ == 1)
        IMX664_EarlyInitReg(_SENSOR1_CHMAP_);
    #endif
#endif

#if defined(_SENSOR_PS5250_)
extern void PS5250_EarlyInitReg(unsigned int);
    #if (_SENSOR_PS5250_ == 0)
        PS5250_EarlyInitReg(_SENSOR0_CHMAP_);
    #elif (_SENSOR_PS5250_ == 1)
        PS5250_EarlyInitReg(_SENSOR1_CHMAP_);
    #endif
#endif

#if defined(_SENSOR_PS5268_)
extern void PS5268_EarlyInitReg(unsigned int);
    #if (_SENSOR_PS5268_ == 0)
        PS5268_EarlyInitReg(_SENSOR0_CHMAP_);
    #elif (_SENSOR_PS5268_ == 1)
        PS5268_EarlyInitReg(_SENSOR1_CHMAP_);
    #endif
#endif

#if defined(_SENSOR_SC835HAI_)
extern void SC835HAI_DUALSNR_AOV_EarlyInitReg(unsigned int);
    #if (_SENSOR_SC835HAI_ == 0)
        SC835HAI_EarlyInitReg(_SENSOR0_CHMAP_);
    #elif (_SENSOR_SC835HAI_ == 1)
        SC835HAI_EarlyInitReg(_SENSOR1_CHMAP_);
    #endif
#endif

#if defined(_SENSOR_PS5520_)
extern void PS5520_EarlyInitReg(unsigned int);
    #if (_SENSOR_PS5520_ == 0)
        PS5520_EarlyInitReg(_SENSOR0_CHMAP_);
    #elif (_SENSOR_PS5520_ == 1)
        PS5520_EarlyInitReg(_SENSOR1_CHMAP_);
    #endif
#endif

#if defined(_SENSOR_PS5270_)
extern void PS5270_EarlyInitReg(unsigned int);
    #if (_SENSOR_PS5270_ == 0)
        PS5270_EarlyInitReg(_SENSOR0_CHMAP_);
    #elif (_SENSOR_PS5270_ == 1)
        PS5270_EarlyInitReg(_SENSOR1_CHMAP_);
    #endif
#endif

#if defined(_SENSOR_PS5416_)
extern void PS5416_EarlyInitReg(unsigned int);
    #if (_SENSOR_PS5416_ == 0)
        PS5416_EarlyInitReg(_SENSOR0_CHMAP_);
    #elif (_SENSOR_PS5416_ == 1)
        PS5416_EarlyInitReg(_SENSOR1_CHMAP_);
    #endif
#endif

#if defined(_SENSOR_SC4238_)
extern void SC4238_EarlyInitReg(unsigned int);
    #if (_SENSOR_SC4238_ == 0)
        SC4238_EarlyInitReg(_SENSOR0_CHMAP_);
    #elif (_SENSOR_SC4238_ == 1)
        SC4238_EarlyInitReg(_SENSOR1_CHMAP_);
    #endif
#endif

#if defined(_SENSOR_IMX415_)
extern void IMX415_EarlyInitReg(unsigned int);
    #if (_SENSOR_IMX415_ == 0)
        IMX415_EarlyInitReg(_SENSOR0_CHMAP_);
    #elif (_SENSOR_IMX415_ == 1)
        IMX415_EarlyInitReg(_SENSOR1_CHMAP_);
    #endif
#endif

#if defined(_SENSOR_IMX675_)
extern void IMX675_EarlyInitReg(unsigned int);
    #if (_SENSOR_IMX675_ == 0)
        IMX675_EarlyInitReg(_SENSOR0_CHMAP_);
    #elif (_SENSOR_IMX675_ == 1)
        IMX675_EarlyInitReg(_SENSOR1_CHMAP_);
    #endif
#endif

#if defined(_SENSOR_IMX678_)
extern void IMX678_EarlyInitReg(unsigned int);
    #if (_SENSOR_IMX678_ == 0)
        IMX678_EarlyInitReg(_SENSOR0_CHMAP_);
    #elif (_SENSOR_IMX678_ == 1)
        IMX678_EarlyInitReg(_SENSOR1_CHMAP_);
    #endif
#endif

#if defined(_SENSOR_F32_)
extern void F32_EarlyInitReg(unsigned int);
    #if (_SENSOR_F32_ == 0)
        F32_EarlyInitReg(_SENSOR0_CHMAP_);
    #elif (_SENSOR_F32_ == 1)
        F32_EarlyInitReg(_SENSOR1_CHMAP_);
    #endif
#endif

#if defined(_SENSOR_PS5258_)
extern void PS5258_EarlyInitReg(unsigned int);
    #if (_SENSOR_PS5258_ == 0)
        PS5258_EarlyInitReg(_SENSOR0_CHMAP_);
    #elif (_SENSOR_PS5258_ == 1)
        PS5258_EarlyInitReg(_SENSOR1_CHMAP_);
    #endif
#endif

#if defined(_SENSOR_PS5260_)
extern void PS5260_EarlyInitReg(unsigned int);
    #if (_SENSOR_PS5260_ == 0)
        PS5260_EarlyInitReg(_SENSOR0_CHMAP_);
    #elif (_SENSOR_PS5260_ == 1)
        PS5260_EarlyInitReg(_SENSOR1_CHMAP_);
    #endif
#endif

#if defined(_SENSOR_SC210IOT_)
extern void SC210iot_EarlyInitReg(unsigned int);
    #if (_SENSOR_SC210IOT_ == 0)
        SC210iot_EarlyInitReg(_SENSOR0_CHMAP_);
    #elif (_SENSOR_SC210IOT_ == 1)
        SC210iot_EarlyInitReg(_SENSOR1_CHMAP_);
    #endif
#endif

#if defined(_SENSOR_H66_)
extern void H66_EarlyInitReg(unsigned int);
    #if (_SENSOR_H66_ == 0)
        H66_EarlyInitReg(_SENSOR0_CHMAP_);
    #elif (_SENSOR_H66_ == 1)
        H66_EarlyInitReg(_SENSOR1_CHMAP_);
    #endif
#endif

#if defined(_SENSOR_GC2053_)
extern void gc2053_EarlyInitReg(unsigned int);
    #if (_SENSOR_GC2053_ == 0)
        gc2053_EarlyInitReg(_SENSOR0_CHMAP_);
    #elif (_SENSOR_GC2053_ == 1)
        gc2053_EarlyInitReg(_SENSOR1_CHMAP_);
    #endif
#endif

#if defined(_SENSOR_GC4653_)
extern void gc4653_EarlyInitReg(unsigned int);
    #if (_SENSOR_GC4653_ == 0)
        gc4653_EarlyInitReg(_SENSOR0_CHMAP_);
    #elif (_SENSOR_GC4653_ == 1)
        gc4653_EarlyInitReg(_SENSOR1_CHMAP_);
    #endif
#endif

#if defined(_SENSOR_SC4336_)
extern void SC4336_EarlyInitReg(unsigned int);
    #if (_SENSOR_SC4336_ == 0)
        SC4336_EarlyInitReg(_SENSOR0_CHMAP_);
    #elif (_SENSOR_SC4336_ == 1)
        SC4336_EarlyInitReg(_SENSOR1_CHMAP_);
    #endif
#endif

#if defined(_SENSOR_K351P_)
extern void K351P_EarlyInitReg(unsigned int);
    #if (_SENSOR_K351P_ == 0)
        K351P_EarlyInitReg(_SENSOR0_CHMAP_);
    #elif (_SENSOR_K351P_ == 1)
        K351P_EarlyInitReg(_SENSOR1_CHMAP_);
    #endif
#endif

#if defined(_SENSOR_SC4336P_)
extern void SC4336P_EarlyInitReg(unsigned int);
    #if (_SENSOR_SC4336P_ == 0)
        SC4336P_EarlyInitReg(_SENSOR0_CHMAP_);
    #elif (_SENSOR_SC4336P_ == 1)
        SC4336P_EarlyInitReg(_SENSOR1_CHMAP_);
    #endif
#endif

#if defined(_SENSOR_SC4336PDUALSNRAOV_)
extern void SC4336P_DUALSNR_AOV_EarlyInitReg(unsigned int);
    #if (_SENSOR_SC4336PDUALSNRAOV_ == 0)
        SC4336P_DUALSNR_AOV_EarlyInitReg(_SENSOR0_CHMAP_);
    #elif (_SENSOR_SC4336PDUALSNRAOV_ == 1)
        SC4336P_DUALSNR_AOV_EarlyInitReg(_SENSOR1_CHMAP_);
    #endif
#endif

#if defined(_SENSOR_OS04D10_)
extern void OS04D10_EarlyInitReg(unsigned int);
    #if (_SENSOR_OS04D10_ == 0)
        OS04D10_EarlyInitReg(_SENSOR0_CHMAP_);
    #elif (_SENSOR_OS04D10_ == 1)
        OS04D10_EarlyInitReg(_SENSOR1_CHMAP_);
    #endif
#endif

#if defined(_SENSOR_SC830AI_)
extern void SC830AI_EarlyInitReg(unsigned int);
    #if (_SENSOR_SC830AI_ == 0)
        SC830AI_EarlyInitReg(_SENSOR0_CHMAP_);
    #elif (_SENSOR_SC830AI_ == 1)
        SC830AI_EarlyInitReg(_SENSOR1_CHMAP_);
    #endif
#endif
#if defined(_SENSOR_OS02N10_)
extern void OS02N10_EarlyInitReg(unsigned int);
    #if (_SENSOR_OS02N10_ == 0)
        OS02N10_EarlyInitReg(_SENSOR0_CHMAP_);
    #elif (_SENSOR_OS02N10_ == 1)
        OS02N10_EarlyInitReg(_SENSOR1_CHMAP_);
    #endif
#endif
#if defined(_SENSOR_SC2336_)
extern void SC2336_EarlyInitReg(unsigned int);
    #if (_SENSOR_SC2336_ == 0)
        SC2336_EarlyInitReg(_SENSOR0_CHMAP_);
    #elif (_SENSOR_SC2336_ == 1)
        SC2336_EarlyInitReg(_SENSOR1_CHMAP_);
    #endif
#endif
#if defined(_SENSOR_SC431HAI_)
extern void SC431HAI_EarlyInitReg(unsigned int);
    #if (_SENSOR_SC431HAI_ == 0)
        SC431HAI_EarlyInitReg(_SENSOR0_CHMAP_);
    #elif (_SENSOR_SC431HAI_ == 1)
        SC431HAI_EarlyInitReg(_SENSOR1_CHMAP_);
    #endif
#endif
#if defined(_SENSOR_SC2336P_)
extern void SC2336P_EarlyInitReg(unsigned int);
    #if (_SENSOR_SC2336P_ == 0)
        SC2336P_EarlyInitReg(_SENSOR0_CHMAP_);
    #elif (_SENSOR_SC2336P_ == 1)
        SC2336P_EarlyInitReg(_SENSOR1_CHMAP_);
    #endif
#endif
#if defined(_SENSOR_SC535HAI_)
extern void SC535HAI_EarlyInitReg(unsigned int);
    #if (_SENSOR_SC535HAI_ == 0)
        SC535HAI_EarlyInitReg(_SENSOR0_CHMAP_);
    #elif (_SENSOR_SC535HAI_ == 1)
        SC535HAI_EarlyInitReg(_SENSOR1_CHMAP_);
    #endif
#endif
#if defined(_SENSOR_SC2356_)
extern void SC2356_EarlyInitReg(unsigned int);
    #if (_SENSOR_SC2356_ == 0)
        SC2356_EarlyInitReg(_SENSOR0_CHMAP_);
    #elif (_SENSOR_SC2356_ == 1)
        SC2356_EarlyInitReg(_SENSOR1_CHMAP_);
    #endif
#endif
}
#endif

void rtk_sensor_module_init(void)
{

#if defined(_SENSOR_IMX307_) || defined(_SENSOR_IMX307MCLK36_)
    #if (_SENSOR_IMX307_ == 0)
        IMX307_HDR_init_driver(_SENSOR0_CHMAP_);
    #elif (_SENSOR_IMX307_ == 1)
        IMX307_HDR_init_driver(_SENSOR1_CHMAP_);
    #endif
#endif

#if defined(_SENSOR_IMX664_)
    #if (_SENSOR_IMX664_ == 0)
        IMX664_init_driver(_SENSOR0_CHMAP_);
    #elif (_SENSOR_IMX664_ == 1)
        IMX664_init_driver(_SENSOR1_CHMAP_);
    #endif
#endif

#if defined(_SENSOR_OS02H10_)
    #if (_SENSOR_OS02H10_ == 0)
        OS02H10_init_driver(_SENSOR0_CHMAP_);
    #elif (_SENSOR_OS02H10_ == 1)
        OS02H10_init_driver(_SENSOR1_CHMAP_);
    #endif
#endif

#if defined(_SENSOR_PS5250_)
    #if (_SENSOR_PS5250_ == 0)
        PS5250_init_driver(_SENSOR0_CHMAP_);
    #elif (_SENSOR_PS5250_ == 1)
        PS5250_init_driver(_SENSOR1_CHMAP_);
    #endif
#endif

#if defined(_SENSOR_PS5520_)
    #if (_SENSOR_PS5520_ == 0)
        PS5520_init_driver(_SENSOR0_CHMAP_);
    #elif (_SENSOR_PS5520_ == 1)
        PS5520_init_driver(_SENSOR1_CHMAP_);
    #endif
#endif

#if defined(_SENSOR_PS5268_)
    #if (_SENSOR_PS5268_ == 0)
        PS5268_init_driver(_SENSOR0_CHMAP_);
    #elif (_SENSOR_PS5268_ == 1)
        PS5268_init_driver(_SENSOR1_CHMAP_);
    #endif
#endif

#if defined(_SENSOR_PS5416_)
    #if (_SENSOR_PS5416_ == 0)
        PS5416_init_driver(_SENSOR0_CHMAP_);
    #elif (_SENSOR_PS5416_ == 1)
        PS5416_init_driver(_SENSOR1_CHMAP_);
    #endif
#endif

#if defined(_SENSOR_IMX291_)
    #if (_SENSOR_IMX291_ == 0)
        IMX291_init_driver(_SENSOR0_CHMAP_);
    #elif (_SENSOR_IMX291_ == 1)
        IMX291_init_driver(_SENSOR1_CHMAP_);
    #endif
#endif

#if defined(_SENSOR_IMX323_)
    #if (_SENSOR_IMX323_ == 0)
        IMX323_init_driver(_SENSOR0_CHMAP_);
    #elif (_SENSOR_IMX323_ == 1)
        IMX323_init_driver(_SENSOR1_CHMAP_);
    #endif
#endif

#if defined(_SENSOR_IMX327_)
    #if (_SENSOR_IMX327_ == 0)
        IMX327_HDR_init_driver(_SENSOR0_CHMAP_);
    #elif (_SENSOR_IMX327_ == 1)
        IMX327_HDR_init_driver(_SENSOR1_CHMAP_);
    #endif
#endif

#if defined(_SENSOR_NVP6124B_)
    #if (_SENSOR_NVP6124B_ == 0)
        NVP6124B_init_driver(_SENSOR0_CHMAP_);
    #elif (_SENSOR_NVP6124B_ == 1)
        NVP6124B_init_driver(_SENSOR1_CHMAP_);
    #endif
#endif

#if defined(_SENSOR_PS5270_)
    #if (_SENSOR_PS5270_ == 0)
        PS5270_init_driver(_SENSOR0_CHMAP_);
    #elif (_SENSOR_PS5270_ == 1)
        PS5270_init_driver(_SENSOR1_CHMAP_);
    #endif
#endif

#if defined(_SENSOR_SC4238_)
    #if (_SENSOR_SC4238_ == 0)
        SC4238_HDR_init_driver(_SENSOR0_CHMAP_);
    #elif (_SENSOR_SC4238_ == 1)
        SC4238_HDR_init_driver(_SENSOR1_CHMAP_);
    #endif
#endif

#if defined(_SENSOR_IMX415_)
    #if (_SENSOR_IMX415_ == 0)
        IMX415_HDR_init_driver(_SENSOR0_CHMAP_);
    #elif (_SENSOR_IMX415_ == 1)
        IMX415_HDR_init_driver(_SENSOR1_CHMAP_);
    #endif
#endif

#if defined(_SENSOR_IMX675_)
    #if (_SENSOR_IMX675_ == 0)
        IMX675_HDR_init_driver(_SENSOR0_CHMAP_);
    #elif (_SENSOR_IMX675_ == 1)
        IMX675_HDR_init_driver(_SENSOR1_CHMAP_);
    #endif
#endif

#if defined(_SENSOR_IMX678_)
    #if (_SENSOR_IMX678_ == 0)
        IMX678_HDR_init_driver(_SENSOR0_CHMAP_);
    #elif (_SENSOR_IMX678_ == 1)
        IMX678_HDR_init_driver(_SENSOR1_CHMAP_);
    #endif
#endif

#if defined(_SENSOR_F32_)
    #if (_SENSOR_F32_ == 0)
        F32_init_driver(_SENSOR0_CHMAP_);
    #elif (_SENSOR_F32_ == 1)
        F32_init_driver(_SENSOR1_CHMAP_);
    #endif
#endif

#if defined(_SENSOR_SC210IOT_)
    #if (_SENSOR_SC210IOT_ == 0)
          SC210iot_init_driver(_SENSOR0_CHMAP_);
    #elif (_SENSOR_SC210IOT_ == 1)
          SC210iot_init_driver(_SENSOR1_CHMAP_);
    #endif
#endif

#if defined(_SENSOR_H66_)
    #if (_SENSOR_H66_ == 0)
          H66_init_driver(_SENSOR0_CHMAP_);
    #elif (_SENSOR_H66_ == 1)
          H66_init_driver(_SENSOR1_CHMAP_);
    #endif
#endif

#if defined(_SENSOR_PS5258_)
    #if (_SENSOR_PS5258_ == 0)
        PS5258_init_driver(_SENSOR0_CHMAP_);
    #elif (_SENSOR_PS5258_ == 1)
        PS5258_init_driver(_SENSOR1_CHMAP_);
    #endif
#endif

#if defined(_SENSOR_PS5260_)
    #if (_SENSOR_PS5260_ == 0)
        PS5260_init_driver(_SENSOR0_CHMAP_);
    #elif (_SENSOR_PS5260_ == 1)
        PS5260_init_driver(_SENSOR1_CHMAP_);
    #endif
#endif

#if defined(_SENSOR_GC2053_)
    #if (_SENSOR_GC2053_ == 0)
        gc2053_init_driver(_SENSOR0_CHMAP_);
    #elif (_SENSOR_GC2053_ == 1)
        gc2053_init_driver(_SENSOR1_CHMAP_);
    #endif
#endif

#if defined(_SENSOR_GC4653_)
    #if (_SENSOR_GC4653_ == 0)
        gc4653_init_driver(_SENSOR0_CHMAP_);
    #elif (_SENSOR_GC4653_ == 1)
        gc4653_init_driver(_SENSOR1_CHMAP_);
    #endif
#endif

#if defined(_SENSOR_SC4336_)
    #if (_SENSOR_SC4336_ == 0)
        SC4336_init_driver(_SENSOR0_CHMAP_);
    #elif (_SENSOR_SC4336_ == 1)
        SC4336_init_driver(_SENSOR1_CHMAP_);
    #endif
#endif

#if defined(_SENSOR_SC835HAI_)
    #if (_SENSOR_SC835HAI_ == 0)
        SC835HAI_init_driver(_SENSOR0_CHMAP_);
    #elif (_SENSOR_SC835HAI_ == 1)
        SC835HAI_init_driver(_SENSOR1_CHMAP_);
    #endif
#endif

#if defined(_SENSOR_K351P_)
    #if (_SENSOR_K351P_ == 0)
        K351P_HDR_init_driver(_SENSOR0_CHMAP_);
    #elif (_SENSOR_K351P_ == 1)
        K351P_HDR_init_driver(_SENSOR1_CHMAP_);
    #endif
#endif

#if defined(_SENSOR_SC4336P_)
    #if (_SENSOR_SC4336P_ == 0)
        SC4336P_init_driver(_SENSOR0_CHMAP_);
    #elif (_SENSOR_SC4336P_ == 1)
        SC4336P_init_driver(_SENSOR1_CHMAP_);
    #endif
#endif

#if defined(_SENSOR_SC4336PDUALSNRAOV_)
    #if (_SENSOR_SC4336PDUALSNRAOV_ == 0)
        SC4336P_DUALSNR_AOV_init_driver(_SENSOR0_CHMAP_);
    #elif (_SENSOR_SC4336PDUALSNRAOV_ == 1)
        SC4336P_DUALSNR_AOV_init_driver(_SENSOR1_CHMAP_);
    #endif
#endif

#if defined(_SENSOR_OS04D10_)
    #if (_SENSOR_OS04D10_ == 0)
        OS04D10_init_driver(_SENSOR0_CHMAP_);
    #elif (_SENSOR_OS04D10_ == 1)
        OS04D10_init_driver(_SENSOR1_CHMAP_);
    #endif
#endif

#if defined(_SENSOR_SC830AI_)
    #if (_SENSOR_SC830AI_ == 0)
        SC830AI_init_driver(_SENSOR0_CHMAP_);
    #elif (_SENSOR_SC830AI_ == 1)
        SC830AI_init_driver(_SENSOR1_CHMAP_);
    #endif
#endif
#if defined(_SENSOR_OS02N10_)
    #if (_SENSOR_OS02N10_ == 0)
        OS02N10_init_driver(_SENSOR0_CHMAP_);
    #elif (_SENSOR_OS02N10_ == 1)
        OS02N10_init_driver(_SENSOR1_CHMAP_);
    #endif
#endif
#if defined(_SENSOR_SC2336_)
    #if (_SENSOR_SC2336_ == 0)
        SC2336_init_driver(_SENSOR0_CHMAP_);
    #elif (_SENSOR_SC2336_ == 1)
        SC2336_init_driver(_SENSOR1_CHMAP_);
    #endif
#endif
#if defined(_SENSOR_SC431HAI_)
    #if (_SENSOR_SC431HAI_ == 0)
        SC431HAI_HDR_init_driver(_SENSOR0_CHMAP_);
    #elif (_SENSOR_SC431HAI_ == 1)
        SC431HAI_HDR_init_driver(_SENSOR1_CHMAP_);
    #endif
#endif
#if defined(_SENSOR_SC2336P_)
    #if (_SENSOR_SC2336P_ == 0)
        SC2336P_init_driver(_SENSOR0_CHMAP_);
    #elif (_SENSOR_SC2336P_ == 1)
        SC2336P_init_driver(_SENSOR1_CHMAP_);
    #endif
#endif
#if defined(_SENSOR_OG0VA1B_)
    #if (_SENSOR_OG0VA1B_ == 0)
        OG0VA1B_init_driver(_SENSOR0_CHMAP_);
    #elif (_SENSOR_OG0VA1B_ == 1)
        OG0VA1B_init_driver(_SENSOR1_CHMAP_);
    #endif
#endif
#if defined(_SENSOR_SC535HAI_)
    #if (_SENSOR_SC535HAI_ == 0)
        SC535HAI_init_driver(_SENSOR0_CHMAP_);
    #elif (_SENSOR_SC535HAI_ == 1)
        SC535HAI_init_driver(_SENSOR1_CHMAP_);
    #endif
#endif
#if defined(_SENSOR_SC2356_)
    #if (_SENSOR_SC2356_ == 0)
        SC2356_init_driver(_SENSOR0_CHMAP_);
    #elif (_SENSOR_SC2356_ == 1)
        SC2356_init_driver(_SENSOR1_CHMAP_);
    #endif
#endif
}

void rtk_sensor_module_deinit(void)
{
#if defined(_SENSOR_IMX307_) || defined(_SENSOR_IMX307MCLK36_)
    #if (_SENSOR_IMX307_ == 0)
        IMX307_HDR_deinit_driver(_SENSOR0_CHMAP_);
    #elif (_SENSOR_IMX307_ == 1)
        IMX307_HDR_deinit_driver(_SENSOR1_CHMAP_);
    #endif
#endif

#if defined(_SENSOR_OS02H10_)
    #if (_SENSOR_OS02H10_ == 0)
       OS02H10_deinit_driver(_SENSOR0_CHMAP_);
    #elif (_SENSOR_OS02H10_ == 1)
        OS02H10_deinit_driver(_SENSOR1_CHMAP_);
    #endif
#endif

#if defined(_SENSOR_IMX415_)
    #if (_SENSOR_IMX415_ == 0)
        IMX415_HDR_deinit_driver(_SENSOR0_CHMAP_);
    #elif (_SENSOR_IMX415_ == 1)
        IMX415_HDR_deinit_driver(_SENSOR1_CHMAP_);
    #endif
#endif

#if defined(_SENSOR_IMX675_)
    #if (_SENSOR_IMX675_ == 0)
        IMX675_HDR_deinit_driver(_SENSOR0_CHMAP_);
    #elif (_SENSOR_IMX675_ == 1)
        IMX675_HDR_deinit_driver(_SENSOR1_CHMAP_);
    #endif
#endif

#if defined(_SENSOR_IMX678_)
    #if (_SENSOR_IMX678_ == 0)
        IMX678_HDR_deinit_driver(_SENSOR0_CHMAP_);
    #elif (_SENSOR_IMX678_ == 1)
        IMX678_HDR_deinit_driver(_SENSOR1_CHMAP_);
    #endif
#endif

#if defined(_SENSOR_SC4336_)
    #if (_SENSOR_SC4336_ == 0)
        SC4336_deinit_driver(_SENSOR0_CHMAP_);
    #elif (_SENSOR_SC4336_ == 1)
        SC4336_deinit_driver(_SENSOR1_CHMAP_);
    #endif
#endif

#if defined(_SENSOR_SC4336P_)
    #if (_SENSOR_SC4336P_ == 0)
        SC4336P_deinit_driver(_SENSOR0_CHMAP_);
    #elif (_SENSOR_SC4336P_ == 1)
        SC4336P_deinit_driver(_SENSOR1_CHMAP_);
    #endif
#endif

#if defined(_SENSOR_SC4336PDUALSNRAOV_)
    #if (_SENSOR_SC4336PDUALSNRAOV_ == 0)
        SC4336P_DUALSNR_AOV_deinit_driver(_SENSOR0_CHMAP_);
    #elif (_SENSOR_SC4336PDUALSNRAOV_ == 1)
        SC4336P_DUALSNR_AOV_deinit_driver(_SENSOR1_CHMAP_);
    #endif
#endif

#if defined(_SENSOR_OS04D10_)
    #if (_SENSOR_OS04D10_ == 0)
        OS04D10_deinit_driver(_SENSOR0_CHMAP_);
    #elif (_SENSOR_OS04D10_ == 1)
        OS04D10_deinit_driver(_SENSOR1_CHMAP_);
    #endif
#endif

#if defined(_SENSOR_SC830AI_)
    #if (_SENSOR_SC830AI_ == 0)
        SC830AI_deinit_driver(_SENSOR0_CHMAP_);
    #elif (_SENSOR_SC830AI_ == 1)
        SC830AI_deinit_driver(_SENSOR1_CHMAP_);
    #endif
#endif

#if defined(_SENSOR_IMX664_)
    #if (_SENSOR_IMX664_ == 0)
        IMX664_deinit_driver(_SENSOR0_CHMAP_);
    #elif (_SENSOR_IMX664_ == 1)
        IMX664_deinit_driver(_SENSOR1_CHMAP_);
    #endif
#endif
#if defined(_SENSOR_OS02N10_)
    #if (_SENSOR_OS02N10_ == 0)
        OS02N10_deinit_driver(_SENSOR0_CHMAP_);
    #elif (_SENSOR_OS02N10_ == 1)
        OS02N10_deinit_driver(_SENSOR1_CHMAP_);
    #endif
#endif
#if defined(_SENSOR_SC2336_)
    #if (_SENSOR_SC2336_ == 0)
        SC2336_deinit_driver(_SENSOR0_CHMAP_);
    #elif (_SENSOR_SC2336_ == 1)
        SC2336_deinit_driver(_SENSOR1_CHMAP_);
    #endif
#endif
#if defined(_SENSOR_SC431HAI_)
    #if (_SENSOR_SC431HAI_ == 0)
        SC431HAI_HDR_deinit_driver(_SENSOR0_CHMAP_);
    #elif (_SENSOR_SC431HAI_ == 1)
        SC431HAI_HDR_deinit_driver(_SENSOR1_CHMAP_);
    #endif
#endif
#if defined(_SENSOR_SC2336P_)
    #if (_SENSOR_SC2336P_ == 0)
        SC2336P_deinit_driver(_SENSOR0_CHMAP_);
    #elif (_SENSOR_SC2336P_ == 1)
        SC2336P_deinit_driver(_SENSOR1_CHMAP_);
    #endif
#endif
#if defined(_SENSOR_OG0VA1B_)
    #if (_SENSOR_OG0VA1B_ == 0)
        OG0VA1B_deinit_driver(_SENSOR0_CHMAP_);
    #elif (_SENSOR_OG0VA1B_ == 1)
        OG0VA1B_deinit_driver(_SENSOR1_CHMAP_);
    #endif
#endif
#if defined(_SENSOR_SC535HAI_)
    #if (_SENSOR_SC535HAI_ == 0)
        SC535HAI_deinit_driver(_SENSOR0_CHMAP_);
    #elif (_SENSOR_SC535HAI_ == 1)
        SC535HAI_deinit_driver(_SENSOR1_CHMAP_);
    #endif
#endif
#if defined(_SENSOR_SC2356_)
    #if (_SENSOR_SC2356_ == 0)
        SC2356_deinit_driver(_SENSOR0_CHMAP_);
    #elif (_SENSOR_SC2356_ == 1)
        SC2356_deinit_driver(_SENSOR1_CHMAP_);
    #endif
#endif
}

//rtos_device_initcall(rtk_sensor_module_init);
