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
#include "controller.h"
#include "common/samplefifo.h"
#include "hardware/gpdsd.h"
#include "common/constants.h"
#include <QApplication>

#define STEP_SIZE 16384
#define FFT_SPECTRUM_LEN 4096

Controller::Controller() : QThread(NULL)
{
    radio = NULL ;

    m_stop = false ;
    m_state = Controller::csInit ;
    channelizer = NULL ;
    processor = new FrameProcessor();
    fftin = (fftwf_complex *)fftwf_malloc(sizeof(fftwf_complex) * FFT_SPECTRUM_LEN );
    plan = fftwf_plan_dft_1d(FFT_SPECTRUM_LEN, fftin, fftin, FFTW_FORWARD, FFTW_ESTIMATE );
    spectrum = (double *)malloc( FFT_SPECTRUM_LEN * sizeof(double));
    hamming_coeffs = (double *)malloc( FFT_SPECTRUM_LEN * sizeof( double ));
    memset( spectrum, 0, FFT_SPECTRUM_LEN );
    semspectrum = new QSemaphore(1);
    hamming_window( hamming_coeffs, FFT_SPECTRUM_LEN ) ;

    connect( processor, SIGNAL(powerLevel(float)), this, SLOT(SLOT_powerLevelChanged(float)));
    connect( processor, SIGNAL(newState(QString)), this, SLOT(SLOT_frameDetectorStateChanged(QString)));
    connect( processor, SIGNAL(newSNRThreshold(float)), this, SLOT(SLOT_FPSetsNewThreshold(float)));

    sempos = new QSemaphore(1);
    m_Latitude = m_Longitude = m_Altitude = 0 ;
    GPSD& gpsd= GPSD::getInstance() ;
    connect( &gpsd, SIGNAL(hasGpsFix(double,double)), this, SLOT(SLOT_hasGpsFix(double,double)), Qt::QueuedConnection );
    connect( &gpsd, SIGNAL(hasGpsTime(int,int,int,int,int,int,int)), this, SLOT(SLOT_hasGpsTime(int,int,int,int,int,int,int)),Qt::QueuedConnection);

    GlobalConfig& gc = GlobalConfig::getInstance() ;
    tp = new TuningPolicy();
    gc.getTuneParameters( gc.cRX_FREQUENCY , tp);

    spectrum_interleave = 1 ;
    spectrum_interleave_value = 1 ;
    reestimate_noise = 0 ;
    webs = NULL ;
}

Controller::~Controller() {
    fftwf_free(fftin);
    fftin = NULL ;
    fftwf_destroy_plan(plan);
    plan = NULL ;
    free(spectrum);
    free(hamming_coeffs);
    delete semspectrum ;
}

void Controller::setWebservice(WebService *ws) {
    this->webs = ws ;
    if( webs != NULL ) {
        connect( webs, SIGNAL(mtuneTo(qint64)),
                 this, SLOT(setRxCenterFrequency(qint64)), Qt::QueuedConnection );
        connect( webs, SIGNAL(mturnOn()),
                 this, SLOT(startAcquisition()), Qt::QueuedConnection);
        connect( webs, SIGNAL(mturnOff()),
                 this, SLOT(stopAcquisition()), Qt::QueuedConnection );
    }
}

void Controller::hamming_window(double *win,  int win_size)
{
    int    i;
    if( win == NULL ) return ;
    for (i = 0; i < win_size; i++)
    {
        win[i] = (0.3635819 - 0.4891775 * cos((K_2PI * i) / (win_size - 1))
                  + 0.1365995 * cos((2.0 * K_2PI * i) / (win_size - 1))
                  - 0.0106411 * cos((3.0 * K_2PI * i) / (win_size - 1)));
    }
}

void Controller::setRadio(RxDevice *radio) {
    int radio_rate = radio->getRxSampleRate() ;

    this->radio = radio ;
    channelizer = new OverlapSave( radio_rate, DEMODULATOR_SAMPLERATE );
    if( radio_rate > 1e6 ) {
        channelizer->configure( 128*1024, 16384 );
    } else {
        channelizer->configure( 32*1024, 16384 );
    }

    connect( this, SIGNAL(radioStart()), radio, SLOT(SLOT_start()), Qt::QueuedConnection );
    connect( this, SIGNAL(radioStop()), radio, SLOT(SLOT_stop()), Qt::QueuedConnection );
}


