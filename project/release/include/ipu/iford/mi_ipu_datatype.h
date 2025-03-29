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

#ifndef _MI_IPU_DATATYPE_H_
#define _MI_IPU_DATATYPE_H_

#include "mi_common_datatype.h"

#define MI_IPU_HEAD_VER "v1.0.1"

#define MI_IPU_MAX_TENSOR_DIM 10
#define MI_IPU_MAX_INPUT_TENSOR_CNT 60
#define MI_IPU_MAX_OUTPUT_TENSOR_CNT 60
#define MI_IPU_MAX_TENSOR_CNT \
                    ((MI_IPU_MAX_INPUT_TENSOR_CNT>MI_IPU_MAX_OUTPUT_TENSOR_CNT)? \
                    MI_IPU_MAX_INPUT_TENSOR_CNT:MI_IPU_MAX_OUTPUT_TENSOR_CNT)

#define MAX_TENSOR_NAME_LEN     256
#define MAX_IPU_INPUT_OUTPUT_BUF_DEPTH 3
#define MI_IPU_MAX_TIMEOUT  (20*1000)    //20 seconds
#define MI_IPU_BOOT_MAX_TIMEOUT (5*1000)   //5 seconds
#define MI_IPU_MAX_BATCH_TYPE_NUM 10

#define YUV420_W_PITCH_ALIGNMENT    2
#define YUV420_H_PITCH_ALIGNMENT        2
#define XRGB_W_PITCH_ALIGNMENT          1


typedef MI_U32 MI_IPU_CHN;
typedef int (*SerializedReadFunc)(void *dst_buf,int offset, int size, char *ctx);

typedef enum
{
    MI_IPU_FORMAT_U8,
    MI_IPU_FORMAT_NV12,
    MI_IPU_FORMAT_INT16,
    MI_IPU_FORMAT_INT32,
    MI_IPU_FORMAT_INT8,
    MI_IPU_FORMAT_FP32,
    MI_IPU_FORMAT_RESERVED,
    MI_IPU_FORMAT_ARGB8888,
    MI_IPU_FORMAT_ABGR8888,
    MI_IPU_FORMAT_GRAY,
    MI_IPU_FORMAT_COMPLEX64,
    MI_IPU_FORMAT_UINT16,
    MI_IPU_FORMAT_UNKNOWN = 255,
} MI_IPU_ELEMENT_FORMAT;

