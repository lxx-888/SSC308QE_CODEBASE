/*************************************************
  Copyright(C), 2011-2014 ZheJiang Dahua Technology Co., Ltd.
  �ļ���:   dhc_dh9931_api.h
  ��  ��:   zhang_jie4(23721)<zhang_jie4@dahuatech.com>
  ��  ��:   1.0.0
  ��  �ڣ�  2016-12-2
  ��  ��:   �˴�Ϊ�ļ�����������������Ҫ����
  
            1��ʹ��˵��
            xx
           
            2��������
            xx
  
  �޶���ʷ:
  1. ��    ��:
     �޶��汾:
     ��    ��:
     �޶���ע:
     
  2. ��    ��:
     �޶��汾:
     ��    ��:
     �޶���ע:
*************************************************/

#ifndef _DHC_DH9931_API_H_ 
#define _DHC_DH9931_API_H_

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /*__cplusplus*/


/*----------------------------------------------*
 * ���������ض���
 *----------------------------------------------*/
typedef unsigned char           DHC_U8;
typedef unsigned short          DHC_U16;
typedef unsigned int            DHC_U32;

typedef signed char             DHC_S8;
typedef short                   DHC_S16;
typedef int                     DHC_S32;

typedef char                    DHC_CHAR;
#define DHC_VOID                void

/*----------------------------------------------*
 * const defination                             *
 *----------------------------------------------*/

typedef enum {
    DHC_FALSE = 0,
    DHC_TRUE  = 1,
} DHC_BOOL;

typedef enum
{
    DHC_I2C_BUS_0 = 0,
    DHC_I2C_BUS_1 = 1,    
}DHC_I2C_BUS_E;

#ifndef NULL
    #define NULL           (0L)
#endif

#define DHC_NULL           (0L)
#define DHC_SUCCESS        (0)
#define DHC_FAILURE        (-1)

#include "dh9931_cfg.h"

/*----------------------------------------------*
 * ������,Ϊ���ݵ�Ƭ����ʹ��8�ֽ�                 *
 *----------------------------------------------*/




/*----------------------------------------------*
 *             DH9931 ��ؽṹ����               *
 *----------------------------------------------*/
 
/*��ƬDH9931֧�����4��ͨ��*/
#define DHC_DH9931_MAX_CHNS    (4)

/*SDK���֧��4ƬDH9931ͬʱ����*/
/*�������оƬ�ܹ��ṩ���豸��ַ�ж��ٸ�����Moscow�ֲ��Ͽ���ֻ֧��4����ַ*/
#define DHC_DH9931_MAX_CHIPS   (4)

/*��Ч���߲�ʹ�õ�ͨ��*/
#define DHC_DH9931_INV_CHN     (0xFF)

/*��Ƶ���ͨ�����ѡ��*/
#define DHC_DH9931_AUDIO_OUTPUT_MAX_CHNS  (0x14)

#define DHC_DH9931_AUDIO_LINEIN_MAX_VOLUME_LEVEL    (0x0f)

#if 1
typedef struct 
{
    DHC_BOOL bEqAdd;     /*������ʽʱ�Ƿ񣬲���EQֵ*/
    DHC_BOOL bEqAuto;    /*��Ƶ���ʱ�Ƿ��Զ�EQ����*/

}DHC_DH9931_DETECT_ATTR;
#endif



/*������Ϣ����*/
typedef enum
{
    DHC_MSG_SILENT = 0,                                 /*��Ĭ��ʽ*/
    DHC_MSG_ERR,                                        /*���󼶱�*/
    DHC_MSG_WARN,                                       /*���漶��*/
    DHC_MSG_INFO,                                       /*��Ϣ����*/    
    DHC_MSG_DEBUG,                                      /*���Լ���*/    
    DHC_MSG_BOTT,                                       /*δ���弶��*/
}DHC_DH9931_MSG_TYPE_E;

/*����������*/
typedef enum
{
    DHC_ERRID_SUCCESS = 0,                              /*����״̬*/
    DHC_ERRIF_FAILED,                                   /*ͨ�ô���*/
    DHC_ERRID_TIMEOUT,                                  /*��ʱ����*/
    DHC_ERRID_INVPARAM,                                 /*��������*/
    DHC_ERRID_NOINIT,                                   /*����δ��ʼ������ID*/
    DHC_ERRID_INVPOINTER,                               /*������Чָ�����ID*/
    DHC_ERRID_UNSUPPORT,                                /*��֧��*/  
    DHC_ERRID_UNKNOW,                                   /*δ֪����*/
}DHC_DH9931_ERR_ID_E;

/*����ѡ��*/
typedef enum
{ 
	DHC_SET_MODE_DEFAULT = 0,                           /*Ĭ������*/
	DHC_SET_MODE_USER    = 1,                           /*�û�����*/
	DHC_SET_MODE_NONE,                                  /*��Ч����*/
}DHC_SET_MODE_E;

/*��Ƶ��ɫ������*/
typedef enum
{
    DHC_VD_E_LV0 = 0,                                  /*5.0mA (3.3V); 3.5mA (1.8V)*/        
    DHC_VD_E_LV1,									   /*7.5mA (3.3V); 5mA (1.8V)*/
    DHC_VD_E_LV2,									   /*7.5mA (3.3V); 5mA (1.8V)*/
    DHC_VD_E_LV3,                                      /*12.5mA (3.3V); 8.5mA (1.8V)*/
    DHC_VD_E_LV4,                                      /*15mA (3.3V); 10.5mA (1.8V)*/
    DHC_VD_E_LV5,                                      /*17.5mA (3.3V); 12mA (1.8V)*/
    DHC_VD_E_LV6,                                      /*20mA (3.3V); 13.5mA (1.8V)*/
    DHC_VD_E_LV7,                                      /*22.5mA (3.3V); 15.5mA (1.8V)*/
	DHC_VD_E_BUTT,                                     /*��Ч*/
	
}DHC_DH9931_VIDEO_ELECTRIC_E;

/*��Ƶ��ɫ������*/
typedef struct
{
	DHC_U8 u8Brightness;                                /*���ȣ�ȡֵ0-100*/
	DHC_U8 u8Contrast;                                  /*�Աȶȣ�ȡֵ0-100*/
	DHC_U8 u8Saturation;                                /*���Ͷȣ�ȡֵ0-100*/
	DHC_U8 u8Hue;                                       /*ɫ����ȡֵ0-100*/
	DHC_U8 u8Gain;                                      /*���棬������*/
	DHC_U8 u8WhiteBalance;                              /*�׵�ƽ,������*/
	DHC_U8 u8Sharpness;                                 /*��ȣ�ȡֵ0-15*/
	DHC_U8 u8Reserved[1];                               /*����λ*/    
}DHC_DH9931_VIDEO_COLOR_S;

