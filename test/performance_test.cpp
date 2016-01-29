#include "opencv.hpp"
#include "basic/others.h"
#include "common/debug.h"

#define MY_LOGD(fmt, ...)    do { FIH_LOGD("[%s] ", __FUNCTION__); FIH_LOGD(fmt, __VA_ARGS__); FIH_LOGD("\n"); } while (0)
#define MY_LOGE              MY_LOGD
#define MY_LOG               printf

using namespace cv;

void Test_DataTraversal(Mat &mSrc)
{
    double time;

    int channels = mSrc.channels();
    int nRows = mSrc.rows;
    int nCols = mSrc.cols * channels;
    
    PfmTest_StartTimer(time);
    for( int i=0; i<nRows; ++i )
	{  
        uchar* pSrc = mSrc.ptr<uchar>(i);

        for( int j=0; j<nCols; j+=3)
        {
            pSrc[j+0] = 1;
            pSrc[j+1] = 1;
            pSrc[j+2] = 1;
        }
    }
    PfmTest_StopTimer(time);
    MY_LOGD("%s() cost %lfs", "Mat regular", time); 

    PfmTest_StartTimer(time);
    if( mSrc.isContinuous() )
    {
        nCols *= nRows;
        nRows = 1;
#pragma omp parallel for
        for( int i=0; i<nRows; ++i )
        {  
            uchar* pSrc = mSrc.ptr<uchar>(i);

            for( int j=0; j<nCols; j+=3)
            {
                pSrc[j+0] = 1;
                pSrc[j+1] = 1;
                pSrc[j+2] = 1;
            }
        }
    }
    else
    {
        MY_LOGD("is not continuous");
    }

    PfmTest_StopTimer(time);
    MY_LOGD("%s() cost %lfs", "Mat continuous", time);


    PfmTest_StartTimer(time);
    nRows = mSrc.rows;
    nCols = mSrc.cols * 3;
    uchar *pBGR = new uchar[nCols*nRows];
    for( int i=0; i<nRows; ++i )
	{  
        for( int j=0; j<nCols; j+=3)
        {
            pBGR[i*nCols+0] = 180;
            pBGR[i*nCols+1] = 180;
            pBGR[i*nCols+2] = 180;
        }
    }
    PfmTest_StopTimer(time);
    MY_LOGD("%s() cost %lfs", "Array regular", time);
}

void Test_Boxfilter(void)
{
    Mat mN;
    boxFilter( Mat::ones(100, 100, CV_32FC1), mN, -1, cv::Size(10, 10) );

    for (int i=0; i<100; i++)
    {
        float *pSrc = mN.ptr<float>(i);

        for (int j=0; j<100; j++)
        {
            MY_LOG("%f, ", pSrc[j]);
        }
    }
}

void Compute_Sum_SqrSum_InFirstRow_back(Mat &mSrc, int r)
{
    int nRows = mSrc.rows;
    int nCols = mSrc.cols;
    int kh = r*2 + 1;                //kernel height
    int kw = r*2 + 1;                //kernel width

    int *pSumRow    = new int[(nCols+1)*kh];
    int *pSqrSumRow = new int[(nCols+1)*kh];
    int *pSum = pSumRow;
    int *pSqrSum = pSqrSumRow;

    memset( pSumRow, 0, (nCols+1)*kh*sizeof(int));
    memset( pSqrSumRow, 0, (nCols+1)*kh*sizeof(int));

    for( int i=0; i<kh; i++ )
    {  
        uchar* pSrc = mSrc.ptr<uchar>(i);

        /* first block */
        /* *-------------------------------------- */
        for( int j=0; j<kw; j++)
        {
            pSum[0]    += pSrc[j];
            pSqrSum[0] += pSrc[j]*pSrc[j];
        }

        /* radius+2 block to radius+1 last block, only compute offset value */
        /* ----********************************--- */
        for( int j=0; j<nCols-kw; j++)
        {
            int sj = j + r + 1; //col index of sum buf
            pSum[sj]    += pSrc[j+kw] - pSrc[j];
            pSqrSum[sj] += pSrc[j+kw]*pSrc[j+kw] - pSrc[j]*pSrc[j];
        }
    }
    /* second block to radius+1 block */
    /* -***------------------------------------ */
    for (int i=1; i<r+1; i++)
    {
        pSum[i]    = pSum[0];
        pSqrSum[i] = pSqrSum[0];
    }

    /* radius+2 block to radius+1 last block, compute the final value by adding reference value to each offset value */
    /* ----********************************--- */
    for( int j=r+1; j<nCols-r; j++)
    {
        pSum[j]    += pSum[j-1];
        pSqrSum[j] += pSqrSum[j-1];
    }
        

    /* last radius block */
    /* -------------------------------------*** */
    for (int i=nCols-r; i<nCols; i++)
    {
        pSum[i]    = pSum[nCols-r-1];
        pSqrSum[i] = pSqrSum[nCols-r-1];
    }

    /************************** print **************************/
	MY_LOGD("fopen");
    FILE *file1 = fopen("d:/1.txt", "w+");
    if( NULL == file1 )
    {
        MY_LOGD("fopen failed");
        return;
    }

    for( int i=0; i<nRows; i++ )
    {  
        uchar* pSrc = mSrc.ptr<uchar>(i);

        for( int j=0; j<nCols; j++)
        {
            char buffer[10];
            sprintf(buffer, "%3d ", pSrc[j]);
            fwrite(buffer, 1, 4, file1);
        }
        fwrite("\n", 1, 1, file1);
    }
    fclose(file1);

    FILE *file2 = fopen("d:/2.txt", "w+");
    if( NULL == file2 )
    {
        MY_LOGD("fopen failed");
        return;
    }

    for( int j=0; j<nCols; j++)
    {
        char buffer[20];
        sprintf(buffer, "%d ", pSum[j]);
        fwrite(buffer, 1, strlen(buffer), file2);
    }
    fwrite("\n", 1, 1, file1);
    for( int j=0; j<nCols; j++)
    {
        char buffer[20];
        sprintf(buffer, "%d ", pSqrSum[j]);
        fwrite(buffer, 1, strlen(buffer), file2);
    }
    fclose(file2);

    delete pSumRow;
    delete pSqrSumRow;
}
#include "common/RowsFIFO.h"

