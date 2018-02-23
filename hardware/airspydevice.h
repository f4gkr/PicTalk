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

#ifndef AIRSPY_H
#define AIRSPY_H

#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>

#include "hardware/rxdevice.h"
#include "common/tuningpolicy.h"
#include "common/datatypes.h"
#include "common/samplefifo.h"
#include "airspy/airspy.h"
#include "airspy/airspywidget.h"

class AirspyDevice : public RxDevice
{
    Q_OBJECT
public:
    AirspyDevice();
    int getDeviceCount() { return( device_count ) ; }
    char* getHardwareName();

    int startAcquisition();
    int stopAcquisition();
    void close();

    int setRxSampleRate(uint32_t sampling_rate) ;
    uint32_t getRxSampleRate() {
        return( sampling_rate );
    }

    bool deviceHasSingleGainStage() { return( false ) ; }

    int setRxCenterFreq( TuningPolicy *freq_hz );
    qint64 getRxCenterFreq();

    qint64 getMin_HWRx_CenterFreq() ;
    qint64 getMax_HWRx_CenterFreq() ;

    QWidget* getDisplayWidget();

    static int sdr_callback( airspy_transfer* transfer ) ;
    struct airspy_device* device ;
    sem_t mutex;
    static  bool m_stop ;
private:

    char *hardwareName ;
    int dev_index  ;
    int device_count ;

    uint64_t freq_hz ; // current center freq
    uint32_t sampling_rate ; // current sampling rate

    float last_gain ;
    int *gain_value ;
    int gain_size ;
    float gainMin, gainMax ;
    pthread_t receive_thread ;
    uint64_t min_tuner_freq ;
    uint64_t max_tuner_freq ;

    AirspyWidget* gui ;

    int processData( TYPECPX* samples, int len ) ;
};

#endif // AIRSPY_H
