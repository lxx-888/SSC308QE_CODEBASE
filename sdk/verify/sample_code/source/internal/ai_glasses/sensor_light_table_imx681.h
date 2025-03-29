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
#ifndef __SENSOR_LIGHT_TABLE_IMX681__
#define __SENSOR_LIGHT_TABLE_IMX681__
#include "st_common_ai_glasses.h"


const static ST_Common_SensorLightTable_t g_stLightTable[2][10] = {
    // capture
    {
        {1,50000,10694,1024},       //lv4
        {3,50000,7096,1024},        //lv5
        {7,50000,4168,1024},        //lv6
        {15,50000,2360,1024},       //lv7
        {31,50000,1304,1024},       //lv8
        {62,34171,1025,1024},       //lv9
        {129,18216,1025,1024},      //lv10
        {252,10482,1025,1025},      //lv11
        {448,5762,1025,1024},       //lv12
        {1638,3169,1025,1025},      //lv13
        //{1638,1729,1025,1024},    //lv14
    },
    // record
    {
        {1,20000,23396,1024},       //lv4
        {3,20000,12706,1024},       //lv5
        {7,20000,7082,1024},        //lv6
        {15,20000,4000,1024},       //lv7
        {31,20000,2222,1024},       //lv8
        {62,20000,1312,1024},       //lv9
        {128,10000,1458,1024},      //lv10
        {253,8074,1025,1024},       //lv11
        {448,4388,1025,1024},       //lv12
        {1638,2369,1025,1024},      //lv13
        //{1638,1330,1025,1024},    //lv14
    }
};

#endif  // __SENSOR_LIGHT_TABLE_IMX681__
