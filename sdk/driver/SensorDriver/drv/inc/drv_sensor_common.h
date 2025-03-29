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

#ifndef _SENSOR_OS_WRAPPER_H_
#define _SENSOR_OS_WRAPPER_H_

#if defined(__KERNEL__)
#define CAM_OS_LINUX_KERNEL
#endif

#ifdef SUPPORT_GPL_SYMBOL
#define MHAL_MODULE_LICENSE() MODULE_LICENSE("GPL v2");
#else
#define MHAL_MODULE_LICENSE() MODULE_LICENSE("Proprietary");
#endif

#include <ms_platform.h>
#include <cam_os_wrapper.h>
#include <drv_ss_cus_sensor.h>
#include <drv_sensor.h>

/******************************************************** RTK ***********************************************************/
#if defined(CAM_OS_RTK)
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#ifndef true
#define true                        1
#define false                       0
#endif

#if defined(_SENSOR_SLAVE_ID_)
static volatile int i2c_slave_id = _SENSOR_SLAVE_ID_;
#else
static volatile int i2c_slave_id = 0;
#endif

#define SENSOR_DRV_ENTRY_IMPL_BEGIN(NAME)

#define SENSOR_DRV_ENTRY_IMPL_END(Name,LinearEntry,HdrSefEntry,HdrLefEntry)\
int Name##_init_driver(unsigned char chmap)\
{\
    int nCamID=0;\
    /*To avoid compile warning*/\
    void* p0 = LinearEntry;\
    void* p1 = HdrSefEntry;\
    void* p2 = HdrLefEntry;\
    for(nCamID=0;nCamID<MAX_CAMID_LEN;++nCamID)\
    {\
        if((chmap>>nCamID)&0x1)\
        {\
            if(p0){ DRV_SENSOR_IF_RegisterSensorDriver(nCamID, LinearEntry);\
                DRV_SENSOR_IF_RegisterSensorI2CSlaveID(nCamID, (i2c_slave_id>>(nCamID*8))&0xFF);\
                SENSOR_DMSG("Connect %s linear to sensor pad %d\n",__FUNCTION__, nCamID);\
            }\
            if(p1){ DRV_SENSOR_IF_RegisterPlaneDriver(nCamID, CUS_SENSOR_PLANE_1, HdrSefEntry);\
                DRV_SENSOR_IF_RegisterPlaneI2CSlaveID(nCamID, CUS_SENSOR_PLANE_1, (i2c_slave_id>>(nCamID*8))&0xFF);\
                SENSOR_DMSG("Connect %s SEF to vif sensor pad %d\n",__FUNCTION__, nCamID);\
            }\
            if(p2){ DRV_SENSOR_IF_RegisterPlaneDriver(nCamID, CUS_SENSOR_PLANE_0, HdrLefEntry);\
                DRV_SENSOR_IF_RegisterPlaneI2CSlaveID(nCamID, CUS_SENSOR_PLANE_0, (i2c_slave_id>>(nCamID*8))&0xFF);\
                SENSOR_DMSG("Connect %s LEF to sensor pad %d\n",__FUNCTION__, nCamID);\
            }\
        }\
    }\
    return 0;\
}

static volatile int lane_num = 2;
static volatile int hdr_lane_num=2;

/*
_SENSOR0_RST_POL_
 0:POS
 1:NEG
refer from SENSORx_OPTIONS in alkaid defconfig, parse in libs_sdk.mak,

if want to get _SENSORx_RST_POL_ DEFINE for multi-sensor,
please refer concat method below.
_SENSOR_SC4336P_ is index of sensor.(0,1,2)
e.g.:
#define CONCAT3(a, b, c) a##b##c
#define CONCAT3_EXPANDED(a, b, c) CONCAT3(a, b, c)
CONCAT3_EXPANDED(_SENSOR, _SENSOR_SC4336P_, _RST_POL_)

*/
static volatile int sensor_rst_pol = CUS_CLK_POL_NEG;

#define SENSOR_DRV_PARAM_MCLK() "NONE"
#define SENSOR_USLEEP_(us) {CamOsUsDelay(us);}
#define SENSOR_MSLEEP_(ms) {CamOsMsDelay(ms);}

/*Extension version*/
#define SENSOR_DRV_ENTRY_IMPL_BEGIN_EX(NAME) SENSOR_DRV_ENTRY_IMPL_BEGIN(NAME)

