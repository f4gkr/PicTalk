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
#include "mainwindow.h"
#include <QWidget>
#include <QPushButton>
#include <QDesktopWidget>
#include <QApplication>
#include <QLabel>
#include <QPalette>
#include <QMessageBox>
#include <stdint.h>

#include "core/controller.h"
#include "hardware/gpdsd.h"
#include "common/QLogger.h"
#include "common/constants.h"



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    radio = NULL ;
    setAttribute(Qt::WA_DeleteOnClose);

    QWidget *center_widget = new QWidget;
    QVBoxLayout *vlayout = new QVBoxLayout;
    center_widget->setLayout(vlayout);
    vlayout->setContentsMargins( 1,1,1,1);
    vlayout->setAlignment( Qt::AlignTop );

    // Create top band
    QWidget *top_band = new QWidget;
    QHBoxLayout *tb_layout = new QHBoxLayout;
    tb_layout->setAlignment( Qt::AlignLeft );
    tb_layout->setContentsMargins( 1,1,1,1);

    mainFDisplay = new CFreqCtrl();
    mainFDisplay->setMinimumWidth(500);
    tb_layout->addWidget(mainFDisplay);
    top_band->setMaximumHeight(40);
    top_band->setLayout(tb_layout);
    vlayout->addWidget( top_band );

    // center band
    QWidget* center_band = new QWidget();
    QHBoxLayout *cb_layout = new QHBoxLayout;
    center_band->setLayout(cb_layout);
    cb_layout->setAlignment( Qt::AlignLeft | Qt::AlignTop );
    cb_layout->setContentsMargins( 1,1,1,1);

    // center right
    QWidget *cr_widget = new QWidget;
    cr_widget->setMaximumWidth(250);
    crlayout = new QVBoxLayout;
    cr_widget->setLayout(crlayout);
    crlayout->setContentsMargins( 1,1,1,1);
    crlayout->setAlignment(Qt::AlignLeft | Qt::AlignTop  );

    //
    QPushButton *startButton = new QPushButton("START");
    //startButton->setMaximumWidth(50);
    startButton->setToolTip(tr("Start SDR"));
    crlayout->addWidget( startButton );

    QPushButton *stopButton = new QPushButton("STOP");
    //stopButton->setMaximumWidth(50);
    stopButton->setToolTip(tr("Stop SDR"));
    crlayout->addWidget( stopButton );

    // Radio Control
    gain_rx = new gkDial(4,tr("RF Gain"));
    gain_rx->setScale(0,40);
    gain_rx->setValue(10);
    crlayout->addWidget(gain_rx);
    cb_layout->addWidget( cr_widget);



    // Waterfall display
    GlobalConfig& cnf = GlobalConfig::getInstance() ;
    tabWidget = new QTabWidget();
    tabWidget->setContentsMargins(0,0,0,0);
    tabWidget->setTabPosition( QTabWidget::South );

    // add wf in tab
    wf = new CPlotter();
    wf->setMinimumWidth(450);
    wf->setSampleRate( 1e6 );
    wf->setCenterFreq( cnf.cRX_FREQUENCY );
    wf->setDemodCenterFreq( cnf.cRX_FREQUENCY );
    wf->setSpanFreq( DEMODULATOR_SAMPLERATE*4 );

    wf->setHiLowCutFrequencies( -DEMODULATOR_SAMPLERATE/2,DEMODULATOR_SAMPLERATE/2);
    wf->setMaxDB(-50.0);
    wf->setMinDB(-130);
    wf->setFftFill(true);
    tabWidget->addTab( wf,tr("Waterfall"));


    // add decoder ui
    pythonText = new QTextEdit();
    QFont font = pythonText->font();
    font.setPointSize(11);
    pythonText->setFont( font );
    pythonText->setReadOnly(true);

    tabWidget->addTab( pythonText, tr("Decoder"));
    cb_layout->addWidget( tabWidget );


    // center left
    QWidget *cl_widget = new QWidget;
    cl_widget->setMaximumWidth(170);
    QVBoxLayout *cllayout = new QVBoxLayout;
    cl_widget->setLayout(cllayout);
    cllayout->setContentsMargins( 1,1,1,1);
    cllayout->setAlignment(Qt::AlignLeft | Qt::AlignTop  );

    zuluDisplay = new QLCDNumber(11);
    zuluDisplay->setSegmentStyle(QLCDNumber::Flat);
    zuluDisplay->display( "00:00:00.0" );
    zuluDisplay->setToolTip(tr("UTC Time"));
    QPalette zpalette = zuluDisplay->palette() ;
    zpalette.setColor( zpalette.WindowText, QColor(85, 85, 255)) ;
    zpalette.setColor( zpalette.Background, QColor(0, 170, 255));
    zpalette.setColor (zpalette.Dark, QColor(0, 255, 0));
    zuluDisplay->setPalette(zpalette) ;
    cllayout->addWidget(zuluDisplay);

    fft_update_rate = new gkDial(4,tr("FFT Rate"));
    fft_update_rate->setScale(1,FFTRATE_MAX);
    fft_update_rate->setValue(FFTRATE_MAX);
    cllayout->addWidget(fft_update_rate);


    detection_threshold= new gkDial(4,tr("Threshold"));
    threshold_level = (float)DEFAULT_AC_THRESHOLD ;
