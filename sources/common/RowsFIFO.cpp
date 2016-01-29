#include "RowsFIFO.h"
#include "stdio.h"

#define FIH_LOGD             printf
#define MY_LOGD(fmt, ...)    do { FIH_LOGD("[%s] ", __FUNCTION__); FIH_LOGD(fmt, __VA_ARGS__); FIH_LOGD("\n"); } while (0)
#define MY_LOGE              MY_LOGD
#define MY_LOG               printf

/* idx: starts from 0 to num-1 */
RowsFIFO::RowsFIFO( int *buf, int rowSize, int num, int usedNum )
{
    _rowSize = rowSize;
    _num = num;
    _usedNum = usedNum;
    _pStart = buf; 
    _pEnd = buf + rowSize*(num-1);                         
    _pFirst = buf; 
    if ( usedNum >= _num )
    {
        _pNext = _pStart;
    }else{
        _pNext = buf + rowSize * usedNum;
    }
    
}

RowsFIFO::~RowsFIFO(void)
{

}

void RowsFIFO::reset( int *buf, int rowSize, int num, int usedNum )
{
    _rowSize = rowSize;
    _num = num;
    _usedNum = usedNum;
    _pStart = buf; 
    _pEnd = buf + rowSize*(num-1);                         
    _pFirst = buf; 
    if ( usedNum >= _num )
    {
        _pNext = _pStart;
    }else{
        _pNext = buf + rowSize * usedNum;
    }
}

int* RowsFIFO::getNextANDPush(void)
{
    int *res = _pNext;

    if ( _usedNum == _num )
    {
        return 0;
    }

    _usedNum++;

    if( _pNext >= _pEnd )
    {
        _pNext = _pStart;
    }else
    {
        _pNext += _rowSize;
    }

    return res;
}

int* RowsFIFO::getFirstANDPop(void)
{
    int *res = _pFirst;

    if ( _usedNum == 0 )
    {
        return 0;
    }

    _usedNum--;

    if (_pFirst >= _pEnd)
    {
        _pFirst = _pStart;
    }else
    {
        _pFirst += _rowSize;
    }

    return res;
}

void RowsFIFO::printRowsFIFOInfo(void)
{
    MY_LOGD("_rowSize:%d, _num:%d, _usedNum:%d, _pFirst:%x, _pNext%x", _rowSize, _num, _usedNum, _pFirst, _pNext);
}



