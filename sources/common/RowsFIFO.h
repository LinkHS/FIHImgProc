#ifndef __ROWSFIFO_H__
#define __ROWSFIFO_H__

class RowsFIFO
{
public:
	                              RowsFIFO( int *buf, int rowSize, int num, int usedNum );
    virtual                      ~RowsFIFO();
    int*                          getNextANDPush(void);
    int*                          getFirstANDPop(void);
    void                          printRowsFIFOInfo(void);
    void                          reset( int *buf, int rowSize, int num, int usedNum );
private:
    int                           _rowSize;
    int                           _num;
    int                           _usedNum;
    int                           *_pStart;
    int                           *_pEnd;
    int                           *_pFirst;
    int                           *_pNext;
};

#endif