/*��Ƶ��������־*/
typedef enum
{
    DHC_VOSIZE_HD = 0,                                  /*����*/        
    DHC_VOSIZE_SD,                                      /*����*/
    DHC_VOSIZE_BUT,                                     /*��Ч*/
}DHC_DH9931_VO_SIZE_E;

/*������ʽ*/
typedef enum
{
    DHC_SDFMT_PAL = 0,                                  /*PAL��*/
    DHC_SDFMT_NTSC,                                     /*NTSC��*/
    DHC_SDFMT_BUT,                                      /*��Ч*/
}DHC_DH9931_SD_FMT_E;

/*DH9931֧�ֵ���ʽ*/
typedef enum
{
    DHC_CVI_1280x720_25HZ = 0,                         /*720P 25֡*/
	DHC_CVI_1280x720_30HZ,                             /*720P 30֡*/
	DHC_CVI_1280x720_50HZ,                             /*720P 50֡*/
	DHC_CVI_1280x720_60HZ,                             /*720P 60֡*/
	DHC_CVI_1920x1080_25HZ,                            /*1080P 25֡*/ 
	DHC_CVI_1920x1080_30HZ = 5,                        /*1080P 30֡*/
	DHC_CVI_1920x1080_50HZ,                            /*1080P 50֡*/
    DHC_CVI_1920x1080_60HZ,                            /*1080P 60֡*/ 
    DHC_CVI_2304x1296_25HZ,                            /*3M(16:9) 25֡*/
    DHC_CVI_2048x1152_30HZ,                            /*3M(16:9) 30֡*/
    DHC_CVI_2048x1536_25HZ = 10,                       /*3M(4:3) 25֡*/
    DHC_CVI_2048x1536_30HZ,                            /*3M(4:3) 30֡*/    
	DHC_CVI_2560x1440_25HZ,                            /*4M(4:3) 25֡*/
	DHC_CVI_2560x1440_30HZ,                            /*4M(4:3) 30֡*/    
	DHC_CVI_3072x1728_20HZ,                            /*5M(16:9) 20֡*/
	DHC_CVI_2560x1920_20HZ = 15,                       /*5M(4:3) 20֡*/
	DHC_CVI_3072x1728_25HZ,                            /*5M(16:9) 25֡*/	
	DHC_CVI_2880x1920_20HZ,                            /*6M(3:2) 18֡*/	
	DHC_CVI_3840x2160_12HZ,                            /*8M(16:9) 12֡*/                                     
	DHC_CVI_3840x2160_15HZ,                            /*8M(16:9) 15֡*/

	DHC_AHD_1280x720_25HZ = 20,                        /*720P 25֡*/
	DHC_AHD_1280x720_30HZ,                             /*720P 30֡*/
	DHC_AHD_1280x720_50HZ,                             /*720P 50֡*/
	DHC_AHD_1280x720_60HZ,                             /*720P 60֡*/
	DHC_AHD_1920x1080_25HZ,                            /*1080P 25֡*/ 
	DHC_AHD_1920x1080_30HZ = 25,                       /*1080P 30֡*/
	DHC_AHD_2048x1536_18HZ,                            /*3M(4:3) 18֡*/
    DHC_AHD_2048x1536_25HZ,                            /*3M(4:3) 25֡*/
    DHC_AHD_2048x1536_30HZ,                            /*3M(4:3) 30֡*/  
    DHC_AHD_2560x1440_15HZ,                            /*4M(4:3) 15֡*/
	DHC_AHD_2560x1440_25HZ = 30,                       /*4M(4:3) 25֡*/
	DHC_AHD_2560x1440_30HZ,                            /*4M(4:3) 30֡*/         
    DHC_AHD_2592x1944_12HZ,                            /*5M(4:3) 12֡*/

	DHC_TVI_1280x720_25HZ,                             /*720P 25֡*/
	DHC_TVI_1280x720_30HZ,                             /*720P 30֡*/
	DHC_TVI_1280x720_50HZ = 35,                        /*720P 50֡*/
	DHC_TVI_1280x720_60HZ,                             /*720P 60֡*/
	DHC_TVI_1920x1080_25HZ,                            /*1080P 25֡*/ 
	DHC_TVI_1920x1080_30HZ,                            /*1080P 30֡*/  
	
	DHC_SD_NTSC_JM,
	DHC_SD_NTSC_443 = 40,
	DHC_SD_PAL_M,
	DHC_SD_PAL_60,
	DHC_SD_PAL_CN,
	DHC_SD_PAL_BGHID,

    DHC_TVI_OLD_FMT_NUM = 45, 
	
	DHC_TVI_1920x1536_18HZ = DHC_TVI_OLD_FMT_NUM,
	DHC_CVI_2560x1440_12HZ,
	DHC_CVI_2560x1440_15HZ,
	DHC_AHD_2592x1944_20HZ,							   /*5M(4:3) 20֡*/
	DHC_TVI_2560x1440_25HZ,
	
	DHC_TVI_2560x1440_30HZ,
	DHC_TVI_2560x1944_12HZ,
	
	DHC_TVI_2560x1944_20HZ,
	DHC_TVI3_1280x720_25HZ,
	DHC_TVI3_1280x720_30HZ,
	DHC_CVI_2592x1944_20HZ,
	
	DHC_INVALID_FMT,
}DHC_DH9931_VIDEO_FMT_E;

/*��Ƶ����������*/
typedef enum
{
	DHC_CABLE_TYPE_COAXIAL = 0,                          /*ͬ������*/
	DHC_CABLE_TYPE_UTP_10OHM,                            /*10 ohm�迹˫����*/	
	DHC_CABLE_TYPE_UTP_17OHM,                            /*17 ohm�迹˫����*/
	DHC_CABLE_TYPE_UTP_25OHM,                            /*25 ohm�迹˫����*/
	DHC_CABLE_TYPE_UTP_35OHM,                            /*35 ohm�迹˫����*/	
	DHC_CABLE_TYPE_BUT,                                  /*��Ч����*/
} DHC_DH9931_CABLE_TYPE_E;

/*��Ƶ״̬*/
typedef enum
{
    DHC_VIDEO_CONNECT = 0,                               /*��Ƶ��������*/
    DHC_VIDEO_LOST = 1,                                  /*��Ƶ��ʧ*/  
    DHC_VIDEO_BUT,
}DHC_DH9931_VIDEO_STATUS_E;

/*ͼ��ƫ����*/
typedef struct
{
	DHC_S8 s8TopOffset;                                  /*����ƫ����*/
	DHC_S8 s8LeftOffset;                                 /*���ƫ����*/ 
}DHC_DH9931_IMG_OFFSET_S;

/*AD ��Ƶ��ʧ����Ƶ��ʽ״̬*/
typedef struct
{
    DHC_DH9931_VIDEO_STATUS_E enVideoLost;              /*1��Ƶ��ʧ��0 ��Ƶ�ָ�*/
	DHC_DH9931_VIDEO_FMT_E enVideoFormat;               /*��ǰ��Ƶ�����ʽ*/
	DHC_DH9931_VIDEO_FMT_E enVideoReportFormat;         /*��ǰ��⵽����Ƶ��ʽ*/
    DHC_U8 u8Reserved[1];                               /*����λ*/
}DHC_DH9931_VIDEO_STATUS_S;

