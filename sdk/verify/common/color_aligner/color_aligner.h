#include "isp_cus3a_if.h"
#include "mi_isp_iq_datatype.h"
#include "mi_isp_af_datatype.h"
#include "mi_isp_ae_datatype.h"
#include "mi_isp_awb_datatype.h"
#include "mi_isp_hw_dep_datatype.h"

#define VERSION_MAJOR (1)
#define VERSION_MINOR (0)
#define VERSION_PATCH (4)

#define MAX_SUPPORT_NUM (MAX_CUST_3A_DEV_NUM*MAX_CUST_3A_CHINFO_NUM)
#define STITCHING_ALIGN_FRAME (60)
#define STITCHING_STA_INLIER_RATIO_BASE (100) // BASE=100
#define STITCHING_STABLE_CHANGE_THD_HIGH (1075) // BASE=1024
#define STITCHING_STABLE_CHANGE_THD_LOW (972) // BASE=1024
#define STITCHING_CONVERGE_RATIO_BASE (1024) // BASE 1024
#define STITCHING_ADJUST_BASE (4096) // BASE=1024
#define STITCHING_USE_AE

#define COLOR_NONE                 "\033[0m"
#define COLOR_BLACK                "\033[0;30m"
#define COLOR_BLUE                 "\033[0;34m"
#define COLOR_GREEN                "\033[0;32m"
#define COLOR_CYAN                 "\033[0;36m"
#define COLOR_RED                  "\033[0;31m"
#define COLOR_YELLOW               "\033[1;33m"
#define COLOR_WHITE                "\033[1;37m"

