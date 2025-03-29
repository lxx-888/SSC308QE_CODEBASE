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
#include <asm/neon.h>
#include <linux/module.h>


#include "mi_common_datatype.h"
#include "mi_common_internal.h"

#include <linux/moduleparam.h>
#include <linux/stat.h>
#include <linux/sched.h>
#include <linux/of_platform.h>
#include <cam_proc_wrapper.h>
#include "cam_sysfs.h"
#include "cam_clkgen.h"
#include "cam_device_wrapper.h"

// TODO: to avoid include cjson.h
#include <cjson.h>
#ifdef MI_COMMON_RGNSUB
#include "mi_common_rgnsub.h"
#endif

typedef enum
{
    E_MI_COMMON_RESOURCE_LEVEL_0,
    E_MI_COMMON_RESOURCE_LEVEL_1,
    E_MI_COMMON_RESOURCE_LEVEL_2,
    E_MI_COMMON_RESOURCE_LEVEL_3,
    E_MI_COMMON_RESOURCE_LEVEL_4,
    E_MI_COMMON_RESOURCE_LEVEL_5,
    E_MI_COMMON_RESOURCE_LEVEL_MAX,
} MI_COMMON_ProcessLevel_e;

typedef enum
{
    E_MI_COMMON_RESOURCE_KEY_INIT,
    E_MI_COMMON_RESOURCE_KEY_DEV,
    E_MI_COMMON_RESOURCE_KEY_CHN,
    E_MI_COMMON_RESOURCE_KEY_PORT,
    E_MI_COMMON_RESOURCE_KEY_HANDLE,
    E_MI_COMMON_RESOURCE_KEY_ID,
    E_MI_COMMON_RESOURCE_KEY_GROUP,
    E_MI_COMMON_RESOURCE_KEY_LAYER,
    E_MI_COMMON_RESOURCE_KEY_ATTACH,
    E_MI_COMMON_RESOURCE_KEY_START,
    E_MI_COMMON_RESOURCE_KEY_BIND,
    E_MI_COMMON_RESOURCE_KEY_INPUT_GETBUF,
    E_MI_COMMON_RESOURCE_KEY_INPUT_USERPIC,
    E_MI_COMMON_RESOURCE_KEY_OUTPUT_GETBUF,
    E_MI_COMMON_RESOURCE_KEY_MMA_ALLOCBUF,
    E_MI_COMMON_RESOURCE_KEY_PRIVPOOL_CONFIGBUF,
    E_MI_COMMON_RESOURCE_KEY_PRIVPOOL_ALLOCBUF,
    E_MI_COMMON_RESOURCE_KEY_MAX,
} MI_COMMON_ProcessKeyType_e;

typedef struct MI_COMMON_ResourceInfo_s
{
    MI_ModuleId_e               eModuleId;
    MI_COMMON_ProcessLevel_e    eProcessLevel;
    MI_U64                      u64Key;
    void                        *pPrivateData;
    MI_U32                      u32DataSize;
    MI_S32                      s32Pid;
} MI_COMMON_ResourceInfo_t;

#define TRUE (1)
#define FALSE (0)

struct MI_DEVICE_Object_s;
typedef struct MI_DEVICE_Object_s MI_DEVICE_Object_t;
typedef MI_S32 (*MI_COMMON_EarlyFreeCb)(MI_ModuleId_e eModId, MI_COMMON_ProcessKeyType_e eKeyType, void *pPrivData);

extern char *gp_jsonPath;
extern char  g_ModParamPath[128];
extern char  config_json_path[128];

module_param_string(g_ModParamPath, g_ModParamPath, sizeof(g_ModParamPath), 0644);
module_param(gp_jsonPath, charp, S_IRUGO);
module_param_string(config_json_path, config_json_path, sizeof(config_json_path), 0644);

void version_show(CamProcSeqFile_t *m, void *pData);
void debug_show(CamProcSeqFile_t *m, void *pData);
void debug_write(char *pBuf, int nLen, void *pData);
void number_show(CamProcSeqFile_t *m, void *pData);
void number_write(char *pBuf, int nLen, void *pData);
CamProcEntry_t *MI_DEVICE_GetProcModuleDir(void);
CamProcEntry_t *MI_DEVICE_GetProcHalDir_Tag(int module_id);
void MI_DEVICE_SetProcHalDir_Tag(int module_id, CamProcEntry_t * pEntry);

