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

#ifndef RXDEVICE_H
#define RXDEVICE_H
#include <QObject>
#include <stdint.h>
#include <qglobal.h>
#include "common/tuningpolicy.h"
#include "common/datatypes.h"
#include "common/samplefifo.h"

class RxDevice: public QObject
{
    Q_OBJECT
public:    
    explicit RxDevice(QObject *parent = 0);
    ~RxDevice();

    virtual void close() = 0 ;

    virtual int getDeviceCount() = 0 ;
    virtual char* getHardwareName()= 0;
    virtual int startAcquisition() = 0;
    virtual int stopAcquisition() = 0;

    virtual int setRxSampleRate(uint32_t sampling_rate ) = 0;
    virtual uint32_t getRxSampleRate() = 0;

    virtual int setRxCenterFreq( TuningPolicy *freq_hz ) = 0 ;
    virtual qint64 getRxCenterFreq() = 0 ;

    // this gives the physical limits (hardware limits) of the receiver by itself, without any RFE
    virtual qint64 getMin_HWRx_CenterFreq() = 0  ;
    virtual qint64 getMax_HWRx_CenterFreq() = 0   ;

    virtual int getFifoSize();
    virtual SampleFifo *getFIFO() { return( fifo ); }

    virtual bool deviceHasSingleGainStage() { return( true ) ; }
    virtual QWidget* getDisplayWidget() {return(NULL) ; }

    virtual int setRxGain(float db );
    virtual float getRxGain();
    virtual float getMinGain() ;
    virtual float getMaxGain();

    virtual float getNoiseFloor() ;

public slots:
    void SLOT_start();
    void SLOT_stop();

protected:
    SampleFifo *fifo ;

};

#endif // RXDEVICE_H