int RowsFIFO_test(void)
{
    int buf[10*11] ={
        10, 15, 25, 522, 56, 10, 15, 25, 522, 56, 12,
        11, 15, 25, 522, 56, 10, 15, 25, 522, 56, 12,
        12, 15, 25, 522, 56, 10, 15, 25, 522, 56, 12,
        13, 15, 25, 522, 56, 10, 15, 25, 522, 56, 12,
        14, 15, 25, 522, 56, 10, 15, 25, 522, 56, 12,
        15, 15, 25, 522, 56, 10, 15, 25, 522, 56, 12,
        16, 15, 25, 522, 56, 10, 15, 25, 522, 56, 12,
        17, 15, 25, 522, 56, 10, 15, 25, 522, 56, 12,
        18, 15, 25, 522, 56, 10, 15, 25, 522, 56, 12,
        19, 15, 25, 522, 56, 10, 15, 25, 522, 56, 12
    };

    //printf("RowsFIFO_test, start:%x, end:%x\n\n", buf, &buf[0]+110); //10

    RowsFIFO accRows( buf, 11, 10, 10 );
	int *p;

    for ( int i=0; i<11; i++ )
    {
        p = accRows.getFirstANDPop();
        if ( (i < 10) && (p[0] != (10+i)) )
        {
            MY_LOGD("step1, failed when i=%d, p[0]=%d should be %d", i, p[0], (10+i));
            return -1;
        }
        if ( (i== 10) && (p != NULL) )
        {
            MY_LOGD("step1, failed when i=%d, p=%x should be 0", i, p);
            return -2;
        }
    }

    for ( int i=0; i<11; i++ )
    {
        p = accRows.getNextANDPush();
        if ( i< 10 ) {
            p[0] = 20 + i;
        }
        if ( (i < 10) && (p[0] != buf[11*i]) )
        {
            MY_LOGD("step2, failed when i=%d, buf[0]=%d should be %d", i, buf[11*i], p[0]);
            return -3;
        }
        if ( (i== 10) && (p != NULL) )
        {
            MY_LOGD("step2, failed when i=%d, p=%x should be 0", i, p);
            return -2;
        }
    }

    for ( int i=0; i<10; i++ )
    {
        for ( int j=0; j<i; j++ )
        {
            p = accRows.getFirstANDPop();
            p = accRows.getNextANDPush();
            if ( p == NULL )
            {
                MY_LOGD("step3, failed when i=%d, p=%x should not be 0", i, p);
                return -2;
            }
        }
    }

#if 1
    accRows.reset( buf, 11, 10, 10 );

    p = accRows.getFirstANDPop();
    accRows.printRowsFIFOInfo();
    printf("RowsFIFO_test, %d\n\n", p[0]); //10

    p = accRows.getFirstANDPop();
    accRows.printRowsFIFOInfo();
    printf("RowsFIFO_test, %d\n\n", p[0]); //11

    p = accRows.getFirstANDPop();
    accRows.printRowsFIFOInfo();
    printf("RowsFIFO_test, %d\n\n", p[0]); //12

    p = accRows.getNextANDPush();
    accRows.printRowsFIFOInfo();
    p[0] = 20;
    printf("RowsFIFO_test, %d %d\n\n", p[0], buf[11*0]); //20 20

    p = accRows.getNextANDPush();
    accRows.printRowsFIFOInfo();
    p[0] = 21;
    printf("RowsFIFO_test, %d %d\n\n", p[0], buf[11*1]); //21 21

    p = accRows.getFirstANDPop();
    accRows.printRowsFIFOInfo();
    printf("RowsFIFO_test, %d\n\n", p[0]); //13

    p = accRows.getFirstANDPop();
    accRows.printRowsFIFOInfo();
    printf("RowsFIFO_test, %d\n\n", p[0]); //14

    p = accRows.getFirstANDPop();
    accRows.printRowsFIFOInfo();
    printf("RowsFIFO_test, %d\n\n", p[0]); //15

    p = accRows.getFirstANDPop();
    accRows.printRowsFIFOInfo();
    printf("RowsFIFO_test, %d\n\n", p[0]); //16

    p = accRows.getFirstANDPop();
    accRows.printRowsFIFOInfo();
    printf("RowsFIFO_test, %d\n\n", p[0]); //17

    p = accRows.getFirstANDPop();
    accRows.printRowsFIFOInfo();
    printf("RowsFIFO_test, %d\n\n", p[0]); //18

    p = accRows.getFirstANDPop();
    accRows.printRowsFIFOInfo();
    printf("RowsFIFO_test, %d\n\n", p[0]); //19

    p = accRows.getFirstANDPop();
    accRows.printRowsFIFOInfo();
    printf("RowsFIFO_test, %d\n\n", p[0]); //20

    p = accRows.getFirstANDPop();
    accRows.printRowsFIFOInfo();
    printf("RowsFIFO_test, %d\n\n", p[0]); //21

    p = accRows.getFirstANDPop();
    accRows.printRowsFIFOInfo();
    printf("RowsFIFO_test, %x\n\n", p);    //0

    p = accRows.getNextANDPush();
    accRows.printRowsFIFOInfo();
    p[0] = 22;
    printf("RowsFIFO_test, %d %d\n\n", p[0], buf[11*2]); //22 22

    p = accRows.getNextANDPush();
    accRows.printRowsFIFOInfo();
    p[0] = 23;
    printf("RowsFIFO_test, %d %d\n\n", p[0], buf[11*3]); //23 23
#endif

	return 0;
}