#ifdef USE_CORRELATOR
    detection_threshold->setScale(1,100);
    detection_threshold->setValue(DEFAULT_AC_THRESHOLD);
#else
    detection_threshold->setScale(2,20);
    detection_threshold->setValue(DEFAULT_DETECTION_THRESHOLD);
#endif
    cllayout->addWidget(detection_threshold);

#ifndef USE_CORRELATOR
    levelWidget = new IndicatorWidget( "Level", -100, -30, "SNR - dBc");
    levelWidget->setMinimumWidth(150);
    levelWidget->setMaximumHeight(150);
    cllayout->addWidget( levelWidget);
#endif

    cllayout->addWidget( new QLabel(tr("Detector state:")));

    QWidget *statusW = new QWidget();
    QHBoxLayout *hbl = new QHBoxLayout();
    statusW->setLayout( hbl );
    decoding = new LedIndicator();
    hbl->addWidget( decoding );

    decoderStatus = new QLineEdit();
    decoderStatus->setToolTip(tr("Current status of frame detector"));
    decoderStatus->setMaxLength(15);
    hbl->addWidget( decoderStatus );

    cllayout->addWidget( statusW );


    cb_layout->addWidget( cl_widget);


    vlayout->addWidget( center_band );

    // detection levels etc
    QWidget *plotWidget = new QWidget();
    QVBoxLayout *pwlayout = new QVBoxLayout();
    pwlayout->setContentsMargins( 0, 0, 0, 0 );
    plotWidget->setLayout( pwlayout );

    levelplot = new QCustomPlot();
    levelplot->addGraph();
    levelplot->graph(0)->setPen(QPen(Qt::blue)); // line color blue for first graph
    levelplot->xAxis->setLabel(tr("Received Frame"));
    levelplot->xAxis->setRange(0, 1);
#ifdef USE_CORRELATOR
    levelplot->yAxis->setLabel("Corr. Level");
    levelplot->yAxis->setRange(0, 100);
#else
    levelplot->yAxis->setLabel("RMS Level");
    levelplot->yAxis->setRange(-70, -30);