typedef enum
{
    E_IPU_ERR_INVALID_CHNID = 1,        /* invalid channel ID */
    E_IPU_ERR_CHNID_EXIST,              /* channel exists */
    E_IPU_ERR_CHNID_UNEXIST,        /* channel unexists */
    E_IPU_ERR_NOMEM,        /* failure caused by malloc memory */
    E_IPU_ERR_NOBUF,            /* failure caused by malloc buffer */
    E_IPU_ERR_BADADDR,      /* bad address, buffer address doesn't get from IPU buffer allocator */
    E_IPU_ERR_SYS_TIMEOUT,  /* system timeout*/
    E_IPU_ERR_FILE_OPERATION,   /* file cannot be open or read or write */
    E_IPU_ERR_ILLEGAL_TENSOR_BUFFER_SIZE,   /* tensor buffer size cannot meet the requirement, usually less than requirement*/
    E_IPU_ERR_ILLEGAL_BUFFER_DEPTH,     /* input or output buffer depth quantum is more than maximum number */
    E_IPU_ERR_ILLEGAL_INPUT_OUTPUT_DESC,    /* network description is illegal, usually mean input or output buffer quantum is wrong */
    E_IPU_ERR_ILLEGAL_INPUT_OUTPUT_PARAM,   /* user's input or output buffer quantum isn't match network description */
    E_IPU_ERR_MAP,  /* address mapping error */
    E_IPU_ERR_INIT_FIRMWARE,    /* fail to init ipu firmware */
    E_IPU_ERR_CREATE_CHANNEL, /* fail to create channel */
    E_IPU_ERR_DESTROY_CHANNEL,  /* fail to destroy channel */
    E_IPU_ERR_INVOKE,   /* fail to invoke */
    E_IPU_ERR_SET_MALLOC_REGION,    /* fail to set malloc region for freertos */
    E_IPU_ERR_SET_IPU_PARAMETER,    /* fail to set IPU parameter */
    E_IPU_ERR_INVALID_PITCH_ALIGNMENT,  /* invalid pitch alignment */
    E_IPU_ERR_NO_CREATED_IPU_DEVICE,   /* there is no created IPU device */
    E_IPU_ERR_GET_IPU_VERSION,      /* fail to get ipu version from ipu firmware */
    E_IPU_ERR_MISMATCH_IPU_HEAD_FILE,    /* ipu head files version isn't matching */
    E_IPU_ERR_NO_SUPPORT_REQ,        /* ipu firmware do not support this request */
    E_IPU_ERR_FAILED,   /* unexpected error */
    E_IPU_ERR_SEND_REQUEST, /* failt to send request to IPU */
    E_IPU_ERR_GET_FIRMWARE_INFO, /* fail to get ipu firmware information */
    E_IPU_ERR_INVALID_IPUCORE_BOOTING_PARAM, /* invalid IPU cores booting parameters */
    E_IPU_ERR_INVALID_IPUCORE_SHUTDOWNING_PARAM, /* invalid IPU cores shutdowning parameters */
    E_IPU_ERR_NO_MULTICORE_ENV, /* multicore mode needs all ipu cores being alive */
    E_IPU_ERR_INVALID_TASK_PRIORITY, /* invalid ipu task priority */
    E_IPU_ERR_DEV_SHUTDOWN, /* ipu core has been shutdown */
    E_IPU_ERR_DEV_FAIL_RESET, /* fail to reset ipu */
    E_IPU_ERR_DEV_FAIL_SHUTDOWN, /* fail to shutdown ipu */
    E_IPU_ERR_NO_AVAILABLE_DEV, /* no available ipu dev */
    E_IPU_ERR_RESET_OFF, /* reset function is off */
    E_IPU_ERR_INVALID_BATCH_NUM, /* batch number error */
    E_IPU_ERR_BATCH_TYPE, /* batch type error */
    E_IPU_ERR_BATCH_MODE, /* batch mode error */
    E_IPU_ERR_NO_AVAILABLE_BATCH_MODE, /* do not find available batch mode */
    E_IPU_ERR_IPU_HANG, /* invoke was dropped due to ipu hang */
    E_IPU_ERR_NO_RESET_DEV, /* no reset ipu dev */
    E_IPU_ERR_NO_BATCH_PARAM, /* no batch parameter */
    E_IPU_ERR_INVALID_MODEL_BUFFER, /* invalid user model buffer physical address or size */
    E_IPU_ERR_INVALID_VARIABLE_BUFFER, /* invalid variable buffer physical address or size */
    E_IPU_ERR_NOT_ASSIGN_CORE, /* not assign ipu core when use user's variable buffer */
    E_IPU_ERR_SWDISP_NOT_REGISTER, /* model has unsupported swdisp function */
    E_IPU_ERR_SWDISP_NOT_FIND_TASKID, /* not find swdisp task id */
    E_IPU_ERR_SWDISP_INVALID_PARAM, /* invalid swdisp parameter */
    E_IPU_ERR_SWDISP_UNEXPECTED, /* unexpected swdisp error */
    E_IPU_ERR_SWDISP_UNKNOWN, /* unknown swdisp error */
    E_IPU_ERR_BAD_PHY_ADDR_ALIGNMENT, /* ipu buffer physical addr not aligned */
    E_IPU_ERR_MISMATCH_INVOKE_FUNC, /* original/customized model should use MI_IPU_Invoke2/MI_IPU_Invoke2Custom */
    E_IPU_ERR_MISMATCH_MODEL, /* other platform's model */
    E_IPU_ERR_INVOKE_CANCELED, /* invoke has been canceled */
    E_IPU_ERR_INVOKE_CANCEL_FAIL, /* fail to cancel invoke */
    E_IPU_ERR_NOT_SUPPORT_CANCELINVOKE, /* do not support cancel invoke */
    E_IPU_ERR_INVOKE_INTERRUPT, /* invoke task was interrupted (maybe on suspend), please try again*/
    E_IPU_ERR_PERMISSION_DENIED, /* permission denied */
    E_IPU_ERR_NO_AVAILABLE_CHNID = 1<<8,   /* there is no available channel */
}IPU_ErrCode_e;

