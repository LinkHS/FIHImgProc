#define LOG_TAG "FIH/FIHImgProc"
#define DEBUG_LEVEL                 0

#include "includes.h"
#include "opencv.hpp"

#include "app_facebeauty.h"

#include "basic_process/cvOthers.h"
#include "basic_process/cvSharpen.h"
#include "modules/cvLocalStatisticsFiltering.h"
#include "modules/VideoLSFilter.h"
#include "porting/debug.h"
#include "porting/MTK_FDManager.h"
#include "FihFaceInfo.h"

using namespace cv;

#define MY_LOGD(fmt, arg...)        FIH_LOGD("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        FIH_LOGI("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        FIH_LOGW("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        FIH_LOGE("[%s] "fmt, __FUNCTION__, ##arg)
#define MY_LOGT(fmt, arg...)        FIH_LOGD("Temp: [%s] "fmt, __FUNCTION__, ##arg) //temp log

#define MY_LOGD_IF(cond, ...)       do { if (cond) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGD_LEVEL(level, ...)   do { if (level >= DEBUG_LEVEL) { MY_LOGD(__VA_ARGS__); } }while(0)


VideoLSFilter g_videoLSFilter;

extern void WYMCAdjustment_RedPixels(Mat& mSrc, Mat& mDst, const int WYMC[4]);
extern void WYMCAdjustment_BlackPixels(Mat& mSrc, Mat& mDst, struct WYMCParams* pWYMCParams);
extern MTK_FDManager *FIH_GetInstanceOfFDManager(void);

void App_FIHFaceBeauty_Capture(FIH_ImageFrame* pFrameInfo, FIH_AppImgProcParams* pParams)
{
    uchar* pYUYV = pFrameInfo->pAddr;
    int width = pFrameInfo->width;
    int height = pFrameInfo->height;
    int smoothLevel = pParams->fb_level;
    /* convert smoothLevel from [-12, 12] to [0, 12] */
    smoothLevel = (smoothLevel + 12) >> 1;
    smoothLevel = (smoothLevel > 10 ) ? 10 : smoothLevel;

    int faceRect[4];
    int smoothSize=0, fw, fh;

    MTK_FDManager *FDManager = FIH_GetInstanceOfFDManager();
    if ( FDManager != NULL )
    {
        if( FDManager->GetFirstFaceRect((int *)faceRect) )
        {
            FDManager->CalibFaceCoord( faceRect, 1, height, width);

            /* Calculate the width and height */
            fw = faceRect[2] - faceRect[0];  /* w = right - left */
            fh = faceRect[3] - faceRect[1];  /* h = bottom - top */

            smoothSize = std::max(fw,fh)/55;
        }
    }

    if ( smoothSize == 0)
    {
        smoothSize = std::max(width, height)/80;
    }


    int fb_gamma = 1, fb_sharpen = 50;

    /* White, Yellow, Magenta, Cyan */
    const int WYMC1[4] = {30, 0, 0, -40};
    const int WYMC2[4] = {30, 0, 0, -40};
    const int WYMC3[4] = {50, 0, 0, -50};
    const int WYMC4[4] = {50, 0, 0, -70};
    int const *fb_pWYMC_Red = WYMC1;

    /* [0, 10] to [0,4] */
    switch (smoothLevel/2) {
        case 0:
            return;
        case 1:
            fb_gamma = 1.1;
            fb_pWYMC_Red = WYMC1;
            fb_sharpen = 50;
            smoothLevel += 2;
            break;
        case 2:
            fb_gamma = 1.1;
            fb_pWYMC_Red = WYMC2;
            fb_sharpen = 60;
            break;
        case 3:
            fb_gamma = 1.1;
            fb_pWYMC_Red = WYMC3;
            fb_sharpen = 60;
            break;
        case 4:
            fb_gamma = 1.2;
            fb_pWYMC_Red = WYMC3;
            fb_sharpen = 70;
            break;
        case 5:
            fb_gamma = 1.2;
            fb_pWYMC_Red = WYMC4;
            fb_sharpen = 70;
            break;
    }

    /* 1. YUY2 convert to BGR(Mat)*/
    Mat mBGR;
    Mat mSrcYUYV( height, width, CV_8UC2, pYUYV );
    cvtColor( mSrcYUYV, mBGR, CV_YUV2BGR_YUY2 );

    /*
     * 2. Image Porcessing for Color Processing 
     * 2.1 Color Whitening 
     */
    Mat mDstBGR;
    WYMCAdjustment_RedPixels(mBGR, mDstBGR, fb_pWYMC_Red);   mBGR = mDstBGR;
    //if (smoothLevel > 6) {
    //    WYMCAdjustment_BlackPixels(mBGR, mDstBGR, NULL); mBGR = mDstBGR;
    //}
    //mBGR.convertTo(mDstBGR, -1, 1.2, 0); 

    /* 2.2 Get Y channel data */
    Mat mYCrCb, mChannels[3];
    cvtColor( mBGR, mYCrCb, CV_BGR2YCrCb );
    split( mYCrCb, mChannels );
    Mat& mY = mChannels[0];

    Mat mDstY;
    cvCorrectGamma( mY, mDstY, fb_gamma ); mY = mDstY;
    //mDstY.convertTo(mDstY, -1, 1.2, 0);

    cvLocalStatisticsFiltering_YChannel(mY, mDstY, smoothSize, smoothLevel);
    mY = mDstY;

    SharpenParams parms = {fb_sharpen, NULL};
    cvSharpen(mY, mDstY, &parms);
    mY = mDstY;

    merge(mChannels, 3, mYCrCb);
    cvtColor( mYCrCb, mBGR , CV_YCrCb2BGR );

    /*
     * 3. BGR convert to YUY2 and copy back
     */
    ColorCvt_BGR2YUYV(mBGR, pYUYV);
}

void App_FIHFaceBeauty_Preview(FIH_ImageFrame* pFrameInfo, FIH_AppImgProcParams* pParams)
{
    int faceRects[3][4];
    int frameH, frameW;

    MTK_FDManager *FDManager = FIH_GetInstanceOfFDManager();
    if ( (FDManager == NULL) )
    {
        MY_LOGE("NULL pointer, FDManager(%d)", FDManager);
        return;
    }

    g_videoLSFilter.pData = pFrameInfo->pAddr;
    
    int faceNum = FDManager->GetFDResult((int *)faceRects);
    if ( faceNum > 0 ) 
    {
        g_videoLSFilter.GetFrameSize(&frameH, &frameW);
        FDManager->CalibFaceCoord((int *)faceRects, faceNum, frameH, frameW);
        if( g_videoLSFilter.CalCumulDist_Rows3((int *)faceRects, faceNum) )
        {
            int smoothLevel = (pParams->fb_level + 12) >> 1; //[-12, 12] to [0, 12]
            g_videoLSFilter.FastLSV_NoiseFiltering_Rough4(smoothLevel);
        }
    }

    //MY_LOGD_LEVEL(0, "faceNum=%d, smoothLevel=%d", faceNum, *(int *)pUserData);
}

void App_FIHFaceBeauty_Init(FIH_ImageFrame* pFrameInfo, FIH_AppImgProcParams* pParams)
{
    g_videoLSFilter.init1( pFrameInfo->height, pFrameInfo->width, 8 );
}

void App_FIHFaceBeauty_Uninit(void)
{
    g_videoLSFilter.uninit1();
}