#define CLRALIGN_NOP(fmt, args...)
#define CLRALIGN_DBG(fmt, args...) \
    printf(COLOR_GREEN "[DBG]:[%s][%d]:  " COLOR_NONE, __FUNCTION__,__LINE__); \
    printf(fmt, ##args); \

#define CLRALIGN_WARN(fmt, args...) \
    printf(COLOR_YELLOW "[WARN]:[%s][%d]: " COLOR_NONE, __FUNCTION__,__LINE__); \
    printf(fmt, ##args); \

#define CLRALIGN_INFO(fmt, args...) \
    printf("[INFO]:[%s][%d]: \n", __FUNCTION__,__LINE__); \
    printf(fmt, ##args); \

#define CLRALIGN_ERR(fmt, args...) \
    printf(COLOR_RED "[ERR]:[%s][%d]: " COLOR_NONE, __FUNCTION__,__LINE__); \
    printf(fmt, ##args); \

#ifndef MAX
#define MAX(a,b) ((a)>(b) ? (a) : (b))
#endif
        
#ifndef MIN
#define MIN(a,b) ((a)<(b) ? (a) : (b))
#endif

#define DBGLVL_ALIGNPROCESS (0)
#define DBGLVL_DUMPSTA      (1)
#define DBGLVL_APPLYINFO    (2)

struct  Stitching_STA_t
{
    unsigned int u4AvgR;
    unsigned int u4AvgG;
    unsigned int u4AvgB;
    static bool compOnR(const Stitching_STA_t& a,const Stitching_STA_t& b) { return a.u4AvgR < b.u4AvgR; }
    static bool compOnG(const Stitching_STA_t& a,const Stitching_STA_t& b) { return a.u4AvgR < b.u4AvgR; }
    static bool compOnB(const Stitching_STA_t& a,const Stitching_STA_t& b) { return a.u4AvgR < b.u4AvgR; }
};


struct Stitching_Dev_Ch_t
{
    unsigned char  uDev;
    unsigned char uCh;
    char mappingx_path [128];
    char mappingy_path [128];
};

struct Stitching_Adjust_t
{
    unsigned short u2AdjustR;
    unsigned short u2AdjustG;
    unsigned short u2AdjustB;
};

#ifdef __cplusplus
class ColorAligner
{
    
    public:
        /**
         * @brief A constrctor.
         *
         * @param camera_map The information of camera from right to left.
         * @param valid_cam_num The number of valid camera.
         * @param image_width The resolution of camera.
         * @param image_height The resolution of camera.
         */
        ColorAligner(Stitching_Dev_Ch_t camera_map[], unsigned int valid_cam_num, unsigned int image_width, unsigned int image_height);
        
        /**
         * @brief A deconstructor.
         */
        ~ColorAligner();

        /**
         * @brief Start do color align.
         * 
         * @param debug_level 0: no log, 1: show align process, 2: dump statistics, 3: show apply information.
         */
        unsigned int Run(unsigned int debug_level=0);

    private:
        unsigned int Init();
        unsigned int UnInit();

        unsigned short u2CollectFrameNum;
        unsigned short u2STAInlierRatio; // base 100
        unsigned short u2STASaturationThd1; // base 1024
        unsigned short u2STASaturationThd2; // base 1024
        unsigned short u2STAMaxRGBThd1; // base 255
        unsigned short u2STAMaxRGBThd2; // base 255
        unsigned short u2STAMinRGBThd1; // base 255
        unsigned short u2STAMinRGBThd2; // base 255
        unsigned short u2LowLuxNotApplyThd;
        unsigned short u2ConvergeRatio;
        unsigned int u4ImageWidth;
        unsigned int u4ImageHeight;
        unsigned char  uMotionThd; // base 255
        unsigned char  uApplyIIRPre;
        unsigned char  uApplyIIRCur;
        unsigned char  uValidCameraNum;
        unsigned int uChangeSceneMotionThd;
        Stitching_Dev_Ch_t mCameraMap[MAX_SUPPORT_NUM];
        unsigned int mMotionStable;
        unsigned short bRetriggerFindMaxIndex;
        unsigned char uPreMaxIndex;
        unsigned int mChangeScene;
        MI_ISP_IQ_AwbAlign_t mStitchingPreGain[MAX_CUST_3A_DEV_NUM][MAX_CUST_3A_CHINFO_NUM];
        MI_ISP_IQ_AwbAlign_t mStitchingApplyGain[MAX_CUST_3A_DEV_NUM][MAX_CUST_3A_CHINFO_NUM];
        Stitching_Adjust_t mStitchingRefAdjust[MAX_CUST_3A_DEV_NUM][MAX_CUST_3A_CHINFO_NUM];
        Stitching_Adjust_t mStitchingApplyAdjust[MAX_CUST_3A_DEV_NUM][MAX_CUST_3A_CHINFO_NUM];
#ifdef STITCHING_USE_AE
        MI_ISP_AE_HW_STATISTICS_t tAeAvg[MAX_CUST_3A_DEV_NUM][MAX_CUST_3A_CHINFO_NUM];
        MI_ISP_AE_HW_STATISTICS_t tAeAvgPre[MAX_CUST_3A_DEV_NUM][MAX_CUST_3A_CHINFO_NUM];
#else
        MI_ISP_AWB_HW_STATISTICS_t tAwbAvg[MAX_CUST_3A_DEV_NUM][MAX_CUST_3A_CHINFO_NUM];
        MI_ISP_AWB_HW_STATISTICS_t tAwbAvgPre[MAX_CUST_3A_DEV_NUM][MAX_CUST_3A_CHINFO_NUM];
#endif
        Stitching_STA_t tStitchingSTA[MAX_CUST_3A_DEV_NUM*MAX_CUST_3A_CHINFO_NUM][2][STITCHING_ALIGN_FRAME];
        Stitching_STA_t tStitchingSTAMean[MAX_CUST_3A_DEV_NUM*MAX_CUST_3A_CHINFO_NUM][2];
        Stitching_STA_t tStitchingSTAMedian[MAX_CUST_3A_DEV_NUM*MAX_CUST_3A_CHINFO_NUM][2];
        MI_U32 mStitchingFrameCount;
        MI_U32 bFirstCal;
        float **ldc_mapping_x;
        float **ldc_mapping_y;
};

#else
typedef struct ColorAligner ColorAligner;
#endif

#ifdef __cplusplus
extern "C" {
#endif

ColorAligner* ColorAligner_create(Stitching_Dev_Ch_t camera_map[], MI_U32 valid_cam_num, MI_U32 image_width, MI_U32 image_height);

void ColorAligner_release(ColorAligner *self);

unsigned int ColorAligner_run(ColorAligner *self, unsigned int debug_level);

#ifdef __cplusplus
}
#endif
