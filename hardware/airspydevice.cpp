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
#include "airspydevice.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/time.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>

void* airspy_acquisition_thread( void *params ) ;
bool AirspyDevice::m_stop ;
bool AirspyDevice::m_quit ;

AirspyDevice::AirspyDevice()
{
    int rc ;
    device_count = 0 ;
    device = NULL ;
    dev_index = 0 ;
    gain_size = 0 ;
    gain_value = NULL ;
    hardwareName = NULL ;
    freq_hz = 0 ;
    sampling_rate = 0 ;
    gui = NULL ;

    rc = airspy_init() ;
    if( rc != AIRSPY_SUCCESS ) {
        airspy_exit();
        return ;
    }
    if( airspy_open( &device ) != AIRSPY_SUCCESS ) {
        airspy_exit();
        return ;
    }

    uint32_t num_rates;
    airspy_get_samplerates(device, &num_rates, 0);
    uint32_t *samplerates = (uint32_t *) malloc(num_rates * sizeof(uint32_t));
    airspy_get_samplerates( device, samplerates, num_rates);
    sampling_rate = 10e6 ;
    // tune to the smallest sampling rate available on the device
    // depends on the type of Airspy
    for( uint32_t i=0 ; i < num_rates ; i++ ) {
        if( samplerates[i] < sampling_rate ) {
            sampling_rate = samplerates[i] ;
        }
    }
    free( samplerates );
    if( sampling_rate == 0 ) {
        airspy_exit();
        return ;
    }
    device_count = 1 ;
    // configure device
    airspy_set_samplerate( device, sampling_rate );
    airspy_set_packing( device, 1 ); // use packing
    airspy_set_sample_type( device, AIRSPY_SAMPLE_FLOAT32_IQ);
    airspy_set_lna_gain( device, 5 );
    airspy_set_vga_gain( device, 5 );
    airspy_set_mixer_gain( device, 5);

    hardwareName = (char *)malloc( 64 * sizeof( char ));
    sprintf( hardwareName, "AirSpy");
    min_tuner_freq = 24000000 ;
    max_tuner_freq = 1900000000ul ;
    sem_init(&mutex, 0, 0);
    m_stop = false ;
    m_quit = false ;
    pthread_create(&receive_thread, NULL, airspy_acquisition_thread, this );
}

void AirspyDevice::close() {
    sem_post(&mutex);
    m_quit = true ;
}

int AirspyDevice::setRxSampleRate(uint32_t sampling_rate) {
    // AirSpy is set to max supported SR, without any possible change
    Q_UNUSED(sampling_rate);
    return( 1 );
}

char* AirspyDevice::getHardwareName() {
    return( hardwareName );
}

int AirspyDevice::setRxCenterFreq( TuningPolicy* freq_hz ) {
    int rc = airspy_set_freq( device, freq_hz->rx_hardware_frequency  );
    if( rc == AIRSPY_SUCCESS ) {
        this->freq_hz = freq_hz->rx_hardware_frequency ;
        return(1);
    }
    return( 0 );
}

qint64 AirspyDevice::getRxCenterFreq() {
    return( freq_hz  );
}


qint64 AirspyDevice::getMin_HWRx_CenterFreq() {
    return( min_tuner_freq ) ;
}

qint64 AirspyDevice::getMax_HWRx_CenterFreq()  {
    return( max_tuner_freq ) ;
}


QWidget* AirspyDevice::getDisplayWidget() {
    if( gui == NULL ) {
        gui = new AirspyWidget( device );
    }
    return( gui );
}

int AirspyDevice::startAcquisition() {
    if( device == NULL ) {
        return(0);
    }

    m_stop = false ;
    sem_post(&mutex);
    return(1);
}


int AirspyDevice::stopAcquisition() {
    m_stop = true ;
    airspy_stop_rx(device);
    return(1);
}

/**
 * @brief sdr_callback called by driver.
 */
int AirspyDevice::sdr_callback( airspy_transfer* transfer ) {
    TYPECPX *samples ;
     if( transfer->ctx == NULL ) {
         return(0) ;
     }

    AirspyDevice* my_device = (AirspyDevice*)transfer->ctx ;
    if( my_device->m_stop == true ) {
        return(0) ; // this value will stop the AirSpy's thread - see airspy.c : 424
    }

    int sample_count = transfer->sample_count ;
    samples = (TYPECPX *)malloc( sample_count * sizeof( TYPECPX ));
    if( samples == NULL ) {
        return(0) ;
    }

    memcpy( (void *)samples, transfer->samples,  transfer->sample_count * sizeof(TYPECPX) ) ;
    my_device->processData( samples, sample_count );
    return(0) ;
}

int AirspyDevice::processData( TYPECPX* samples, int len ) {
    if( len == 0 )
        return(0) ;
    if( samples == NULL )
        return(0);

    if( fifo->EnqueueData( (void *)samples, len, 0, NULL ) < 0 ) {
        qDebug() << "AirspyDevice::processData() problem - queue full ???" ;
    }
    return(1);
}

/**
 * @brief airspy_acquisition_thread This function is locked by the mutex and waits before starting the acquisition in asynch mode
 * @param params
 * @return
 */
void* airspy_acquisition_thread( void *params ) {
    int rc ;
    AirspyDevice* my_device = (AirspyDevice*)params ;
    airspy_device *device = my_device->device ;
    while( !my_device->m_quit ) {
        sem_wait( &my_device->mutex );

        airspy_set_samplerate( device, my_device->getRxSampleRate() );
        airspy_set_packing( device, 1 ); // use packing
        airspy_set_sample_type( device, AIRSPY_SAMPLE_FLOAT32_IQ);
        airspy_set_rf_bias( device, 0);
        rc = airspy_start_rx(device, &my_device->sdr_callback, (void *)my_device );
        if( rc == AIRSPY_SUCCESS ) {
            while (airspy_is_streaming(device) == AIRSPY_TRUE) {
                 usleep(20000);
            }
        }
    }
    return(NULL);
}
