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
#include "constants.h"
#include <QStandardPaths>
#include <QDebug>
#include <QApplication>
#include <QSettings>
#include <QFile>
#include <QDir>
#define DEBUG_GC (0)

GlobalConfig::GlobalConfig() {

    if( !QDir( QStandardPaths::writableLocation( QStandardPaths::HomeLocation) + "/" + QString(DATAFOLDER) ).exists() ) {
        QDir().mkpath(QStandardPaths::writableLocation( QStandardPaths::HomeLocation) + "/" + QString(DATAFOLDER) ) ;
    }
    QSettings settings(QStandardPaths::writableLocation( QStandardPaths::HomeLocation) +"/" + QString(DATAFOLDER) +
                       "/" + QString(CONFIG_FILENAME), QSettings::IniFormat);

    settings.beginGroup("Radio");
    if( !settings.contains("RX_FREQUENCY")) {
        settings.setValue("RX_FREQUENCY", (double)(DEFAULT_RX_FREQUENCY ) );
        cRX_FREQUENCY = (qint64)DEFAULT_RX_FREQUENCY ;
    } else {
        // value stored in MHz
        cRX_FREQUENCY = (qint64)(settings.value( "RX_FREQUENCY", (double)DEFAULT_RX_FREQUENCY ).toDouble() );
        if( (cRX_FREQUENCY < 0) || (cRX_FREQUENCY>10e9) ) {
            cRX_FREQUENCY = (qint64)DEFAULT_RX_FREQUENCY ;
        }
    }

    ppm_error = 0 ;
    if( settings.contains("PPM_ERROR")) {
        ppm_error = settings.value("PPM_ERROR").toInt();
    } else {
        settings.setValue("PPM_ERROR", (int)0 );
    }

    settings.endGroup();
    mLatitude = "" ;
    mLongitude = "" ;
    CALLSIGN = "" ;

    settings.beginGroup("Station");
    if( settings.contains("mLatitude") && settings.contains("mLongitude")) {
        mLatitude = settings.value("mLatitude").toString();
        mLongitude = settings.value("mLongitude").toString();
    }
    if( settings.contains("CALLSIGN")) {
        CALLSIGN = settings.value("CALLSIGN").toString() ;
    }
    settings.endGroup();
}


void GlobalConfig::getTuneParameters(qint64 frequencyOfInterest , TuningPolicy *tp) {

    tp->channelizer_offset = FRAME_OFFSET_LOW + DEMODULATOR_SAMPLERATE/2 ;
    tp->rx_hardware_frequency = frequencyOfInterest - tp->channelizer_offset ;

    if( DEBUG_GC ) {
        qDebug() << "--------------------------------------" ;
        qDebug() << "getTuneParameters() f0=" << frequencyOfInterest/1e6 << " MHz" ;
        qDebug() << "TuningPolicy.channelizer_offset=" << tp->channelizer_offset << " Hz." ;
        qDebug() << "TuningPolicy.rx_hardware_frequency=" << tp->rx_hardware_frequency/1e6 << " MHz." ;
    }

}

qint64 GlobalConfig::getReceivedFrequency( TuningPolicy *tp ) {
    return( tp->rx_hardware_frequency + (qint64)tp->channelizer_offset ) ;
}

