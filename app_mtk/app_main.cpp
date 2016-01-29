

void CaptureProcess_LocalHistEqual_noSaturation(uchar* pYUYV, int width, int height)
{
#define PRINT_COST_TIME    1
#define USE_OPENCL         1

#if PRINT_COST_TIME
    double t;
    cvStartTimer(t);
#endif

    Mat mDst, mChannels[2];
    Mat mSrcYUYV( height, width, CV_8UC2, pYUYV );

    split( mSrcYUYV, mChannels );
    Mat& mY = mChannels[0];

#if USE_OPENCL
    ocl::setUseOpenCL(true);
    if(  ocl::haveOpenCL() )
      MY_LOGD("OCL");
    else
      MY_LOGD("OCL Failed");

    UMat umY;
    mY.copyTo(umY);
#endif

	Ptr<CLAHE> clahe = createCLAHE();
	clahe->setClipLimit(1);
	clahe->apply(umY,mDst);

    /*
     * Convert to BGR
     */
    int channels = mDst.channels();
    int nRows = mDst.rows;
    int nCols = mDst.cols * channels;
    if( mDst.isContinuous() )
    {
        nCols *= nRows;
        nRows = 1;
        uchar* pY = mDst.ptr<uchar>(0);
  #pragma omp parallel for 
        for( int j = 0; j < nCols; j++)
        {
            *pYUYV++ = *pY++;
            *pYUYV++;
        }
    }
    else{
  #pragma omp parallel for 
        for( int i = 0; i < nRows; ++i)
        {
            uchar* pY = mDst.ptr<uchar>(i);
            for( int j = 0; j < nCols; j++)
            {
                *pYUYV++ = *pY++;
                *pYUYV++;
            }
        }
    }
  
#if PRINT_COST_TIME  
    cvStopTimer(t);
    MY_LOGT("Local Hist Equal, %d x %d, cost %lfs",height, width, t);
#endif

#undef PRINT_COST_TIME
#undef USE_OPENCL
}

void CaptureProcess_LocalHistEqual(uchar* pYUYV, int width, int height)
{
#define PRINT_COST_TIME    1
#define USE_OPENCL         0

#if PRINT_COST_TIME  
    double t;
    cvStartTimer(t);
#endif

    /*
     * Get Y channel
     */
    Mat mDst, mChannels[2];
    Mat mSrcYUYV( height, width, CV_8UC2, pYUYV );

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
    //const int levels[] = {20,20,30,30,40,40,50,50,60,60,70};
    //int level = levels[g_level];
    int level = 30;

    cvSaturation(mBGR, mDst, level);
    ColorCvt_BGR2YUYV(mBGR, pYUYV);


#if PRINT_COST_TIME  
    cvStopTimer(t);
    MY_LOGD("Local Hist Equal, %d x %d, cost %lfs",height, width, t);
#endif

#undef PRINT_COST_TIME
#undef USE_OPENCL
}




void CaptureProcess_Dehazing(uchar* pYUYV, int width, int height)
{
    Mat mBGR;
    Mat mSrcYUYV( height>>1, width>>1, CV_8UC2, pYUYV );
    cvtColor( mSrcYUYV, mBGR, CV_YUV2BGR_YUY2 );

    //MY_LOGD("mBGR.cols %d, mBGR.rows %d", mBGR.cols, mBGR.rows);
    dehazing dehazingImg(mBGR.cols, mBGR.rows, 16, false, false, 5.0f, 1.0f, 40);
    
    Mat mResult;
	dehazingImg.ImageHazeRemoval(mBGR, mResult);

    ColorCvt_BGR2YUYV(mResult, pYUYV);
}

void CaptureProcess_Denoising1(uchar* pYUYV, int width, int height)
{
   const int levels[] = {1,1,3,3,3,5,5,5,7,7,7};
    int level = levels[g_level];

    Mat mDst, mChannels[2];
    Mat mSrcYUYV( height, width, CV_8UC2, pYUYV );

    split( mSrcYUYV, mChannels );
    Mat& mY = mChannels[0];

  #if 1
    //bilateralFilter(mY, mDst, level, level<<1, level>>1);
    //fastNlMeansDenoising(mY, mDst, level);

#pragma omp parallel for    
	for(int i=0; i<8; i++) //row
	{
        Mat mSubY(mY, Rect(0, (height >> 4)*i, width, (height >> 4)));
        Mat mSubDY;
        fastNlMeansDenoising(mSubY, mSubDY, level);
        mSubDY.copyTo(mSubY);
    }
    mDst = mY;

  #else
    UMat umSrc;
    mY.copyTo(umSrc);
    UMat umDst;

    ocl::setUseOpenCL(true);
    if(  ocl::haveOpenCL() )
      MY_LOGD("OCL");
    else
      MY_LOGD("OCL Failed");

    //bilateralFilter(umSrc, umDst, level, level<<1, level>>1);
    fastNlMeansDenoising(umSrc, umDst, level, 7, 15);
    umDst.copyTo(mDst);
  #endif

    int channels = mDst.channels();
    int nRows = mDst.rows;
    int nCols = mDst.cols * channels;
    if( mDst.isContinuous() )
    {
        nCols *= nRows;
        nRows = 1;
    }

    for( int i = 0; i < nRows; ++i)
    {
        uchar* pY = mDst.ptr<uchar>(i);
        for( int j = 0; j < nCols; j++)
        {
            *pYUYV++ = *pY++;
            *pYUYV++;
        }
    }
}



void CaptureProcess_Denoising2(uchar* pYUYV, int width, int height)
{
    const int levels[] = {1,1,3,3,3,5,5,5,7,7,7};
    int level = levels[g_level];

    Mat mDst, mChannels[2];
    Mat mSrcYUYV( height, width, CV_8UC2, pYUYV );

    split( mSrcYUYV, mChannels );
    Mat& mY = mChannels[0];

    //bilateralFilter(mY, mDst, level, level<<1, level>>1);
  #if 0
    fastNlMeansDenoising(mY, mDst, level);
  #else
    UMat umSrc;
    mY.copyTo(umSrc);
    UMat umDst;

    ocl::setUseOpenCL(true);
    if(  ocl::haveOpenCL() )
      MY_LOGD("OCL");
    else
      MY_LOGD("OCL Failed");

    fastNlMeansDenoising(umSrc, umDst, level);

    //mDst = umDst;
    umDst.copyTo(mDst);
  #endif

    int channels = mDst.channels();
    int nRows = mDst.rows;
    int nCols = mDst.cols * channels;
    if( mDst.isContinuous() )
    {
        nCols *= nRows;
        nRows = 1;
    }

    for( int i = 0; i < nRows; ++i)
    {        
        uchar* pY = mDst.ptr<uchar>(i);
        for( int j = 0; j < nCols; j++)
        {
            *pYUYV++ = *pY++;
            *pYUYV++;
        }
    }
}



