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

#ifndef DRV_SENSOR_H
#define DRV_SENSOR_H

#include <cam_os_wrapper.h>
#define SENSOR_DMSG(args...) do{}while(0)
#define SENSOR_EMSG(args...) CamOsPrintf(args)
#define SENSOR_IMSG(args...) CamOsPrintf(args)
#if 0
typedef enum {
    I2C_FMT_A8D8, /**< 8 bits Address, 8 bits Data */
    I2C_FMT_A16D8,/**< 16 bits Address 8 bits Data */
    I2C_FMT_A8D16,/**< 8 bits Address 16 bits Data */
    I2C_FMT_A16D16,/**< 16 bits Address 16 bits Data */
    I2C_FMT_END/**< Reserved */
} ISP_I2C_FMT;
#endif

//get param for android
/* The cJSON structure: */
typedef struct cJSON
{
    /* next/prev allow you to walk array/object chains. Alternatively, use GetArraySize/GetArrayItem/GetObjectItem */
    struct cJSON *next;
    struct cJSON *prev;
    /* An array or object item will have a child pointer pointing to a chain of the items in the array/object. */
    struct cJSON *child;

    /* The type of the item, as above. */
    int type;

    /* The item's string, if type==cJSON_String  and type == cJSON_Raw */
    char *valuestring;
    /* writing to valueint is DEPRECATED, use cJSON_SetNumberValue instead */
    int valueint;
    /* The item's number, if type==cJSON_Number */
    double valuedouble;

    /* The item's name string, if this item is the child of, or is in the list of subitems of an object. */
    char *string;
} cJSON;


extern struct cJSON*  g_ModParam_cJSON;
cJSON* mi_common_CjsonGetObjectItem(const cJSON * const object, const char * const string);
int mi_common_CjsonGetArraySize(const cJSON *array);
cJSON* mi_common_CjsonGetArrayItem(const cJSON *array, int index);