void Compute_Loacl_Sum_SqrSum_InFirstRow(Mat &mSrc, int r)
{
    int nRows = mSrc.rows;
    int nCols = mSrc.cols;
    int kh = r*2 + 1;                //kernel height
    int kw = r*2 + 1;                //kernel width

    int *pSumRow    = new int[(nCols+1)*kh];
    int *pSqrSumRow = new int[(nCols+1)*kh];
    int *pSum = pSumRow;
    int *pSqrSum = pSqrSumRow;

    memset( pSumRow, 0, (nCols+1)*kh*sizeof(int));
    memset( pSqrSumRow, 0, (nCols+1)*kh*sizeof(int));

    for( int i=0; i<kh; i++ )
    {  
        uchar* pSrc = mSrc.ptr<uchar>(i);

        /* first block */
        /* *-------------------------------------- */
        for( int j=0; j<kw; j++)
        {
            pSum[0]    += pSrc[j];
            pSqrSum[0] += pSrc[j]*pSrc[j];
        }

        /* radius+2 block to radius+1 last block, only compute offset value */
        /* ----********************************--- */
        for( int j=0; j<nCols-kw; j++)
        {
            int sj = j + r + 1; //col index of sum buf
            pSum[sj]    += pSrc[j+kw] - pSrc[j];
            pSqrSum[sj] += pSrc[j+kw]*pSrc[j+kw] - pSrc[j]*pSrc[j];
        }
    }
    /* Copy to 'second block to radius+1 block' */
    /* -***------------------------------------ */
    for (int i=1; i<r+1; i++)
    {
        pSum[i]    = pSum[0];
        pSqrSum[i] = pSqrSum[0];
    }

    /* radius+2 block to radius+1 last block, compute the final value by adding reference value to each offset value */
    /* ----********************************--- */
    for( int j=r+1; j<nCols-r; j++)
    {
        pSum[j]    += pSum[j-1];
        pSqrSum[j] += pSqrSum[j-1];
    }
        
    /* Copy to last radius block */
    /* -------------------------------------*** */
    for (int i=nCols-r; i<nCols; i++)
    {
        pSum[i]    = pSum[nCols-r-1];
        pSqrSum[i] = pSqrSum[nCols-r-1];
    }

    delete pSumRow;
    delete pSqrSumRow;
}


