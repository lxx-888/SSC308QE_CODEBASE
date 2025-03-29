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

/* �ɿ��ر���꣬Ϊ�˼��ݸ�����������ֻ���ü򵥵�
    ifdef�����ж�
*/

#ifdef __cplusplus
extern "C" {
#endif

/* Win */
#undef DHC_DH9931SDK_WIN

/* Linux ������ */
#undef DHC_DH9931SDK_LINUX_DRV

/* Linux Ӧ�ò� */
#define DHC_DH9931SDK_LINUX_LIB
//#undef DHC_DH9931SDK_LINUX_LIB

/* MDK������ */
#undef DHC_DH9931SDK_MDK

/* IAR������ */
#undef DHC_DH9931SDK_IAR

#ifdef MODULE_NAME
#undef MODULE_NAME
#endif

/* 
    ���ݱ��뿪��ȷ��ͷ�ļ�����,����ƽ̨����
    ���к궨��
*/

/* Linux�� */
#ifdef DHC_DH9931SDK_LINUX_LIB

	//#include <stdio.h>
	//#include <pthread.h>
	//#include <string.h>
	//#include <sys/time.h>
	//#include <sys/types.h>
	//#include <unistd.h>
	
    /* ���嵼���������� */
    #define DHC_DEF_API
    
    /* ���屾�غ������� */
    #define DHC_DEF_LOCAL               static
    
    /* ���庯�����ŵ��� */
    #define DHC_DEF_EXP_SYMBOL(x)
    
    /* ����inline */
    #define DHC_DEF_INLINE              inline
    
    /* ����ȫ�ֱ����Ƿ�ʹ��static����const */
    #define DHC_DEF_GLOBAL_VAR          const
    
    /* ʹ��ϵͳC�� */
    //#define DHC_DEF_DEFAULTC
    #undef DHC_DEF_DEFAULTC

    #define DHC_DEF_XDATA
    
    
    /* ����ģ���� */
    #define MODULE_NAME                 "dh9931_sdk.a"

#endif

#ifdef DHC_DH9931SDK_MDK
	
    /* ���嵼���������� */
    #define DHC_DEF_API
    
    /* ���屾�غ������� */
    #define DHC_DEF_LOCAL					static               
    
    /* ���庯�����ŵ��� */
    #define DHC_DEF_EXP_SYMBOL(x)
    
    /* ����inline */
    #define DHC_DEF_INLINE
    
    /* ����ȫ�ֱ����Ƿ�ʹ��static����const */
    #define DHC_DEF_GLOBAL_VAR			static

	#define DHC_DEF_XDATA
    
    /* ʹ��ϵͳC�� */
    #define DHC_DEF_DEFAULTC
	//#undef DHC_DEF_DEFAULTC
    
    
    /* ����ģ���� */
    #define MODULE_NAME                 "dh9931_sdk.a"
		
		#pragma anon_unions

#endif

#ifdef __cplusplus
}
#endif


#endif /* __DH9931_H__ */