/*��Ƶ����ѡ��ģʽ*/
typedef enum
{
    DHC_AUDIO_LINE_IN = 0,                              /*LINE_IN����*/
    DHC_AUDIO_COAXIAL = 1,                              /*ͬ������*/
    DHC_AUDIO_BUT                                       /*��Ч*/
}DHC_DH9931_AUDIO_CHN_SEL_E;

/*��Ƶ�����ʣ�֧��8K(Ĭ��)��16K*/
typedef enum
{
	DHC_AUDIO_SAMPLERATE_8K  = 0,                       /*8K����Ƶ��*/
	DHC_AUDIO_SAMPLERATE_16K = 1,                       /*16K����Ƶ��*/
}DHC_DH9931_AUDIO_SAMPLERATE_E;

/*��Ƶʱ������ģʽ*/
typedef enum
{
	DHC_AUDIO_ENCCLK_MODE_SLAVE  = 0,                   /*��ģʽ*/	
	DHC_AUDIO_ENCCLK_MODE_MASTER = 1,                   /*Ĭ����ģʽ*/
}DHC_DH9931_AUDIO_CLK_MODE_E;

/*��Ƶ�������ģʽ*/
typedef enum
{
	DHC_AUDIO_SYNC_MODE_I2S = 0,                        /*Ĭ��I2S*/
	DHC_AUDIO_SYNC_MODE_DSP = 1,                        /*DSP���ģʽ*/
}DHC_DH9931_AUDIO_SYNC_MODE_E;


/*������ģʽ�£�λͬ����֡ͬ��Ƶ�ʵı�����ϵ*/
typedef enum
{
	DHC_AUDIO_DEC_FREQ_32   = 0,
	DHC_AUDIO_DEC_FREQ_64   = 1,
	DHC_AUDIO_DEC_FREQ_128  = 2,
	DHC_AUDIO_DEC_FREQ_256  = 3,
}DHC_DH9931_AUDIO_DEC_FREQ_E;

/*��Ƶͨ��ѡ��*/
typedef enum
{
    DHC_AUDIO_DEC_I2S_CHN_RIGHT  = 0,
	DHC_AUDIO_DEC_I2S_CHN_LEFT   = 1,
}DHC_DH9931_AUDIO_DEC_CHN_E;

typedef enum
{
    DHC_RS485_PROTOCOL_DAHUA = 0,
    DHC_RS485_PROTOCOL_PELCO_C = 1,
}DHC_DH9931_RS485_PROTOCOL_E;

/*9931оƬIO��������*/
typedef enum 
{
	DHC_DRIVER_POWER_18V = 0,                          /*1.8V*/
	DHC_DRIVER_POWER_33V = 1,                          /*3.3V*/
}DHC_DH9931_DRIVER_POWER_E;

/*EQģʽѡ��*/
typedef enum
{
    DHC_EXTERNAL_COUPLING = 0,                         /*�ⲿ���*/
    DHC_INTERNAL_CASCADE = 1,                          /*�ڲ�����*/
}DHC_DH9931_EQ_MODE_E;

/*halfģʽѡ��*/
typedef enum
{
    DHC_HALF_MODE_37_125 = 0,                          /*37.125M*/ 
    DHC_HALF_MODE_74_25 = 1,                           /*74.25M*/ 
    DHC_HALF_MODE_148_5 = 2,                           /*148.5M*/ 
    DHC_HALF_MODE_144 = 3,                             /*144M*/ 
    DHC_HALF_MODE_BUT,
}DHC_DH9931_HALFMODE_E;

/*��Ƶ����ģʽѡ��*/
typedef enum
{
    DHC_ENC_MODE_SLAVE = 0,                             /*��ģʽ*/
    DHC_ENC_MODE_MASTER = 1,                            /*��ģʽ*/
    DHC_ENC_MODE_BUT,                                   /*��Ч*/
}DHC_DH9931_AUDIO_ENC_MODE_E;

/*��Ƶ����ģʽѡ��*/
typedef enum
{
    DHC_DEC_MODE_SLAVE = 0,                             /*��ģʽ*/
    DHC_DEC_MODE_MASTER = 1,                            /*��ģʽ*/
    DHC_DEC_MODE_BUT,                                   /*��Ч*/
}DHC_DH9931_AUDIO_DEC_MODE_E;

typedef enum
{
    DHC_COAXIAL_MODE_OLD = 0,                           /*ͬ����Ƶ��ģʽ*/
    DHC_COAXIAL_MODE_NEW = 1,                           /*ͬ����Ƶ��ģʽ*/
    DHC_COAXIAL_MODE_BUT,
}DHC_DH9931_AUDIO_COAXIAL_MDOE_E;

/*��ƵDAC����Դѡ��*/
typedef enum
{
    DHC_DAC_SOURCE_IIS      = 0,
    DHC_DAC_SOURCE_ADC1     = 1,
    DHC_DAC_SOURCE_ADC2     = 2,
    DHC_DAC_SOURCE_ADC3     = 3,
    DHC_DAC_SOURCE_ADC4     = 4,
    DHC_DAC_SOURCE_ADC5     = 5,
    DHC_DAC_SOURCE_BUT,
}DHC_DH9931_AUDIO_DAC_SOURCE;

/*��Ƶ�������*/
typedef struct
{
    DHC_U8 u8PgaGain;                                      /*PGA ����*/
    DHC_U8 u8DigitalGain;                                  /*Digital ����*/
 }DHC_DH9931_AUDIO_INVOLUME_CFG_S;

typedef struct
{
	DHC_U8 u8CascadeNum;                                    /*������Ŀ1 - 4*/
	DHC_U8 u8CascadeStage;                                  /*��������0 - 3*/
}DHC_DH9931_AUDIO_CONNECT_MODE_S;

typedef struct
{
    DHC_BOOL bEncEn;                                        /*����ģ��ʹ��*/
    DHC_BOOL bEncFreqEn;                                    /*Mclkʹ���ź�*/
    DHC_DH9931_AUDIO_ENC_MODE_E enEncMode;                  /*����ģʽѡ��*/
    DHC_DH9931_AUDIO_SAMPLERATE_E enSampleRate;             /*������*/
    DHC_DH9931_AUDIO_SYNC_MODE_E enSyncMode;                /*��Ƶ�������ģʽ*/

    DHC_DH9931_AUDIO_CHN_SEL_E enChnSel[DHC_DH9931_MAX_CHNS];/*����Դѡ��*/
}DHC_DH9931_AUDIO_ENC_CFG_S;

