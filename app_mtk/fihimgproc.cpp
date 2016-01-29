#define LOG_TAG "FIH/imgproc"
#define DEBUG_LEVEL                 0

#include "fihimgproc.h"
#include "opencv.hpp"
#include "common/debug.h"
#include "fihimgproc_params.h"

#define MY_LOGD(fmt, arg...)        FIH_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        FIH_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        FIH_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        FIH_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGT(fmt, arg...)        FIH_LOGD("Temp: [%s] " fmt, __FUNCTION__, ##arg) //temp log

#define MY_LOGD_IF(cond, ...)       do { if (cond) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGD_LEVEL(level, ...)   do { if (level >= DEBUG_LEVEL) { MY_LOGD(__VA_ARGS__); } }while(0)

using namespace cv;

extern void App_SHDR_Capture(FIH_ImageFrame* pFrameInfo, FIH_SHDR_Params* pParams);

void FIH_CaptureProcess_Handler(FIH_IMGPROC_CMD cmd, FIH_ImageFrame* pFrameInfo, void* pParams)
{
    MY_LOGI("+, p1 %x, p2 %x", pFrameInfo, pParams);

    switch( cmd )
    {
    case FIH_FaceBeauty_MODE:
        //App_FIHFaceBeauty_Capture( pFrameInfo, pParams );
        break;
    case FIH_LocalHistEqual_MODE:
        //CaptureProcess_LocalHistEqual( pFrameInfo->pAddr, pFrameInfo->width, pFrameInfo->height );
        App_SHDR_Capture(pFrameInfo, (FIH_SHDR_Params *)pParams);
        break;
    default:
        MY_LOGE("Unspported FIH_CaptureProcess Mode %d", cmd);
        break;
    }
}
