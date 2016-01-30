#include "others.h"

using namespace cv;

void CorrectGamma( Mat& mSrc, Mat &mDst, double gamma )
{
    if ( gamma == 1 ) {
        mDst = mSrc;
        return;
    }

    double inverse_gamma = 1.0 / gamma;

	Mat lut_matrix(1, 256, CV_8UC1 );
	uchar * ptr = lut_matrix.ptr();
	for( int i = 0; i < 256; i++ )
		ptr[i] = (int)( pow( (double) i / 255.0, inverse_gamma ) * 255.0 );

	LUT( mSrc, lut_matrix, mDst );
}

/*
 * Extract Y(light) channel from RGB channels.
 * Currently this function only supports for CV_8UC3 type.
 */
void Mat_ConvertAndSplit(Mat& mSrc, Mat* pChannels, int code)
{
    Mat mXYZ;

    cvtColor(mSrc, mXYZ, code);
    split(mXYZ, pChannels);
}

/*
 * Parmameter - code: CV_BGR2GRAY, CV_BGR2YCrCb(default)
 */
void Mat_MergeAndConvert(Mat* pChannels, Mat& mDst, int code)
{
    Mat mXYZ;

    merge(pChannels, 3, mXYZ);
    cvtColor(mXYZ, mDst, code);
}

/*
 * Resize an image to a smaller one without changing the ratio. 
 * If the original size is smaller than the parameter(rect_size), just return.
 */
void Mat_ResizeDown(Mat &mSrc, Mat &mDst, int rect_size)
{
    int width  = mSrc.cols;
    int height = mSrc.rows;
    int max_dim = ( width >= height ) ? width : height;
    
    if( (max_dim > rect_size) && (rect_size > 0))
    {
        double scale = (double)max_dim / (double)rect_size;
        width  = (int)((double)width / scale);
        height = (int)((double)height / scale);
	    resize(mSrc, mDst, Size(width, height));
    }else
    {
        mDst = mSrc;
    }
}

/*
 * Resize an image to a bigger one without changing the ratio. 
 * If the original size is bigger than the parameter(rect_size), just return.
 */
void Mat_ResizeUp(Mat &mSrc, Mat &mDst, int rect_size)
{
    int width  = mSrc.cols;
    int height = mSrc.rows;
    int min_dim = ( width <= height ) ? width : height;
    
    if( (min_dim < rect_size) && (rect_size > 0) )
    {
        double scale = (double)rect_size / (double)min_dim;
        width  = (int)((double)width * scale);
        height = (int)((double)height * scale);
	    resize(mSrc, mDst, Size(width, height));
    }else
    {
        mDst = mSrc;
    }
}

static void doSaturation(void)
{

}

/*
    'mDst' can be as same as 'mSrc'
    'increment' range from (-100.0, 100.0)  
*/
void Color_Saturation(Mat& mSrc, int increment)
{
    int channels = mSrc.channels();
    int nRows = mSrc.rows;
    int nCols = mSrc.cols * channels;
    if( mSrc.isContinuous() )
    {
        nCols *= nRows;
        nRows = 1;
    }

    for( int i=0; i<nRows; ++i )
	{  
        uchar* pSrc = mSrc.ptr<uchar>(i);

      #pragma omp parallel for
        for( int j=0; j<nCols; j+=3)
        {
    		int t1 = pSrc[j+0];
    		int t2 = pSrc[j+1];
    		int t3 = pSrc[j+2];
    		int minVal = std::min(std::min(t1,t2), t3);  
    		int maxVal = std::max(std::max(t1,t2), t3);

            //float delta = maxVal - minVal;  
            //float L = 0.5*(maxVal + minVal);  
            //float S = max(0.5*delta/L, 0.5*delta/(255-L)) * 100;
    		float delta = (float)(maxVal-minVal);  
    		float L = (float)(maxVal+minVal);  
    		float S = std::max(delta/L, delta/(510-L));  

            if (increment > 0)
    		{  
    			float alpha = max(S, 1-(float)increment/100);  
    			alpha = (float)1.0/alpha - 1;  
    			pSrc[j+0] = (uchar)(t1 + (t1 - L/2.0) * alpha);  
    			pSrc[j+1] = (uchar)(t2 + (t2 - L/2.0) * alpha);  
    			pSrc[j+2] = (uchar)(t3 + (t3 - L/2.0) * alpha);  
    		}  
			else  
    		{  
    			//alpha = increment;  
    			//pSrc[j+0] = (L + (t1 - L) * (1+alpha));  
    			//pSrc[j+1] = (L + (t2 - L) * (1+alpha));  
    			//pSrc[j+2] = (L + (t3 - L) * (1+alpha));  
    		} 
        } 
	}  
}

