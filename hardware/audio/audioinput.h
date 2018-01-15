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
#ifndef AUDIOINPUT_H
#define AUDIOINPUT_H

#include <QObject>
#include <QtMultimedia/QAudioFormat>
#include <QtMultimedia/QAudioInput>
#include <QIODevice>
#include "common/samplefifo.h"
#include "common/datatypes.h"


class AudioInput: public QIODevice
{
    Q_OBJECT
public:
    AudioInput(QString inputName, int desired_sr  );
    ~AudioInput() {
        closing = true ;
    }

    SampleFifo *getFifo() {
        return( fifo );
    }

    void setIQCorrectionFactors( TYPEREAL gainI, TYPEREAL gainQ,
                                 TYPEREAL crossGain );
    void getIQCorrectionFactors( TYPEREAL *gainI,
                                 TYPEREAL *gainQ,
                                 TYPEREAL *crossGain );


    void clearIQCorrection() {
        AmPhAAAA = 1.0 ;
        AmPhCCCC = 0.0 ;
        AmPhDDDD = 1.0 ;
    }

    bool startAudio() ;
    void stopAudio() ;

    int getSampleRate() { return( m_samplingRate ) ; }

private slots:
    void stateChanged(QAudio::State state);

private:
    bool closing ;
    int m_samplingRate ;
    SampleFifo* fifo ;
    QAudioFormat format;
    QAudioInput *audio ;
    QIODevice *inputDevice;

    TYPECPX *OutBuf ;
    int wr_pos ;
    int buf_len ;

    TYPECPX xn_1, yn_1 ;
    TYPEREAL AmPhAAAA ;
    TYPEREAL AmPhCCCC ;
    TYPEREAL AmPhDDDD ;

    void run();
    virtual qint64 readData(char* data, qint64 maxLen);
    virtual qint64 writeData(const char* data, qint64 len);
};

#endif // AUDIOINPUT_H
