//==========================================================================================
// + + +   This Software is released under the "Simplified BSD License"  + + +
// Copyright F4GKR Sylvain AZARIAN . All rights reserved.
//
//Redistribution and use in source and binary forms, with or without modification, are
//permitted provided that the following conditions are met:
//
//   1. Redistributions of source code must retain the above copyright notice, this list of
//	  conditions and the following disclaimer.
//
//   2. Redistributions in binary form must reproduce the above copyright notice, this list
//	  of conditions and the following disclaimer in the documentation and/or other materials
//	  provided with the distribution.
//
//THIS SOFTWARE IS PROVIDED BY Sylvain AZARIAN F4GKR ``AS IS'' AND ANY EXPRESS OR IMPLIED
//WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
//FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL Sylvain AZARIAN OR
//CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
//CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
//ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
//ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//The views and conclusions contained in the software and documentation are those of the
//authors and should not be interpreted as representing official policies, either expressed
//or implied, of Sylvain AZARIAN F4GKR.
//==========================================================================================
#include <stdio.h>
#include <stdlib.h>
#include <QDebug>
#include "samplefifo.h"
#define DEBUG (0)

Chunk* newchunk( int pri, void* id ) {
    Chunk *result ;

    result = (Chunk*)malloc( sizeof( Chunk ));
    if( result == NULL ) {
        return( NULL );
    }

    result->pri = pri ;
    result->id = id ;
    result->next = NULL ;
    result->prev = NULL ;

    return( result );
}


SampleFifo::SampleFifo(QObject *parent) : QObject(parent)
{

    fifo = (Fifo*)malloc( sizeof(Fifo ));
    fifo->size = 0 ;
    fifo->seq = 0 ;
    fifo->head = NULL ;
    fifo->queue = NULL ;

    cumulate_upto = 0 ;
    byte_size = 0 ;
    sem = new QSemaphore(1);
    semPackets = new QSemaphore(0);
    warned = false ;
    acceptsData = true ;
}

SampleFifo::~SampleFifo() {
    free( fifo );
    fifo = NULL ;
    delete sem ;
    delete semPackets;
}


int SampleFifo::getSize() {
    int size = 0 ;
    if( fifo != NULL ) {
        size = fifo->size ;
    }
    if( DEBUG ) printf("Queue size is %d\n", size );
    return( size );
}

int SampleFifo::isEmpty() {
    int size = 0 ;
    if( fifo != NULL ) {
        sem->acquire();
        size = fifo->size ;
        sem->release();
    }

    return( size == 0 );
}


Chunk* SampleFifo::dequeue(bool withSemWait) {
    Chunk *result ;

    //printf("Taking head from fifo\n");

    if( fifo == NULL ) return( NULL );
    if( !withSemWait ) {
        sem->acquire();
        if( fifo->size == 0)  {
            byte_size = 0 ;
            sem->release();
            return( NULL );
        }
    } else {
        if( semPackets->tryAcquire( 1, 200 ) == false ) {
            return( NULL );
        }
        sem->acquire();
    }

    if( fifo->size == 0)  {
        byte_size = 0 ;
        sem->release();
        return( NULL );
    }

    result = fifo->head ;

    if( fifo->size == 1 ) {
        fifo->head = NULL ;
        fifo->queue = NULL ;        
    } else if( fifo->size > 0 ) {
        fifo->head = fifo->head->next ;
        fifo->head->prev = NULL ; // first elt        
    }
    fifo->size = fifo->size - 1 ;
    byte_size -= result->data_len ;
    sem->release();

    return( result );
}

Chunk* SampleFifo::dequeuePrio(int prio , bool withSemWait) {
    Chunk *result ;
        //printf("Taking head from fifo\n");

    if( fifo == NULL ) return( NULL );
        if( !withSemWait && (fifo->size == 0) ) {
            return( NULL );
        }

        sem->acquire();

        result = fifo->head ;
        if( fifo->size == 1 ) {
            fifo->head = NULL ;
            fifo->queue = NULL ;
            fifo->size = 0 ;
            semPackets->acquire(1);
        } else {
            fifo->size = fifo->size - 1 ;
            while( result != NULL ) {
                if( result->pri == prio ) {
                    if( result->prev != NULL )  {
                        result->prev->next = result->next ;
                    } else {
                        fifo->head = result->next ;
                    }
                    if( result->next != NULL ) {
                        result->next->prev = result->prev ;
                    } else {
                       fifo->queue = result->prev ;
                    }
                    sem->release();
                    byte_size -= result->data_len ;
                    return( result );
                }
                result = result->next ;
            }
            result = NULL ;
        }
        sem->release();

        return( result );
}