#if 0
void Color_Saturation_float(Mat& mSrc, float increment)
{
	// increment (-100.0, 100.0)  
    increment = increment/100.0;  

    int channels = mSrc.channels();
    int nRows = mSrc.rows;
    int nCols = mSrc.cols * channels;
    if( mSrc.isContinuous() )
    {
        nCols *= nRows;
        nRows = 1;
    }

    for( int i=0; i<nRows; ++i )
	{  
        uchar* pSrc = mSrc.ptr<uchar>(i);
  #pragma omp parallel for 
        for( int j=0; j<nCols; j+=3)
        {
        	float alpha; 

    		float t1 = pSrc[j];
    		float t2 = pSrc[j+1];
    		float t3 = pSrc[j+2];

    		float minVal = std::min(std::min(t1,t2), t3);  
    		float maxVal = std::max(std::max(t1,t2), t3);  
    		float delta = (maxVal-minVal)/255.0;  
    		float L = 0.5*(maxVal+minVal)/255.0;  
    		float S = std::max(0.5*delta/L, 0.5*delta/(1-L));  

            if (increment > 0)
    		{  
    			alpha = max(S, 1-increment);  
    			alpha = 1.0/alpha-1;  
    			pSrc[j+0] = (t1 + (t1 - L*255.0) * alpha);  
    			pSrc[j+1] = (t2 + (t2 - L*255.0) * alpha);  
    			pSrc[j+2] = (t3 + (t3 - L*255.0) * alpha);  
    		}  
    		else  
    		{  
    			alpha = increment;  
    			pSrc[j+0] = (L*255.0 + (t1 - L*255.0) * (1+alpha));  
    			pSrc[j+1] = (L*255.0 + (t2 - L*255.0) * (1+alpha));  
    			pSrc[j+2] = (L*255.0 + (t3 - L*255.0) * (1+alpha));  
    		} 
        } 
	}  
}
#endif

/*
    Color Convert
*/
void ColorCvt_YUYV2BGR(uchar* pYUYV, uchar* pBGR, int length)
{
    for( int j = 0; j < length; )
    {
        uchar Y1 = *pYUYV++; 
        uchar U  = *pYUYV++; 
        uchar Y2 = *pYUYV++; 
        uchar V  = *pYUYV++; 
        
        int C1 = Y1 - 16;
        int C2 = Y2 - 16;
        int D  = U - 128;
        int E  = V - 128;

        pBGR[j++] = saturate_cast<uchar>(( 298 * C1 + 516 * D + 128) >> 8);           //B1
        pBGR[j++] = saturate_cast<uchar>(( 298 * C1 - 100 * D - 208 * E + 128) >> 8); //G1
        pBGR[j++] = saturate_cast<uchar>(( 298 * C1 + 409 * E + 128) >> 8);           //R1

        pBGR[j++] = saturate_cast<uchar>(( 298 * C2 + 516 * D + 128) >> 8);           //B2
        pBGR[j++] = saturate_cast<uchar>(( 298 * C2 - 100 * D - 208 * E + 128) >> 8); //G2
        pBGR[j++] = saturate_cast<uchar>(( 298 * C2 + 409 * E + 128) >> 8);           //R2
    }
}

void ColorCvt_YUYV2BGR(uchar* pYUYV, Mat& mBGR, int width, int height)
{
    Mat mSrcYUYV( height, width, CV_8UC2, pYUYV );

    cvtColor( mSrcYUYV, mBGR, CV_YUV2BGR_YUY2 );
}

void ColorCvt_BGR2YUYV(Mat& mBGR, uchar* pYUYV)
{
    int channels = mBGR.channels();
    int nRows = mBGR.rows;
    int nCols = mBGR.cols * channels;
    if( mBGR.isContinuous() )
    {
        nCols *= nRows;
        nRows = 1;
    }

    for( int i = 0; i < nRows; ++i)
    {
        uchar* pBGR = mBGR.ptr<uchar>(i);
        for( int j = 0; j < nCols; )
        {
            int B = pBGR[j++];
            int G = pBGR[j++];
            int R = pBGR[j++];
                        
            *pYUYV++ = ((66*R + 129*G + 25*B + 128) >> 8) + 16;
            *pYUYV++ = (j&0x01) ? (((-38*R - 74*G + 112*B + 128) >> 8) + 128) : (((112 * R - 94 * G - 18 * B + 128) >> 8) + 128);
        }
    }
}

