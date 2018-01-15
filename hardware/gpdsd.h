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
#ifndef GPDSD_H
#define GPDSD_H

#include <QObject>
#include <QSemaphore>
#include <QThread>
#include <QTimer>

#ifndef WIN32
#include <gps.h>
#else
#include "windows/rs232.h"
#endif

#define ERROR_NO_GPSD (-1)

class GPSD : public QThread
{
    Q_OBJECT

public:
    static GPSD& getInstance()  {
        static GPSD instance;
        return instance;
    }

    ~GPSD() ;
    void shutdown();

    bool hasDevice();


signals:

    void hasError( int code );
    void hasGpsFix( double latitude, double longitude );
    void hasGpsTime( int year, int month, int day,
                     int hour, int min, int sec, int msec );
public slots:
    void processError( int errorCode );

private slots:
    void SLOT_timer();

private:
    bool m_stop ;
    bool device_found ;
    QTimer *timer ;
    int comm_port ;

    bool log_NMEA ;
    QString nmea_fileName ;



    GPSD(QObject *parent = 0);
    GPSD(const GPSD &); // hide copy constructor
    GPSD& operator=(const GPSD &);

    void run();
    void startLocalTimer();
};

#endif // GPDSD_H