#endif
    levelplot->setMinimumHeight(150);
    levelplot->addGraph();
    levelplot->graph(1)->setPen(QPen(Qt::red));
    min_level = 0 ;
    max_level = -100 ;
    msg_count = 0 ;

    pwlayout->addWidget( levelplot );
    vlayout->addWidget( plotWidget );

    QAction *quit = new QAction( tr("&Quit"), this);
    QMenu *file;
    file = menuBar()->addMenu( tr("&File"));
    file->addAction(quit);
    connect(quit, SIGNAL(triggered()), this, SLOT(endProgram()));


    setCentralWidget(center_widget);
    // resize
    //QDesktopWidget *desktop = QApplication::desktop();
    //resize( desktop->availableGeometry(this).size() * .7 );

    connect( startButton, SIGNAL(pressed()), this, SLOT(SLOT_startPressed()));
    connect( stopButton, SIGNAL(pressed()), this, SLOT(SLOT_stopPressed()));

    connect( mainFDisplay, SIGNAL(newFrequency(qint64)),
             this, SLOT(SLOT_userTunesFreqWidget(qint64)) );
    connect( fft_update_rate, SIGNAL(valueChanged(int)), this, SLOT(SLOT_userChangesFFTRate(int)));
    connect( gain_rx, SIGNAL(valueChanged(int)), this, SLOT(SLOT_setRxGain(int)));
    connect( detection_threshold, SIGNAL(valueChanged(int)), this, SLOT(SLOT_setDetectionThreshold(int)));

    connect( wf, SIGNAL(newDemodFreq(qint64, qint64)), this, SLOT(SLOT_NewDemodFreq(qint64, qint64)));


    GPSD& gpsd= GPSD::getInstance() ;
    connect( &gpsd, SIGNAL(hasError(int)), this, SLOT(SLOT_gpsdAsError(int)), Qt::QueuedConnection );
    gpsd.start();

    connect( &gpsd, SIGNAL(hasGpsFix(double,double)), this,
             SLOT(SLOT_hasGpsFix(double,double)), Qt::QueuedConnection );
    connect( &gpsd, SIGNAL(hasGpsTime(int,int,int,int,int,int,int)), this,
             SLOT(SLOT_hasGpsTime(int,int,int,int,int,int,int)),Qt::QueuedConnection);

    dec = NULL ;

}

void MainWindow::setRadio(RxDevice *device ) {
    radio = device ;

    setWindowTitle( "PicSat using : " + QString::fromLocal8Bit( device->getHardwareName() ));

    mainFDisplay->setup( 11,
                         radio->getMin_HWRx_CenterFreq() ,
                         radio->getMax_HWRx_CenterFreq(),
                         10, UNITS_MHZ );

    wf->setSampleRate( radio->getRxSampleRate() );
    mainFDisplay->resetToFrequency( radio->getRxCenterFreq() );

    Controller& ctrl = Controller::getInstance() ;
    connect( &ctrl, SIGNAL(newSpectrumAvailable(int, TuningPolicy*)),  this,
             SLOT(SLOT_newSpectrum(int, TuningPolicy*)), Qt::QueuedConnection );
    connect( &ctrl, SIGNAL(powerLevel(float)), this, SLOT(SLOT_powerLevel(float)), Qt::QueuedConnection );
    connect( &ctrl, SIGNAL(newState(QString)), this, SLOT(SLOT_frameDetectorStateChanged(QString)), Qt::QueuedConnection );
    connect( &ctrl, SIGNAL(newSNRThreshold(float)), this, SLOT(SLOT_NewSNRThreshold(float)), Qt::QueuedConnection );

    // adapt GUI to SDR
    if( device->deviceHasSingleGainStage() ) {
        gain_rx->setScale( device->getMinGain() ,device->getMaxGain() );
        gain_rx->setValue( device->getRxGain()  );
    } else {
        gain_rx->setVisible(false);
        if( radio->getDisplayWidget() != NULL ) {
            crlayout->addWidget(  radio->getDisplayWidget() );
        }
    }

    GlobalConfig& gc = GlobalConfig::getInstance() ;
    mainFDisplay->setFrequency(gc.cRX_FREQUENCY);

    // configure Python decoder interface
    zmqConsole = new ZmqPython();

    connect( zmqConsole, SIGNAL(message(QString)), this, SLOT(PythonMessage(QString)),
             Qt::QueuedConnection );
    connect( zmqConsole, SIGNAL(newFrame(QString)), this, SLOT(PythonFrame(QString)),
             Qt::QueuedConnection );
    connect( zmqConsole, SIGNAL(absTune(QString)), this, SLOT(PythonAbsTune(QString)),
             Qt::QueuedConnection );
    connect( zmqConsole, SIGNAL(relTune(QString)), this, SLOT(PythonRelTune(QString)),
             Qt::QueuedConnection );

    dec = new PythonDecoder();
    dec->start(); // start the decoder
    zmqConsole->start();
}

