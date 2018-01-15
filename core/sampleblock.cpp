#include "sampleblock.h"


SampleBlock::SampleBlock( TYPECPX* IQsamples, int L ) {
    mSize = L ;
    memory = (TYPECPX *)malloc( mSize * sizeof(TYPECPX));
    memcpy( memory, IQsamples, mSize * sizeof(TYPECPX));
    lastBlock = false ;
}

SampleBlock::~SampleBlock() {
    free( memory );
    memory = NULL ;
    mSize = 0 ;
}

void SampleBlock::markAsLastBlock() {
    lastBlock = true ;
}

bool SampleBlock::isLastBlock() {
    return( lastBlock) ;
}

//pushback
