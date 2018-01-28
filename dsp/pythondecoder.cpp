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
#include <zmq.h>
#include <cmath>
#include <Python.h>
#include "pythondecoder.h"
#include <QByteArray>
#include <QApplication>
#include <QDebug>

//pip3 install zmq
ZmqPython::ZmqPython(QObject *parent) : QThread(parent)
{
}


PythonDecoder::PythonDecoder(QObject *parent) : QThread(parent)
{
    mPythonScript = "" ;
}

void PythonDecoder::run() {

    QString pythonProgramName = "PythonPicsat" ;
    wchar_t array[1024];

    if( mPythonScript.length() == 0 ) {
        mPythonScript = QApplication::applicationDirPath() + "/python/decodeZ3.py" ;
        qDebug() << "using Python :" << mPythonScript ;
    }

    QByteArray tochar = mPythonScript.toLocal8Bit() ;
    qDebug() << "starting PythonDecoder::run()" ;

    pythonProgramName.toWCharArray(array);
    Py_SetProgramName(array);
    Py_Initialize();

//    PyRun_SimpleString("from time import time,ctime\n"
//                         "print('Today is',ctime(time()) )\n");
    PyObject *obj = Py_BuildValue("s", (const char *)tochar.data() );
    FILE *file = _Py_fopen_obj(obj, "r+");
    if( file != NULL ) {
        qDebug() << "starting python" ;

        emit pythonStarts();
        PyRun_SimpleFile(file, (const char *)tochar.data() );
        emit pythonEnds();

        qDebug() << "python ends";
    } else {
        qDebug() << "Error with script" ;
    }
    Py_Finalize();
    fclose(file);
}

#define BUFFER_SIZE (1024)
void ZmqPython::run() {
    char *rxbuff ;
    int length ;
    int rc ;
    void *context = zmq_ctx_new ();
    void *socket = zmq_socket (context, ZMQ_SUB);
    rc = zmq_connect (socket, "tcp://localhost:5564");
    rc = zmq_setsockopt (socket, ZMQ_SUBSCRIBE, NULL, 0);
    rxbuff = (char *)malloc( BUFFER_SIZE * sizeof(char));

    for( ; ; ) {
        memset( rxbuff, 0, BUFFER_SIZE );
        length = zmq_recv( socket, (void *)rxbuff, BUFFER_SIZE, 0);
        if( length > 0 ) {
            if( strcmp( rxbuff, (const char *)"MSG") == 0 ) {
                memset( rxbuff, 0, BUFFER_SIZE );
                length = zmq_recv( socket, (void *)rxbuff, BUFFER_SIZE, 0);
                if( length > 0 ) {
                    emit message( ">" + QString::fromLocal8Bit( rxbuff) + "\n" ) ;
                }
            }

            if( strcmp( rxbuff, (const char *)"PACKET") == 0 ) {
                memset( rxbuff, 0, BUFFER_SIZE );
                length = zmq_recv( socket, (void *)rxbuff, BUFFER_SIZE, 0);
                if( length > 0 ) {
                    QString frame = "" ;
                    for( int i=0 ; i < length ; i++ ) {
                         char c = rxbuff[i] ;
                         frame += QString::number( (int)c & 0xFF, 16) + " ";
                    }
                    emit newFrame(frame) ;
                }
            }

            if( strcmp( rxbuff, (const char *)"FREQ") == 0 ) {
                memset( rxbuff, 0, BUFFER_SIZE );
                length = zmq_recv( socket, (void *)rxbuff, BUFFER_SIZE, 0);
                if( length > 0 ) {
                    if( strcmp( rxbuff, (const char *)"ABS") == 0 ) {
                        memset( rxbuff, 0, BUFFER_SIZE );
                        length = zmq_recv( socket, (void *)rxbuff, BUFFER_SIZE, 0);
                        if( length > 0 ) {
                            emit absTune(QString::fromLocal8Bit( rxbuff)) ;
                        }
                    }
                    if( strcmp( rxbuff, (const char *)"REL") == 0 ) {
                        memset( rxbuff, 0, BUFFER_SIZE );
                        length = zmq_recv( socket, (void *)rxbuff, BUFFER_SIZE, 0);
                        if( length > 0 ) {
                            emit relTune( QString::fromLocal8Bit(rxbuff) ) ;
                        }
                    }
                }
            }
        }

    }
    zmq_close (socket);
    zmq_ctx_destroy (context);
}
