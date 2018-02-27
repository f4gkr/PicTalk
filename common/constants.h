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
#ifndef CONSTANTS_H
#define CONSTANTS_H
#include <QObject>
#include "datatypes.h"
#include "tuningpolicy.h"

#define DATAFOLDER "PicTalk-data"
#define LOGGER_NAME  "PicTalk"
#define LOGGER_FILENAME "pictalk.log"
#define VER_PRODUCTNAME_STR "pictalk"
/**
  In current version the frame is (SymbolRate x Oversample ratio) wide
  This is the DEMODULATOR_SAMPLERATE constant

  For example : Symbol Rate is 9600 , Oversample Ratio = 4
                we have a sampling rate of 38400 Hz
                DEMODULATOR_SAMPLERATE = 38400


  To avoid RX Dc at center and enable energy detction (power based or autocorrelation), we
  shift the subband of interest by FRAME_OFFSET_LOW Hz + DEMODULATOR_SAMPLERATE/2
        example : Offset set at 10 000 Hz
        sub-band is centered at (FRAME_OFFSET_LOW Hz + DEMODULATOR_SAMPLERATE/2)=10 000 + 38400/2
                                                                             = 29200 Hz

  Finally:
    if frequency of interst is f0 :
    - receiver is tuned to f0 -> we have a DC residual
    - we extract a subwindow (using Overalp/save channelizer) of (SymbolRate x Oversample ratio) Hz
    - This subband is not centered at f0, but at f0 - FFAME_OFFSET_LOW Hz + DEMODULATOR_SAMPLERATE/2

  example :
    - want to extract signal Symbol Rate is 9600 , Oversample Ratio = 4, centered at f = 436.500 MHz
    - DEMODULATOR_SAMPLERATE = 38400
    - (FRAME_OFFSET_LOW Hz + DEMODULATOR_SAMPLERATE/2)= 29200 Hz
    - Rx is tuned to f = 436.500 MHz - 29200 Hz ;
    - Channelizer (ddc) is centerd at +(FRAME_OFFSET_LOW Hz + DEMODULATOR_SAMPLERATE/2) = 29200
    - Received bandwidth is DEMODULATOR_SAMPLERATE = 38400


  */
#define FRAME_OFFSET_LOW 10e3

#define SYMBOL_RATE 9600 /* picsat modem bitrate */
#define OVERSAMPLE_RATIO (4)
#define DEMODULATOR_SAMPLERATE (9600*OVERSAMPLE_RATIO) /* baseband sample rate */
#define PREAMBLE_LENGTH (OVERSAMPLE_RATIO*10*8)
#define BASEBAND_BLOCK_SIZE (SYMBOL_RATE)
#define MAXSECONDS_IN_QUEUE (10)
#define MINFRAME_LENGTH (OVERSAMPLE_RATIO*1024)
#define RX_OFFSET (1e3) /* how far away from rx center we shift to avoid DC component */

#define CONFIG_FILENAME "pictalk.conf"
#define FRAMEFILE "pictalkframes.txt"
#define DEFAULT_RX_FREQUENCY (435.525*1e6)

#define FFTRATE_MAX (10)

#define USE_CORRELATOR /* undef to use power */

#define RF_NO_GAIN (-9999)

class GlobalConfig : public QObject
{
    Q_OBJECT
public:
    static GlobalConfig& getInstance()  {
        static GlobalConfig instance;
        return instance;
    }

    int ppm_error ;
    int threshold ;
    int rf_gain ;
    int fft_rate ;
    QString cFIFO_FileName ;
    qint64 cRX_FREQUENCY ;
    QString CALLSIGN ;
    QString LOCATOR ;
    QString mLatitude ;
    QString mLongitude;

    void getTuneParameters( qint64 frequencyOfInterest, TuningPolicy* tp ) ;
    qint64 getReceivedFrequency( TuningPolicy* tp );

    void saveNewThreshold( int value );
    void saveNewGain( int stage, int value );
    void saveNewFFTRate( int rate );

private:

    GlobalConfig();

    GlobalConfig(const GlobalConfig &); // hide copy constructor
    GlobalConfig& operator=(const GlobalConfig &);

};

#endif // CONSTANTS_H