#define MI_COMMON_MODPARAM_GETVALUE(modid, name, type, pval)                \
({                                                                          \
    int __ret = 0;                                                          \
    do                                                                      \
    {                                                                       \
        cJSON *item_mod = NULL;                                             \
        cJSON *item_v = NULL;                                               \
                                                                            \
        if(g_ModParam_cJSON == NULL)                                        \
        {                                                                   \
        pr_info("Get g_ModParam_cJSON non exist\n");                        \
        CamOsPrintf("Get g_ModParam_cJSON non exist\n");                    \
            __ret = -1;                                                     \
            break;                                                          \
        }                                                                   \
                                                                            \
        item_mod = mi_common_CjsonGetObjectItem(g_ModParam_cJSON, #modid);  \
        if(!item_mod)                                                       \
        {                                                                   \
            __ret = -1;                                                     \
            break;                                                          \
        }                                                                   \
                                                                            \
        item_v = mi_common_CjsonGetObjectItem(item_mod, #name);             \
        if(!item_v)                                                         \
        {                                                                   \
            __ret = -1;                                                     \
            break;                                                          \
        }                                                                   \
                                                                            \
        *(type *)pval = (type)item_v->valueint;                             \
                                                                            \
    } while(0);                                                             \
                                                                            \
    __ret;                                                                  \
})

#define MI_COMMON_MODPARAM_GETARRAY_STRING(modid, name, size, pval, prealsize)          \
        ({                                                                              \
            int __ret    = 0;                                                           \
            do                                                                          \
            {                                                                           \
                int __i      = 0;                                                       \
                int __count  = 0;                                                       \
                cJSON *item_mod = NULL;                                                 \
                cJSON *item_a   = NULL;                                                 \
                cJSON *item_tmp = NULL;                                                 \
                                                                                        \
                if(g_ModParam_cJSON == NULL)                                            \
                {                                                                       \
                    CamOsPrintf("Get g_ModParam_cJSON non exist\n");                    \
                    __ret = -1;                                                         \
                    break;                                                              \
                }                                                                       \
                                                                                        \
                item_mod = mi_common_CjsonGetObjectItem(g_ModParam_cJSON, #modid);      \
                if(!item_mod)                                                           \
                {                                                                       \
                    __ret = -1;                                                         \
                    break;                                                              \
                }                                                                       \
                                                                                        \
                item_a = mi_common_CjsonGetObjectItem(item_mod, #name);                 \
                if(!item_a)                                                             \
                {                                                                       \
                    __ret = -1;                                                         \
                    break;                                                              \
                }                                                                       \
                                                                                        \
                __count = mi_common_CjsonGetArraySize(item_a);                          \
                __count = __count < size ? __count : size;                              \
                *(int *)prealsize = __count;                                            \
                                                                                        \
                for(__i = 0; __i < __count; __i++)                                      \
                {                                                                       \
                    item_tmp = mi_common_CjsonGetArrayItem(item_a, __i);                \
                    *(pval + __i) = item_tmp->valuestring;                              \
                }                                                                       \
                                                                                        \
            } while(0);                                                                 \
                                                                                        \
            __ret;                                                                      \
        })

#define MI_COMMON_MODPARAM_GESTRING_INARRAY(modid, name, pval, index)                       \
            ({                                                                              \
                int __ret    = 0;                                                           \
                do                                                                          \
                {                                                                           \
                    cJSON *item_mod = NULL;                                                 \
                    cJSON *item_a   = NULL;                                                 \
                    cJSON *item_tmp = NULL;                                                 \
                                                                                            \
                    if(g_ModParam_cJSON == NULL)                                            \
                    {                                                                       \
                        CamOsPrintf("Get g_ModParam_cJSON non exist\n");                    \
                        __ret = -1;                                                         \
                        break;                                                              \
                    }                                                                       \
                                                                                            \
                    item_mod = mi_common_CjsonGetObjectItem(g_ModParam_cJSON, #modid);      \
                    if(!item_mod)                                                           \
                    {                                                                       \
                        __ret = -1;                                                         \
                        break;                                                              \
                    }                                                                       \
                                                                                            \
                    item_a = mi_common_CjsonGetObjectItem(item_mod, #name);                 \
                    if(!item_a)                                                             \
                    {                                                                       \
                        __ret = -1;                                                         \
                        break;                                                              \
                    }                                                                       \
                                                                                            \
                    item_tmp =  mi_common_CjsonGetArrayItem(item_a, index);                 \
                    if(!item_tmp)                                                           \
                    {                                                                       \
                        __ret = -1;                                                         \
                        break;                                                              \
                    }                                                                       \
                    pval = item_tmp->valuestring;                                           \
                                                                                            \
                } while(0);                                                                 \
                                                                                            \
                __ret;                                                                      \
            })

#define MI_COMMON_MODPARAM_GETINT_INARRAY(modid, name, type,  pval, index)              \
        ({                                                                              \
            int  __ret    = 0;                                                          \
            do                                                                          \
            {                                                                           \
                cJSON *item_mod = NULL;                                                 \
                cJSON *item_a   = NULL;                                                 \
                cJSON *item_tmp = NULL;                                                 \
                                                                                        \
                if(g_ModParam_cJSON == NULL)                                            \
                {                                                                       \
                    CamOsPrintf("Get g_ModParam_cJSON non exist\n");                    \
                    __ret = -1;                                                         \
                    break;                                                              \
                }                                                                       \
                                                                                        \
                item_mod = mi_common_CjsonGetObjectItem(g_ModParam_cJSON, #modid);      \
                if(!item_mod)                                                           \
                {                                                                       \
                    __ret = -1;                                                         \
                    break;                                                              \
                }                                                                       \
                                                                                        \
                item_a = mi_common_CjsonGetObjectItem(item_mod, #name);                 \
                if(!item_a)                                                             \
                {                                                                       \
                    __ret = -1;                                                         \
                    break;                                                              \
                }                                                                       \
                                                                                        \
                item_tmp = mi_common_CjsonGetArrayItem(item_a, index);                  \
                if(!item_tmp)                                                           \
                {                                                                       \
                    __ret = -1;                                                         \
                    break;                                                              \
                }                                                                       \
                *((type *)pval) = (type)item_tmp->valueint;                             \
                                                                                        \
            } while(0);                                                                 \
                                                                                        \
            __ret;                                                                      \
        })


#ifdef ANDROID
#define MI_SNR_MODPARAMINIT(name)\
    int sensorcfg_num;\
    int sensorcfg_id;\
    char * sensor_name[4];\
    pr_info("Sensor module name:%s\n", name);\
    MI_COMMON_MODPARAM_GETVALUE(E_MI_MODULE_ID_SNR, sensor_num, int, &sensorcfg_num);\
    MI_COMMON_MODPARAM_GETARRAY_STRING(E_MI_MODULE_ID_SNR, sensor_name, 4, sensor_name, &sensorcfg_num);\
    if(sensorcfg_num > 0){\
        for(sensorcfg_id = 0; sensorcfg_id < sensorcfg_num; sensorcfg_id++){\
            if(strcmp(name,sensor_name[sensorcfg_id]) == 0){\
                pr_info("find config:sensorcfg id %d is %s",sensorcfg_id, sensor_name[sensorcfg_id]);\
                break;\
            }\
        }\
        if(sensorcfg_id == sensorcfg_num){\
            pr_info("FAIL to find matching cfg");\
            sensorcfg_id = 0;\
        }\
        MI_COMMON_MODPARAM_GESTRING_INARRAY(E_MI_MODULE_ID_SNR, mclk, mclk, sensorcfg_id);\
        MI_COMMON_MODPARAM_GETINT_INARRAY(E_MI_MODULE_ID_SNR, chmap, int, &chmap, sensorcfg_id);\
        MI_COMMON_MODPARAM_GETINT_INARRAY(E_MI_MODULE_ID_SNR, lane_num, int, &lane_num, sensorcfg_id);\
        MI_COMMON_MODPARAM_GETINT_INARRAY(E_MI_MODULE_ID_SNR, hdr_lane_num, int, &hdr_lane_num, sensorcfg_id);\
        MI_COMMON_MODPARAM_GETINT_INARRAY(E_MI_MODULE_ID_SNR, i2c_slave_id, ulong, &i2c_slave_id,sensorcfg_id);\
    }\

#define PARA_DBG()\
    pr_info("Connect %s get modparam chmap %d\n",__FUNCTION__, chmap);\
    pr_info("Connect %s get modparam mclk %s\n",__FUNCTION__, mclk);\
    pr_info("Connect %s get modparam lane_num %d\n",__FUNCTION__, lane_num);\
    pr_info("Connect %s get modparam hdr_lane_num %d\n",__FUNCTION__, hdr_lane_num);\
    pr_info("Connect %s get modparam i2c_slave_id %ld\n",__FUNCTION__, i2c_slave_id);\

#else
#define MI_SNR_MODPARAMINIT(name) ;
#define PARA_DBG() ;
#endif

#ifndef MIN
    #define MIN(_a_, _b_)               ((_a_) < (_b_) ? (_a_) : (_b_))
#endif
#ifndef MAX
    #define MAX(_a_, _b_)               ((_a_) > (_b_) ? (_a_) : (_b_))
#endif

#define DrvUnregisterSensorDriverEx(nSNRPadID) DRV_SENSOR_IF_Release((nSNRPadID))

//extern s32 I2C_HACKING_I2CWriteRegisterPair(int slaveAddr, short reg, unsigned short value, ISP_I2C_FMT fmt);
extern u64 intlog10(u32 value);
extern u64 intlog2(u32 value);
extern s32 DRV_SENSOR_IF_RegisterSensorDriver(u32 nCamID, SensorInitHandle pfnSensorInitHandle);
extern s32 DRV_SENSOR_IF_RegisterPlaneDriver(u32 nCamID, u32 nPlaneID, SensorInitHandle pfnSensorInitHandle);
extern s32 DRV_SENSOR_IF_RegisterSensorDriverEx(u32 nSNRPadID, SensorInitHandle pfnSensorInitHandle, void *pPrivateData);
extern s32 DRV_SENSOR_IF_RegisterPlaneDriverEx(u32 nSNRPadID, u32 nPlaneID, SensorInitHandle pfnSensorInitHandle, void *pPrivateData);
extern s32 DRV_SENSOR_IF_Release(u32 nSNRPadID);
extern s32 DRV_SENSOR_IF_EarlyInitSync(u32 nSNRPadID);
extern s32 DRV_SENSOR_IF_RegisterSensorI2CSlaveID(u32 nCamID, u32 Slaveid);
extern s32 DRV_SENSOR_IF_RegisterPlaneI2CSlaveID(u32 nCamID, u32 nPlaneID, u32 Slaveid);
extern s32 DRV_SENSOR_IF_SensorHandleVer(u32 version_major, u32 version_minor);
extern s32 DRV_SENSOR_IF_SensorIFVer(u32 version_major, u32 version_minor);
extern s32 DRV_SENSOR_IF_SensorI2CVer(u32 version_major, u32 version_minor);


#define SENSOR_USLEEP(us) CamOsUsSleep(us)
#define SENSOR_MSLEEP(ms) CamOsMsSleep(ms)
#define SENSOR_UDELAY(us) CamOsUsDelay(us)
#define SENSOR_MDELAY(ms) CamOsMsDelay(ms)
#endif
