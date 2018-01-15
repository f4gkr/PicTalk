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
#include "gpdsd.h"
#include <QDebug>
#include <QSettings>
#include <QApplication>
#include <QDateTime>
#include <QDate>
#include <QTime>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common/QLogger.h"
#ifdef WIN32
#include "windows/rs232.h"
#include "windows/tinygps.h"
#endif
#include "common/constants.h"

#define BUFLEN (1024)
#define DEBUG_GPSD (0)

#define Sleep(n) usleep(n)
#define sleep(n) usleep( 1000* n)

GPSD::GPSD( QObject *parent) :
    QThread(parent)
{
    int comm_speed ;
    QLogger::QLoggerManager *manager = QLogger::QLoggerManager::getInstance();
    manager->addDestination("./logs/gps.log", QStringList("GPSD"),  QLogger::TraceLevel);
    QLogger::QLog_Trace("GPSD", "Starting" );
    timer = NULL ;
    log_NMEA = false ;


    QDateTime dateTime = QDateTime::currentDateTime();
    QString date4file  = dateTime.toString("ddMMyyyy_hhmmss") ;
    nmea_fileName = "./record/NMEA_" +date4file+ +".gkmsr" ;
    device_found = false ;

#ifdef WIN32
    comm_port = -1 ;
    comm_speed= -1 ;
    QSettings settings( QApplication::applicationDirPath() + "/conf/" + QString(CONFIG_FILENAME), QSettings::IniFormat );
    settings.beginGroup("GPS");

    if( !settings.value("enable_gps").isNull() ) {
        if( settings.value("enable_gps").toInt() == 1 ) {

            if( settings.value("gps_port").isNull() ) {
                settings.setValue("gps_port", 1 );
                settings.setValue("bauds", 4800 );
                settings.setValue("log_NMEA", false );
            } else {
                comm_port = settings.value("gps_port").toInt();
                log_NMEA  = settings.value("log_NMEA").toBool();
                if( DEBUG_GPSD ) qDebug() << "GPSD::GPSD() settings.value('gps_port')" << comm_port ;
                QLogger::QLog_Trace( LOGGER_NAME, "GPSD::GPSD() settings.value('gps_port') " + QString::number(comm_port) );
                comm_port-- ;
            }

            comm_speed = settings.value("bauds").toInt()  ;
            if( comm_speed == 0 ) {
                comm_speed = 4800 ;
            }
        }

    } else {
        settings.setValue( "enable_gps", "0");
        settings.setValue("gps_port", 1 );
        settings.setValue("bauds", 4800 );
        settings.setValue("log_NMEA", false );
    }

    settings.endGroup();


    if( comm_port >= 0 ) {
        if( DEBUG_GPSD ) qDebug() << "GPSD::GPSD() open gps_port" << comm_port << comm_speed ;
        if( RS232_OpenComport( comm_port, comm_speed) == 1 ) {
            if( DEBUG_GPSD ) qDebug() << "GPSD::GPSD() open gps_port error";
            QLogger::QLog_Error( LOGGER_NAME, "GPSD::GPSD() open gps_port error " );
            comm_port = -1 ;
        } else {
            device_found = true ;
            QLogger::QLog_Error( LOGGER_NAME, "GPSD::GPSD() open ok " );
        }
    }

    if( comm_port < 0 ) {
        startLocalTimer();
    }

#else
    struct gps_data_t gps_data;
    if(gps_open("localhost", "2947", &gps_data)<0){
        QLogger::QLog_Error( LOGGER_NAME, "GPSD::GPSD() open gps_port error " );
        startLocalTimer();
    } else {
        gps_close(&gps_data);
        device_found = true ;
    }
#endif
}

bool GPSD::hasDevice() {
    return( device_found );
}

GPSD::~GPSD() {
#ifdef WIN32
    if( comm_port >= 0 ) {
        RS232_CloseComport( comm_port );
        QThread::quit();
    }
#endif

}

