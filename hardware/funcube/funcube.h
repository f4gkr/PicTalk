//==========================================================================================
// + + +   This Software is released under the "Simplified BSD License"  + + +
// Copyright 2014 F4GKR Sylvain AZARIAN . All rights reserved.
// SPECIAL THANKS TO F6BHI FOR LETTING ME TESTING THE FUNCUBE
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
#ifndef FUNCUBE_H
#define FUNCUBE_H

#include <QThread>
#include <QComboBox>
#include <pthread.h>

#include <semaphore.h>
#include <stdint.h>
#include "hardware/rxdevice.h"
#include "common/tuningpolicy.h"
#include "common/datatypes.h"
#include "common/samplefifo.h"
#include "hardware/audio/audioinput.h"
#include "hidapi_libusb.h"

/** \brief FCD mode enumeration. */
typedef enum {
    FCD_MODE_NONE,  /*!< No FCD detected. */
    FCD_MODE_BL,    /*!< FCD present in bootloader mode. */
    FCD_MODE_APP    /*!< FCD present in application mode. */
} FCD_MODE_ENUM; // The current mode of the FCD: none inserted, in bootloader mode or in normal application mode


class FUNCube : public RxDevice
{
public:
    explicit FUNCube( bool pro = false );
    int getDeviceCount() ;

    void close();

    ~FUNCube() {}

    bool deviceHasSingleGainStage() { return( false ) ; }


    char* getHardwareName();
    int startAcquisition() ;
    int stopAcquisition();

    int setRxSampleRate( unsigned int sample_rate );
    uint32_t getRxSampleRate() ;

    int setRxCenterFreq( TuningPolicy *freq_hz ) ;
    qint64 getRxCenterFreq()  ;


    // this gives the physical limits (hardware limits) of the receiver by itself, without any RFE
    qint64 getMin_HWRx_CenterFreq()  ;
    qint64 getMax_HWRx_CenterFreq()  ;

    int getFifoSize();
    SampleFifo *getFIFO() ;


    void setIQCorrectionFactors( TYPEREAL gainI, TYPEREAL gainQ,
                                 TYPEREAL crossGain );
    void getIQCorrectionFactors( TYPEREAL *gainI,
                                 TYPEREAL *gainQ,
                                 TYPEREAL *crossGain );
    void setppmError( double ppmError )  ;
    double getppmError() ;

    QWidget* getDisplayWidget();
    FCD_MODE_ENUM fcdAppSetParam(unsigned char u8Cmd, unsigned char *pu8Data, unsigned char u8len);
    FCD_MODE_ENUM fcdAppGetParam(unsigned char u8Cmd, unsigned char *pu8Data, unsigned char u8len);

private:
    char *hardwareName ;
    double ppm_error ;
    AudioInput *audio_input ;
    qint64 centerFreq ; // FCD has 1Khz resolution  - freq stored in KHZ
    float gainMin ;
    float gainMax ;
    int sampling_rate ; // current sampling rate

    QWidget* gui ;

    hid_device *fcdOpen(void) ;
    void fcdClose(hid_device *phd);

    FCD_MODE_ENUM fcdAppSetFreqkHz(int nFreq ) ;
    FCD_MODE_ENUM fcdBlReset(void) ;
    hid_device *fcd ;
    unsigned short _PID ;

};

#endif // FUNCUBE_H
