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
#ifndef SAMPLEFIFO_H
#define SAMPLEFIFO_H

/* SampleFifo : Multithreaded priority queue / FIFO
 * Sylvain AZARIAN - 2013
 * used for interthread communication, typically sampler / sdr processing
 *
*/
#include <QSemaphore>
#include <QtCore>
#include <QLibrary>

struct _chunk {
   int data_len ;
   int pri ;

   void *id ;
   void* samples;

   struct _chunk *next;
   struct _chunk *prev;
};

typedef struct _chunk Chunk;

struct _fifo {
   int size;
   int seq ;
   Chunk *head, *queue;
};

typedef struct _fifo  Fifo;

class SampleFifo : public QObject
{
    Q_OBJECT

public:
    explicit SampleFifo(QObject *parent = 0);
    ~SampleFifo();

    int getSize();
    int isEmpty();
    void printq();



    int EnqueueData( void* data, int data_size, int data_prio, void* data_id ) ;
    int EnqueueWithExpand( void* data, int data_size, void* data_id,
                           int sizeofelem,
                           int cumulateUpTo ) ;

    void *DequeueData( int* data_size, int data_prio, void** data_id,
                       bool withSemWait=false);

    void setAcceptData( bool accept ) { this->acceptsData = accept ; }
    bool fifoAcceptsData() { return( this->acceptsData ) ; }

    long getElementCount() {
        return( byte_size );
    }
signals:
    void fifoHasData();

public slots:

private:
    int cumulate_upto ;
    bool acceptsData ;
    bool warned ;
    QSemaphore *sem ;
    QSemaphore *semPackets ;
    Fifo *fifo ;
    long byte_size ;

    int enqueue( Chunk *c ) ;
    Chunk* dequeue(bool withSemWait);
    Chunk* dequeuePrio( int prio , bool withSemWait);
};

#endif // SAMPLEFIFO_H