void Controller::setRxCenterFrequency(qint64 frequency) {
    this->rx_tune_request = frequency ;

}

void Controller::startAcquisition() {
    qDebug() << "Controller::startAcquisition() " ;
    if( !this->isRunning() )
        return ;

    if( m_state == Controller::csIdle ) {
        next_state = Controller::csStart ;
        qDebug() << "Controller::startAcquisition() change state " ;
    }
    return ;
}

void Controller::stopAcquisition() {
    if( !this->isRunning() )
        return ;

    if( m_state == Controller::csRun ) {
        next_state = Controller::csStop ;
    }
    return ;
}

bool Controller::isAcquiring() {
    return( m_state ==Controller::csRun ) ;
}

void Controller::close() {
    m_stop = true ;
    radio->stopAcquisition() ;
    while( m_state != Controller::csEnded ) {
        QThread::msleep(10);
    }
    radio->close();    
}

float Controller::setDetectionThreshold(float level) {
    return( processor->setDetectionThreshold(level) );
}

void Controller::SLOT_FPSetsNewThreshold( float value ) {
    emit newSNRThreshold(value);
}

void Controller::setSpectrumInterleaveValue( int interleave ) {
    spectrum_interleave = qMax( FFTRATE_MAX - interleave , 1);
}

void Controller::doNoiseEstimation() {
    reestimate_noise++ ;
}

void Controller::run() {
    int i ;
    next_state = m_state ;
    SampleFifo *fifo = NULL ;
    TYPECPX* samples ;
    int sample_count ;

    GlobalConfig& gc = GlobalConfig::getInstance() ;

    qDebug() << "Controller::run() " ;
    //processor->moveToThread(this);

    while( !m_stop ) {

        if( next_state != m_state ) {
            qDebug() << "transition from " << m_state << " to " << next_state ;
            m_state = next_state ;
        }

        switch( m_state ) {

        case Controller::csInit:
            if( radio == NULL) {
                break ;
            }
            fifo = radio->getFIFO() ;
            next_state = Controller::csIdle ;
            break ;

        case Controller::csIdle:
            QThread::msleep(100);
            break ;

        case Controller::csStart:
            for( i=0 ; i < FFT_SPECTRUM_LEN ; i++ ) {
                spectrum[i] = -100 ;
            }
            if( radio != NULL )
                radio->setRxCenterFreq( tp );

            processor->raz();
            channelizer->reset();
            channelizer->setCenterOfWindow( tp->channelizer_offset );

            emit radioStart();
            if( webs != NULL ) {
                webs->reportStatus( true, tp->rx_hardware_frequency + tp->channelizer_offset );
            }
            next_state = Controller::csRun ;

            break ;

        case Controller::csRun:
            if( fifo == NULL ) {
                next_state = Controller::csInit ;
                break ;
            }
            samples = (TYPECPX *)fifo->DequeueData( &sample_count, 0,  NULL, true );
            if( (samples == NULL ) || (sample_count==0)) {
                QApplication::processEvents();
                msleep(1);
                continue ;
            }
            //qDebug() << " fifo " << fifo->getSize() ;
            process( samples, sample_count );
            if( rx_tune_request > 0 ) {
                gc.getTuneParameters( rx_tune_request, tp );
                rx_tune_request = 0 ;
                if( radio != NULL ) {
                    radio->setRxCenterFreq( tp );
                }
                processor->raz();
            }
            break ;

        case Controller::csStop:
            emit radioStop();
            next_state = Controller::csIdle ;
            if( webs != NULL ) {
                webs->reportStatus( false, tp->rx_hardware_frequency + tp->channelizer_offset  );
            }
            break ;
        }
    }
    m_state = csEnded ;
    qDebug() << "Controller::run()  ends" ;
}