#define MAX_CAMID_LEN 9
#define SENSOR_DRV_ENTRY_IMPL_END_EX(Name,LinearEntry,HdrSefEntry,HdrLefEntry,PrivateDataType)\
static PrivateDataType* g_pData[2][MAX_CAMID_LEN] = {{0,0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0,0}};\
static PrivateDataType g_PrivData[2][MAX_CAMID_LEN];\
int Name##_init_driver(unsigned char chmap)\
{\
    int nCamID=0;\
    /*To avoid compile warning*/\
    void* p0 = LinearEntry;\
    void* p1 = HdrSefEntry;\
    void* p2 = HdrLefEntry;\
    for(nCamID=0;nCamID<MAX_CAMID_LEN;++nCamID)\
    {\
        if((chmap>>nCamID)&0x1)\
        {\
            if(p0){\
                /*void* pData = CamOsMemAlloc(sizeof(PrivateDataType));*/\
                void* pData = &g_PrivData[0][nCamID];/*Change private data to static var*/\
                CamOsMemset(pData,0,sizeof(PrivateDataType));\
                DRV_SENSOR_IF_RegisterSensorDriverEx(nCamID, LinearEntry,pData);\
                DRV_SENSOR_IF_RegisterSensorI2CSlaveID(nCamID, (i2c_slave_id>>(nCamID*8))&0xFF);\
                SENSOR_DMSG("Connect %s linear to sensor pad %d, slavd_id 0x%x\n",__FUNCTION__, nCamID, (i2c_slave_id>>(nCamID*8))&0xFF);\
                g_pData[0][nCamID] = pData;\
            }\
            if(p1||p2){\
                /*void* pData = CamOsMemAlloc(sizeof(PrivateDataType));*/\
                void* pData = &g_PrivData[1][nCamID];/*Change private data to static var*/\
                CamOsMemset(pData,0,sizeof(PrivateDataType));\
                if(p1){\
                    DRV_SENSOR_IF_RegisterPlaneDriverEx(nCamID, CUS_SENSOR_PLANE_1, HdrSefEntry,pData);\
                    DRV_SENSOR_IF_RegisterPlaneI2CSlaveID(nCamID, CUS_SENSOR_PLANE_1, (i2c_slave_id>>(nCamID*8))&0xFF);\
                    SENSOR_DMSG("Connect %s SEF to vif sensor pad %d, slavd_id 0x%x\n",__FUNCTION__, nCamID, (i2c_slave_id>>(nCamID*8))&0xFF);\
                }\
                if(p2){\
                    DRV_SENSOR_IF_RegisterPlaneDriverEx(nCamID, CUS_SENSOR_PLANE_0, HdrLefEntry,pData);\
                    DRV_SENSOR_IF_RegisterPlaneI2CSlaveID(nCamID, CUS_SENSOR_PLANE_0, (i2c_slave_id>>(nCamID*8))&0xFF);\
                    SENSOR_DMSG("Connect %s LEF to sensor pad %d, slavd_id 0x%x\n",__FUNCTION__, nCamID, (i2c_slave_id>>(nCamID*8))&0xFF);\
                }\
                g_pData[1][nCamID] = pData;\
            }\
            DRV_SENSOR_IF_EarlyInitSync(nCamID);\
        }\
    }\
    return 0;\
}\
\
int Name##_deinit_driver(unsigned char chmap)\
{\
    int nCamID=0;\
    for(nCamID=0;nCamID<MAX_CAMID_LEN;++nCamID)\
    {\
        if((chmap>>nCamID)&0x1)\
        {\
            if(g_pData[0][nCamID] || g_pData[1][nCamID])\
            {\
                DrvUnregisterSensorDriverEx(nCamID);\
                if(g_pData[0][nCamID]){\
                    /*CamOsMemRelease((void*)g_pData[0][nCamID]);*/\
                    g_pData[0][nCamID] = 0;\
                }\
                if(g_pData[1][nCamID]){\
                    /*CamOsMemRelease((void*)g_pData[1][nCamID]);*/\
                    g_pData[1][nCamID] = 0;\
                }\
            }\
        }\
    }\
    return 0;\
}

