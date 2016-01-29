#ifndef __SHARPEN_GMIP_H__
#define __SHARPEN_GMIP_H__

#include "opencv.hpp"
//#include "hopePixelRgn.h"

typedef struct
{
  int  sharpen_percent;
  //HopePixelRgn* roi;          /* Region of interest */
} SharpenParams;

void cvSharpen (cv::Mat& mSrc, cv::Mat& mDst, SharpenParams* params=NULL);

#endif //__SHARPEN_GMIP_H__

