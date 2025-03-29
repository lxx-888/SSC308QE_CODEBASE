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
#ifndef _MI_COMMON_INTERNEL_H_
#define _MI_COMMON_INTERNEL_H_
#ifdef CONFIG_ARCH_SSTAR
#include <drv_miu.h>
#include <mdrv_mma_heap.h>
#include <mstar_chip.h>
#include <ms_platform.h>
#include <ms_msys.h>
#endif
#include "cam_os_wrapper.h"
#include "cam_proc_wrapper.h"
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,11,0)
#endif
#include "mi_common_datatype.h"
void MI_COMMON_ModuleIdToPrefixName(MI_ModuleId_e eModuleId, char *prefix_name);
#define CURRENT_CTX ((void *)current->mm)
#define CPU_BUS_ADDR_VALID(addr) (pfn_valid(__phys_to_pfn(addr)))

typedef enum {
    E_MI_COMMON_CALL_FROM_API,
    E_MI_COMMON_CALL_FROM_IOCTL,
} MI_COMMON_CallFrom_e;

typedef enum
{
    E_MI_MODPARAMTYPE_MI_U8,
    E_MI_MODPARAMTYPE_MI_U16,
    E_MI_MODPARAMTYPE_MI_U32,
    E_MI_MODPARAMTYPE_MI_U64,
    E_MI_MODPARAMTYPE_MI_S8,
    E_MI_MODPARAMTYPE_MI_S16,
    E_MI_MODPARAMTYPE_MI_S32,
    E_MI_MODPARAMTYPE_MI_S64,
    E_MI_MODPARAMTYPE_MI_BOOL,
    E_MI_MODPARAMTYPE_MI_VIRT,
} MI_COMMON_ModParamType_e;

typedef struct MI_COMMON_Client_s
{
    struct MI_DEVICE_Object_s *pstDevice;
    int                        pid;
    int                        tid;
    void *private;
} MI_COMMON_Client_t;

typedef struct MI_DEVICE_Context_s
{
    const char *         strModuleName;
    MI_COMMON_CallFrom_e eFrom;
    CamOsRwsem_t *       pstRwsem;
    MI_BOOL              bRwsem;
    MI_COMMON_Client_t * pstClient;
} MI_DEVICE_Context_t;

#ifndef CONFIG_ARCH_SSTAR

#define MMA_HEAP_NAME_LENG 32
// enable max mma areas be a large value .
#define MAX_MMA_AREAS 30

struct MMA_BootArgs_Config {
     int miu;//input :from bootargs or dts
     unsigned long size;//input :from bootargs or dts
     char name[MMA_HEAP_NAME_LENG];//input :from bootargs or dts
     unsigned long max_start_offset_to_curr_bus_base;
     unsigned long max_end_offset_to_curr_bus_base;//input:for vdec use.

     phys_addr_t reserved_start;//out: reserved_start
};


#define MSTAR_MIU0_BUS_BASE                      0x20000000UL

#define MSTAR_MIU1_BUS_BASE                      0xA0000000UL

#define ARM_MIU0_BUS_BASE                        MSTAR_MIU0_BUS_BASE
#define ARM_MIU1_BUS_BASE                        MSTAR_MIU1_BUS_BASE
#define ARM_MIU2_BUS_BASE                        0xFFFFFFFFUL
#define ARM_MIU3_BUS_BASE                        0xFFFFFFFFUL

#define ARM_MIU0_BASE_ADDR                       0x00000000UL
#define ARM_MIU1_BASE_ADDR                       0x80000000UL
#define ARM_MIU2_BASE_ADDR                       0xFFFFFFFFUL
#define ARM_MIU3_BASE_ADDR                       0xFFFFFFFFUL

#endif //CONFIG_ARCH_SSTAR

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 1, 0)
#include <linux/stdarg.h>
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
#define MI_FILE_OPERATIONS proc_ops
#define mi_proc_open proc_open
#define mi_proc_read proc_read
#define mi_proc_lseek proc_lseek
#define mi_proc_write proc_write
#define mi_proc_release proc_release
#else
#define MI_FILE_OPERATIONS file_operations
#define mi_proc_open open
#define mi_proc_read read
#define mi_proc_lseek llseek
#define mi_proc_write write
#define mi_proc_release release
#endif
#define _MACRO_CAT(s1, s2) s1##s2
#define MACRO_CAT(s1, s2)  _MACRO_CAT(s1, s2)

