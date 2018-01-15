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
#include "frameprocessor.h"
#include <stdlib.h>
#include <QDebug>
#include <math.h>
#include <stdio.h>
#include "common/constants.h"

#define FRAME_DEBUG (1)
#define FFT_SIZE (1024)


FrameProcessor::FrameProcessor(QObject *parent) : QObject(parent)
{
    m_bandwidth = 0 ;
#ifdef USE_CORRELATOR
    threshold = detection_threshold = -40 ;
#else
    detection_threshold = DEFAULT_DETECTION_THRESHOLD ;
#endif
    noise_floor = 0 ;
    m_state = next_state = FrameProcessor::sInit ;
    queueSampleCount = 0 ;
    m_update_noise_request = 0 ;
    fftin = (fftwf_complex *)fftwf_malloc(sizeof(fftwf_complex) * FFT_SIZE);
    plan = fftwf_plan_dft_1d( FFT_SIZE, fftin, fftin, FFTW_FORWARD, FFTW_ESTIMATE );
    plan_rev = fftwf_plan_dft_1d( FFT_SIZE, fftin, fftin, FFTW_BACKWARD, FFTW_ESTIMATE );
    w_pos = 0 ;
    A = B = C = -100 ;

    zmqs = new ZmqServer();
    zmqs->start();

    connect( this, SIGNAL(newDataDetected()), this, SLOT(SLOT_newDataReady()));
}

float FrameProcessor::setDetectionThreshold(float level) {
#ifdef USE_CORRELATOR
    threshold = -1*level ;
    if( FRAME_DEBUG ) qDebug() << " threshold is now " << threshold ;
    return( threshold );
#else
    detection_threshold = level ;
    threshold = noise_floor + detection_threshold ;
    if( FRAME_DEBUG ) qDebug() << " noise level was " << noise_floor ;
    if( FRAME_DEBUG ) qDebug() << " threshold is now " << threshold ;
    return( threshold );
#endif
}

void FrameProcessor::updateNoiseLevel() {
    m_update_noise_request++ ;
}


void FrameProcessor::raz() {
    next_state = sInit ;
}

QString FrameProcessor::stateToS(int s) {
    switch(s) {
    case sInit: return( QString("sInit"));
    case sMeasureNoise: return( QString("sMeasureNoise"));
    case sSearchFrame: return( QString("sSearchFrame"));
    case sFrameStart: return( QString("sFrameStart"));
    case sFrameEnds: return( QString("sFrameEnds"));
    }
    return("");
}

int FrameProcessor::newData( TYPECPX* IQsamples, int L , int sampleRate ) {
#ifdef USE_CORRELATOR
    return( processDataAD(IQsamples, L, sampleRate));
#else
    return( processDataRMS(IQsamples, L, sampleRate));
#endif
}

float FrameProcessor::modulus(int i) {
    float a = (float)fftin[i][0];
    float b = (float)fftin[i][1];
    return( a*a + b*b );
}

void FrameProcessor::ac(tSComplex *start, int L) {
    // copy PREAMBLE_LENGTH samples in FFT buffer
    memset( (void *)fftin, 0, FFT_SIZE*sizeof(TYPECPX));
    memcpy( (void *)fftin, start, L*sizeof(TYPECPX));
    // X  = fft(x_pad);
    fftwf_execute( plan );
    // X_psd = abs(X).^2;
    for( int i=0 ; i < FFT_SIZE ; i++ ) {
         fftin[i][0] = modulus(i) ;
         fftin[i][1] = 0 ;
    }
    // r_xx = ifft(X_psd);
    fftwf_execute(plan_rev);
}

