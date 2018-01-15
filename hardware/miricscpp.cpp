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
#include "miricscpp.h"
#include "common/constants.h"
#include "common/QLogger.h"
#include <QDebug>

#define DEBUG_RADIO (0)

void* miri_acquisition_thread( void *params ) ;
bool MiricsSDR::m_stop ;

MiricsSDR::MiricsSDR() {
    sdr_device = NULL ;
    dev_index = 0 ;
    device_count = 0 ;
    gain_size = 0 ;
    gain_value = NULL ;
    hardwareName = NULL ;
    freq_hz = 0 ;
    sampling_rate = 0 ;
    wr_pos = 0 ;
    OutBuf = NULL ;
    buf_len= 64*1024 ;
}

MiricsSDR::MiricsSDR( int select_index)
{
    int rc ;
    sdr_device = NULL ;
    dev_index = select_index ;
    device_count = 0 ;
    gain_size = 0 ;
    gain_value = NULL ;
    hardwareName = NULL ;
    freq_hz = 0 ;
    sampling_rate = 0 ;
    wr_pos = 0 ;
    OutBuf = NULL ;
    buf_len= 32*1024 ;

    rc = (int)mirisdr_get_device_count();
    if( rc < 1 ) {
        return ;
    }
    device_count = rc ;
    rc = (int)mirisdr_open( &sdr_device, 0 );
    if (rc < 0) {
        qDebug() << "Failed to open mirics device" << dev_index << "rc:" << rc ;
        device_count = 0 ;
        return ;
    }

    if( sdr_device == NULL ) {
        qDebug() << "Failed to open mirics device" <<  dev_index ;
        dev_index = -1 ;
        return ;
    }
    rc = mirisdr_reset( sdr_device );
    if( rc < 0 ) {
        // cannot reset ?
        return ;
    }

    gain_size = mirisdr_get_tuner_gains( sdr_device, NULL );
    gain_value = (int *)malloc( gain_size * sizeof(int) );
    gainMin = 999 ;
    gainMax = 0 ;
    mirisdr_get_tuner_gains( sdr_device, gain_value );
    for( rc = 0 ; rc < gain_size ;rc ++ ) {
        float v = gain_value[rc]  ;
        if( v > 0 ) {
            gainMin = qMin( gainMin, v );
            gainMax = qMax( gainMax, v );
        }
    }
    last_gain = gainMin + (gainMax - gainMin)/2 ;

    // freq coverage
    hardwareName = (char *)malloc( 64 * sizeof( char ));
    sprintf( hardwareName, "MiricsSDR");
    min_tuner_freq = 150e3 ;
    max_tuner_freq = 2000e6 ;
    sem_init(&mutex, 0, 0);
    m_stop = false ;

    mirisdr_set_if_freq( sdr_device, 0 );
    mirisdr_set_tuner_gain( sdr_device, last_gain );
    mirisdr_set_sample_format( sdr_device, (char *)"AUTO");
    mirisdr_set_sample_rate( sdr_device, 1536*1000 ) ;

    pthread_create(&receive_thread, NULL, miri_acquisition_thread, this );
}


char* MiricsSDR::getHardwareName() {
    return( hardwareName );
}

int MiricsSDR::setRxCenterFreq( TuningPolicy* freq_hz ) {
    int rc = mirisdr_set_center_freq( sdr_device, freq_hz->rx_hardware_frequency  );
    if( !rc ) {
        this->freq_hz = freq_hz->rx_hardware_frequency ;
    }
    return( rc );
}

qint64 MiricsSDR::getRxCenterFreq() {
    freq_hz = (uint64_t)mirisdr_get_center_freq( sdr_device ) ;
    return( freq_hz  );
}


bool MiricsSDR::setAutoGainMode() {
    return(false);
}

int MiricsSDR::setRxGain( float db )  {
    int k ;
    // set manual gain
    if( gain_size > 0 ) {
        for( k=0 ; k < gain_size-1 ; k++ ) {
            if( (gain_value[k]<=db) && (gain_value[k+1]>=db)) {
                db = gain_value[k];
                break ;
            }
        }
    }
    int rc = mirisdr_set_tuner_gain( sdr_device, db );
    return( rc );
}

float MiricsSDR::getRxGain() {
    float value = mirisdr_get_tuner_gain( sdr_device );
    return( value );
}

int MiricsSDR::startAcquisition() {
    int rc ;

    if( DEBUG_RADIO )
        qDebug() << "MiricsSDR::startAcquisition()" ;

    if( sdr_device == NULL ) {
        qDebug() << "ERROR:MiricsSDR::startAcquisition() sdr_device == NULL" ;
        return(0);
    }

    if( sampling_rate == 0 ) {
        return(0);
    }

    rc = mirisdr_reset_buffer(sdr_device);
    if (rc < 0) {
        qDebug() << "ERROR:MiricsSDR::startAcquisition() mirisdr_reset_buffer" ;
        return(0);
    }
    if( DEBUG_RADIO )
        qDebug() << "MiricsSDR::startAcquisition() sem_post" ;
    xn_1.re = 0 ;
    xn_1.im = 0 ;
    yn_1.re = 0 ;
    yn_1.im = 0 ;
    wr_pos  = 0 ;
    sem_post(&mutex);
    return(1);
}


