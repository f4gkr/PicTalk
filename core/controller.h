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
#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QObject>
#include <QThread>
#include <QSemaphore>
#include <stdint.h>
#include <fftw3.h>

#include "hardware/rxdevice.h"
#include "dsp/overlapsave.h"
#include "dsp/frameprocessor.h"
#include "webinterface/webservice.h"

class Controller : public QThread
{
    Q_OBJECT
public:

    static Controller& getInstance()  {
        static Controller instance;
        return instance;
    }

    void setWebservice( WebService *ws );
    void setRadio( RxDevice* radio ) ;
    void doNoiseEstimation();

    bool isAcquiring();

    void close();
    void getSpectrum( double* values );

    float setDetectionThreshold(float level);
    void setSpectrumInterleaveValue( int interleave );

    TuningPolicy *getFrequency() { return( tp ) ; }

signals:

    void newSpectrumAvailable(int len, TuningPolicy* tp);
    void powerLevel( float level ) ;
    void newState( QString stateName );
    void newSNRThreshold( float threshold );

    void radioStart();
    void radioStop();

public slots:
    void setRxCenterFrequency( qint64 frequency ) ;
    void startAcquisition();
    void stopAcquisition();


    void SLOT_frameDetectorStateChanged( QString stateName );
    void SLOT_powerLevelChanged( float level ) ;
    void SLOT_FPSetsNewThreshold( float value );

private:
    enum  { csInit=0, csIdle=1, csStart=2,csRun=3, csStop=4, csEnded=99 } ;
    WebService* webs ;
    RxDevice *radio ;

    OverlapSave *channelizer ;
    FrameProcessor *processor ;

    TuningPolicy *tp ;
    uint64_t rx_tune_request ;

    bool m_stop ;
    int m_state, next_state ;
    int reestimate_noise ;

    double *spectrum ;
    double smin, smax ;
    double *hamming_coeffs ;
    fftwf_complex * fftin ;
    fftwf_plan plan ;

    QSemaphore *semspectrum ;
    int spectrum_interleave ;
    int spectrum_interleave_value ;

    double m_Latitude ;
    double m_Longitude ;
    double m_Altitude ;
    QSemaphore *sempos ;

    Controller();
    Controller(const Controller &); // hide copy constructor
    ~Controller();
    Controller& operator=(const Controller &);
    int year, month, day, hour, min, sec, msec ;
    void run();
    void process( TYPECPX*samples, int L );
    void generateSpectrum( TYPECPX *samples );
    void hamming_window(double *win,  int win_size) ;

};

#endif // CONTROLLER_H
