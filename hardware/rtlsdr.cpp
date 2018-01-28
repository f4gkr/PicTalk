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
#include "rtlsdr.h"
#include "common/constants.h"
#include "common/QLogger.h"

#include <QDebug>
#define DEBUG_RADIO (0)
void* acquisition_thread( void *params ) ;
bool RTLSDR::m_stop ;

RTLSDR::RTLSDR() {
    rtlsdr_device = NULL ;
    dev_index = 0 ;
    device_count = 0 ;
    gain_size = 0 ;
    gain_value = NULL ;
    hardwareName = NULL ;
    freq_hz = 0 ;
    sampling_rate = 0 ;
}

RTLSDR::RTLSDR( int select_index)
{
    int rc ;
    char manufact[256], product[256], serial[256] ;
    rtlsdr_device = NULL ;
    dev_index = select_index ;
    device_count = 0 ;
    gain_size = 0 ;
    gain_value = NULL ;
    hardwareName = NULL ;
    freq_hz = 0 ;
    sampling_rate = 0 ;
    gainMin = 1000 ;
    gainMax = -1000 ;

    rc = (int)rtlsdr_get_device_count();
    if( rc < 1 ) {
        QLogger::QLog_Error(  "RTLSDR", "No RTL-SDR device connected, forgiving");
        return ;
    }
    device_count = rc ;
    if( dev_index <= 0 ) {
        rc = (int)rtlsdr_open( &rtlsdr_device, 0 );
        if (rc < 0) {
            qDebug() << "Failed to open rtlsdr device" << dev_index << "rc:" << rc ;
            QLogger::QLog_Trace( "RTLSDR", "RTLSDR::RTLSDR Failed to open rtlsdr device " + QString::number(dev_index) + " rc:" + QString::number(rc) );
            device_count = 0 ;
            return ;
        }
    } else {
        // try to find first available device
        for( int i = 0 ; i < device_count ; i++ ) {
            qDebug() << "testing device #" << QString::number(i);
            rc = (int)rtlsdr_open( &rtlsdr_device, i );
            if( rc >= 0 ) {
                qDebug() << "using device #" << QString::number(i);
                QLogger::QLog_Trace( "RTLSDR", "RTLSDR::RTLSDR using device # " + QString::number( i) );
                dev_index = i ;
                break ;
            } else {
                qDebug() << "failed with device #" << QString::number(i) << "rc=" << rc ;
                QLogger::QLog_Trace( "RTLSDR", "RTLSDR::RTLSDR failed with device # " + QString::number( i) );
            }
            rtlsdr_device = NULL ;
        }
    }

    if( rtlsdr_device == NULL ) {
        QLogger::QLog_Fatal("RTLSDR", "RTLSDR::RTLSDR failed to open device, check permissions with driver" );
        qDebug() << "Failed to open rtlsdr device" <<  dev_index ;
        dev_index = -1 ;
        return ;
    }

    gain_size = rtlsdr_get_tuner_gains( rtlsdr_device, NULL );
    if( gain_size > 0 ) {
        gain_value = (int *)malloc( gain_size * gain_size );
        rtlsdr_get_tuner_gains( rtlsdr_device, gain_value );
        for( rc = 0 ; rc < gain_size ;rc ++ ) {
            float v = gain_value[rc] / 10.0 ;
            if( v < this->gainMin ) this->gainMin = v ;
            if( v > this->gainMax ) this->gainMax = v ;
        }
    }

    // freq coverage
    hardwareName = (char *)malloc( 64 * sizeof( char ));
    char *typeName = (char *)malloc( 16 * sizeof( char ));
    min_tuner_freq = 70e6 ;
    max_tuner_freq = 1700e6 ;
    enum rtlsdr_tuner ttype = rtlsdr_get_tuner_type( rtlsdr_device ) ;
    switch( ttype ) {
    case RTLSDR_TUNER_UNKNOWN:
        sprintf( typeName, "%s", "SDR");
        QLogger::QLog_Trace( "RTLSDR", "RTLSDR::RTLSDR RTLSDR_TUNER_UNKNOWN");
        break ;
    case RTLSDR_TUNER_E4000 :
        sprintf( typeName, "%s", "E4000");
        min_tuner_freq = 52e6 ;
        max_tuner_freq = 2200e6 ;
        break ;
    case RTLSDR_TUNER_R820T:
        sprintf( typeName, "%s", "820T");
        min_tuner_freq = 24e6 ;
        max_tuner_freq = 1766e6 ;
        break ;
    case RTLSDR_TUNER_R828D:
        sprintf( typeName, "%s", "828D");
        min_tuner_freq = 24e6 ;
        max_tuner_freq = 1766e6 ;
        break ;
    case RTLSDR_TUNER_FC0013:
        sprintf( typeName, "%s", "FC13");
        min_tuner_freq = 22e6 ;
        max_tuner_freq = 1100e6 ;
        break ;
    case RTLSDR_TUNER_FC0012:
        sprintf( typeName, "%s", "FC12");
        min_tuner_freq = 22e6 ;
        max_tuner_freq = 948e6 ;
        break ;
    case RTLSDR_TUNER_FC2580:
        sprintf( typeName, "%s", "FC2580");
        min_tuner_freq = 146e6 ;
        max_tuner_freq = 924e6 ;
        break ;
    }

    qDebug() << " Tuner type:" << ttype << " min_tuner_freq=" << min_tuner_freq << " max_tuner_freq=" << max_tuner_freq ;
    qDebug() << " gain range : " << gainMin << " to " << gainMax ;

    rc = rtlsdr_get_usb_strings( rtlsdr_device, manufact, product, serial );
    if( rc == 0 ) {
        if( manufact != NULL )
            qDebug() << " manufacturer ::" << manufact ;
        if( product != NULL )
            qDebug() << " product ::" << product ;
        if( serial != NULL )
            qDebug()   << " serial ::" << serial ;
        QLogger::QLog_VATrace("gkSDR", "RTLSDR::RTLSDR Found manufacturer %s  product %s  serial %s", manufact,  product, serial );
    }
    if( select_index > 0 ) {
        sprintf( hardwareName, "RTL%s_%d", typeName, select_index );
    } else {
        sprintf( hardwareName, "RTL%s", typeName );
    }
    free( typeName );
    last_gain = 0 ;

    sem_init(&mutex, 0, 0);
    m_stop = false ;

    setRxSampleRate();
    setRxGain(10);

    TuningPolicy *tp = new TuningPolicy();
    tp->channelizer_offset = 0 ;
    tp->rx_hardware_frequency = min_tuner_freq+10e6 ;
    setRxCenterFreq(tp) ;
    delete tp ;

    pthread_create(&receive_thread, NULL, acquisition_thread, this );
    for( int k=0 ; k < 255 ; k++ ) {
         LUT[k] = (k-127)/127.0f ;
    }
}