int MiricsSDR::stopAcquisition() {
    mirisdr_cancel_async( sdr_device ) ;
    return(1);
}

int MiricsSDR::setRxSampleRate(uint32_t sampling_rate ) {
    if( this->sampling_rate == sampling_rate ) {
        return( 1 ) ;
    }
    int rc = mirisdr_set_sample_rate( sdr_device, sampling_rate );
    if( rc == 0 ) {
        mirisdr_set_bandwidth( sdr_device, sampling_rate );
        this->sampling_rate = sampling_rate ;
    } else {
        this->sampling_rate = mirisdr_get_sample_rate( sdr_device );
    }
    return( rc );
}

qint64 MiricsSDR::getMin_HWRx_CenterFreq() {
    return( min_tuner_freq ) ;
}

qint64 MiricsSDR::getMax_HWRx_CenterFreq()  {
    return( max_tuner_freq ) ;
}

#define ALPHA_DC (0.999)
#define USE_DC_REMOVAL
int MiricsSDR::processData( unsigned char *buf, uint32_t len  ) {
    TYPECPX tmp ;
    float I,Q ;
    int16_t *rawsamples = (int16_t *)buf ;
    int sample_count = len / (sizeof(int16_t) * 2) ;

    if( OutBuf == NULL ) {
        // allocate new buffer
        OutBuf = (TYPECPX*)malloc( buf_len * sizeof(TYPECPX) );
        wr_pos = 0 ;
    }

    if( OutBuf == NULL ) {
        qDebug() << "MALLOC ???? int MIRISDR::processData " ;
        return(0);
    }

    for( int i=0 ; i < sample_count ; i++ ) {
        int j = 2*i ;
        I =  (float)rawsamples[j  ] * 1.0/32767.0f   ;
        Q =  (float)rawsamples[j+1] * 1.0/32767.0f   ;
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
        OutBuf[wr_pos].re = tmp.re ;
        OutBuf[wr_pos].im = tmp.im ;
#else
        //----
        OutBuf[wr_pos].re = I ;
        OutBuf[wr_pos].im = Q ;
#endif
        wr_pos++ ;
        if( wr_pos == buf_len ) {
            // our buffer is full, send it
            if( fifo->EnqueueData( (void *)OutBuf, buf_len, 0, NULL ) < 0 ) {
                qDebug() << "MIRISDR::processData() problem - queue full ???" ;
            }
            OutBuf = (TYPECPX*)malloc( buf_len * sizeof(TYPECPX) );
            wr_pos = 0 ;
        }
    }

    if( DEBUG_RADIO )
        qDebug() << "MiricsSDR::processData() queueing samples: " << sample_count ;


    return( (int)len ) ;
}

void MiricsSDR::sdr_device_callback(unsigned char *buf, uint32_t len, void *ctx) {
    MiricsSDR *dongle = (MiricsSDR *)ctx ;
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
#ifndef _WIN32
#define DEFAULT_ASYNC_BUF_NUMBER 4
#define DEFAULT_BUF_LENGTH      (1<<14)
#else
#define DEFAULT_ASYNC_BUF_NUMBER 4
#define DEFAULT_BUF_LENGTH      (1<<14)
#endif
void* miri_acquisition_thread( void *params ) {
    int rc ;
    MiricsSDR* dev = (MiricsSDR*)params ;
    mirisdr_dev_t *sdr_device = dev->sdr_device ;
    if( DEBUG_RADIO )
        qDebug() << "acquisition_thread() starting" ;

    for( ; ; ) {
        sem_wait( &dev->mutex );
        if(MiricsSDR::m_stop )
            break ;

        if( DEBUG_RADIO )
            qDebug() << "acquisition_thread() got mutex, start SDR" ;

        rc = mirisdr_read_async(sdr_device, dev->sdr_device_callback, params,
                           DEFAULT_ASYNC_BUF_NUMBER, DEFAULT_BUF_LENGTH);
        if( rc != 0 ) {
            qDebug() << "Could not start device !" ;
            MiricsSDR::m_stop = true ;
        }
        if(MiricsSDR::m_stop )
            break ;
    }
    return(NULL);
}

void MiricsSDR::close() {
    if( sdr_device != NULL ) {
        stopAcquisition();
        m_stop = true ;
        sem_post(&mutex);
        if( DEBUG_RADIO )
            qDebug() << "closing SDR device" ;
        mirisdr_close(sdr_device);

    }
}