typedef enum {
    E_IPU_BATCH_N_BUF_MODE = 0,
    E_IPU_BATCH_ONE_BUF_MODE,
} MI_IPU_BatchMode_e;

typedef enum {
    E_IPU_LAYOUT_TYPE_NHWC = 0,
    E_IPU_LAYOUT_TYPE_NCHW,
} MI_IPU_LayoutType_e;

typedef enum {
    E_IPU_IPU_WORK_MODE_SINGLECORE = 0,
    E_IPU_IPU_WORK_MODE_MULTICORE,
} MI_IPU_IpuWorkMode_e;

typedef struct MI_IPU_TensorDesc_s {
    MI_U32 u32TensorDim;
    MI_IPU_ELEMENT_FORMAT eElmFormat;
    MI_U32 u32TensorShape[MI_IPU_MAX_TENSOR_DIM]; // 3,299,299
    char name[MAX_TENSOR_NAME_LEN];
    MI_U32 u32InnerMostStride;
    MI_FLOAT fScalar;
    MI_S64 s64ZeroPoint;
    MI_S32 s32AlignedBufSize;
    MI_U32 u32BufSize;
    MI_U32 u32InputWidthAlignment;
    MI_U32 u32InputHeightAlignment;
    MI_IPU_LayoutType_e eLayoutType;
    MI_U32 au32Reserve[4]; // reserve
} MI_IPU_TensorDesc_t;

typedef struct MI_IPU_SubNet_InputOutputDesc_s {
    MI_U32 u32InputTensorCount;
    MI_U32 u32OutputTensorCount;
    MI_IPU_TensorDesc_t astMI_InputTensorDescs[MI_IPU_MAX_INPUT_TENSOR_CNT];
    MI_IPU_TensorDesc_t astMI_OutputTensorDescs[MI_IPU_MAX_OUTPUT_TENSOR_CNT];
} MI_IPU_SubNet_InputOutputDesc_t;

typedef struct MI_IPU_Tensor_s {
    void *ptTensorData[2];
    MI_PHY phyTensorAddr[2];//notice that this is miu bus addr,not cpu bus addr.
} MI_IPU_Tensor_t;

typedef struct MI_IPU_TensorVector_s {
    MI_U32 u32TensorCount;
    MI_IPU_Tensor_t  astArrayTensors[MI_IPU_MAX_TENSOR_CNT];
} MI_IPU_TensorVector_t;

typedef enum {
    IPU_DEV_0 = (1<<0),
    IPU_DEV_1 = (1<<1),
} IPU_DEV_ID;

typedef struct MI_IPU_DevAttr_s {
    MI_U32 u32MaxVariableBufSize;
    MI_U32 u32YUV420_W_Pitch_Alignment; // unused
    MI_U32 u32YUV420_H_Pitch_Alignment; // unused
    MI_U32 u32XRGB_W_Pitch_Alignment;   // unused
    MI_U32 u32VariableGroup;            // variable group ID
    MI_U32 u32CoreMask;                 // ipu core mask
    MI_U32 au32Reserve[6];              // reserve
} MI_IPU_DevAttr_t;

typedef struct MI_IPUChnAttr_s {
    MI_U32 u32SubNetId;
    /*
     * pre-allocated input tensor vector buffers:  u32BatchMax * u32InputBufDepth
     * pre-allocated output tensor vector buffers: u32BatchMax * u32OutputBufDepth
     */
    MI_U32 u32OutputBufDepth;
    MI_U32 u32InputBufDepth;
    MI_U32 u32BatchMax;
    MI_U32 au32Reserve[8]; // reserve
} MI_IPUChnAttr_t;