//define 3 frame HDR application impl
#define SENSOR_DRV_ENTRY_IMPL_END_EX2(Name,LinearEntry,HdrSefEntry,HdrLefEntry,HdrMefEntry,PrivateDataType) \
static PrivateDataType* g_pData[2][MAX_CAMID_LEN] = {{0,0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0,0}};\
int Name##_init_driver(unsigned char chmap)\
{\
    int nCamID=0;\
    void* p0 = LinearEntry;\
    void* p1 = HdrSefEntry;\
    void* p2 = HdrLefEntry;\
    void* p3 = HdrMefEntry;\
    MI_SNR_MODPARAMINIT(#Name);\
    PARA_DBG();\
    if(DRV_SENSOR_IF_SensorHandleVer(CUS_CAMSENSOR_HANDLE_MAJ_VER, CUS_CAMSENSOR_HANDLE_MIN_VER)==FAIL)\
    return FAIL;\
    if(DRV_SENSOR_IF_SensorIFVer(CUS_CAMSENSORIF_MAJ_VER, CUS_CAMSENSORIF_MIN_VER)==FAIL)\
    return FAIL;\
    if(DRV_SENSOR_IF_SensorI2CVer(CUS_CAMSENSOR_I2C_MAJ_VER, CUS_CAMSENSOR_I2C_MIN_VER)==FAIL)\
    return FAIL;\
    for(nCamID=0;nCamID<MAX_CAMID_LEN;++nCamID)\
    {\
        if((chmap>>nCamID)&0x1)\
        {\
            if(p0){\
                void* pData = CamOsMemAlloc(sizeof(PrivateDataType));\
                CamOsMemset(pData,0,sizeof(PrivateDataType));\
                DRV_SENSOR_IF_RegisterSensorDriverEx(nCamID, LinearEntry,pData);\
                DRV_SENSOR_IF_RegisterSensorI2CSlaveID(nCamID, (i2c_slave_id>>(nCamID*8))&0xFF);\
                CamOsPrintf("Connect %s linear to sensor pad %d\n",__FUNCTION__, nCamID);\
                g_pData[0][nCamID] = pData;\
            }\
            if(p1||p2||p3){\
                void* pData = CamOsMemAlloc(sizeof(PrivateDataType));\
                CamOsMemset(pData,0,sizeof(PrivateDataType));\
                if(p1){\
                    DRV_SENSOR_IF_RegisterPlaneDriverEx(nCamID, CUS_SENSOR_PLANE_1, HdrSefEntry,pData);\
                    DRV_SENSOR_IF_RegisterPlaneI2CSlaveID(nCamID, CUS_SENSOR_PLANE_1, (i2c_slave_id>>(nCamID*8))&0xFF);\
                    CamOsPrintf("Connect %s SEF to vif sensor pad %d\n",__FUNCTION__, nCamID);\
                }\
                if(p2){\
                    DRV_SENSOR_IF_RegisterPlaneDriverEx(nCamID, CUS_SENSOR_PLANE_0, HdrLefEntry,pData);\
                    DRV_SENSOR_IF_RegisterPlaneI2CSlaveID(nCamID, CUS_SENSOR_PLANE_0, (i2c_slave_id>>(nCamID*8))&0xFF);\
                    CamOsPrintf("Connect %s LEF to sensor pad %d\n",__FUNCTION__, nCamID);\
                }\
                if(p3){\
                    DRV_SENSOR_IF_RegisterPlaneDriverEx(nCamID, CUS_SENSOR_PLANE_2, HdrMefEntry,pData);\
                    DRV_SENSOR_IF_RegisterPlaneI2CSlaveID(nCamID, CUS_SENSOR_PLANE_2, (i2c_slave_id>>(nCamID*8))&0xFF);\
                    CamOsPrintf("Connect %s MEF to sensor pad %d\n",__FUNCTION__, nCamID);\
                }\
                g_pData[1][nCamID] = pData;\
            }\
            DRV_SENSOR_IF_EarlyInitSync(nCamID);\
        }\
    }\
    return 0;\
}\
void Name##_deinit_driver(unsigned char chmap)\
{\
    int nCamID=0;\
    for(nCamID=0;nCamID<MAX_CAMID_LEN;++nCamID)\
    {\
        if((chmap>>nCamID)&0x1)\
        {\
            if(g_pData[0][nCamID] || g_pData[1][nCamID])\
            {\
                DrvUnregisterSensorDriverEx(nCamID);\
                if(g_pData[0][nCamID]){\
                    CamOsMemRelease((void*)g_pData[0][nCamID]);\
                    g_pData[0][nCamID] = 0;\
                }\
                if(g_pData[1][nCamID]){\
                    CamOsMemRelease((void*)g_pData[1][nCamID]);\
                    g_pData[1][nCamID] = 0;\
                }\
            }\
        }\
    }\
}

//define 4 ch AHD application impl
#define SENSOR_DRV_ENTRY_4CHAHD_IMPL_END_EX(Name, LinearEntry, AhdCh0Entry, AhdCh1Entry, AhdCh2Entry, AhdCh3Entry, PrivateDataType)\
static PrivateDataType* g_pData[2][MAX_CAMID_LEN] = {{0,0,0,0}, {0,0,0,0}};\
static PrivateDataType g_Priv4CHData[2][MAX_CAMID_LEN];\
int Name##_init_driver(unsigned char chmap)\
{\
    int nCamID=0;\
    void* p0 = LinearEntry;\
    void* p1 = AhdCh0Entry;\
    void* p2 = AhdCh1Entry;\
    void* p3 = AhdCh2Entry;\
    void* p4 = AhdCh3Entry;\
    for(nCamID=0;nCamID<MAX_CAMID_LEN;++nCamID)\
    {\
        if((chmap>>nCamID)&0x1)\
        {\
            if(p0){\
                /*void* pData = CamOsMemAlloc(sizeof(PrivateDataType));*/\
                void* pData = &g_Priv4CHData[0][nCamID];/*Change private data to static var*/\
                CamOsMemset(pData,0,sizeof(PrivateDataType));\
                DRV_SENSOR_IF_RegisterSensorDriverEx(nCamID, LinearEntry,pData);\
                DRV_SENSOR_IF_RegisterSensorI2CSlaveID(nCamID, (i2c_slave_id>>(nCamID*8))&0xFF);\
                SENSOR_DMSG("Connect %s linear to sensor pad %d\n",__FUNCTION__, nCamID);\
                g_pData[0][nCamID] = pData;\
            }\
            if(p1||p2||p3||p4){\
                /*void* pData = CamOsMemAlloc(sizeof(PrivateDataType));*/\
                void* pData = &g_Priv4CHData[1][nCamID];/*Change private data to static var*/\
                CamOsMemset(pData,0,sizeof(PrivateDataType));\
                if(p1){\
                    DRV_SENSOR_IF_RegisterPlaneDriverEx(nCamID, CUS_SENSOR_PLANE_0, AhdCh0Entry,pData);\
                    DRV_SENSOR_IF_RegisterPlaneI2CSlaveID(nCamID, CUS_SENSOR_PLANE_0, (i2c_slave_id>>(nCamID*8))&0xFF);\
                    SENSOR_DMSG("Connect %s AHD0 to vif sensor pad %d\n",__FUNCTION__, nCamID);\
                }\
                if(p2){\
                    DRV_SENSOR_IF_RegisterPlaneDriverEx(nCamID, CUS_SENSOR_PLANE_1, AhdCh1Entry,pData);\
                    DRV_SENSOR_IF_RegisterPlaneI2CSlaveID(nCamID, CUS_SENSOR_PLANE_1, (i2c_slave_id>>(nCamID*8))&0xFF);\
                    SENSOR_DMSG("Connect %s AHD1 to sensor pad %d\n",__FUNCTION__, nCamID);\
                }\
                if(p3){\
                    DRV_SENSOR_IF_RegisterPlaneDriverEx(nCamID, CUS_SENSOR_PLANE_2, AhdCh2Entry,pData);\
                    DRV_SENSOR_IF_RegisterPlaneI2CSlaveID(nCamID, CUS_SENSOR_PLANE_2, (i2c_slave_id>>(nCamID*8))&0xFF);\
                    SENSOR_DMSG("Connect %s AHD2 to sensor pad %d\n",__FUNCTION__, nCamID);\
                }\
                if(p4){\
                    DRV_SENSOR_IF_RegisterPlaneDriverEx(nCamID, CUS_SENSOR_PLANE_3, AhdCh3Entry,pData);\
                    DRV_SENSOR_IF_RegisterPlaneI2CSlaveID(nCamID, CUS_SENSOR_PLANE_3, (i2c_slave_id>>(nCamID*8))&0xFF);\
                    SENSOR_DMSG("Connect %s AHD3 to sensor pad %d\n",__FUNCTION__, nCamID);\
                }\
                g_pData[1][nCamID] = pData;\
            }\
            DrvSensorEarlyInitSync(nCamID);\
        }\
    }\
    return 0;\
}\
\
int Name##_deinit_driver(unsigned char chmap)\
{\
    int nCamID=0;\
    for(nCamID=0;nCamID<MAX_CAMID_LEN;++nCamID)\
    {\
        if((chmap>>nCamID)&0x1)\
        {\
            if(g_pData[0][nCamID] || g_pData[1][nCamID])\
            {\
                DrvUnregisterSensorDriverEx(nCamID);\
                if(g_pData[0][nCamID]){\
                    /*CamOsMemRelease((void*)g_pData[0][nCamID]);*/\
                    g_pData[0][nCamID] = 0;\
                }\
                if(g_pData[1][nCamID]){\
                    /*CamOsMemRelease((void*)g_pData[1][nCamID]);*/\
                    g_pData[1][nCamID] = 0;\
                }\
            }\
        }\
    }\
    return 0;\
}

/******************************************************** LINUX ***********************************************************/
#elif defined(CAM_OS_LINUX_KERNEL)
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/delay.h>
#define SENSOR_DRV_ENTRY_IMPL_BEGIN(Name) \
int chmap = 0;\
module_param(chmap, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);\
MODULE_PARM_DESC(chmap, "VIF channel mapping");\
char *mclk = "use default parameter";\
module_param(mclk, charp, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);\
MODULE_PARM_DESC(mclk, "Assign MCLK");
int lane_num = 2;\
module_param(lane_num, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);\
MODULE_PARM_DESC(lane_num, "sensor output lane number");
int hdr_lane_num = 4;\
module_param(hdr_lane_num, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);\
MODULE_PARM_DESC(hdr_lane_num, "sensor output lane number");
ulong i2c_slave_id = 0;\
module_param(i2c_slave_id, ulong, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);\
MODULE_PARM_DESC(i2c_slave_id, "sensor i2c slave id");
int mipi_user_def = 0;\
module_param(mipi_user_def, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);\
MODULE_PARM_DESC(mipi_user_def, "sony mipi sensor user_def 1: PDAF; 2 Gyro");
int sensor_rst_pol = CUS_CLK_POL_NEG;\
module_param(sensor_rst_pol, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);\
MODULE_PARM_DESC(sensor_rst_pol, "sensor reset pin level which can make sensor in reset state");

#define SENSOR_DRV_ENTRY_IMPL_END(Name,LinearEntry,HdrSefEntry,HdrLefEntry) \
static int __init Name##_init_driver(void)\
{\
    int nCamID=0;\
    void* p0 = LinearEntry;\
    void* p1 = HdrSefEntry;\
    void* p2 = HdrLefEntry;\
    MI_SNR_MODPARAMINIT(#Name);\
    PARA_DBG();\
    if(DRV_SENSOR_IF_SensorHandleVer(CUS_CAMSENSOR_HANDLE_MAJ_VER, CUS_CAMSENSOR_HANDLE_MIN_VER)==FAIL)\
    return FAIL;\
    if(DRV_SENSOR_IF_SensorIFVer(CUS_CAMSENSORIF_MAJ_VER, CUS_CAMSENSORIF_MIN_VER)==FAIL)\
    return FAIL;\
    if(DRV_SENSOR_IF_SensorI2CVer(CUS_CAMSENSOR_I2C_MAJ_VER, CUS_CAMSENSOR_I2C_MIN_VER)==FAIL)\
    return FAIL;\
    for(nCamID=0;nCamID<MAX_CAMID_LEN;++nCamID)\
    {\
        if((chmap>>nCamID)&0x1)\
        {\
            if(p0){ DRV_SENSOR_IF_RegisterSensorDriver(nCamID, LinearEntry);\
            DRV_SENSOR_IF_RegisterSensorI2CSlaveID(nCamID, (i2c_slave_id>>(nCamID*8))&0xFF);\
            pr_info("Connect %s linear to sensor pad %d\n",__FUNCTION__, nCamID);\
            }\
            if(p1){ DRV_SENSOR_IF_RegisterPlaneDriver(nCamID, CUS_SENSOR_PLANE_1, HdrSefEntry);\
            DRV_SENSOR_IF_RegisterPlaneI2CSlaveID(nCamID, CUS_SENSOR_PLANE_1, (i2c_slave_id>>(nCamID*8))&0xFF);\
            pr_info("Connect %s SEF to vif sensor pad %d\n",__FUNCTION__, nCamID);\
            }\
            if(p2){ DRV_SENSOR_IF_RegisterPlaneDriver(nCamID, CUS_SENSOR_PLANE_0, HdrLefEntry);\
            DRV_SENSOR_IF_RegisterPlaneI2CSlaveID(nCamID, CUS_SENSOR_PLANE_0, (i2c_slave_id>>(nCamID*8))&0xFF);\
            pr_info("Connect %s LEF to sensor pad %d\n",__FUNCTION__, nCamID);\
            }\
        }\
    }\
    return 0;\
}\
static void __exit Name##_exit_driver(void)\
{\
    pr_info("sensordrv exit");\
}\
subsys_initcall(Name##_init_driver);\
module_exit(Name##_exit_driver);\
MODULE_DESCRIPTION("Sensor_"#Name);\
MODULE_AUTHOR("SigmaStar");\
MHAL_MODULE_LICENSE()

#define SENSOR_DRV_PARAM_MCLK() (mclk)
#define SENSOR_USLEEP_(us) CamOsUsSleep(us)
#define SENSOR_MSLEEP_(ms) CamOsMsSleep(ms)

/*Extension version*/
#define SENSOR_DRV_ENTRY_IMPL_BEGIN_EX(Name) SENSOR_DRV_ENTRY_IMPL_BEGIN(Name)

#define MAX_CAMID_LEN 9
#define SENSOR_DRV_ENTRY_IMPL_END_EX(Name,LinearEntry,HdrSefEntry,HdrLefEntry,PrivateDataType) \
static PrivateDataType* g_pData[2][MAX_CAMID_LEN] = {{0,0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0,0}};\
static int __init Name##_init_driver(void)\
{\
    int nCamID=0;\
    void* p0 = LinearEntry;\
    void* p1 = HdrSefEntry;\
    void* p2 = HdrLefEntry;\
    MI_SNR_MODPARAMINIT(#Name);\
    PARA_DBG();\
    if(DRV_SENSOR_IF_SensorHandleVer(CUS_CAMSENSOR_HANDLE_MAJ_VER, CUS_CAMSENSOR_HANDLE_MIN_VER)==FAIL)\
    return FAIL;\
    if(DRV_SENSOR_IF_SensorIFVer(CUS_CAMSENSORIF_MAJ_VER, CUS_CAMSENSORIF_MIN_VER)==FAIL)\
    return FAIL;\
    if(DRV_SENSOR_IF_SensorI2CVer(CUS_CAMSENSOR_I2C_MAJ_VER, CUS_CAMSENSOR_I2C_MIN_VER)==FAIL)\
    return FAIL;\
    for(nCamID=0;nCamID<MAX_CAMID_LEN;++nCamID)\
    {\
        if((chmap>>nCamID)&0x1)\
        {\
            if(p0){\
                void* pData = CamOsMemAlloc(sizeof(PrivateDataType));\
                CamOsMemset(pData,0,sizeof(PrivateDataType));\
                DRV_SENSOR_IF_RegisterSensorDriverEx(nCamID, LinearEntry,pData);\
                DRV_SENSOR_IF_RegisterSensorI2CSlaveID(nCamID, (i2c_slave_id>>(nCamID*8))&0xFF);\
                pr_info("Connect %s linear to sensor pad %d\n",__FUNCTION__, nCamID);\
                g_pData[0][nCamID] = pData;\
            }\
            if(p1||p2){\
                void* pData = CamOsMemAlloc(sizeof(PrivateDataType));\
                CamOsMemset(pData,0,sizeof(PrivateDataType));\
                if(p1){\
                    DRV_SENSOR_IF_RegisterPlaneDriverEx(nCamID, CUS_SENSOR_PLANE_1, HdrSefEntry,pData);\
                    DRV_SENSOR_IF_RegisterPlaneI2CSlaveID(nCamID, CUS_SENSOR_PLANE_1, (i2c_slave_id>>(nCamID*8))&0xFF);\
                    pr_info("Connect %s SEF to vif sensor pad %d\n",__FUNCTION__, nCamID);\
                }\
                if(p2){\
                    DRV_SENSOR_IF_RegisterPlaneDriverEx(nCamID, CUS_SENSOR_PLANE_0, HdrLefEntry,pData);\
                    DRV_SENSOR_IF_RegisterPlaneI2CSlaveID(nCamID, CUS_SENSOR_PLANE_0, (i2c_slave_id>>(nCamID*8))&0xFF);\
                    pr_info("Connect %s LEF to sensor pad %d\n",__FUNCTION__, nCamID);\
                }\
                g_pData[1][nCamID] = pData;\
            }\
        }\
    }\
    return 0;\
}\
static void __exit Name##_exit_driver(void)\
{\
    int nCamID=0;\
    for(nCamID=0;nCamID<MAX_CAMID_LEN;++nCamID)\
    {\
        if((chmap>>nCamID)&0x1)\
        {\
            if(g_pData[0][nCamID] || g_pData[1][nCamID])\
            {\
                DrvUnregisterSensorDriverEx(nCamID);\
                if(g_pData[0][nCamID]){\
                    CamOsMemRelease((void*)g_pData[0][nCamID]);\
                    g_pData[0][nCamID] = 0;\
                }\
                if(g_pData[1][nCamID]){\
                    CamOsMemRelease((void*)g_pData[1][nCamID]);\
                    g_pData[1][nCamID] = 0;\
                }\
            }\
        }\
    }\
}\
subsys_initcall(Name##_init_driver);\
module_exit(Name##_exit_driver);\
MODULE_DESCRIPTION("Sensor_"#Name);\
MODULE_AUTHOR("SigmaStar");\
MHAL_MODULE_LICENSE()

#define SENSOR_DRV_ENTRY_IMPL_END_EX2(Name,LinearEntry,HdrSefEntry,HdrLefEntry,HdrMefEntry,PrivateDataType) \
static PrivateDataType* g_pData[2][MAX_CAMID_LEN] = {{0,0,0,0}, {0,0,0,0}};\
static int __init Name##_init_driver(void)\
{\
    int nCamID=0;\
    void* p0 = LinearEntry;\
    void* p1 = HdrSefEntry;\
    void* p2 = HdrLefEntry;\
    void* p3 = HdrMefEntry;\
    if(DRV_SENSOR_IF_SensorHandleVer(CUS_CAMSENSOR_HANDLE_MAJ_VER, CUS_CAMSENSOR_HANDLE_MIN_VER)==FAIL)\
    return FAIL;\
    if(DRV_SENSOR_IF_SensorIFVer(CUS_CAMSENSORIF_MAJ_VER, CUS_CAMSENSORIF_MIN_VER)==FAIL)\
    return FAIL;\
    if(DRV_SENSOR_IF_SensorI2CVer(CUS_CAMSENSOR_I2C_MAJ_VER, CUS_CAMSENSOR_I2C_MIN_VER)==FAIL)\
    return FAIL;\
    for(nCamID=0;nCamID<MAX_CAMID_LEN;++nCamID)\
    {\
        if((chmap>>nCamID)&0x1)\
        {\
            if(p0){\
                void* pData = CamOsMemAlloc(sizeof(PrivateDataType));\
                CamOsMemset(pData,0,sizeof(PrivateDataType));\
                DRV_SENSOR_IF_RegisterSensorDriverEx(nCamID, LinearEntry,pData);\
                DRV_SENSOR_IF_RegisterSensorI2CSlaveID(nCamID, (i2c_slave_id>>(nCamID*8))&0xFF);\
                pr_info("Connect %s linear to sensor pad %d\n",__FUNCTION__, nCamID);\
                g_pData[0][nCamID] = pData;\
            }\
            if(p1||p2||p3){\
                void* pData = CamOsMemAlloc(sizeof(PrivateDataType));\
                CamOsMemset(pData,0,sizeof(PrivateDataType));\
                if(p1){\
                    DRV_SENSOR_IF_RegisterPlaneDriverEx(nCamID, CUS_SENSOR_PLANE_1, HdrSefEntry,pData);\
                    DRV_SENSOR_IF_RegisterPlaneI2CSlaveID(nCamID, CUS_SENSOR_PLANE_1, (i2c_slave_id>>(nCamID*8))&0xFF);\
                    pr_info("Connect %s LEF to sensor pad %d\n",__FUNCTION__, nCamID);\
                }\
                if(p2){\
                    DRV_SENSOR_IF_RegisterPlaneDriverEx(nCamID, CUS_SENSOR_PLANE_0, HdrLefEntry,pData);\
                    DRV_SENSOR_IF_RegisterPlaneI2CSlaveID(nCamID, CUS_SENSOR_PLANE_0, (i2c_slave_id>>(nCamID*8))&0xFF);\
                    pr_info("Connect %s MEF to sensor pad %d\n",__FUNCTION__, nCamID);\
                }\
                if(p3){\
                    DRV_SENSOR_IF_RegisterPlaneDriverEx(nCamID, CUS_SENSOR_PLANE_2, HdrMefEntry,pData);\
                    DRV_SENSOR_IF_RegisterPlaneI2CSlaveID(nCamID, CUS_SENSOR_PLANE_2, (i2c_slave_id>>(nCamID*8))&0xFF);\
                    pr_info("Connect %s SEF to vif sensor pad %d\n",__FUNCTION__, nCamID);\
                }\
                g_pData[1][nCamID] = pData;\
            }\
        }\
    }\
    return 0;\
}\
static void __exit Name##_exit_driver(void)\
{\
    int nCamID=0;\
    for(nCamID=0;nCamID<MAX_CAMID_LEN;++nCamID)\
    {\
        if((chmap>>nCamID)&0x1)\
        {\
            if(g_pData[0][nCamID])\
            {\
                DrvUnregisterSensorDriverEx(nCamID);\
                if(g_pData[0][nCamID]){\
                    CamOsMemRelease((void*)g_pData[0][nCamID]);\
                    g_pData[0][nCamID] = 0;\
                }\
                if(g_pData[1][nCamID]){\
                    CamOsMemRelease((void*)g_pData[1][nCamID]);\
                    g_pData[1][nCamID] = 0;\
                }\
            }\
        }\
    }\
}\
subsys_initcall(Name##_init_driver);\
module_exit(Name##_exit_driver);\
MODULE_DESCRIPTION("Sensor_"#Name);\
MODULE_AUTHOR("SigmaStar");\
MHAL_MODULE_LICENSE()

//define 3 ch AHD application impl
#define SENSOR_DRV_ENTRY_3CHAHD_IMPL_END_EX(Name,LinearEntry,AhdCh0Entry,AhdCh1Entry, AhdCh2Entry,PrivateDataType)\
static PrivateDataType* g_pData[2][MAX_CAMID_LEN] = {{0,0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0,0}};\
static int __init Name##_init_driver(void)\
{\
    int nCamID=0;\
    void* p0 = LinearEntry;\
    void* p1 = AhdCh0Entry;\
    void* p2 = AhdCh1Entry;\
    void* p3 = AhdCh2Entry;\
    MI_SNR_MODPARAMINIT(#Name);\
    PARA_DBG();\
    if(DRV_SENSOR_IF_SensorHandleVer(CUS_CAMSENSOR_HANDLE_MAJ_VER, CUS_CAMSENSOR_HANDLE_MIN_VER)==FAIL)\
    return FAIL;\
    if(DRV_SENSOR_IF_SensorIFVer(CUS_CAMSENSORIF_MAJ_VER, CUS_CAMSENSORIF_MIN_VER)==FAIL)\
    return FAIL;\
    if(DRV_SENSOR_IF_SensorI2CVer(CUS_CAMSENSOR_I2C_MAJ_VER, CUS_CAMSENSOR_I2C_MIN_VER)==FAIL)\
    return FAIL;\
    for(nCamID=0;nCamID<MAX_CAMID_LEN;++nCamID)\
    {\
        if((chmap>>nCamID)&0x1)\
        {\
            if(p0){\
                void* pData = CamOsMemAlloc(sizeof(PrivateDataType));\
                CamOsMemset(pData,0,sizeof(PrivateDataType));\
                DRV_SENSOR_IF_RegisterSensorDriverEx(nCamID, LinearEntry,pData);\
                DRV_SENSOR_IF_RegisterSensorI2CSlaveID(nCamID, (i2c_slave_id>>(nCamID*8))&0xFF);\
                pr_info("Connect %s linear to sensor pad %d\n",__FUNCTION__, nCamID);\
                g_pData[0][nCamID] = pData;\
            }\
            if(p1||p2||p3){\
                void* pData = CamOsMemAlloc(sizeof(PrivateDataType));\
                CamOsMemset(pData,0,sizeof(PrivateDataType));\
                if(p1){\
                    DRV_SENSOR_IF_RegisterPlaneDriverEx(nCamID, CUS_SENSOR_PLANE_0, AhdCh0Entry,pData);\
                    DRV_SENSOR_IF_RegisterPlaneI2CSlaveID(nCamID, CUS_SENSOR_PLANE_0, (i2c_slave_id>>(nCamID*8))&0xFF);\
                    pr_info("Connect %s AHD0 to vif sensor pad %d\n",__FUNCTION__, nCamID);\
                }\
                if(p2){\
                    DRV_SENSOR_IF_RegisterPlaneDriverEx(nCamID, CUS_SENSOR_PLANE_1, AhdCh1Entry,pData);\
                    DRV_SENSOR_IF_RegisterPlaneI2CSlaveID(nCamID, CUS_SENSOR_PLANE_1, (i2c_slave_id>>(nCamID*8))&0xFF);\
                    pr_info("Connect %s AHD1 to sensor pad %d\n",__FUNCTION__, nCamID);\
                }\
                if(p3){\
                    DRV_SENSOR_IF_RegisterPlaneDriverEx(nCamID, CUS_SENSOR_PLANE_2, AhdCh2Entry,pData);\
                    DRV_SENSOR_IF_RegisterPlaneI2CSlaveID(nCamID, CUS_SENSOR_PLANE_2, (i2c_slave_id>>(nCamID*8))&0xFF);\
                    pr_info("Connect %s AHD2 to sensor pad %d\n",__FUNCTION__, nCamID);\
                }\
                g_pData[1][nCamID] = pData;\
            }\
        }\
    }\
    return 0;\
}\
static void __exit Name##_exit_driver(void)\
{\
    int nCamID=0;\
    for(nCamID=0;nCamID<MAX_CAMID_LEN;++nCamID)\
    {\
        if((chmap>>nCamID)&0x1)\
        {\
            if(g_pData[0][nCamID] || g_pData[1][nCamID])\
            {\
                DrvUnregisterSensorDriverEx(nCamID);\
                if(g_pData[0][nCamID]){\
                    CamOsMemRelease((void*)g_pData[0][nCamID]);\
                    g_pData[0][nCamID] = 0;\
                }\
                if(g_pData[1][nCamID]){\
                    CamOsMemRelease((void*)g_pData[1][nCamID]);\
                    g_pData[1][nCamID] = 0;\
                }\
            }\
        }\
    }\
}\
subsys_initcall(Name##_init_driver);\
module_exit(Name##_exit_driver);\
MODULE_DESCRIPTION("Sensor_"#Name);\
MODULE_AUTHOR("SigmaStar");\
MHAL_MODULE_LICENSE()

//define 4 ch AHD application impl
#define SENSOR_DRV_ENTRY_4CHAHD_IMPL_END_EX(Name, LinearEntry, AhdCh0Entry, AhdCh1Entry, AhdCh2Entry, AhdCh3Entry, PrivateDataType)\
static PrivateDataType* g_pData[2][MAX_CAMID_LEN] = {{0,0,0,0}, {0,0,0,0}};\
static int __init Name##_init_driver(void)\
{\
    int nCamID=0;\
    void* p0 = LinearEntry;\
    void* p1 = AhdCh0Entry;\
    void* p2 = AhdCh1Entry;\
    void* p3 = AhdCh2Entry;\
    void* p4 = AhdCh3Entry;\
    MI_SNR_MODPARAMINIT(#Name);\
    PARA_DBG();\
    if(DRV_SENSOR_IF_SensorHandleVer(CUS_CAMSENSOR_HANDLE_MAJ_VER, CUS_CAMSENSOR_HANDLE_MIN_VER)==FAIL)\
    return FAIL;\
    if(DRV_SENSOR_IF_SensorIFVer(CUS_CAMSENSORIF_MAJ_VER, CUS_CAMSENSORIF_MIN_VER)==FAIL)\
    return FAIL;\
    if(DRV_SENSOR_IF_SensorI2CVer(CUS_CAMSENSOR_I2C_MAJ_VER, CUS_CAMSENSOR_I2C_MIN_VER)==FAIL)\
    return FAIL;\
    for(nCamID=0;nCamID<MAX_CAMID_LEN;++nCamID)\
    {\
        if((chmap>>nCamID)&0x1)\
        {\
            if(p0){\
                void* pData = CamOsMemAlloc(sizeof(PrivateDataType));\
                CamOsMemset(pData,0,sizeof(PrivateDataType));\
                DRV_SENSOR_IF_RegisterSensorDriverEx(nCamID, LinearEntry,pData);\
                DRV_SENSOR_IF_RegisterSensorI2CSlaveID(nCamID, (i2c_slave_id>>(nCamID*8))&0xFF);\
                pr_info("Connect %s linear to sensor pad %d\n",__FUNCTION__, nCamID);\
                g_pData[0][nCamID] = pData;\
            }\
            if(p1||p2||p3||p4){\
                void* pData = CamOsMemAlloc(sizeof(PrivateDataType));\
                CamOsMemset(pData,0,sizeof(PrivateDataType));\
                if(p1){\
                    DRV_SENSOR_IF_RegisterPlaneDriverEx(nCamID, CUS_SENSOR_PLANE_0, AhdCh0Entry,pData);\
                    DRV_SENSOR_IF_RegisterPlaneI2CSlaveID(nCamID, CUS_SENSOR_PLANE_0, (i2c_slave_id>>(nCamID*8))&0xFF);\
                    pr_info("Connect %s AHD0 to vif sensor pad %d\n",__FUNCTION__, nCamID);\
                }\
                if(p2){\
                    DRV_SENSOR_IF_RegisterPlaneDriverEx(nCamID, CUS_SENSOR_PLANE_1, AhdCh1Entry,pData);\
                    DRV_SENSOR_IF_RegisterPlaneI2CSlaveID(nCamID, CUS_SENSOR_PLANE_1, (i2c_slave_id>>(nCamID*8))&0xFF);\
                    pr_info("Connect %s AHD1 to sensor pad %d\n",__FUNCTION__, nCamID);\
                }\
                if(p3){\
                    DRV_SENSOR_IF_RegisterPlaneDriverEx(nCamID, CUS_SENSOR_PLANE_2, AhdCh2Entry,pData);\
                    DRV_SENSOR_IF_RegisterPlaneI2CSlaveID(nCamID, CUS_SENSOR_PLANE_2, (i2c_slave_id>>(nCamID*8))&0xFF);\
                    pr_info("Connect %s AHD2 to sensor pad %d\n",__FUNCTION__, nCamID);\
                }\
                if(p4){\
                    DRV_SENSOR_IF_RegisterPlaneDriverEx(nCamID, CUS_SENSOR_PLANE_3, AhdCh3Entry,pData);\
                    DRV_SENSOR_IF_RegisterPlaneI2CSlaveID(nCamID, CUS_SENSOR_PLANE_3, (i2c_slave_id>>(nCamID*8))&0xFF);\
                    pr_info("Connect %s AHD3 to sensor pad %d\n",__FUNCTION__, nCamID);\
                }\
                g_pData[1][nCamID] = pData;\
            }\
        }\
    }\
    return 0;\
}\
static void __exit Name##_exit_driver(void)\
{\
    int nCamID=0;\
    for(nCamID=0;nCamID<MAX_CAMID_LEN;++nCamID)\
    {\
        if((chmap>>nCamID)&0x1)\
        {\
            if(g_pData[0][nCamID] || g_pData[1][nCamID])\
            {\
                DrvUnregisterSensorDriverEx(nCamID);\
                if(g_pData[0][nCamID]){\
                    CamOsMemRelease((void*)g_pData[0][nCamID]);\
                    g_pData[0][nCamID] = 0;\
                }\
                if(g_pData[1][nCamID]){\
                    CamOsMemRelease((void*)g_pData[1][nCamID]);\
                    g_pData[1][nCamID] = 0;\
                }\
            }\
        }\
    }\
}\
subsys_initcall(Name##_init_driver);\
module_exit(Name##_exit_driver);\
MODULE_DESCRIPTION("Sensor_"#Name);\
MODULE_AUTHOR("SigmaStar");\
MHAL_MODULE_LICENSE()

#endif //end of CAM_OS_LINUX_KERNEL


#endif //endif _SENSOR_OS_WRAPPER_H_