typedef struct
{
    DHC_BOOL bDecEn;                                         /*����ģ��ʹ��*/
    DHC_DH9931_AUDIO_SYNC_MODE_E enSyncMode;                 /*��Ƶ�������ģʽ*/
    DHC_DH9931_AUDIO_DEC_MODE_E enDecMode;                   /*��Ƶ����ģʽ*/
    DHC_U8 u8OutputChnSel;                                   /*�������ͨ��ѡ��*/ 
}DHC_DH9931_AUDIO_DEC_CFG_S;

/*��Ƶ�������� :(1)���ӷ�ʽ:�����Ǽ���
**(2) ������(3) ����ģʽ
*/
typedef struct
{
	DHC_DH9931_AUDIO_CONNECT_MODE_S stConnectMode;           /*��Ƶ��������*/	
	DHC_DH9931_AUDIO_ENC_CFG_S stEncCfg;                     /*��������*/
    DHC_DH9931_AUDIO_DEC_CFG_S stDecCfg;                     /*��������*/  
    DHC_DH9931_AUDIO_SAMPLERATE_E enSampleRate;              /*����Ƶ��*/
}DHC_DH9931_AUDIO_ATTR_S;

/*���ģʽ*/
typedef enum 
{
    DHC_WORK_MODE_1Multiplex = 0,                           /*1xģʽ*/
    DHC_WORK_MODE_2Multiplex,                               /*2xģʽ*/
    DHC_WORK_MODE_4Multiplex,                               /*4xģʽ*/  
    DHC_WORK_MODE_BUTT,
}DHC_DH9931_VO_WORK_MODE_E;

/*ͨ����*/
typedef enum 
{
    DHC_INPUT_CHN_0 = 0,                                    /*ͨ��0*/
    DHC_INPUT_CHN_1,                                        /*ͨ��1*/
    DHC_INPUT_CHN_2,                                        /*ͨ��2*/
    DHC_INPUT_CHN_3,                                        /*ͨ��3*/    
    DHC_INPUT_CHN_BUTT,
}DHC_DH9931_INPUT_CHN_E;

/*ͨ����*/
typedef enum 
{
    DHC_CHN_ID_0 = 0,                                       /*ͨ��ID 0*/
    DHC_CHN_ID_1,                                           /*ͨ��ID 1*/
    DHC_CHN_ID_2,                                           /*ͨ��ID 2*/
    DHC_CHN_ID_3,                                           /*ͨ��ID 3*/    
    DHC_CHN_ID_BUTT,
}DHC_DH9931_CHN_ID_E;

/*Netra ��ʽ*/
typedef enum 
{
    DHC_NETRA_MODE_DOUBLE = 0,                              /*˫ͷ��ʽ*/
    DHC_NETRA_MODE_SINGLE,                                  /*��ͷ��ʽ*/    
    DHC_NETRA_MODE_BUTT,
}DHC_DH9931_NETRA_MODE_E;

/*���ģʽѡ��*/
typedef enum 
{
    DHC_VO_MODE_BT1120 = 0,                                 /*BT1120*/
    DHC_VO_MODE_BT656,                                      /*BT656*/   
    DHC_VO_MODE_BUTT,
}DHC_DH9931_VO_MODE_E;

/*���Ƶ��ѡ��*/
typedef enum 
{
    DHC_VO_HD_GM_MODE_148M = 0,                             /*148.5M ʱ��*/ 
    DHC_VO_HD_GM_MODE_74M,                                  /*74.25M ʱ��*/
   
    DHC_VO_HD_GM_MODE_BUTT,
}DHC_DH9931_VO_HD_GM_MODE_E;

/*���ģʽѡ��*/
typedef enum 
{
    DHC_VO_SD_MODE_720H = 0,                                /*720H*/
    DHC_VO_SD_MODE_960H,                                    /*960H*/  
    DHC_VO_SD_MODE_BUTT,
}DHC_DH9931_VO_SD_MODE_E;

/*���زɼ�ģʽ*/
typedef enum
{
    DHC_VOCLK_EDGE_RISING = 0,                             /*�����زɼ�*/
    DHC_VOCLK_EDGE_DUAL = 1,                               /*���±��زɼ�ģʽ*/   
    DHC_VOCLK_EDGE_BUT,                                    /*��Ч*/
}DHC_DH9931_VO_CLK_EDGE_E;

/*ͨ��ID���ط�ʽ*/
typedef enum
{
    DHC_CHN_ID_MODE_NO = 0,                              /*ID�Ų���*/
    DHC_CHN_ID_MODE_HBLANK,                              /*ID�ż�����������*/   
    DHC_CHN_ID_MODE_HEADINFO,                            /*ID�ż���ͷ��Ϣ��*/
    DHC_CHN_ID_MODE_BOTH,                                /*ID��ͬʱ����������������ͷ��Ϣ��*/
}DHC_DH9931_CHN_ID_MODE_E;

typedef struct
{
    DHC_DH9931_INPUT_CHN_E enFirstInputChn;              /*ѡ�������·��Ƶ*/
    DHC_DH9931_INPUT_CHN_E enSecondInputChn;             /*ѡ�������·��Ƶ,��2xģʽ����Ч*/
    DHC_DH9931_CHN_ID_MODE_E enChnIdMode;                /*ѡ��ͨ��Id���������е�λ��*/
    DHC_DH9931_CHN_ID_E enChnId;                         /*ͨ��Id*/
    DHC_DH9931_NETRA_MODE_E enNetraMode;                 /*ѡ��Netra��ʽ*/
    DHC_DH9931_VO_MODE_E enVoMode;                       /*���ģʽѡ��*/
    DHC_DH9931_VO_HD_GM_MODE_E enVoHdGmMode;             /*ѡ�����ʱ��*/
    DHC_DH9931_VO_SD_MODE_E enVoSdMode;                  /*ѡ������ı���ģʽ*/
    DHC_DH9931_VO_CLK_EDGE_E enVoClkEdge;                /*ѡ��ʱ�ӱ��ز�ģʽ*/
}DHC_DH9931_VO_CFG_S;

/*freerun �����ƵЭ��*/
typedef enum
{
    DHC_PROTOCOL_CVI = 0,                                 /*CVIЭ��*/                
    DHC_PROTOCOL_AHD = 1,                                 /*AHDЭ��*/
    DHC_PROTOCOL_TVI = 2,                                 /*TVIЭ��*/   
    DHC_PROTOCOL_BUT = 3,
}DHC_DH9931_PROTOCOL_E;

/*Freerun �����ʽ*/
typedef enum
{
    DHC_SD_FREERUN_FMT_P = 0,                             /*P��*/                
    DHC_SD_FREERUN_FMT_N = 1,                             /*N��*/  
    DHC_SD_FREERUN_FMT_BUT = 2,
}DHC_DH9931_FREERUN_SD_FMT_E;