char* RTLSDR::getHardwareName() {
    return( hardwareName );
}

int RTLSDR::setRxCenterFreq( TuningPolicy* freq_hz ) {
    //printf("%s freq=%f\n", __func__, ((double)freq_hz->rx_hardware_frequency)/1e6 ) ;
    int rc = rtlsdr_set_center_freq( rtlsdr_device, freq_hz->rx_hardware_frequency  );
    if( !rc ) {
        this->freq_hz = freq_hz->rx_hardware_frequency ;
    }
    return( rc );
}

qint64 RTLSDR::getRxCenterFreq() {
    freq_hz = (uint64_t)rtlsdr_get_center_freq( rtlsdr_device ) ;
    return( freq_hz  );
}


bool RTLSDR::setAutoGainMode() {
    int rc = rtlsdr_set_tuner_gain_mode( rtlsdr_device, 0);
    return( rc == 0 );
}

int RTLSDR::setRxGain( float db )  {
    int k ;
    db *= 10 ; // gain stored in tenth of dB
    // set manual gain
    if( gain_size > 0 ) {
        for( k=0 ; k < gain_size-1 ; k++ ) {
            if( (gain_value[k]<=db) && (gain_value[k+1]>=db)) {
                db = gain_value[k];
                break ;
            }
        }
    }
    int rc = rtlsdr_set_tuner_gain_mode( rtlsdr_device, 1);
    if( !rc ) {
        // set value
        rc = rtlsdr_set_tuner_gain( rtlsdr_device, db );
        return( rc );
    }
    return( rc );
}

float RTLSDR::getRxGain() {
      int value = rtlsdr_get_tuner_gain( rtlsdr_device );
      return( value / 10.0 );
}

int RTLSDR::startAcquisition() {
    int rc ;

     if( DEBUG_RADIO )
         qDebug() << "RTLSDR::startAcquisition()" ;

    if( rtlsdr_device == NULL ) {
        qDebug() << "ERROR:RTLSDR::startAcquisition() rtlsdr_device == NULL" ;
        return(0);
    }

    if( sampling_rate == 0 ) {
         setRxSampleRate();
         setRxGain( 30 );
    }

    rc = rtlsdr_reset_buffer(rtlsdr_device);
    if (rc < 0) {
         qDebug() << "ERROR:RTLSDR::startAcquisition() rtlsdr_reset_buffer" ;
        return(0);
    }
    if( DEBUG_RADIO )
        qDebug() << "RTLSDR::startAcquisition() sem_post" ;
    xn_1.re = 0 ;
    xn_1.im = 0 ;
    yn_1.re = 0 ;
    yn_1.im = 0 ;
    sem_post(&mutex);
    return(1);
}


