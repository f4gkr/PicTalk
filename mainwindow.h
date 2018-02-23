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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLCDNumber>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QTextEdit>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QUrl>
#include <QNetworkReply>

#include "common/tuningpolicy.h"
#include "ui/plotter.h"
#include "hardware/rxdevice.h"
#include "ui/freqctrl.h"
#include "ui/spectrumplot.h"
#include "ui/indicatorwidget.h"
#include "ui/qcustomplot.h"
#include "ui/gkdial.h"
#include "ui/gkpushbutton.h"
#include "ui/ledindicator.h"
#include "common/constants.h"
#include "dsp/pythondecoder.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void setRadio( RxDevice* device );

public slots:
    void SLOT_powerLevel( float level ) ;
    void SLOT_timer();
    void SLOT_ExternalSetFrequency( qint64 frequency );

private slots:
    void SLOT_userChangesFFTRate(int value);

    void SLOT_userTunesFreqWidget(qint64 newFrequency);
    void SLOT_NewDemodFreq(qint64 freq, qint64 delta);
    void SLOT_newSpectrum(int len  , TuningPolicy *tp);
    void SLOT_frameDetectorStateChanged( QString stateName );
    void SLOT_startPressed();
    void SLOT_stopPressed();

    void SLOT_setRxGain(int) ;
    void SLOT_setDetectionThreshold(int);
    void SLOT_NewSNRThreshold( float value );
    void endProgram();

    void PythonMessage( QString msg );
    void PythonFrame( QString frame );
    void PythonAbsTune( QString freqabs );
    void PythonRelTune( QString reltune );

    void sidsSubmitted( QNetworkReply *rep ) ;

private:
    int received_frame ;
    int accepted_frames;
    int msg_count ;
    QTabWidget *tabWidget ;
    CPlotter *wf ;
    CFreqCtrl *mainFDisplay ;
    gkDial *fft_update_rate ;
    gkDial *gain_rx ;
    QVBoxLayout *crlayout  ;
    gkDial *detection_threshold ;
    QLCDNumber *zuluDisplay ;
    QLCDNumber *reportedFrames ;
    QTimer *timer ;

    LedIndicator* decoding ;
    QLineEdit* decoderStatus ;
    RxDevice* radio ;

    PythonDecoder* dec ;
    ZmqPython* zmqConsole ;
    QTextEdit* pythonText ;

#ifndef USE_CORRELATOR
    IndicatorWidget *levelWidget ;
#endif
    QCustomPlot *levelplot ;
    float threshold_level ;
    float min_level ;
    float max_level ;

    QNetworkAccessManager *networkManager ;
    void uploadFrame( QString frame );
};

#endif // MAINWINDOW_H