/*Freerun �����ʽ*/
typedef enum
{
    DHC_SD_FREERUN_HD = 0,                                /*����*/                
    DHC_SD_FREERUN_SD = 1,                                /*����*/   
    DHC_SD_FREERUN_BUT = 2,
}DHC_DH9931_FREERUN_HD_SD_SEL_E;

/*Freerun �����ʽ*/
typedef enum
{
    DHC_SD_FREERUN_HD_74_25 = 0,                          /*74.25M*/                
    DHC_SD_FREERUN_HD_148_5 = 1,                          /*148.5M*/
    DHC_SD_FREERUN_HD_37_125 = 2,                         /*37.125M*/     
    DHC_SD_FREERUN_HD_297 = 3,                            /*297M*/
    DHC_SD_FREERUN_HD_144 = 4,                            /*144M*/ 
    DHC_SD_FREERUN_HD_288 = 5,                            /*288M*/    
    DHC_SD_FREERUN_HD_BUT = 6,
}DHC_DH9931_FREERUN_HD_CLK_E;

/*FreeRun��ɫ����*/
typedef enum
{
    DHC_FREERUN_COLOR_WHITE = 0,                         /*��ɫ*/
    DHC_FREERUN_COLOR_YELLOW,                            /*��ɫ*/  
    DHC_FREERUN_COLOR_CYAN,                              /*��ɫ*/ 
    DHC_FREERUN_COLOR_GREEN,                             /*��ɫ*/    
    DHC_FREERUN_COLOR_MAGENTA,                           /*���ɫ*/ 
    DHC_FREERUN_COLOR_RED,                               /*��ɫ*/
    DHC_FREERUN_COLOR_BLUE,                              /*��ɫ*/
    DHC_FREERUN_COLOR_BLACK,                             /*��ɫ*/
	DHC_FREERUN_COLOR_BUT,                               /*��Ч*/
}DHC_DH9931_COLOR_E;

typedef struct
{
    DHC_DH9931_PROTOCOL_E enProtocol;
    DHC_DH9931_FREERUN_SD_FMT_E enSdFmt;
    DHC_DH9931_FREERUN_HD_SD_SEL_E enHdSdSel;
    DHC_DH9931_FREERUN_HD_CLK_E enHdClk;
    DHC_DH9931_VIDEO_FMT_E enFreeRunFmt;
    DHC_DH9931_COLOR_E enColor;
}DHC_DH9931_FREERUN_CFG_S;

typedef struct
{
   DHC_DH9931_VO_CFG_S stVoCfg;
   DHC_DH9931_FREERUN_CFG_S stFreeRunCfg;
}DHC_DH9931_VIDEO_ATTR_S;

typedef struct
{
    DHC_I2C_BUS_E enI2cBus;                                                /*ѡ��I2C����*/
	DHC_U8 u8ChipAddr;                                                     /*I2C��ַ*/	
    DHC_U8 u8ProtocolType;                                                 /*CO485Э������0: С���ڣ�1: �󴰿�*/     
    DHC_BOOL bCheckChipId;                                                 /*Ч��оƬ��ID0: ��У�飬1: У��*/ 
    DHC_DH9931_DRIVER_POWER_E enDriverPower;
    DHC_DH9931_EQ_MODE_E enEqMode;    
    DHC_DH9931_VO_WORK_MODE_E enVoWorkMode;                                /*����ģʽ*/
    DHC_DH9931_AUDIO_COAXIAL_MDOE_E enCoaxialMode[DHC_DH9931_MAX_CHNS];    /*ͬ����Ƶ�¾�ģʽģʽ����*/
	DHC_DH9931_VIDEO_ATTR_S   stVideoAttr[DHC_DH9931_MAX_CHNS];
	DHC_DH9931_AUDIO_ATTR_S   stAudioAttr;
}DHC_DH9931_ATTR_S;

typedef struct
{
     /*������AD ����*/
	DHC_U8 u8AdCount;
	
	/*�Լ���ʱʱ�����ã���λ10ms�������Ҫ���� 500ms
	    ����û��뾫ȷ����ʱ�䣬�����ó�0
	 */
	DHC_U8 u8DetectPeriod;
	
	/*ÿһƬDH9901������Ϣ*/
	DHC_DH9931_ATTR_S stDH9931Attr[DHC_DH9931_MAX_CHIPS];
	
	/*��д�Ĵ�����������ʼ��ʱ��Ҫע��*/
	DHC_DH9931_ERR_ID_E(* DH9931_WriteByte)(DHC_I2C_BUS_E enI2cBus, DHC_U8 u8ChipAddr, DHC_U16 u16RegAddr, DHC_U8 u8RegValue);
	DHC_U8(* DH9931_ReadByte)(DHC_I2C_BUS_E enI2cBus, DHC_U8 u8ChipAddr, DHC_U16 u16RegAddr);
	
	/*ƽ̨�޹ػ�����Ҫע����غ���*/
	
	/*���뼶�����ʱ����*/
	DHC_VOID(*DH9931_MSleep)(DHC_U8 u8MsDly);
	
	/*���Դ�ӡ�ӿ�*/
	DHC_VOID(*DH9931_Printf)(DHC_CHAR *pszStr);
	
	/*��ȡ������*/
	DHC_VOID(*DH9931_GetLock)(void);
	
	/*�ͷ���*/
	DHC_VOID(*DH9931_ReleaseLock)(void);  
}DHC_DH9931_USR_CFG_S;

/*��������*/

/*************************************************
  ������ :  DHC_DH9931_SDK_Init
  ��  �� :  SDK��ʼ�����
  ��  �� :  ����                ����
            pstDh9931UsrCfg   �û���������ú�ע����Ϣ
  ��  �� :  ��
  ��  �� :  ����ֵ                  ����
            DHC_ERRID_SUCCESS       ��ʼ���ɹ�
            DHC_ERRID_INVPOINTER    �����Чָ��
            DHC_ERRIF_FAILED        ��ʼ��ʧ��  
*************************************************/
DHC_DH9931_ERR_ID_E DHC_DEF_API DHC_DH9931_SDK_Init(DHC_DH9931_USR_CFG_S *pstDh9931UsrCfg);

/*************************************************
  ������ :  DHC_DH9931_SDK_DeInit
  ��  �� :  SDKȥ��ʼ�����
  ��  �� :  ����                ����
            pstDh9931UsrCfg   �û���������ú�ע����Ϣ
  ��  �� :  ��
  ��  �� :  ����ֵ                  ����
            DHC_ERRID_SUCCESS       ȥ��ʼ���ɹ�
            DHC_ERRID_NOINIT        �޳�ʼ��  
*************************************************/
DHC_DH9931_ERR_ID_E DHC_DEF_API DHC_DH9931_SDK_DeInit(DHC_VOID);