int RTLSDR::stopAcquisition() {
    rtlsdr_cancel_async( rtlsdr_device ) ;    
    return(1);
}

int RTLSDR::setRxSampleRate(uint32_t sampling_rate ) {
    int rc = rtlsdr_set_sample_rate( rtlsdr_device, sampling_rate );
    if( !rc ) {
        this->sampling_rate = sampling_rate ;
    }
    return( rc );
}

qint64 RTLSDR::getMin_HWRx_CenterFreq() {
    return( min_tuner_freq ) ;
}

qint64 RTLSDR::getMax_HWRx_CenterFreq()  {
    return( max_tuner_freq ) ;
}

#define ALPHA_DC (0.999)
//#define USE_DC_REMOVAL
int RTLSDR::processData( unsigned char *buf, uint32_t len ) {
      uint32_t lendata ;
      uint32_t i,j ;

      TYPEREAL I,Q ;
      TYPECPX tmp ;
      lendata = (len/2) * sizeof( TYPECPX ) ;
      TYPECPX *OutBuf = (TYPECPX*)malloc( lendata );
      if( OutBuf == NULL ) {
          qDebug() << "MALLOC ???? int RTLSDR::processData " ;
          return(0);
      }

      for( i=0 ; i < len/2 ; i++ ) {
          j = 2*i ;
          //I =  ((int)buf[j  ] - 127)/ 127.0f   ;
          //Q =  ((int)buf[j+1] - 127)/ 127.0f   ;
          I = LUT[buf[j]] ;
          Q = LUT[buf[j+1]];
#ifdef USE_DC_REMOVAL
          // DC
          // y[n] = x[n] - x[n-1] + alpha * y[n-1]
          // see http://peabody.sapp.org/class/dmp2/lab/dcblock/
          tmp.re = I - xn_1.re + ALPHA_DC * yn_1.re ;
          tmp.im = Q - xn_1.im + ALPHA_DC * yn_1.im ;

          xn_1.re = I ;
          xn_1.im = Q ;

          yn_1.re = tmp.re ;
          yn_1.im = tmp.im ;
          //----
          OutBuf[i].re = tmp.re ;
          OutBuf[i].im = tmp.im ;
#else
          //----
          OutBuf[i].re = I ;
          OutBuf[i].im = Q ;
#endif

      }

      if( fifo->EnqueueData( (void *)OutBuf, len/2, 0, NULL ) < 0 ) {
          qDebug() << "RTLSR::processData() problem - queue full ???" ;
      }
      return( (int)len/2 ) ;
}

void RTLSDR::rtlsdr_callback(unsigned char *buf, uint32_t len, void *ctx) {
    RTLSDR *dongle = (RTLSDR *)ctx ;
    if( len == 0 ) return ;
    if( buf == NULL ) return ;

    /* now dispatch data for local processing */
    if( dongle != NULL ) {
        dongle->processData( buf, len );
    }
}

/**
 * @brief acquisition_thread This function is locked by the mutex and waits before starting the acquisition in asynch mode
 * @param params
 * @return
 */
void* acquisition_thread( void *params ) {
    RTLSDR* dev = (RTLSDR*)params ;
    rtlsdr_dev_t *rtlsdr_device = dev->rtlsdr_device ;
    if( DEBUG_RADIO )
        qDebug() << "acquisition_thread() starting" ;
    for( ; ; ) {
        sem_wait( &dev->mutex );
        if(RTLSDR:: m_stop )
            break ;
        if( DEBUG_RADIO )
            qDebug() << "acquisition_thread() got mutex, start SDR" ;
        rtlsdr_read_async(rtlsdr_device, dev->rtlsdr_callback, (void *)dev, 0, 65536);
        if( RTLSDR::m_stop )
            break ;
    }
    return(NULL);
}

void RTLSDR::close() {
    if( rtlsdr_device != NULL ) {
        stopAcquisition();
        m_stop = true ;
        sem_post(&mutex);
        if( DEBUG_RADIO )
            qDebug() << "closing SDR device" ;
        rtlsdr_close(rtlsdr_device);

    }
}