#ifdef NON_COMMERCIAL_FOR_INTERNAL_TEST_ONLY
#define MI_MODULE_LICENSE() MODULE_LICENSE("GPL v2");
#else
#define MI_MODULE_LICENSE() MODULE_LICENSE("Proprietary");
#endif
#ifndef DEBUG_STRLEN
#define DEBUG_STRLEN (256)
#endif // !DEBUG_STRLEN

typedef int (*MI_MODULE_INIT_FUNC)(void);
typedef void (*MI_MODULE_EXIT_FUNC)(void);

#define MI_DEVICE_INIT MACRO_CAT(EXTRA_MODULE_NAME, __module_init)
#define MI_DEVICE_EXIT MACRO_CAT(EXTRA_MODULE_NAME, __module_exit)
#define debug_level MACRO_CAT(EXTRA_MODULE_NAME, _debug_level)
#define debug_file MACRO_CAT(EXTRA_MODULE_NAME, _debug_file)
#define debug_func MACRO_CAT(EXTRA_MODULE_NAME, _debug_func)
#define MI_MODULE_INIT MACRO_CAT(EXTRA_MODULE_NAME, _mod_init)
#define MI_MODULE_EXIT MACRO_CAT(EXTRA_MODULE_NAME, _mod_exit)

#define MI_DEFINE_MODULE_INIT_EXIT(init, exit)           \
    static MI_MODULE_INIT_FUNC mod_init_func = init;     \
    static MI_MODULE_EXIT_FUNC mod_exit_func = exit;     \
    int MI_MODULE_INIT(void)                             \
    {                                                    \
        if(mod_init_func)                                \
        {                                                \
            return mod_init_func();                      \
        }                                                \
        return 0;                                        \
    }                                                    \
    void MI_MODULE_EXIT(void)                            \
    {                                                    \
        if(mod_exit_func)                                \
        {                                                \
            mod_exit_func();                             \
        }                                                \
    }

#define MI_DECLEAR_MODULE_PARAM()                                          \
    extern MI_DBG_LEVEL_e debug_level;                                     \
    extern char           debug_file[DEBUG_STRLEN];                        \
    extern char           debug_func[DEBUG_STRLEN];                        \
    module_param(debug_level, uint, 0644);                                 \
    module_param_string(debug_file, debug_file, sizeof(debug_file), 0644); \
    module_param_string(debug_func, debug_func, sizeof(debug_func), 0644);

#define MI_DECLEAR_MODULE_ENTRY()                                          \
    static int __init mod_init(void)                                       \
    {                                                                      \
        return MI_MODULE_INIT();                                           \
    }                                                                      \
    static void __exit mod_exit(void)                                      \
    {                                                                      \
        MI_MODULE_EXIT();                                                  \
    }                                                                      \
    module_init(mod_init);                                                 \
    module_exit(mod_exit);

#ifndef COMBINE_MI_MODULE

#define DECLEAR_MODULE_INIT_EXIT                                           \
    extern int  MI_DEVICE_INIT(void);                                      \
    extern void MI_DEVICE_EXIT(void);                                      \
    MI_DECLEAR_MODULE_PARAM()                                              \
    MI_DEFINE_MODULE_INIT_EXIT(MI_DEVICE_INIT, MI_DEVICE_EXIT)             \
    MI_DECLEAR_MODULE_ENTRY()

#define DECLEAR_MODULE_INIT_EXIT_EXTRA(init, exit)                         \
    MI_DEFINE_MODULE_INIT_EXIT(init, exit)                                 \
    MI_DECLEAR_MODULE_ENTRY()

#else

#define DECLEAR_MODULE_INIT_EXIT                                           \
    extern int  MI_DEVICE_INIT(void);                                      \
    extern void MI_DEVICE_EXIT(void);                                      \
    MI_DEFINE_MODULE_INIT_EXIT(MI_DEVICE_INIT, MI_DEVICE_EXIT)

#define DECLEAR_MODULE_INIT_EXIT_EXTRA(init, exit)                         \
    MI_DEFINE_MODULE_INIT_EXIT(init, exit)

#endif

CamProcEntry_t *MI_DEVICE_GetProcHalDir_Tag(int module_id);
void MI_DEVICE_SetProcHalDir_Tag(int module_id, CamProcEntry_t * pEntry);
#endif /* _MI_COMMON_INTERNEL_H_ */