/*************************************************
  ������ :  DHC_DH9931_SDK_DetectThr
  ��  �� :  DH9931��⺯�����Զ����
  ��  �� :  ����                ����
  ��  �� :  ����                ����
            pu8Value            �Ĵ���ֵ
  ��  �� :  ����ֵ              ����
            DHC_ERRID_SUCCESS  ִ�гɹ�
*************************************************/
//DHC_VOID DHC_DEF_API DHC_DH9931_SDK_DetectThr(DHC_VOID);
DHC_VOID DHC_DEF_API DHC_DH9931_SDK_DetectThr(DHC_DH9931_DETECT_ATTR *pstDetectAttr);



/*************************************************
  ������ :  DHC_DH9931_SDK_CleanCo485Buf
  ��  �� :  ���485buffer
  ��  �� :  ����                ����
            ucChipIndex     оƬ����,ָ����ƬDH9931
            u8Chn           DH9931��ͨ��
  ��  �� :  ��
  ��  �� :  ����ֵ                  ����
            DHC_ERRID_SUCCESS        ִ�гɹ�
*************************************************/
DHC_DH9931_ERR_ID_E DHC_DEF_API DHC_DH9931_SDK_CleanCo485Buf(DHC_U8 u8ChipIndex, DHC_U8 u8Chn);

/*************************************************
  ������ :  DHC_DH9931_SDK_SendCo485Enable
  ��  �� :  DH9931 485����ʹ�ܣ�����ͨ������
            ÿ�β���485ǰʹ�ܣ�������Ϻ�رգ�
            ��Ҫ���ͨ��ͬʱʹ��
  ��  �� :  ����                ����
            ucChipIndex     оƬ����,ָ����ƬDH9931
            u8Chn           DH9931��ͨ��
            bEnable         ʹ�ܿ���
  ��  �� :  ��
  ��  �� :  ����ֵ                  ����
            DHC_ERRID_SUCCESS      ִ�гɹ�
*************************************************/
DHC_DH9931_ERR_ID_E DHC_DEF_API DHC_DH9931_SDK_SendCo485Enable(DHC_U8 u8ChipIndex, DHC_U8 u8Chn, DHC_BOOL bEnable);

/*************************************************
  ������ :  DHC_DH9931_SDK_SendCo485Buf
  ��  �� :  д485buffer
  ��  �� :  ����                ����
            ucChipIndex     оƬ����,ָ����ƬDH9931
            u8Chn           DH9931��ͨ��
            bEnable         ʹ�ܿ���
  ��  �� :  ��
  ��  �� :  ����ֵ                  ����
            DHC_ERRID_SUCCESS        ִ�гɹ�
*************************************************/
DHC_DH9931_ERR_ID_E DHC_DEF_API DHC_DH9931_SDK_SendCo485Buf(DHC_U8 u8ChipIndex, DHC_U8 u8Chn, DHC_U8 *pu8Buf, DHC_U8 u8Lenth);

/*************************************************
  ������ :  DHC_DH9931_SDK_RecvCo485Enable
  ��  �� :  DH9931 485����ʹ�ܣ�����ͨ������
            ÿ�β���485ǰʹ�ܣ�������Ϻ�رգ�
            ��Ҫ���ͨ��ͬʱʹ��
  ��  �� :  ����                ����
            ucChipIndex     оƬ����,ָ����ƬDH9931
            u8Chn           DH9931��ͨ��
            bEnable         ʹ�ܿ���
  ��  �� :  ��
  ��  �� :  ����ֵ                  ����
            DHC_ERRID_SUCCESS      ִ�гɹ�
*************************************************/
DHC_DH9931_ERR_ID_E DHC_DEF_API DHC_DH9931_SDK_RecvCo485Enable(DHC_U8 u8ChipIndex, DHC_U8 u8Chn, DHC_BOOL bEnable);

/*************************************************
  ������ :  DHC_DH9931_SDK_RecvCo485Buf
  ��  �� :  ��485buffer
  ��  �� :  ����                ����
            ucChipIndex     оƬ����,ָ����ƬDH9931
            u8Chn           DH9931��ͨ��
            pu8Buf          �����ݻ���
            pu8Lenth        ���ݸ���
  ��  �� :  ��
  ��  �� :  ����ֵ                  ����
            DHC_ERRID_SUCCESS        ִ�гɹ�
*************************************************/
DHC_DH9931_ERR_ID_E DHC_DEF_API DHC_DH9931_SDK_RecvCo485Buf(DHC_U8 u8ChipIndex, DHC_U8 u8Chn, DHC_U8 *pu8Buf, DHC_U8 *pu8Lenth);

/*************************************************
  ������ :  DHC_DH9931_SDK_TestCo485Enable
  ��  �� :  DH9931 485����ģʽʹ�ܣ�����ͨ������
            ��Ҫ���ͨ��ͬʱʹ��
  ��  �� :  ����                ����
            ucChipIndex     оƬ����,ָ����ƬDH9931
            u8Chn           DH9931��ͨ��
            bEnable         ����ʹ�ܿ��� ��Ч
  ��  �� :  ��
  ��  �� :  ����ֵ                  ����
            DHC_ERRID_SUCCESS      ִ�гɹ�
*************************************************/
DHC_DH9931_ERR_ID_E DHC_DEF_API DHC_DH9931_SDK_TestCo485Enable(DHC_U8 u8ChipIndex, DHC_U8 u8Chn, DHC_BOOL bEnable, DHC_DH9931_RS485_PROTOCOL_E eRs485Pro);

/*************************************************
  ������ :  DHC_DH9931_SDK_ClearEq
  ��  �� :  DH9931���ͼ�����,ͼ����������������,
            ��ȷ���غ�ſɽ���ɫ�ʶ�ȡ��
  ��  �� :  ����                ����
            ucChipIndex     оƬ����,ָ����ƬDH9931
            u8Chn           DH9931��ͨ��
  ��  �� :  ��
  ��  �� :  ����ֵ                  ����
            DHC_ERRID_SUCCESS       ������ɫ�ɹ�
*************************************************/
DHC_DH9931_ERR_ID_E DHC_DEF_API DHC_DH9931_SDK_ClearEq(DHC_U8 u8ChipIndex, DHC_U8 u8Chn);

/*************************************************
  ������ :  DHC_DH9931_SDK_SetVideoPos
  ��  �� :  ͼ��ƫ�Ƶ���, ����ǰ�ȶ�ȡ��ǰλ����Ϣ�����ݵ�ǰλ�õ���ƫ��
            ������Χ0 - 15
  ��  �� :  ����                ����
            ucChipIndex     оƬ����,ָ����ƬDH9931
            u8Chn           DH9931��ͨ��
            pstVideoPos     ƫ�Ʋ���
  ��  �� :  ��
  ��  �� :  ����ֵ                  ����
            DHC_ERRID_SUCCESS        ִ�гɹ�
*************************************************/
DHC_DH9931_ERR_ID_E DHC_DEF_API DHC_DH9931_SDK_SetVideoPos(DHC_U8 u8ChipIndex, DHC_U8 u8Chn, DHC_DH9931_IMG_OFFSET_S *pstVideoOffset);