// Return the number of bits in each channel of the given Mat. ie: 8, 16, 32 or 64.
int Mat_GetBitDepth(const Mat &M)
{
    switch (CV_MAT_DEPTH(M.type())) {
        case CV_8U:
        case CV_8S:
            return 8;
        case CV_16U:
        case CV_16S:
            return 16;
        case CV_32S:
        case CV_32F:
            return 32;
        case CV_64F:
            return 64;
    }

    return -1;
}

/* Compute cumulative sum and cumulative sum of square in row */
void Compute_CumulSum_CumulSumSqr_RowByRow(Mat &mSrc, int *pAccSumRows, int *pAccSqrSumRows, int firstRow, int rowNum, int padded)
{
    int nRows = mSrc.rows;
    int nCols = (padded == 1) ? (mSrc.cols+1) : mSrc.cols;

    memset( pAccSumRows,    0, nCols*rowNum*sizeof(int));
    memset( pAccSqrSumRows, 0, nCols*rowNum*sizeof(int));

    if ( padded == 1 )
    {
        for (int i = 0; i < rowNum; i++)
        {  
            uchar* pSrc = mSrc.ptr<uchar>(i+firstRow);
            int *pAccSumRow    = pAccSumRows    + nCols*i;
            int *pAccSqrSumRow = pAccSqrSumRows + nCols*i;

            for( int j=1; j<nCols; j++)
            {
                pAccSumRow[j]    = pAccSumRow[j-1]    + pSrc[j-1];
                pAccSqrSumRow[j] = pAccSqrSumRow[j-1] + pSrc[j-1]*pSrc[j-1];
            }
        }
    }else
    {
        for (int i = 0; i < rowNum; i++)
        {  
            uchar* pSrc = mSrc.ptr<uchar>(i+firstRow);
            int *pAccSumRow    = pAccSumRows    + nCols*i;
            int *pAccSqrSumRow = pAccSqrSumRows + nCols*i;

            pAccSumRow[0]    = pSrc[0];
            pAccSqrSumRow[0] = pSrc[0]*pSrc[0];

            for( int j=1; j<nCols; j++)
            {
                pAccSumRow[j]    = pAccSumRow[j-1]    + pSrc[j];
                pAccSqrSumRow[j] = pAccSqrSumRow[j-1] + pSrc[j]*pSrc[j];
            }
        }
    }
}

/* Compute cumulative sum and cumulative sum of square in row */
void Compute_CumulSum_CumulSumSqr(Mat &mSrc, Mat &mCumulSums, Mat &mCumulSqrSums, int padded)
{
    int nRows = mSrc.rows;
    int nCols = (padded == 1) ? (mSrc.cols+1) : mSrc.cols;

    mCumulSums.create(nRows, nCols, CV_32SC1);
    mCumulSqrSums.create(nRows, nCols, CV_32SC1);

    if ( padded == 1 )
    {
        for (int i = 0; i < nRows; i++)
        {  
            uchar* pSrc = mSrc.ptr<uchar>(i);
            int *pCumulSum    = mCumulSums.ptr<int>(i);
            int *pCumulSqrSum = mCumulSqrSums.ptr<int>(i);

            pCumulSum[0] = 0;
            pCumulSqrSum[0] = 0;

            for( int j=1; j<nCols; j++)
            {
                pCumulSum[j]    = pCumulSum[j-1]    + pSrc[j-1];
                pCumulSqrSum[j] = pCumulSqrSum[j-1] + pSrc[j-1]*pSrc[j-1];
            }
        }
    }else
    {
        for (int i = 0; i < nRows; i++)
        {  
            uchar* pSrc = mSrc.ptr<uchar>(i);
            int *pCumulSum    = mCumulSums.ptr<int>(i);
            int *pCumulSqrSum = mCumulSqrSums.ptr<int>(i);

            pCumulSum[0] = pSrc[0];
            pCumulSqrSum[0] = pSrc[0]*pSrc[0];

            for( int j=1; j<nCols; j++)
            {
                pCumulSum[j]    = pCumulSum[j-1]    + pSrc[j];
                pCumulSqrSum[j] = pCumulSqrSum[j-1] + pSrc[j]*pSrc[j];
            }
        }
    }
}