#define FIFO_MAX_LENGH (200)
int  SampleFifo::enqueue( Chunk *c ) {
    Chunk *tmp ;
    int lsize ;
    if( DEBUG ) printf("+adding %d\n", c->pri );
    if( fifo == NULL ) return(0) ;

    sem->acquire();
    if( fifo->size > FIFO_MAX_LENGH ) {
        if( !warned ) {
            qDebug() << "Warning... element not pushed ,queue too long" << fifo->size ;
            warned = true ;
        }
        sem->release();
        return(-1) ;
    }
    warned = false ;
    byte_size += c->data_len ;

    fifo->seq++ ;
    if( fifo->size == 0 ) {
        if( DEBUG ) printf("+empty\n");
        fifo->head = c ;
        fifo->queue = c ;
        fifo->size = 1 ;

        c->next = NULL ;
        c->prev = NULL ;

    } else  {
        tmp = fifo->queue ;
        while( tmp->pri < c->pri ) {
            if( DEBUG ) printf("+while:tmp=%d\n", tmp->pri );
            if( tmp->prev == NULL ) break ;
            tmp = tmp->prev ;
        }

        if( DEBUG ) printf("+tmp=%d\n", tmp->pri );
        if( tmp->pri >= c->pri ) {
            c->next = tmp->next ;
            tmp->next = c ;
            c->prev = tmp ;
            if( c->next != NULL ) c->next->prev = c ;
        } else {
            c->next = tmp ;
            c->prev = tmp->prev ;
            tmp->prev = c ;
            if( c->prev != NULL ) c->prev->next = c ;
        }


        if( c->prev == NULL ) {
            fifo->head = c ;
        }
        if( c->next == NULL ) {
            fifo->queue = c ;
        }
        ++(fifo->size) ;
    }
    lsize = fifo->size ;
    sem->release();
    semPackets->release(1);
    return( lsize );
}


void SampleFifo::printq() {
    Chunk *tmp ;
    int k ;
    printf("-------------\n");
    k = 0 ;

    sem->acquire();
    tmp = fifo->head ;
    while( tmp != NULL ) {
        printf("%d pri=%d \n", k++, tmp->pri  );
        tmp = tmp->next ;
    }

    sem->release();
    if( DEBUG ) printf("-------------\n");
}

int SampleFifo::EnqueueData( void* data, int data_size, int data_prio, void* data_id ) {
    Chunk *tmp ;
    int rc ;

    if( fifo == NULL ) return(0) ;
    if( !acceptsData )
        return(-1);

    tmp = newchunk( data_prio, data_id );
    if( tmp == NULL ) {
        return(0) ;
    }
    tmp->data_len = data_size ;
    tmp->samples = data ;
    tmp->pri = data_prio ;
    tmp->id = data_id ;

    //qDebug() << "SampleFifo::EnqueueData" << data_size ;
    rc = enqueue(tmp);
    if( rc > 0 ) {
        emit fifoHasData();
    }
    return( rc );
}

int SampleFifo::EnqueueWithExpand( void* data, int data_size, void* data_id,
                       int sizeofelem,
                       int cumulateUpTo ) {

    int s ;
    if( DEBUG ) qDebug() << "SampleFifo::EnqueueWithExpand data_size=" << data_size ;
    if( fifo == NULL ) {
        if( DEBUG ) qDebug() << "SampleFifo::EnqueueWithExpand fifo is null" ;
        return(0);
    }

    if( !acceptsData )
        return(-1);

    sem->acquire(1);
    s = fifo->size ;
    if( s == 0 ) {
        sem->release(1);
        if( DEBUG ) qDebug() << "SampleFifo::EnqueueWithExpand queue size is 0" ;
        return( EnqueueData( data, data_size, 0, data_id));
    }

    Chunk *tmp = fifo->queue ;
    if( tmp->data_len < cumulateUpTo ) {
        // estimate new memory room space needed
        int newSize = (tmp->data_len + data_size) * sizeofelem ;
        tmp->samples = realloc( tmp->samples , newSize );

        char *ptrw = (char *)tmp->samples ; // warning with pointer arithmetics..........
        ptrw += (tmp->data_len*sizeofelem) ;
        memcpy( (void *)ptrw, data, data_size*sizeofelem );
        tmp->data_len += data_size ;
        if( DEBUG ) qDebug() << "SampleFifo::EnqueueWithExpand expanded to " << tmp->data_len ;

        if( data_id != NULL ) {
            if( tmp->id != NULL ) free( tmp->id );
            tmp->id = data_id ;
        }
        sem->release(1);
        emit fifoHasData();
        free(data);
        return( s ) ; // size of fifo stays unchanged...
    } else {
         if( DEBUG ) qDebug() << "SampleFifo::EnqueueWithExpand no expand, tmp->data_len=" << tmp->data_len ;
    }
    sem->release(1);
    return( EnqueueData( data, data_size, 0, data_id));
}



void *SampleFifo::DequeueData(int* data_size, int data_prio, void** data_id , bool withSemWait) {
    Chunk *tmp ;
    void* res ;

    if( fifo == NULL ) return(NULL) ;
    if( data_size != NULL ) *data_size = 0 ;

    if( data_prio == 0 ) {
        tmp = dequeue(withSemWait) ;
    } else {
        tmp = dequeuePrio(data_prio,withSemWait) ;
    }
    if( tmp == NULL ) return( NULL );

    if( data_size != NULL ) *data_size = tmp->data_len ;
    if( data_id != NULL ) *data_id = tmp->id ;

    res = tmp->samples ;
    free( tmp );
    return( res );
}