void MainWindow::PythonMessage( QString msg ) {
    pythonText->moveCursor (QTextCursor::End);
    pythonText->insertPlainText (msg);
    pythonText->moveCursor (QTextCursor::End);
}

void MainWindow::PythonFrame( QString frame ) {
    QFile file( QApplication::applicationDirPath() + "/PicTalkFrames.txt");
    if (!file.open(QIODevice::Append | QIODevice::Text))
        return;

    QTextStream out(&file);
    out << frame << "\n" ;
    file.close();

    pythonText->moveCursor (QTextCursor::End);
    pythonText->insertPlainText ( "Frame: {\n" + frame + "\n}\n" );
    pythonText->moveCursor (QTextCursor::End);
}

void MainWindow::PythonAbsTune( QString freqabs ) {
    Controller& ctrl = Controller::getInstance() ;
    ctrl.setRxCenterFrequency( freqabs.toDouble() );
}

void MainWindow::PythonRelTune( QString reltune ) {

    Controller& ctrl = Controller::getInstance() ;
    TuningPolicy *tp = ctrl.getFrequency() ;
    if( tp == NULL ) {
        return ;
    }
    GlobalConfig& gc = GlobalConfig::getInstance() ;
    qint64 rxF = gc.getReceivedFrequency( tp ) ;

    rxF += reltune.toInt() ;
    ctrl.setRxCenterFrequency( rxF );
    mainFDisplay->blockSignals(true);
    mainFDisplay->resetToFrequency( rxF );

    qint64 offset = tp->channelizer_offset ;
    wf->blockSignals(true);
    wf->setCenterFreq( tp->rx_hardware_frequency +reltune.toInt()  );
    wf->setDemodCenterFreq( tp->rx_hardware_frequency +reltune.toInt() + offset );

    mainFDisplay->blockSignals(false);
    wf->blockSignals(false);

}

void MainWindow::SLOT_userTunesFreqWidget(qint64 newFrequency) {
    Controller& ctrl = Controller::getInstance() ;
    ctrl.setRxCenterFrequency( newFrequency );
}

void MainWindow::SLOT_NewDemodFreq(qint64 freq, qint64 delta){
    Controller& ctrl = Controller::getInstance() ;
    ctrl.setRxCenterFrequency( freq+delta );

    mainFDisplay->blockSignals(true);
    mainFDisplay->resetToFrequency( freq+delta);
    mainFDisplay->blockSignals(false);
}

// start SDR pressed
void MainWindow::SLOT_startPressed() {
    Controller& ctrl = Controller::getInstance() ;
    if( radio == NULL )
        return ;

    if( ctrl.isAcquiring()  )
        return ;

    received_frame = msg_count = 0 ;
    levelplot->graph(0)->clearData();
    ctrl.startAcquisition();
}

void MainWindow::SLOT_stopPressed() {
    Controller& ctrl = Controller::getInstance() ;
    if( radio == NULL )
        return ;
    if( !ctrl.isAcquiring() ) {
        return ;
    }
    ctrl.stopAcquisition();
}

void MainWindow::SLOT_userChangesFFTRate(int value) {
    Controller& ctrl = Controller::getInstance() ;
    ctrl.setSpectrumInterleaveValue(value);
}


void MainWindow::SLOT_frameDetectorStateChanged( QString stateName ) {
    decoderStatus->setText( stateName );
    if( stateName == "sFrameStart") {
        decoding->setState(true);
    } else {
        decoding->setState(false);
    }
}

