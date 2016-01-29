#ifndef __APP_FACEBEAUTY_H__
#define __APP_FACEBEAUTY_H__

#include "FIHImgProc.h"

void App_FIHFaceBeauty_Init(FIH_ImageFrame* pFrameInfo, FIH_AppImgProcParams* pParams);
void App_FIHFaceBeauty_Uninit(void);
void App_FIHFaceBeauty_Preview(FIH_ImageFrame* pFrameInfo, FIH_AppImgProcParams* pParams);
void App_FIHFaceBeauty_Capture(FIH_ImageFrame* pFrameInfo, FIH_AppImgProcParams* pParams);

#endif //__APP_FACEBEAUTY_H__