#define SUBSAMPLERATE (3)
#define CX1 (.8)
#define DEBUG_ADETECTOR (0)
int FrameProcessor::processDataAD( TYPECPX* IQsamples, int L , int sampleRate ) {
    Q_UNUSED(sampleRate ) ;
    int remaining_samples = L ;
    int N ;
    TYPECPX* start = IQsamples ;
    float v ;
    SampleBlock *sb ;

    static int subsample = SUBSAMPLERATE ;
    static float level4display = 0 ;

    while( remaining_samples > 0 ) {
        if( next_state != m_state ) {
            if( FRAME_DEBUG )  qDebug() << " state transition from " << stateToS(m_state) << " to " << stateToS(next_state);
            m_state = next_state ;
            emit newState( stateToS(m_state));
        }

        switch( m_state ) {
        case sInit:
            noise_floor = -1 ;
            rms_power = 0 ;
            next_state = sSearchFrame ;
            A = B = C = level4display = -50 ;
            for( int i=0 ; i < FFT_SIZE ; i++ ) {
                fftin[i][0] = fftin[i][1] = 0 ;
            }

            continue ;

        case sMeasureNoise:
            next_state = sSearchFrame ;

        case sSearchFrame:
            if( remaining_samples >= PREAMBLE_LENGTH ) {
                ac(start, PREAMBLE_LENGTH );
                //
                double root = modulus(0);
                A = 0 ;
                N = 0 ;
                for( int i=1 ; i < FFT_SIZE/2 ; i++) {
                    A += modulus(i);
                    N++ ;
                }
                A = A / N ;
                v = 20*log10f( A / root );

                if( v > threshold ) {
                    if( (L-remaining_samples) > 0 ) {
                        TYPECPX* cstart = start ;
                        cstart -= (L-remaining_samples) ;
                        SampleBlock *sb = new SampleBlock( cstart, (L-remaining_samples) + PREAMBLE_LENGTH );
                        queue.enqueue( sb );
                        queueSampleCount = (L-remaining_samples) + PREAMBLE_LENGTH ;

                    } else {
                        SampleBlock *sb = new SampleBlock( start, PREAMBLE_LENGTH );
                        queue.enqueue( sb );
                        queueSampleCount = PREAMBLE_LENGTH ;
                    }
                    next_state = sFrameStart ;
                    emit frameDetected( v ) ;
                    start += PREAMBLE_LENGTH ;
                    remaining_samples -= PREAMBLE_LENGTH ;

                    emit newDataDetected();
                } else {
                    start += PREAMBLE_LENGTH/2 - 1 ;
                    remaining_samples -= PREAMBLE_LENGTH/2 - 1 ;                    
                }

                level4display = level4display * .6 + v * .4 ;
                subsample-- ;
                if( subsample <= 0 ) {
                    emit powerLevel(level4display);
                    subsample = SUBSAMPLERATE ;
                    //qDebug() << v << threshold;
                }

            } else {
                return(remaining_samples);
            }
            break ;


        case sFrameStart:

            sb = new SampleBlock( start, remaining_samples );
            // Check that last samples have something inside by fast autocorrelation test
            if( remaining_samples >= FFT_SIZE/2) {
                memset( (void *)fftin, 0, FFT_SIZE*sizeof(TYPECPX));
                TYPECPX* end = start ;
                end += remaining_samples - PREAMBLE_LENGTH ;
                ac( end, PREAMBLE_LENGTH );
                //
                double root = modulus(0);
                A = 0 ;
                N = 0 ;
                for( int i=1 ; i < FFT_SIZE/2 ; i++) {
                    A += modulus(i);
                    N++ ;
                }
                A = A / N ;
                v = 20*log10f( A / root );

                if( v < threshold ) {
                    next_state = sFrameEnds ;
                    sb->markAsLastBlock(); // this is the end of the sequence
                }
                level4display = level4display * .6 + v * .4 ;
                subsample-- ;
                if( subsample <= 0 ) {
                    emit powerLevel(v);
                    subsample = SUBSAMPLERATE ;
                }
            }
            // put block in queue
            queueSampleCount+= remaining_samples ;            
            queue.enqueue( sb );
            remaining_samples = 0 ;
            emit newDataDetected();
            break ;

        case sFrameEnds:
            next_state = sSearchFrame ;
            break ;

        }
    }
    return(0) ;
}