/*************************************************
  ������ :  DHC_DH9931_SDK_GetVideoPos
  ��  �� :  ��ȡͼ��ƫ��λ����Ϣ
  ��  �� :  ����                ����
            ucChipIndex         оƬ����,ָ����ƬDH9931
            u8Chn               DH9931��ͨ��
  ��  �� :  ����                ����
            pstVideoPos         ƫ�Ʋ���
  ��  �� :  ����ֵ              ����
            DHC_ERRID_SUCCESS  ִ�гɹ�
*************************************************/
DHC_DH9931_ERR_ID_E DHC_DEF_API DHC_DH9931_SDK_GetVideoPos(DHC_U8 u8ChipIndex, DHC_U8 u8Chn, DHC_DH9931_IMG_OFFSET_S *pstVideoOffset);

/*************************************************
  ������ :  DHC_DH9931_SDK_SetClkPhase
  ��  �� :  ����DH9931ʱ����λ�����ڵ�����Ƶ����ʱ��ƥ��
  ��  �� :  ����                    ����
            ucChipIndex             оƬ����,ָ����ƬDH9931
            u8Chn                   DH9931��ͨ��
            u8Invert                ʱ��ȡ��,  0 : ���� 1 : ȡ��
            u8Delay                 ʱ���ӳ٣� ȡֵ��Χ 00..0f
  ��  �� :  ����                    ����
            ��                      ��
  ��  �� :  ����ֵ                  ����
            DHC_ERRID_SUCCESS       ִ�гɹ�
            DHC_ERRID_UNSUPPORT     ��֧�ִ����
*************************************************/
DHC_DH9931_ERR_ID_E DHC_DEF_API DHC_DH9931_SDK_SetClkPhase(DHC_U8 u8ChipIndex, DHC_U8 u8Chn, DHC_BOOL bInvert, DHC_U8 u8Delay);

/*************************************************
  ������ :  DHC_DH9931_SDK_GetVideoStatus
  ��  �� :  ��ȡADоƬ��ʧ����Ƶ��ʽ
  ��  �� :  ����                ����
            ucChipIndex         оƬ����,ָ����ƬDH9931
            u8Chn               DH9931��ͨ��
  ��  �� :  ����                ����
            peVideoStatus       ��ǰ��Ƶ״̬
  ��  �� :  ����ֵ              ����
            DHC_ERRID_SUCCESS  ִ�гɹ�
*************************************************/
DHC_DH9931_ERR_ID_E DHC_DEF_API DHC_DH9931_SDK_GetVideoStatus(DHC_U8 u8ChipIndex, DHC_U8 u8Chn, DHC_DH9931_VIDEO_STATUS_S *pstVideoStatus);

/*************************************************
  ������ :  DHC_DH9931_SDK_SetColor
  ��  �� :  DH9931������ɫ����
  ��  �� :  ����                ����
            u8ChipIndex     оƬ����,ָ����ƬDH9931
            u8Chn           DH9931��ͨ��
            pstVideoColor   ��ɫ����
  ��  �� :  ��
  ��  �� :  ����ֵ                  ����
            DHC_ERRID_SUCCESS       ������ɫ�ɹ�
*************************************************/
DHC_DH9931_ERR_ID_E DHC_DEF_API DHC_DH9931_SDK_SetColor(DHC_U8 u8ChipIndex, DHC_U8 u8Chn, DHC_DH9931_VIDEO_COLOR_S *pstVideoColor, DHC_SET_MODE_E enColorMode);

/*************************************************
  ������ :  DHC_DH9931_SDK_GetColor
  ��  �� :  DH9931��ȡ��ɫ����
  ��  �� :  ����                ����
            u8ChipIndex     оƬ����,ָ����ƬDH9931
            u8Chn           DH9931��ͨ��
            pstVideoColor   ��ɫ����
            eColorMode      ����ģʽ��Ĭ�ϻ����û�
  ��  �� :  ��
  ��  �� :  ����ֵ                  ����
            DHC_ERRID_SUCCESS       ������ɫ�ɹ�
*************************************************/
DHC_DH9931_ERR_ID_E DHC_DEF_API DHC_DH9931_SDK_GetColor(DHC_U8 u8ChipIndex, DHC_U8 u8Chn, DHC_DH9931_VIDEO_COLOR_S *pstVideoColor);

/*************************************************
  ������ :  DHC_DH9931_SDK_SetCableType
  ��  �� :  ���ô���������
  ��  �� :  ����                ����
            ucChipIndex         оƬ����,ָ����ƬDH9931
            u8Chn               DH9931��ͨ��
            u8Volume            �߲����ͣ�ȡֵDHC_CableType_E
  ��  �� :  ����                ����
            ��
  ��  �� :  ����ֵ              ����
            DHC_ERRID_SUCCESS  ִ�гɹ�
*************************************************/
DHC_DH9931_ERR_ID_E DHC_DEF_API DHC_DH9931_SDK_SetCableType(DHC_U8 u8ChipIndex, DHC_U8 u8Chn, DHC_DH9931_CABLE_TYPE_E enUserCableType);

/*************************************************
  ������ :  DHC_DH9931_SDK_SetMsgLevel
  ��  �� :  ��ӡ��Ϣ����������ã�ע�⣬���SDK��֧�ִ�ӡ
            ����������κ�Ч��
  ��  �� :  ����                    ����
            eMsgLevel               ��ӡ��Ϣ�������
  ��  �� :  ����                    ����
            ��                      ��
  ��  �� :  ����ֵ                  ����
            DHC_ERRID_SUCCESS       ִ�гɹ�
            DHC_ERRID_UNSUPPORT     ��֧�ִ����
*************************************************/
DHC_DH9931_ERR_ID_E DHC_DEF_API DHC_DH9931_SDK_SetMsgLevel(DHC_DH9931_MSG_TYPE_E eMsgLevel);

/*************************************************
  ������ :  DHC_DH9931_SDK_SetEqLevel
  ��  �� :  ����EQ�ĸߵ�
  ��  �� :  ����                ����
            ucChipIndex         оƬ����,ָ����ƬDH9931
            u8Chn               DH9931��ͨ��
            u8Level             ����
  ��  �� :  ����                ����
            ��
  ��  �� :  ����ֵ              ����
            DHC_ERRID_SUCCESS  ִ�гɹ�
*************************************************/
DHC_DH9931_ERR_ID_E DHC_DEF_API DHC_DH9931_SDK_SetEqLevel(DHC_U8 u8ChipIndex, DHC_U8 u8Chn, DHC_U8 u8Level);

