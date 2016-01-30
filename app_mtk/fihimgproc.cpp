#define LOG_TAG "FIH/imgproc"
#define DEBUG_LEVEL                 0

#include "fihimgproc.h"
#include "opencv.hpp"
#include "common/debug.h"
#include "fihimgproc_params.h"
#include "basic/others.h"
#include "modules/dehazing/dehazing.h"

#define MY_LOGD(fmt, arg...)        FIH_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        FIH_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        FIH_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        FIH_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGT(fmt, arg...)        FIH_LOGD("Temp: [%s] " fmt, __FUNCTION__, ##arg) //temp log

#define MY_LOGD_IF(cond, ...)       do { if (cond) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGD_LEVEL(level, ...)   do { if (level >= DEBUG_LEVEL) { MY_LOGD(__VA_ARGS__); } }while(0)

using namespace cv;

extern void App_SHDR_Capture(FIH_ImageFrame* pFrameInfo, FIH_SHDR_Params* pParams);

void App_Dehazing_Capture(FIH_ImageFrame* pFrameInfo)
{
#define PRINT_COST_TIME    0

#if PRINT_COST_TIME  
    double t;
    PfmTest_StartTimer(t);
#endif

    int height = pFrameInfo->height;
    int width = pFrameInfo->width;

    /* 1. YUY2 convert to BGR(Mat)*/
    Mat mBGR;
    Mat mSrcYUYV( height, width, CV_8UC2, pFrameInfo->pAddr );
    cvtColor( mSrcYUYV, mBGR, CV_YUV2BGR_YUY2 );

    Mat mResult;
    int dehazing_blk_size = 16;
    int nWid = mBGR.cols;
    int nHei = mBGR.rows;
    
    dehazing dehazingImg(nWid, nHei, 16, false, false, 5.0f, 1.0f, 40);
    dehazingImg.ImageHazeRemoval(mBGR, mResult);

    ColorCvt_BGR2YUYV(mResult, pFrameInfo->pAddr);

#if PRINT_COST_TIME  
    PfmTest_StopTimer(t);
    MY_LOGD("Local Dehazing Equal, %d x %d, cost %lfs",height, width, t);
#endif

#undef PRINT_COST_TIME
}

void FIH_CaptureProcess_Handler(FIH_IMGPROC_CMD cmd, FIH_ImageFrame* pFrameInfo, void* pParams)
{
    MY_LOGI("+, p1 %x, p2 %x", pFrameInfo, pParams);

    switch( cmd )
    {
    case FIH_FaceBeauty_MODE:
        //App_FIHFaceBeauty_Capture( pFrameInfo, pParams );
        break;
    case FIH_LocalHistEqual_MODE:
        {
            FIH_SHDR_Params params = { .saturation = 30 };
            App_SHDR_Capture( pFrameInfo, &params );
        }
        break;
    case FIH_Dehazing_MODE:
        {
            App_Dehazing_Capture( pFrameInfo, NULL );
		}
        break;
    default:
        MY_LOGE("Unspported FIH_CaptureProcess Mode %d", cmd);
        break;
    }
}

