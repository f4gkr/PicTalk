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
#ifndef UAVPROCESSOR_H
#define UAVPROCESSOR_H

#include <QObject>
#include <QQueue>
#include <fftw3.h>

#include "common/samplefifo.h"
#include "common/datatypes.h"
#include "core/sampleblock.h"
#include "zmqserver.h"

#define DEFAULT_DETECTION_THRESHOLD (4)
#define DEFAULT_AC_THRESHOLD (40)

class FrameProcessor : public QObject
{
    Q_OBJECT
public:

    explicit FrameProcessor(QObject *parent = 0);

    // change detection threshold for the correlator
    // default is DEFAULT_DETECTION_THRESHOLD dB
    float setDetectionThreshold(float level);

    // call this function each time a paquet of samples is available
    int newData(TYPECPX* IQsamples, int L , int sampleRate );

    void raz();
    void updateNoiseLevel();

signals:
    void newState( QString stateName );
    void powerLevel( float level ) ;
    void frameDetected( float signal_level  ) ;
    void newSNRThreshold( float threshold );

    void newDataDetected() ;

private slots:
    void SLOT_newDataReady();

private:
    enum  { sInit=0 ,
            sMeasureNoise=1,
            sSearchFrame=2,
            sFrameStart=3,
            sFrameEnds=4
          } ;

    enum {
            sUseRmsPower = 0,
            sUseCorrelator=1
    };

    ZmqServer* zmqs ;
    int m_update_noise_request ;
    int m_state, next_state ;
    QQueue<SampleBlock*> queue ;
    long queueSampleCount ;

    int m_bandwidth ;
    float threshold ; // effective RMS value considered for frame proc
    float noise_floor;
    float detection_threshold ; // snr required for detection

    double rms_power ;
    long samples_for_powerestimation ;

    // variables used for autocorrelation test
    fftwf_complex *fftin ;
    fftwf_complex *buffer ;
    fftwf_plan plan, plan_rev ;
    double A,B,C;
    int w_pos ;

    float detector( TYPECPX *samples, int L);
    float rmsp(TYPECPX *samples, int L );
    float correlator( TYPECPX *samples, int L);

    QString stateToS(int s);

    int processDataRMS(TYPECPX* IQsamples, int L , int sampleRate );
    int processDataAD(TYPECPX* IQsamples, int L , int sampleRate );
    float modulus(int i);
    void ac(TYPECPX *start, int L);
};

#endif // UAVPROCESSOR_H