void Controller::process( TYPECPX*samples, int L ) {
    TYPECPX* pt = samples ;
    TYPECPX out[STEP_SIZE] ;
    int rc,pushback_samples ;
    int left = L ;

    //qDebug() << "Controller::process() L=" << L ;

    if( (L >= FFT_SPECTRUM_LEN ) && (spectrum_interleave>0)){
        spectrum_interleave_value-- ;
        if( spectrum_interleave_value <= 0 ) {
            generateSpectrum(samples);
            emit newSpectrumAvailable(FFT_SPECTRUM_LEN, tp );
            spectrum_interleave_value = spectrum_interleave ;
        }
    }

    while( left > 0 ) {
        int qty = qMin(left, (int)STEP_SIZE) ;
        rc = channelizer->put(pt, qty) ;
        if( rc < 0 ) {
            break ;
        }
        left -= qty ;
        pt += qty ;

        if( rc == GET_DATA_OUT ) {
            rc = channelizer->get( out, STEP_SIZE, PREAMBLE_LENGTH )  ;
            while( rc > 0 ) {
                pushback_samples = processor->newData( out, rc, channelizer->getOLASOutSampleRate() );
                if( pushback_samples ) {                    
                    // rc is number of samples to pushback
                    TYPECPX *tmp = &out[0] ;
                    tmp += rc - 1 - pushback_samples ; // shift pointer
                    channelizer->pushback( tmp, pushback_samples );
                }
                rc = channelizer->get( out, STEP_SIZE , PREAMBLE_LENGTH)  ;
            }
        }
    }

    //
    if( reestimate_noise > 0 ) {
        processor->updateNoiseLevel();
        reestimate_noise = 0 ;
    }

    free(samples);
    return ;
}

void Controller::generateSpectrum( TYPECPX *samples ) {
    int i,j ;
    double cpow = 2.0/FFT_SPECTRUM_LEN ;

    smin = 0 ;
    smax = -200 ;
    // apply window
    for (i = 0;  i < FFT_SPECTRUM_LEN;i++)
    {
        fftin[i][0] = samples[i].re * hamming_coeffs[i];
        fftin[i][1] = samples[i].im * hamming_coeffs[i];
    }
    //compute FFT
    fftwf_execute( plan );

    //extract ft and compute power
    semspectrum->acquire(1);
    // neg portion of spectrum
    j = 0 ;
    for( i=FFT_SPECTRUM_LEN/2 ; i < FFT_SPECTRUM_LEN ; i++ ) {
        float a = fftin[i][0];
        float b = fftin[i][1];
        float modulus = sqrtf( a*a + b*b );
        double dbFs = 20*log10( cpow * modulus + 1e-10 );
        spectrum[j] = .9*spectrum[j] + .1*dbFs ;
        if( spectrum[j] > smax ) smax = spectrum[j] ;
        if( spectrum[j] < smin ) smin = spectrum[j] ;
        j++ ;
    }
    // pos spectrum
    for( i=0 ; i < FFT_SPECTRUM_LEN/2 ; i++ ) {
        float a = fftin[i][0];
        float b = fftin[i][1];
        float modulus = sqrtf( a*a + b*b );
        double dbFs = 20*log10( cpow * modulus + 1e-10);
        spectrum[j] = .9*spectrum[j] + .1*dbFs ;
        if( spectrum[j] > smax ) smax = spectrum[j] ;
        if( spectrum[j] < smin ) smin = spectrum[j] ;
        j++ ;
    }
    semspectrum->release(1);
}

void  Controller::getSpectrum( double* values ) {
    if( values == NULL )
        return ;
    semspectrum->acquire(1);
    memcpy( values, spectrum, FFT_SPECTRUM_LEN*sizeof(double));
    semspectrum->release(1);
}

void Controller::SLOT_frameDetectorStateChanged( QString stateName ) {
    emit newState( stateName );
    //qDebug() << "Controller::SLOT_frameDetectorStateChanged" << stateName ;
}

void Controller::SLOT_powerLevelChanged( float level )  {
    emit powerLevel(level);
}


void Controller::SLOT_hasGpsFix(double latitude, double longitude) {
    sempos->acquire(1);
    m_Latitude = latitude ;
    m_Longitude = longitude ;
    sempos->release(1);
}

void Controller::SLOT_hasGpsTime(int year, int month, int day, int hour, int min, int sec, int msec) {
    this->year = year ;
    this->month = month ;
    this->day = day ;
    this->hour = hour ;
    this->min = min ;
    this->sec = sec ;
    this->msec = msec ;
}
