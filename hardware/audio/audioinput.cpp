//==========================================================================================
// + + +   This Software is released under the "Simplified BSD License"  + + +
// Copyright 2014 F4GKR Sylvain AZARIAN . All rights reserved.
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
#include "audioinput.h"
#include <QDebug>



AudioInput::AudioInput(QString inputName, int desired_sr )
{
    closing = false ;
    fifo = new SampleFifo();
    m_samplingRate = 0 ;
    xn_1.im = xn_1.re = 0 ;
    yn_1.im = yn_1.re = 0 ;

    format.setSampleRate(desired_sr);
    format.setChannelCount(2);
    format.setSampleSize(16);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::SignedInt);
    inputDevice = NULL ;

    wr_pos = 0 ;
    OutBuf = NULL ;
    buf_len= 8*1024 ;

    foreach (const QAudioDeviceInfo &deviceInfo, QAudioDeviceInfo::availableDevices(QAudio::AudioInput)) {
        QString deviceName = deviceInfo.deviceName();

        qDebug() << "Available (detected) audio interface :" << deviceName ;

        if( deviceName.indexOf( inputName ) > 0 ) {
            qDebug() << "Selected device name: " << deviceInfo.deviceName();
            QList<int> rates = deviceInfo.supportedSampleRates() ;
            for( int i=0 ; i < rates.size() ; i++ ) {
                int ssr = rates.at(i);
                qDebug() << "supported sample rate :" << i << ssr ;
            }

            if( deviceInfo.isFormatSupported( format )) {
                qDebug() << "using " << deviceName ;
                audio = new QAudioInput( deviceInfo, format, this );
                audio->setVolume(1);
                connect( audio, SIGNAL(stateChanged(QAudio::State)), this, SLOT(stateChanged(QAudio::State)));
                m_samplingRate = audio->format().sampleRate() ;
                qDebug() << "sample rate set to " << m_samplingRate ;
                return ;
            }
        }

    }

    m_samplingRate = -1 ;
}

bool AudioInput::startAudio() {
    if( inputDevice != NULL ) {
        return(true);
    }

    QIODevice::open(QIODevice::ReadWrite);
    audio->start(this);

    qDebug() << "AudioInput::startAudio()" ;
    return( true );
}

void AudioInput::stopAudio() {
    audio->stop();
}


void AudioInput::stateChanged(QAudio::State state) {
    qDebug() << "AudioInput::stateChanged" ;
    switch( state ) {
    case QAudio::ActiveState:
        qDebug() << "ActiveState" ;
        m_samplingRate = audio->format().sampleRate() ;
        qDebug() << "sample rate set to " << m_samplingRate ;
        break ;
    case QAudio::SuspendedState: qDebug() << "SuspendedState" ; break ;
    case QAudio::StoppedState: qDebug() << "StoppedState" ; break ;
    case QAudio::IdleState: qDebug() << "IdleState" ; break ;
    default: qDebug() << "other state" ; break ;

    }
}

void AudioInput::setIQCorrectionFactors( TYPEREAL gainI, TYPEREAL gainQ,
                                         TYPEREAL crossGain ) {
    AmPhAAAA = gainI ;
    AmPhDDDD = gainQ ;
    AmPhCCCC = crossGain ;

}

void AudioInput::getIQCorrectionFactors( TYPEREAL *gainI,
                                         TYPEREAL *gainQ,
                                         TYPEREAL *crossGain ) {
    *gainI = AmPhAAAA ;
    *gainQ = AmPhDDDD ;
    *crossGain = AmPhCCCC ;
}

qint64 AudioInput::readData(char* data, qint64 maxLen)
{
    Q_UNUSED(data);
    Q_UNUSED(maxLen);
    return 0;
}

#define ALPHA_DC (0.999)
#define USE_DC_REMOVAL
qint64 AudioInput::writeData(const char *data, qint64 len)
{
    TYPECPX tmp ;
    float I,Q ;
    int16_t *rawsamples = (int16_t *)data ;
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
            //qDebug() << "pushing " << buf_len << " audio samples" ;
            if( fifo->EnqueueData( (void *)OutBuf, buf_len, 0, NULL ) < 0 ) {
                qDebug() << "AudioInput::writeData() problem - queue full ???" ;
            }
            OutBuf = (TYPECPX*)malloc( buf_len * sizeof(TYPECPX) );
            wr_pos = 0 ;
        }
    }

    return(len) ;
}
