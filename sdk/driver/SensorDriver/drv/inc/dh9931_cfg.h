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

#ifndef __DH9931_CFG_H__
#define __DH9931_CFG_H__

/* 可开关编译宏，为了兼容各个编译器，只是用简单的
    ifdef进行判断
*/

#ifdef __cplusplus
extern "C" {
#endif

/* Win */
#undef DHC_DH9931SDK_WIN

/* Linux 驱动层 */
#undef DHC_DH9931SDK_LINUX_DRV

/* Linux 应用层 */
#define DHC_DH9931SDK_LINUX_LIB
//#undef DHC_DH9931SDK_LINUX_LIB

/* MDK编译器 */
#undef DHC_DH9931SDK_MDK

/* IAR编译器 */
#undef DHC_DH9931SDK_IAR

#ifdef MODULE_NAME
#undef MODULE_NAME
#endif

/* 
    根据编译开关确定头文件包含,并对平台区分
    进行宏定义
*/

/* Linux库 */
#ifdef DHC_DH9931SDK_LINUX_LIB

	//#include <stdio.h>
	//#include <pthread.h>
	//#include <string.h>
	//#include <sys/time.h>
	//#include <sys/types.h>
	//#include <unistd.h>
	
    /* 定义导出函数声明 */
    #define DHC_DEF_API
    
    /* 定义本地函数声明 */
    #define DHC_DEF_LOCAL               static
    
    /* 定义函数符号导出 */
    #define DHC_DEF_EXP_SYMBOL(x)
    
    /* 定义inline */
    #define DHC_DEF_INLINE              inline
    
    /* 定义全局变量是否使用static或者const */
    #define DHC_DEF_GLOBAL_VAR          const
    
    /* 使用系统C库 */
    //#define DHC_DEF_DEFAULTC
    #undef DHC_DEF_DEFAULTC

    #define DHC_DEF_XDATA
    
    
    /* 定义模块名 */
    #define MODULE_NAME                 "dh9931_sdk.a"

#endif

#ifdef DHC_DH9931SDK_MDK
	
    /* 定义导出函数声明 */
    #define DHC_DEF_API
    
    /* 定义本地函数声明 */
    #define DHC_DEF_LOCAL					static               
    
    /* 定义函数符号导出 */
    #define DHC_DEF_EXP_SYMBOL(x)
    
    /* 定义inline */
    #define DHC_DEF_INLINE
    
    /* 定义全局变量是否使用static或者const */
    #define DHC_DEF_GLOBAL_VAR			static

	#define DHC_DEF_XDATA
    
    /* 使用系统C库 */
    #define DHC_DEF_DEFAULTC
	//#undef DHC_DEF_DEFAULTC
    
    
    /* 定义模块名 */
    #define MODULE_NAME                 "dh9931_sdk.a"
		
		#pragma anon_unions

#endif

#ifdef __cplusplus
}
#endif


#endif /* __DH9931_H__ */
