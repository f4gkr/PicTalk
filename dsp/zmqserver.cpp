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
#include "zmqserver.h"
#include <zmq.h>
#include <unistd.h>
#include <QDebug>
#include <QApplication>
#define DEBUG_ZMQ_SRV (1)

ZmqServer::ZmqServer(QObject *parent) : QThread(parent)
{
    L = 0 ;
    synchro = new QSemaphore(0);
    m_end_request = false ;    
}

void ZmqServer::addBlock( SampleBlock *b ) {
    if( b == NULL )
        return ;
    queue.enqueue(b);
    synchro->release(1);
}

void ZmqServer::endRequest() {
    m_end_request = true ;
    synchro->release(1);
}

void ZmqServer::run() {
    char message[3] ;
    int length ;
    void *context = zmq_ctx_new ();
    void *socket = zmq_socket (context, ZMQ_PUB);
    zmq_bind (socket, "tcp://*:5563");
    if( DEBUG_ZMQ_SRV ) qDebug() << "ZmqServer::run() starting" ;

    message[0] = 'I' ;
    message[1] = 'Q' ;
    message[3] = 0 ;

    while( !m_end_request ) {
        synchro->acquire(1);

        if( m_end_request ) {
            // tell decoder that we are leaving
            // send header
            zmq_send( socket, (const void *)message, 2, ZMQ_SNDMORE ) ;
            // say that size is < 0, this is understood by decoder as "end of mission"
            length = -1 ;
            zmq_send( socket, (const void *)&length, sizeof(int), 0 );
            break ;
        }

        SampleBlock *b = queue.dequeue() ;
        if( b == NULL )
            continue ;

        TYPECPX *samples = b->getData() ;
        length = b->getLength() ;

        // send header
        zmq_send( socket, (const void *)message, 2, ZMQ_SNDMORE ) ;

        // send size
        zmq_send( socket, (const void *)&length, sizeof(int), ZMQ_SNDMORE );

        // send data
        zmq_send( socket, (const void *)samples, length*sizeof(TYPECPX), 0);

        if( b->isLastBlock() ) {
            // this is the last block of current frame
            // send another ZMQ frame with 0 length to inform readers
            zmq_send( socket, (const void *)message, 2, ZMQ_SNDMORE ) ;
            length = 0 ;
            zmq_send( socket, (const void *)&length, sizeof(int), 0 );
        }

        delete b ;

        if( DEBUG_ZMQ_SRV ) qDebug() << "zmq frame pushed" ;

    }
    if( DEBUG_ZMQ_SRV ) qDebug() << "ZmqServer::run() end ???" ;
    zmq_close (socket);
    zmq_ctx_destroy (context);
}