#define MI_IPU_MAX_BATCH_TENSOR_CNT 256
typedef struct MI_IPU_BatchInvokeParam_s {
    /* user's variable buffer physical address */
    MI_PHY u64VarBufPhyAddr;
    MI_U32 u32VarBufSize;

    MI_U32 u32BatchN;
    /*
     * rt: [-10,-1], -10 is the highest priority of rt priority
     * normal: [0,39], 0 is the highest priority of normal priority
     */
    MI_S32 s32TaskPrio;
    /*
     *  0 or (IPU_DEV_0|IPU_DEV_1): scheduled by ipu
     *  IPU_DEV_0: bind to ipu0
     *  IPU_DEV_1: bind to ipu1
     */
    MI_U32 u32IpuAffinity;
    MI_IPU_Tensor_t astArrayTensors[MI_IPU_MAX_BATCH_TENSOR_CNT];
    MI_U32 au32Reserve[8]; // reserve
} MI_IPU_BatchInvokeParam_t;

typedef struct MI_IPU_RuntimeInfo_s {
    MI_U64 u64BandWidth;
    MI_U64 u64IpuTime; //micro-seconds
    MI_U64 u64BandWidthRead;
    MI_U64 u64BandWidthWrite;
    MI_U32 au32Reserve[8]; // reserve
} MI_IPU_RuntimeInfo_t;

typedef struct MI_IPU_OfflineModelStaticInfo_s {
    MI_U32 u32VariableBufferSize;
    MI_U32 u32OfflineModelSize;
    MI_IPU_BatchMode_e eBatchMode;
    /*
     *  The number of batchNum types that this model supports.
     *  if eBatchMode == E_IPU_BATCH_N_BUF_MODE:
     *      u32TotalBatchNumTypes always equals to 2.
     *  if eBatchMode == E_IPU_BATCH_ONE_BUF_MODE:
     *      u32TotalBatchNumTypes equals to 1~10.
     */
    MI_U32 u32TotalBatchNumTypes;
    /*
     *  All types of batchNum supported.
     *  if eBatchMode == E_IPU_BATCH_N_BUF_MODE:
     *      It returns the max batchNum supported in this model at au32BatchNumTypes[0],
     *      It returns the biggest number of all the batches suggested in this model at au32BatchNumTypes[1].
     *          (The batchNum suggested shall be a number or several numbers of the form 2^n where n>=0.)
     *          (When au32BatchNumTypes[1] = 2^n, which means the batchNums suggested of this model are 2^0, 2^1, ..., 2^(n-1), 2^n)
     *  if eBatchMode == E_IPU_BATCH_ONE_BUF_MODE:
     *      It returns all batchNum types supported in this modelat au32BatchNumTypes[0] ~ au32BatchNumTypes[u32TotalBatchNumTypes - 1]
     *
     *  ex1: N_BUF mode model supports up to 20 batches (1~20 batches), and batchnums suggested of this model are (1,2,4,8)
     *     eBatchMode = E_IPU_BATCH_N_BUF_MODE
     *     u32TotalBatchNumTypes = 2
     *     au32BatchNumTypes[0] = 20
     *     au32BatchNumTypes[1] = 8
     *  ex2: ONE_BUF mode model supports 10 batches, 20 batches, 30 batches
     *     eBatchMode = E_IPU_BATCH_ONE_BUF_MODE
     *     u32TotalBatchNumTypes = 3
     *     au32BatchNumTypes[0] = 10; au32BatchNumTypes[1] = 20; au32BatchNumTypes[2] = 30
     */
    MI_U32 au32BatchNumTypes[MI_IPU_MAX_BATCH_TYPE_NUM];
    /* This model's ipu work mode is single-core or multi-core */
    MI_IPU_IpuWorkMode_e eIpuWorkMode;
    MI_U32 au32Reserve[8]; // reserve
} MI_IPU_OfflineModelStaticInfo_t;


#endif // !_MI_IPU_DATATYPE_H_