#if 0
void Compute_Loacl_Mean_SqrMean(Mat &mSrc, Mat &mLMean, Mat &mLSqrMean, int r)
{
    int nRows = mSrc.rows;
    int nCols = mSrc.cols;
    int kh = r*2 + 1;                //kernel height
    int kw = r*2 + 1;                //kernel width

    //Mat mLMean, mLSqrMean;
    //uchar* pLMean    = mLMean.ptr<uchar>(0);
    //uchar* pLSqrMean = mLSqrMean.ptr<uchar>(0);

    int *pMem = new int[nCols*4];
    memset(pMem, 0, nCols*4*sizeof(int));

    int *pPreLSum;     //previous local sum
    int *pPreLSqrSum;  //previous local square sum
    int *pCurLSum    = pMem + nCols*2;     //previous local sum
    int *pCurLSqrSum = pMem + nCols*3;;    //previous local square sum

    /* radius+1 section to radius+1 last section, only compute offset value */
    /* ----********************************--- */
    for( int i=0; i<kh; i++ )
    {
        int *pAccSumRow    = pAccSumRows    + (nCols+1)*i;
        int *pAccSqrSumRow = pAccSqrSumRows + (nCols+1)*i;

        for( int j=r; j<nCols-r; j++)
        {
            //int sj1 = j + r + 1; //col index of acc sum buf, which to be added
            //int sj2 = j - r1;    //col index of acc sum buf, which to be removed

            pCurLSum[j]     += pAccSumRow[j+r+1]    - pAccSumRow[j-r];
            pCurLSqrSum[j]  += pAccSqrSumRow[j+r+1] - pAccSqrSumRow[j-r];
        }
    }
    /* Copy to 'first col to radius-1 col' */
    for( int j=0; j<r; j++)
    {
        pCurLSum[j]    = pCurLSum[r];
        pCurLSqrSum[j] = pCurLSqrSum[r];
    }
    /* Copy to last radius cols */
    for( int j=nCols-r; j<nCols; j++)
    {
        pCurLSum[j]    = pCurLSum[nCols-r-1];
        pCurLSqrSum[j] = pCurLSqrSum[nCols-r-1];
    }


    for( int i=1; i<nRows; i++ )
    {
        uchar* pSrc = mSrc.ptr<uchar>(i+firstRow);

        int *temp = pPreLSum;
        pPreLSum = pCurLSum;
        pCurLSum = temp;

        temp = pPreLSqrSum;
        pPreLSqrSum = pCurLSqrSum;
        pCurLSqrSum = temp;

        pCurLSum[r] = 0;
        pCurLSqrSum[r] = 0;
        for( int j=0; j<kw; j++ )
        {
            pCurLSum[r]    += pSrc[j];
            pCurLSqrSum[r] += pSrc[j]*pSrc[j];
        }

        for (int j=r+1; j<nCols-r; j++)
        {
            pCurLSum[j]    += pCurLSum[j-1]    - pSrc[j-r-1] + pSrc[j+r];
            pCurLSqrSum[j] += pCurLSqrSum[j-1] - pSrc[j-r-1]*pSrc[j-r-1] + pSrc[j+r]*pSrc[j+ r];
        }
    }
    /* second */
    delete pMem;
}
#endif

void Test_myBoxfilter(Mat &mSrc, Mat &mDst, int r, int alpha)
{
	int nCols = mSrc.cols + 1;
    int kh = r*2 + 1;
    int *pAccSumRows    = new int[nCols*kh];
    int *pAccSqrSumRows = new int[nCols*kh];

    Compute_CumulSum_CumulSumSqr_RowByRow(mSrc, pAccSumRows, pAccSqrSumRows, 1, kh, 1);


#if 1
    MY_LOGD("fopen");
    FILE *file1 = fopen("d:/CumulSum_CumulSumSqr.txt", "w+");
    if( NULL == file1 )
    {
        MY_LOGD("fopen failed");
        return;
    }
    for( int i=0; i<kh; i++ )
    {  
        int *pAccSumRow    = pAccSumRows    + nCols*i;
        int *pAccSqrSumRow = pAccSqrSumRows + nCols*i;
        for( int j=0; j<nCols; j++)
        {
            char buffer[20];
            sprintf(buffer, "%d ", pAccSqrSumRow[j]);
            fwrite(buffer, 1, strlen(buffer), file1);
        }
        fwrite("\n", 1, 1, file1);
    }
    fclose(file1);
#endif

    delete pAccSumRows;
    delete pAccSqrSumRows;
}