void MainWindow::SLOT_newSpectrum(int len , TuningPolicy *tp ) {
    double power_dB[len] ;

    Controller& ctrl = Controller::getInstance() ;
    ctrl.getSpectrum( power_dB );

    //qDebug() << "SLOT_newSpectrum.channelizer_offset=" << tp->channelizer_offset/1e3 << " Kz." ;
    //qDebug() << "SLOT_newSpectrum.rx_hardware_frequency=" << tp->rx_hardware_frequency/1e3 << " KHz." ;

    wf->blockSignals(true);
    wf->setSampleRate( radio->getRxSampleRate() );
    wf->setCenterFreq( tp->rx_hardware_frequency );
    wf->setDemodCenterFreq( tp->rx_hardware_frequency + tp->channelizer_offset );
    wf->setNewFttData( power_dB, len );
    wf->blockSignals(false);

}

void MainWindow::SLOT_ExternalSetFrequency(qint64 frequency) {
    mainFDisplay->blockSignals(true);
    mainFDisplay->resetToFrequency( frequency );
    mainFDisplay->blockSignals(false);
}

MainWindow::~MainWindow()
{
    SLOT_stopPressed();

    GPSD& gpsd= GPSD::getInstance() ;
    gpsd.shutdown();

    Controller& ctrl = Controller::getInstance() ;
    ctrl.close();

}

void MainWindow::SLOT_powerLevel( float level )  {


    min_level = qMin( min_level, level );
    min_level = qMin( min_level, threshold_level );

    max_level = qMax( max_level, level) ;
    max_level = qMax( max_level, threshold_level );
    if( fabs(max_level-min_level) < 5 ) {
        min_level -= 2.5 ;
        max_level += 2.5 ;
    }


    //qDebug() << "level=" << level << min_level << max_level ;

    levelplot->graph(0)->addData( msg_count/10.0, level);
    levelplot->graph(1)->addData( msg_count/10.0, threshold_level);
    levelplot->xAxis->setRange( msg_count/10.0,60,Qt::AlignRight );
    levelplot->yAxis->setRange( min_level, max_level);
    levelplot->replot();
    msg_count++ ;
}


// called by GPSD when we have a fix - not used
void MainWindow::SLOT_hasGpsFix(double latitude, double longitude ) {
    Q_UNUSED(latitude);
    Q_UNUSED(longitude);
}

void MainWindow::SLOT_hasGpsTime(int year, int month, int day, int hour, int min, int sec, int msec) {
    Q_UNUSED(year);
    Q_UNUSED(month);
    Q_UNUSED(day);
    Q_UNUSED(msec);
    QString zuluTime = QString("%1").arg(hour, 2, 10, QChar('0')) + ":" +
            QString("%1").arg(min, 2, 10, QChar('0')) + ":" +
            QString("%1").arg(sec, 2, 10, QChar('0')) + "." + QString::number(msec);


    zuluDisplay->display( zuluTime );
}

void MainWindow::SLOT_setRxGain(int g) {
    if( radio == NULL )
        return ;

    radio->setRxGain( g );
    // Tell controller that noise level must be re-estimated
    Controller& ctrl = Controller::getInstance() ;
    ctrl.doNoiseEstimation();
}

void MainWindow::SLOT_setDetectionThreshold(int level) {
    Controller& ctrl = Controller::getInstance() ;
    ctrl.setDetectionThreshold(level);
    threshold_level = (float)level;
}

void MainWindow::SLOT_NewSNRThreshold( float value ) {
#ifdef USE_CORRELATOR
    Q_UNUSED(value);
#else
    levelWidget->setValue( value);
#endif
}

void MainWindow::SLOT_gpsdAsError( int code ) {
    GPSD& gpsd= GPSD::getInstance() ;
    gpsd.processError(code);
}


void MainWindow::endProgram() {
    radio->stopAcquisition() ;
    GPSD& gpsd= GPSD::getInstance() ;
    gpsd.shutdown();
    Controller& ctrl = Controller::getInstance() ;
    ctrl.close();
    exit(0);
}
