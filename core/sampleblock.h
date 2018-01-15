#ifndef SAMPLEBLOCK_H
#define SAMPLEBLOCK_H

#include <QQueue>
#include "common/samplefifo.h"
#include "common/datatypes.h"

class SampleBlock {
public:
    SampleBlock( TYPECPX* IQsamples, int L );
    ~SampleBlock();

    void markAsLastBlock();
    TYPECPX *getData() { return( memory ) ; }
    int getLength() { return( mSize ) ; }
    bool isLastBlock()  ;

private:
    TYPECPX *memory ;
    int mSize ;
    bool lastBlock ;
};

#endif // SAMPLEBLOCK_H
