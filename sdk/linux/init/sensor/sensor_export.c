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

#include <linux/module.h>
#include "drv_ss_cus_sensor.h"
#include "drv_ss_cus_vcm.h"
#include <cam_os_wrapper.h>

// mi_sensor_internal
struct mi_sensor_internal_apis_t;
extern struct mi_sensor_internal_apis_t g_mi_sensor_internal_apis;

s32 DRV_SENSOR_IF_RegisterSensorI2CSlaveID(u32 nSNRPadID, u32 Slaveid);
s32 DRV_SENSOR_IF_RegisterPlaneI2CSlaveID(u32 nSNRPadID, u32 nPlaneID, u32 Slaveid);
s32 DRV_SENSOR_IF_SensorHandleVer(u32 version_major, u32 version_minor);
s32 DRV_SENSOR_IF_SensorIFVer(u32 version_major, u32 version_minor);
s32 DRV_SENSOR_IF_SensorI2CVer(u32 version_major, u32 version_minor);
s32 DRV_SENSOR_IF_RegisterSensorDriver(u32 nSNRPadID, SensorInitHandle pfnSensorInitHandle);
s32 DRV_SENSOR_IF_RegisterPlaneDriver(u32 nSNRPadID, u32 nPlaneID, SensorInitHandle pfnSensorInitHandle);
s32 DRV_SENSOR_IF_RegisterSensorDriverEx(u32 nSNRPadID, SensorInitHandle pfnSensorInitHandle, void *pPrivateData);
s32 DRV_SENSOR_IF_RegisterPlaneDriverEx(u32 nSNRPadID, u32 nPlaneID, SensorInitHandle pfnSensorInitHandle);
s32 DRV_SENSOR_IF_Release(u32 nSNRPadID);
u64 EXT_log_2(u32 value);
u64 intlog10(u32 value);
u32 round_float(u32 x, u32 piont_offset);
u64 base2_exp_float_pow(u64 x);
s32 DRV_SENSOR_IF_RegisterVcmDriver(u32 nSNRPadID, VcmInitHandle pfnVcmInitHandle);
s32 DRV_SENSOR_IF_ReleaseVcmDriver(u32 nCamID);

#ifndef COMBINE_MI_MODULE
EXPORT_SYMBOL(g_mi_sensor_internal_apis);
#endif
EXPORT_SYMBOL(DRV_SENSOR_IF_RegisterSensorI2CSlaveID);     //called by sensor drvier
EXPORT_SYMBOL(DRV_SENSOR_IF_RegisterPlaneI2CSlaveID);
EXPORT_SYMBOL(DRV_SENSOR_IF_SensorHandleVer);
EXPORT_SYMBOL(DRV_SENSOR_IF_SensorIFVer);
EXPORT_SYMBOL(DRV_SENSOR_IF_SensorI2CVer);
EXPORT_SYMBOL(DRV_SENSOR_IF_RegisterSensorDriver);
EXPORT_SYMBOL(DRV_SENSOR_IF_RegisterPlaneDriver);
EXPORT_SYMBOL(DRV_SENSOR_IF_RegisterSensorDriverEx);
EXPORT_SYMBOL(DRV_SENSOR_IF_RegisterPlaneDriverEx);
EXPORT_SYMBOL(DRV_SENSOR_IF_Release);

EXPORT_SYMBOL(EXT_log_2);
EXPORT_SYMBOL(intlog10);
EXPORT_SYMBOL(round_float);
EXPORT_SYMBOL(base2_exp_float_pow);
EXPORT_SYMBOL(DRV_SENSOR_IF_RegisterVcmDriver);
EXPORT_SYMBOL(DRV_SENSOR_IF_ReleaseVcmDriver);