int FrameProcessor::processDataRMS( TYPECPX* IQsamples, int L , int sampleRate ) {
    float v ;
    TYPECPX* start = IQsamples ;
    int remaining_samples = L ;
    SampleBlock *sb ;

    static int subsample = SUBSAMPLERATE ;
    static float level4display = 0 ;

    //qDebug() << "FrameProcessor::newData()" << L ;

    while( remaining_samples > 0 ) {
        if( next_state != m_state ) {
            if( FRAME_DEBUG )  qDebug() << " state transition from " << stateToS(m_state) << " to " << stateToS(next_state);
            m_state = next_state ;
            emit newState( stateToS(m_state));
        }

        switch( m_state ) {
        case sInit:
            noise_floor = -1 ;
            rms_power = 0 ;
            next_state = sMeasureNoise ;
            samples_for_powerestimation = 2*sampleRate ; // use 2 seconds of signal
            continue ;

            // estimate noise level
        case sMeasureNoise:
            if( m_update_noise_request > 0 ) {
                m_update_noise_request = 0 ;
            }
            if( remaining_samples >= PREAMBLE_LENGTH ) {
                samples_for_powerestimation -= PREAMBLE_LENGTH ;
                if( samples_for_powerestimation > 0 ) {
                    v = rmsp( start, PREAMBLE_LENGTH ) ;
                    level4display = level4display * .6 + v * .4 ;
                    subsample-- ;
                    if( subsample <= 0 ) {
                        emit powerLevel(v);
                        subsample = SUBSAMPLERATE ;
                    }
                    rms_power += v ;
                    start += PREAMBLE_LENGTH ;
                    remaining_samples -= PREAMBLE_LENGTH ;

                } else {
                    rms_power /= (2*sampleRate/PREAMBLE_LENGTH) ; // how many times we accumulated power
                    noise_floor = rms_power ;
                    threshold = noise_floor + detection_threshold ;
                    next_state = sSearchFrame ;

                    if( FRAME_DEBUG )  qDebug() << " noise level is void SLOT_DeleteWriter( FrameToDecoder *writer );set to " << noise_floor ;
                    if( FRAME_DEBUG )  qDebug() << " threshold is " << threshold ;

                    emit newSNRThreshold(threshold);
                }
            } else {
                return(remaining_samples);
            }
            break ;

        case sSearchFrame:
            if( remaining_samples >= PREAMBLE_LENGTH ) {
                v = rmsp( start, PREAMBLE_LENGTH ) ;
                level4display = level4display * .6 + v * .4 ;
                subsample-- ;
                if( subsample <= 0 ) {
                    emit powerLevel(v);
                    subsample = SUBSAMPLERATE ;
                }
                if( v >= threshold ) {
                    // considered block contains enough energy (RMS(block) > (noise level + threshold)
                    // start to queue samples

                    if( (L-remaining_samples) > 0 ) {
                        TYPECPX* cstart = start ;
                        cstart -= (L-remaining_samples) ;
                        SampleBlock *sb = new SampleBlock( cstart, (L-remaining_samples) + PREAMBLE_LENGTH );
                        queue.enqueue( sb );
                        queueSampleCount = (L-remaining_samples) + PREAMBLE_LENGTH ;

                    } else {
                        SampleBlock *sb = new SampleBlock( start, PREAMBLE_LENGTH );
                        queue.enqueue( sb );
                        queueSampleCount = PREAMBLE_LENGTH ;
                    }
                    next_state = sFrameStart ;
                    emit frameDetected( v ) ;
                    start += PREAMBLE_LENGTH ;
                    remaining_samples -= PREAMBLE_LENGTH ;

                    emit newDataDetected();

                } else {
                    start += PREAMBLE_LENGTH/2 ;
                    remaining_samples -= PREAMBLE_LENGTH/2 ;
                    // if there was a pending update request, we process it now
                    if( m_update_noise_request > 0 ) {
                        next_state = sMeasureNoise ;
                        m_update_noise_request = 0 ;
                        samples_for_powerestimation = 2*sampleRate ; // use 2 seconds of signal
                    }
                }
            } else {
                return(remaining_samples);
            }
            break ;


        case sFrameStart:
            // test queue length
            //length = queueSampleCount / DEMODULATOR_SAMPLERATE ; // int number of seconds in queue

            /*****
             * disabled: when sat is transmitting scientific payload, the frames are coming one after one in a long transmission
             * cutting after MAXSECONDS_IN_QUEUE will discard data...
            if( length > MAXSECONDS_IN_QUEUE ) {
                // not normal ? maybe level is wrong
                flushQueue(queueSampleCount);
                next_state = sMeasureNoise ;
                return(0) ;
            }
            */
            // put block in queue
            queueSampleCount+= remaining_samples ;
            sb = new SampleBlock( start, remaining_samples );
            queue.enqueue( sb );

            v = rmsp( start, remaining_samples ) ;
            emit powerLevel(v);
            remaining_samples = 0 ;
            // check what is the power level at the end of the block
            if( queueSampleCount > MINFRAME_LENGTH ) {
                start = IQsamples + L - PREAMBLE_LENGTH ;
                v = rmsp( start, PREAMBLE_LENGTH ) ;
                if( v < threshold ) {
                    sb->markAsLastBlock(); // this is the end of the sequence
                    next_state = sFrameEnds ;
                }
            }

            emit newDataDetected();

            break ;

        case sFrameEnds:
            queueSampleCount = 0 ;
            next_state = sSearchFrame ;
            // if there was a pending update request, we process it now
            if( m_update_noise_request > 0 ) {
                next_state = sMeasureNoise ;
                m_update_noise_request = 0 ;
                samples_for_powerestimation = 2*sampleRate ; // use 2 seconds of signal
            }

            break ;

        }
    }
    return(0) ;
}


void FrameProcessor::SLOT_newDataReady() {

    while( !queue.isEmpty() ) {
        SampleBlock *b = queue.dequeue() ;
        zmqs->addBlock(b);
    }
}


float FrameProcessor::rmsp( TYPECPX *samples, int L ) {
    float res = 0 ;
    int k = 0 ;
    for( k=0 ; k < L ; k++ ) {
        res += samples[k].re*samples[k].re + samples[k].im*samples[k].im ;
    }
    res = 20*log10(sqrtf( 1.0/L * res ));
    return(res);
}


