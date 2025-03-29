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
 * The full name of 'ptree' is 'Pipeline tree', which use the idea of 'Amigos'
 * for reference and auther is 'pedro.peng' from Sigmastar.
 */

#ifndef __PTREE_H__
#define __PTREE_H__

typedef struct PTREE_Config_s
{
    void *pArenaHandle; /* Arena buffer is created by user. */
    void *pDbInstance;  /* Ini or json instance for user used */
} PTREE_Config_t;

typedef struct PTREE_ModuleDesc_s
{
    const char * modName;
    unsigned int devId;
    unsigned int chnId;
} PTREE_ModuleDesc_t;

enum PTREE_Ioctl_e
{
    E_PTREE_IOCTL_INIT,
    E_PTREE_IOCTL_DEINIT,
    E_PTREE_IOCTL_BIND,
    E_PTREE_IOCTL_UNBIND,
    E_PTREE_IOCTL_START,
    E_PTREE_IOCTL_STOP,
    E_PTREE_IOCTL_PREPARE,
    E_PTREE_IOCTL_UNPREPARE,
    E_PTREE_IOCTL_BIND_ALL,
    E_PTREE_IOCTL_UNBIND_ALL,
    E_PTREE_IOCTL_START_IN,
    E_PTREE_IOCTL_STOP_IN,
    E_PTREE_IOCTL_START_OUT,
    E_PTREE_IOCTL_STOP_OUT,
    E_PTREE_IOCTL_START_IN_ALL,
    E_PTREE_IOCTL_STOP_IN_ALL,
    E_PTREE_IOCTL_START_OUT_ALL,
    E_PTREE_IOCTL_STOP_OUT_ALL,
    E_PTREE_IOCTL_CREATE_DELAY_PASS,
    E_PTREE_IOCTL_DESTROY_DELAY_PASS,
    E_PTREE_IOCTL_INIT_DELAY_PASS,
    E_PTREE_IOCTL_DEINIT_DELAY_PASS,
    E_PTREE_IOCTL_BIND_DELAY_PASS,
    E_PTREE_IOCTL_UNBIND_DELAY_PASS,
    E_PTREE_IOCTL_START_DELAY_PASS,
    E_PTREE_IOCTL_STOP_DELAY_PASS,
    /* Mark the resoruce to be released */
    E_PTREE_IOCTL_MARK_STOP,
    E_PTREE_IOCTL_MARK_UNPREPARE,
    E_PTREE_IOCTL_MARK_STOP_OUT,
    E_PTREE_IOCTL_MARK_STOP_IN,
    E_PTREE_IOCTL_MARK_UNBIND,
    E_PTREE_IOCTL_MARK_DEINIT,
};

/* Max parameter count is 16, it can be expanded if u want. */
#define PTREE_ARGC(__0, __1, __2, __3, __4, __5, __6, __7, __8, __9, __10, __11, __12, __13, __14, __15, __16, __N, \
                   ...)                                                                                             \
    __N
#define PTREE_CMD_PARA_CNT(...) PTREE_ARGC(__VA_ARGS__, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

/*
 * A simple use of sending command.
 */
#define PTREE_CMD(__mod, __dev, __chn, __cmd, ...)                                     \
    do                                                                                 \
    {                                                                                  \
        PTREE_ModuleDesc_t __modDesc;                                                  \
        unsigned long      __cmdData[] = {__cmd, ##__VA_ARGS__};                       \
        __modDesc.modName              = __mod;                                        \
        __modDesc.devId                = __dev;                                        \
        __modDesc.chnId                = __chn;                                        \
        PTREE_RunCmd(&__modDesc, __cmdData, PTREE_CMD_PARA_CNT(__cmd, ##__VA_ARGS__)); \
    } while (0)

#define PTREE_CMD_STR(...)            \
    do                                \
    {                                 \
        PTREE_RunCmdStr(__VA_ARGS__); \
    } while (0)

/*
 * Configurate the pipeline tree's and return pHandle of ptree instance.
 * Return: NULL if failure.
 */
void *PTREE_CreateInstance(const PTREE_Config_t *config);

/*
 * Destroy tree's instance and return the specific value.
 * Return: 0 if success, and -1 if failure.
 */
int PTREE_DestroyInstance(void *pHandle);

/*
 * Build the pipeline tree's data structure.
 * Return: 0 if success, and -1 if failure.
 */
int PTREE_ConstructPipeline(void *pHandle);

/*
 * Destroy the pipeline tree's data structure.
 * Return: 0 if success.
 */
int PTREE_DestructPipeline(void *pHandle);

/*
 * Activate the pipeline according to the pipeline tree data structure.
 * Return: 0 if success.
 */
int PTREE_StartPipeline(void *pHandle);

/*
 * Deactivate the pipeline according to the pipeline tree data structure.
 * Return: 0 if success.
 */
int PTREE_StopPipeline(void *pHandle);

/*
 * Control the pipeline command.
 * Return: 0 if success.
 */
int PTREE_IoCtl(const PTREE_ModuleDesc_t *modDesc, enum PTREE_Ioctl_e enCmd, long cmdArg);

/*
 * Control the pipeline command by the formating string.
 * Return: 0 if success.
 */
int PTREE_IoCtlStr(const char *cmdStr, ...);

/*
 * Control the module in pipeline by using specified 'unsigned long' type value.
 * Return: 0 if success.
 */
int PTREE_RunCmd(const PTREE_ModuleDesc_t *modDesc, unsigned long cmdIdPara[], unsigned int paraCnt);

/*
 * Control the module pipeline by using the formating string.
 * Return: 0 if success.
 */
int PTREE_RunCmdStr(const char *cmdStr, ...);
#endif //__PTREE_H__