struct CamClass *MI_DEVICE_GetClass(void);
MI_S32 MI_DEVICE_RemoteWrapper(unsigned short socid, MI_ModuleId_e module_id, int cmd, void *arg, int len);
int MI_DEVICE_Register(MI_DEVICE_Object_t *device);
void MI_DEVICE_Unregister(MI_DEVICE_Object_t *device);
int core_module_init(void);
void core_module_exit(void);
void MI_DEVICE_NotifyMiSysExit(void);
void MI_COMMON_ModuleIdToPrefixName(MI_ModuleId_e eModuleId, char *prefix_name);
cJSON *      mi_common_ModParamGetCjsonObjectItem(const char *const modid, const char *const val_name);
MI_U32 MI_COMMON_GetResKeyVal(MI_U8* u8String);
MI_S32 MI_COMMON_FindResourcePidByKey(MI_ModuleId_e eModId, MI_U64 u64Key);
MI_S32 MI_COMMON_CheckResourceExist(MI_ModuleId_e eModId, MI_U64 u64Key, MI_S32 s32Pid);
MI_S32 MI_COMMON_AddResource(MI_COMMON_ResourceInfo_t *pstResourceInfo);
MI_S32 MI_COMMON_DelResource(MI_COMMON_ResourceInfo_t *pstResourceInfo);
MI_S32 MI_COMMON_ProcessExit(MI_ModuleId_e eModId, MI_S32 s32Pid, MI_S32 (*mi_common_FreeResource)(MI_COMMON_ResourceInfo_t *));
void MI_COMMON_ProcessRegisterEarlyFreeCb(MI_COMMON_EarlyFreeCb pfEarlyFreeCb);
void MI_COMMON_MODPARAM_GETVALUE(const char *pModuleId, const char *pname, MI_COMMON_ModParamType_e eType, void *pval);
void MI_COMMON_MODPARAM_GETSTRING(const char *pModuleId, const char *pname, MI_U16 u16Size, void *pval);
void MI_COMMON_MODPARAM_GETARRAY(const char *pModuleId, const char *pname, MI_COMMON_ModParamType_e eType,
                                         MI_U16 u16Size, void *pval, MI_U32 *p32RealSize);


// common cjson interface
char *mi_common_GetConfigJsonPath(void);
cJSON *mi_common_CjsonGetCustomerCjson(const char *path);
void mi_common_CjsonFreeCustomerCjson(const char *path);
CJSON_PUBLIC(cJSON *) mi_common_CjsonLoad(const char*filename);
CJSON_PUBLIC(cJSON *) mi_common_CjsonParse(const char *value);
CJSON_PUBLIC(cJSON_bool) mi_common_CjsonHasObjectItem(const cJSON *object, const char *string);
CJSON_PUBLIC(cJSON *) mi_common_CjsonGetObjectItem(const cJSON *const object, const char *const string);
CJSON_PUBLIC(int) mi_common_CjsonGetArraySize(const cJSON *array);
CJSON_PUBLIC(cJSON *) mi_common_CjsonGetArrayItem(const cJSON *array, int index);
CJSON_PUBLIC(void) mi_common_CjsonDelete(cJSON *c);
CJSON_PUBLIC(char *) mi_common_CjsonPrint(const cJSON *item);
#ifdef CONFIG_MI_COMMON_SUPPORT_CJSON_WRITE
CJSON_PUBLIC(cJSON *) mi_common_CjsonCreateObject(void);
CJSON_PUBLIC(cJSON *) mi_common_CjsonCreateString(const char *string);
CJSON_PUBLIC(cJSON *) mi_common_CjsonCreateArray(void);
CJSON_PUBLIC(cJSON *) mi_common_CjsonCreateIntArray(const int *numbers, int count);
CJSON_PUBLIC(cJSON *) mi_common_CjsonCreateStringArray(const char **strings, int count);
CJSON_PUBLIC(cJSON*) mi_common_CjsonAddBoolToObject(cJSON * const object, const char * const name, const cJSON_bool boolean);
CJSON_PUBLIC(cJSON*) mi_common_CjsonAddNumberToObject(cJSON * const object, const char * const name, const int number);
CJSON_PUBLIC(cJSON*) mi_common_CjsonAddStringToObject(cJSON * const object, const char * const name, const char * const string);
CJSON_PUBLIC(void) mi_common_CjsonAddItemToArray(cJSON *array, cJSON *item);
CJSON_PUBLIC(void) mi_common_CjsonAddItemToObject(cJSON *object, const char *string, cJSON *item);
CJSON_PUBLIC(cJSON_bool) mi_common_CjsonPrintPreallocated(cJSON *item, char *buf, const int len, const cJSON_bool fmt);
#endif
MI_BOOL MI_DEVICE_IsCurrentExiting(void)
{
    return current->flags & PF_EXITING;
}

int MI_DEVICE_IsMmExist(void)
{
    return current->mm != NULL ? TRUE : FALSE;
}

int MI_DEVICE_GetPid(void)
{
    return current->pid;
}

int MI_DEVICE_GetTgid(void)
{
    return current->tgid;
}