void GPSD::processError( int errorCode ) {
    if( errorCode == ERROR_NO_GPSD ) {
        startLocalTimer();
    }
}

void GPSD::startLocalTimer() {
    if( timer != NULL ) return ;
    QLogger::QLog_Error( LOGGER_NAME, "GPSD:: start local timer" );
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(SLOT_timer()));
    timer->start( 500.0 );
    device_found = true ;
}

void GPSD::SLOT_timer() {
    static int last_sec = 0;
    int gps_hour, gps_min, gps_sec  ;
    int gps_day, gps_month, gps_year ;

    QTime now = QDateTime::currentDateTimeUtc().time() ;
    gps_hour = now.hour();
    gps_min = now.minute();
    gps_sec = now.second();

    QDate today = QDate::currentDate();
    gps_day = today.day();
    gps_month = today.month();
    gps_year  = today.year();

    if( gps_sec != last_sec ) {
        last_sec = gps_sec ;
        emit( hasGpsTime( gps_year, gps_month, gps_day, gps_hour,
                          gps_min, gps_sec, 0) );
    }
}

void GPSD::shutdown() {
    m_stop = true ;
}

void GPSD::run() {
    double latitude = 0 ;
    double longitude = 0  ;
    double altitude = 0 ;

    long ttl_measurements = 0 ;

    int gps_hour, gps_min, gps_sec  ;
    int gps_day, gps_month, gps_year ;
    int last_sec = 0 ;
    qDebug() << "GPSD::run() start " ;



#ifdef WIN32
    int  i,n ;
    unsigned char buf[BUFLEN];
    unsigned long fix_age,raw_date, raw_time ;
    long raw_lat , raw_long ;
    unsigned char r_gps_hour, r_gps_min, r_gps_sec ;
    unsigned char r_gps_day,  r_gps_month;
    int r_gps_year ;
    m_stop = false ;
    QLogger::QLog_Trace( LOGGER_NAME, "GPSD::run() ");
    if( comm_port > 0 ) {
        if( DEBUG_GPSD ) qDebug() << "GPSD::run() windows loop " ;
        TinyGPS *gps = new TinyGPS();
        while( !m_stop ) {

            n = RS232_PollComport( comm_port, buf, BUFLEN);
            buf[n] = 0;
            if(n > 0) {
                if( log_NMEA ) {
                    QByteArray ba = nmea_fileName.toLatin1() ;
                    FILE *nmea_log = fopen(  ba.data(), "a");
                    if( nmea_log != NULL ) {
                        fwrite( buf, 1, n, nmea_log);
                        fclose( nmea_log );
                    }
                }
                //qDebug() << "RS232_PollComport" << QString::fromLocal8Bit( (const char *)buf );
                //QLogger::QLog_Debug("GPSD", QString::fromLocal8Bit( (const char *)buf ) );
                for( i=0 ; i < n ; i++ ) {
                    if( gps->encode( buf[i]) ) {
                        if( gps->isDataGood() == false ) {
                                continue ;
                        }

                        //qDebug() << "good" << good++ ;
                        gps->get_position( &raw_lat, &raw_long, &fix_age);
                        gps->get_datetime( &raw_date, &raw_time, &fix_age);
                        gps->crack_datetime( &r_gps_year, &r_gps_month, &r_gps_day,&r_gps_hour,&r_gps_min,&r_gps_sec );

                        gps_year = r_gps_year ;
                        gps_month = r_gps_month;
                        gps_day = r_gps_day ;
                        gps_hour = r_gps_hour ;
                        gps_min = r_gps_min ;
                        gps_sec = r_gps_sec ;

                        double new_latitude = raw_lat / 1000000.0 ;
                        double new_longitude = raw_long / 1000000.0 ;
                        double new_altitude = (double)gps->getAltitude() ;
                        bool altitude_valid = (new_altitude < 5000 ? true:false);
                        //qDebug() << "new_altutide=" << new_altitude ;

                        // estimate change
                        double dlat = latitude - new_latitude ;
                        double dlon = longitude - new_longitude ;
                        double dalt = altitude - new_altitude ;
                        // keep old
                        latitude = new_latitude ;
                        longitude = new_longitude ;
                        if( altitude_valid)
                                altitude = new_altitude ;


                        if( (abs(dlat)> 1e-5) || (abs(dlon)> 1e-5) || (abs(dalt)>5) ) {

                            emit hasGpsFix(latitude,longitude);
                            QLogger::QLog_Debug("GPSD",
                                                QString::number(r_gps_hour) + ":" + QString::number(r_gps_min) + ":" + QString::number(r_gps_sec) + ":" +
                                                ", " + QString::number(new_latitude,'f',6) + "," + QString::number(new_longitude,'f',6) + "," + QString::number(new_altitude,'f',2) );
                        }

                        if( last_sec != gps_sec ) {
                            emit( hasGpsTime( gps_year, gps_month, gps_day, gps_hour, gps_min, gps_sec, 0) );
                            if( DEBUG_GPSD ) qDebug() << "GPSD:hasGpsTime" << gps_hour << gps_min <<  gps_sec;
                            last_sec = gps_sec ;
                        }
                    }
                }
            } else {
                if( n < 0 ) {
                    comm_port = 0 ;
                    break ;
                }
                sleep(50); // wait to receive something
            }
        }
    } else {
        if( DEBUG_GPSD ) qDebug() << "GPSD::run() comm_port ? " << comm_port ;
        QLogger::QLog_Error( LOGGER_NAME, "GPSD::run() comm_port ? " );
        emit( hasError(ERROR_NO_GPSD)) ;
    }
#else
    struct gps_data_t gps_data;
    char tbuf[128];


    //connect to GPSd
    if(gps_open("localhost", "2947", &gps_data)<0){
        if( DEBUG_GPSD ) qDebug() << "Could not connect to GPSd" ;
        emit( hasError(ERROR_NO_GPSD)) ;
        device_found = false ;
        return ;
    }
    device_found = true ;
    //register for updates
    gps_stream(&gps_data, WATCH_ENABLE | WATCH_JSON | WATCH_TIMING, NULL);

    for( ; ; ) {
        if (!gps_waiting(&gps_data, 5000000)) {
            emit( hasError(ERROR_NO_GPSD)) ;
            gps_close(&gps_data);
            return ;
            break ;

        }
        if (gps_read(&gps_data) == -1) {
            emit( hasError(ERROR_NO_GPSD)) ;
            gps_close(&gps_data);
            comm_port = 0 ;
            return ;
            break ;
        }

        if ((gps_data.fix.mode == MODE_2D) || (gps_data.fix.mode == MODE_3D) ) {
            float dlat = latitude - gps_data.fix.latitude ;
            float dlon = longitude - gps_data.fix.longitude ;

            if( (fabs(dlat)>1e-4) || (fabs(dlon)>1e-4) ) {
                latitude = gps_data.fix.latitude ;
                longitude = gps_data.fix.longitude ;
                emit( hasGpsFix(latitude, longitude));
            }

            unix_to_iso8601(gps_data.fix.time, tbuf, sizeof(tbuf));
            qDebug() << "gpsd:" << QString::fromLocal8Bit( tbuf);
            sscanf(tbuf, "%d-%d-%dT%d:%d:%d", &gps_year, &gps_month, &gps_day,&gps_hour,&gps_min,&gps_sec);

            if( last_sec != gps_sec ) {
                last_sec = gps_sec ;

                emit( hasGpsTime( gps_year, gps_month, gps_day, gps_hour, gps_min, gps_sec, 0) );
            }
        }
    }
    gps_close(&gps_data);
#endif
    qDebug() << "GPSD thread ending" ;
}
