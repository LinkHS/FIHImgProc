#define LOG_TAG "FIH/app_shdr"
#define DEBUG_LEVEL                 0

#include "opencv.hpp"

//#include "app_shdr.h"
#include "common/debug.h"
#include "fihimgproc_params.h"
#include "basic/others.h"

using namespace cv;

#define MY_LOGD(fmt, arg...)        FIH_LOGD("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        FIH_LOGI("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        FIH_LOGW("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        FIH_LOGE("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGT(fmt, arg...)        FIH_LOGD("Temp: [%s] "fmt, __FUNCTION__, ##arg) //temp log

#define MY_LOGD_IF(cond, ...)       do { if (cond) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGD_LEVEL(level, ...)   do { if (level >= DEBUG_LEVEL) { MY_LOGD(__VA_ARGS__); } }while(0)

void App_SHDR_Capture(FIH_ImageFrame* pFrameInfo, FIH_SHDR_Params* pParams)
{
#define PRINT_COST_TIME    0

#if PRINT_COST_TIME  
    double t;
    PfmTest_StartTimer(t);
#endif

    int height = pFrameInfo->height;
    int width = pFrameInfo->width;

    /*
     * Get Y channel
     */
    Mat mDst, mChannels[2];
    Mat mSrcYUYV( height, width, CV_8UC2, pFrameInfo->pAddr );

    split( mSrcYUYV, mChannels );
    Mat& mY = mChannels[0];

    /*
     * Local Histogram Equalization in Y Channel
     */
	Ptr<CLAHE> clahe = createCLAHE();
	clahe->setClipLimit(1);
	clahe->apply(mY,mDst);

    /*
     * Convert to BGR
     */
    Mat mBGR;
    mChannels[0] = mDst;
    merge(mChannels, 2, mSrcYUYV);
    cvtColor( mSrcYUYV, mBGR, CV_YUV2BGR_YUY2 );

    /*
     * Increase Saturation
     */
    Color_Saturation( mBGR, pParams->saturation );
    ColorCvt_BGR2YUYV( mBGR, pFrameInfo->pAddr );


#if PRINT_COST_TIME  
    PfmTest_StopTimer(t);
    MY_LOGD("Local Hist Equal, %d x %d, cost %lfs",height, width, t);
#endif

#undef PRINT_COST_TIME
}