DECLEAR_MODULE_INIT_EXIT_EXTRA(core_module_init, core_module_exit);

EXPORT_SYMBOL(debug_show);
EXPORT_SYMBOL(version_show);
EXPORT_SYMBOL(debug_write);
EXPORT_SYMBOL(number_show);
EXPORT_SYMBOL(number_write);
EXPORT_SYMBOL(MI_DEVICE_GetProcModuleDir);
EXPORT_SYMBOL(MI_DEVICE_GetProcHalDir_Tag);
EXPORT_SYMBOL(MI_DEVICE_SetProcHalDir_Tag);
EXPORT_SYMBOL(MI_COMMON_ModuleIdToPrefixName);
EXPORT_SYMBOL(MI_DEVICE_Register);
EXPORT_SYMBOL(MI_DEVICE_Unregister);

#ifndef COMBINE_MI_MODULE
EXPORT_SYMBOL(MI_DEVICE_NotifyMiSysExit);
EXPORT_SYMBOL(MI_DEVICE_IsMmExist);
EXPORT_SYMBOL(MI_DEVICE_GetTgid);
EXPORT_SYMBOL(MI_DEVICE_GetClass);
EXPORT_SYMBOL(MI_DEVICE_RemoteWrapper);
EXPORT_SYMBOL(mi_common_ModParamGetCjsonObjectItem);
EXPORT_SYMBOL(MI_COMMON_MODPARAM_GETVALUE);
EXPORT_SYMBOL(MI_COMMON_MODPARAM_GETSTRING);
EXPORT_SYMBOL(MI_COMMON_MODPARAM_GETARRAY);
EXPORT_SYMBOL(MI_COMMON_GetResKeyVal);
EXPORT_SYMBOL(MI_COMMON_FindResourcePidByKey);
EXPORT_SYMBOL(MI_COMMON_CheckResourceExist);
EXPORT_SYMBOL(MI_COMMON_AddResource);
EXPORT_SYMBOL(MI_COMMON_DelResource);
EXPORT_SYMBOL(MI_COMMON_ProcessExit);
EXPORT_SYMBOL(MI_COMMON_ProcessRegisterEarlyFreeCb);

// common cjson
EXPORT_SYMBOL(mi_common_GetConfigJsonPath);
EXPORT_SYMBOL(mi_common_CjsonGetCustomerCjson);
EXPORT_SYMBOL(mi_common_CjsonFreeCustomerCjson);
EXPORT_SYMBOL(mi_common_CjsonLoad);
EXPORT_SYMBOL(mi_common_CjsonParse);
EXPORT_SYMBOL(mi_common_CjsonHasObjectItem);
EXPORT_SYMBOL(mi_common_CjsonGetObjectItem);
EXPORT_SYMBOL(mi_common_CjsonGetArraySize);
EXPORT_SYMBOL(mi_common_CjsonGetArrayItem);
EXPORT_SYMBOL(mi_common_CjsonDelete);
EXPORT_SYMBOL(mi_common_CjsonPrint);
#ifdef CONFIG_MI_COMMON_SUPPORT_CJSON_WRITE
EXPORT_SYMBOL(mi_common_CjsonCreateObject);
EXPORT_SYMBOL(mi_common_CjsonCreateString);
EXPORT_SYMBOL(mi_common_CjsonCreateArray);
EXPORT_SYMBOL(mi_common_CjsonCreateIntArray);
EXPORT_SYMBOL(mi_common_CjsonCreateStringArray);
EXPORT_SYMBOL(mi_common_CjsonAddBoolToObject);
EXPORT_SYMBOL(mi_common_CjsonAddNumberToObject);
EXPORT_SYMBOL(mi_common_CjsonAddStringToObject);
EXPORT_SYMBOL(mi_common_CjsonAddItemToArray);
EXPORT_SYMBOL(mi_common_CjsonAddItemToObject);
EXPORT_SYMBOL(mi_common_CjsonPrintPreallocated);
#endif
// rgn sub
#ifdef MI_COMMON_RGNSUB
EXPORT_SYMBOL(mi_common_RgnsubIsEmptyClipRgn);
EXPORT_SYMBOL(mi_common_RgnsubEmptyClipRgn);
EXPORT_SYMBOL(mi_common_RgnsubInitClipRgn);
EXPORT_SYMBOL(mi_common_RgnsubSubtractRgn);
EXPORT_SYMBOL(mi_common_RgnsubAddClipRect);
EXPORT_SYMBOL(mi_common_RgnsubDbgDumpRgn);
EXPORT_SYMBOL(mi_common_RgnsubInitRgn);
EXPORT_SYMBOL(mi_common_RgnsubDeinitRgn);
#endif
#endif
MI_MODULE_LICENSE();