/*************************************************
  ������ :  DHC_DH9931_SDK_GetEqLevel
  ��  �� :  ��ȡEQ�ĸߵ�
  ��  �� :  ����                ����
            ucChipIndex         оƬ����,ָ����ƬDH9931
            u8Chn               DH9931��ͨ��
            *u8Level             �ȼ�
  ��  �� :  ����                ����
            ��
  ��  �� :  ����ֵ              ����
            DHC_ERRID_SUCCESS  ִ�гɹ�
*************************************************/
DHC_DH9931_ERR_ID_E DHC_DEF_API DHC_DH9931_SDK_GetEqLevel(DHC_U8 u8ChipIndex, DHC_U8 u8Chn, DHC_U8 *u8Level);

/*************************************************
  ������ :  DHC_DH9931_SDK_SetFreeRum
  ��  �� :  ����FreeRun
  ��  �� :  ����                    ����
            ucChipIndex             оƬ����,ָ����ƬDH9931
            u8Chn                   DH9931��ͨ��
            bEnable                 0:�Զ�ģʽ  1:�ֶ�ģʽ
            DHC_DH9931_COLOR_E          FreeRun��ɫ
            DHC_DH9931_VIDEO_FMT_E          FreeRun��ʽ
  ��  �� :  ����                    ����
            ��                      ��
  ��  �� :  ����ֵ                  ����
            DHC_ERRID_SUCCESS       ִ�гɹ�
            DHC_ERRID_UNSUPPORT     ��֧�ִ����
*************************************************/
DHC_DH9931_ERR_ID_E DHC_DEF_API DHC_DH9931_SDK_SetFreeRun(DHC_U8 u8ChipIndex, DHC_U8 u8Chn, DHC_BOOL bEnable, DHC_DH9931_COLOR_E enColorBar, DHC_DH9931_VIDEO_FMT_E enVideoFmt);

/*************************************************
  ������ :  DHC_DH9931_SDK_SetHalfMode
  ��  �� :  ����DH9931��Ƶ�����ʱ
  ��  �� :  ����                    ����
            ucChipIndex             оƬ����,ָ����ƬDH9931
            u8Chn                   DH9931��ͨ��
            bHalfEn                 halfģʽʹ��
            DHC_DH9931_HALFMODE_E   ģʽѡ��
  ��  �� :  ����                    ����
            ��                      ��
  ��  �� :  ����ֵ                  ����
            DHC_ERRID_SUCCESS       ִ�гɹ�
            DHC_ERRID_UNSUPPORT     ��֧�ִ����
*************************************************/
DHC_DH9931_ERR_ID_E DHC_DEF_API DHC_DH9931_SDK_SetHalfMode(DHC_U8 u8ChipIndex, DHC_U8 u8Chn, DHC_BOOL bHalfEn, DHC_DH9931_HALFMODE_E enHalfMode);

/*************************************************
  ������ :  DHC_DH9931_SDK_SetAudioInVolume
  ��  �� :  ����ͬ��������Ƶ����
  ��  �� :  ����                ����
            ucChipIndex         оƬ����,ָ����ƬDH9931
            u8Chn               DH9931��ͨ��
            u8Volume            ����
  ��  �� :  ����                ����
            ��
  ��  �� :  ����ֵ              ����
            DHC_ERRID_SUCCESS  ִ�гɹ�
*************************************************/
DHC_DH9931_ERR_ID_E DHC_DEF_API DHC_DH9931_SDK_SetCoaxialVolume(DHC_U8 u8ChipIndex, DHC_U8 u8Chn, DHC_U8 u8Volume);

/*************************************************
  ������ :  DHC_DH9931_SDK_SetLineInVolume
  ��  �� :  ����LineIn������Ƶ����
  ��  �� :  ����                ����
            ucChipIndex         оƬ����,ָ����ƬDH9931
            u8Chn               LineIn����ͨ��
            stVolume            ��Ƶ��������
  ��  �� :  ����                ����
            ��
  ��  �� :  ����ֵ              ����
            DHC_ERRID_SUCCESS  ִ�гɹ�
*************************************************/
DHC_DH9931_ERR_ID_E DHC_DEF_API DHC_DH9931_SDK_SetLineInVolume(DHC_U8 u8ChipIndex, DHC_U8 u8Chn, DHC_DH9931_AUDIO_INVOLUME_CFG_S stVolume);

/*************************************************
  ������ :  DHC_DH9931_SDK_SetLineOutVolume
  ��  �� :  ����LineOut�����Ƶ����
  ��  �� :  ����                ����
            ucChipIndex         оƬ����,ָ����ƬDH9931
            u8Volume            �������ܹ���Ϊ16������(0--15), 0��С��15���
  ��  �� :  ����                ����
            ��
  ��  �� :  ����ֵ              ����
            DHC_ERRID_SUCCESS  ִ�гɹ�
*************************************************/
DHC_DH9931_ERR_ID_E DHC_DEF_API DHC_DH9931_SDK_SetLineOutVolume(DHC_U8 u8ChipIndex, DHC_U8 u8Volume);

/*************************************************
  ������ :  DHC_DH9931_SDK_SelectDecOutputChn
  ��  �� :  �����������ͨ��ѡ��
  ��  �� :  ����                ����
            ucChipIndex         оƬ����,ָ����ƬDH9931
            u8OutputChn         ���ͨ����
  ��  �� :  ����                ����
            ��
  ��  �� :  ����ֵ              ����
            DHC_ERRID_SUCCESS  ִ�гɹ�
*************************************************/
DHC_DH9931_ERR_ID_E DHC_DEF_API DHC_DH9931_SDK_SelectDecOutputChn(DHC_U8 u8ChipIndex, DHC_U8 u8OutputChn);

/*******************************************************************************
 * ������  : DHC_DH9931_SDK_SelectDACSource
 * ��  ��  : ѡ��DAC���������Դ��
 * ��  ��  : ����                ����
 *         : u8ChipIndex         оƬ����ָ����һƬDH9931
 *         : enSource            DAC���������Դѡ��
 * ��  ��  : ��
 * ����ֵ  : DHC_ERRID_SUCCESS: �ɹ�
 *******************************************************************************/
DHC_DH9931_ERR_ID_E DHC_DEF_API DHC_DH9931_SDK_SelectDACSource(DHC_U8 u8ChipIndex, DHC_DH9931_AUDIO_DAC_SOURCE enSource);

/*******************************************************************************
 * ������  : DHC_DH9931_SDK_SetIOElectric
 * ��  ��  : ���Ƹ�ͨ��vo������������ı书��
 * ��  ��  : ����                ����
 *         : u8ChipIndex         оƬ����ָ����һƬDH9931
 *         : vd_lv               ��������ȼ�
 * ��  ��  : ��
 * ����ֵ  : DHC_ERRID_SUCCESS: �ɹ�
 *******************************************************************************/
DHC_DH9931_ERR_ID_E DHC_DEF_API DHC_DH9931_SDK_SetIOElectric(DHC_U8 u8ChipIndex, DHC_U8 u8Chn, DHC_DH9931_VIDEO_ELECTRIC_E enVideoElec);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /*_DHC_DH9931_API_H_*/
