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
#include <QDebug>
#include <QApplication>
#include <QStyleFactory>
#include <QSplashScreen>
#include <QMessageBox>
#include <QStandardPaths>

#include "common/constants.h"
#include "mainwindow.h"
#include "core/controller.h"

#include "hardware/rxhardwareselector.h"
#include "common/QLogger.h"

#include "httpserver/httplistener.h"
#include "webinterface/webservice.h"

#define SPLASH_NAME ":/logoSmall"

class I : public QThread
{
public:
    static void sleep(unsigned int secs) {
        for( unsigned int z=0 ; z < secs*10 ; z ++ ) {
            qApp->processEvents();
            QThread::msleep(100);
        }
    }
};


int main(int argc, char *argv[])
{

    RxDevice *radio = NULL ;

    QApplication a(argc, argv);
    QApplication::setStyle(QStyleFactory::create("plastique"));

    QPixmap pixmap( SPLASH_NAME );
    QSplashScreen splash(pixmap);
    splash.show();
    a.processEvents();
    splash.showMessage( a.translate( "QObject", "Loading..."),  Qt::AlignLeft | Qt::AlignTop, Qt::white);
    I::sleep(1);

    // load configuration file
    GlobalConfig& global = GlobalConfig::getInstance() ;
    (void) global ;

    QLogger::QLoggerManager *manager = QLogger::QLoggerManager::getInstance();
    manager->addDestination( LOGGER_FILENAME, QStringList( LOGGER_NAME ),  QLogger::TraceLevel);
    QLogger::QLog_Trace( LOGGER_NAME, "------------------------------------------------------------------------------" );
    QLogger::QLog_Trace( LOGGER_NAME, "Build date : " + QString(BUILD_DATE));
    QLogger::QLog_Trace( LOGGER_NAME, "Starting" );


    Controller& control = Controller::getInstance() ;

    QMessageBox msgBox;
    RxHardwareSelector *rxs = new RxHardwareSelector();
    radio = rxs->getReceiver() ;
    if( radio == NULL ) {
        QLogger::QLog_Error( LOGGER_NAME, "No SDR device connected, cannot continue.");
        msgBox.setWindowTitle( VER_PRODUCTNAME_STR );
        msgBox.setText("ERROR:  No SDR device detected !");
        msgBox.setStandardButtons(QMessageBox::Yes );
        msgBox.exec() ;
        return(-1);
    }
    // start web server
    if( !QDir( QStandardPaths::writableLocation( QStandardPaths::HomeLocation) + "/pictalk").exists() ) {
        QDir().mkpath(QStandardPaths::writableLocation( QStandardPaths::HomeLocation) + "/pictalk") ;
    }

    QSettings settings( QStandardPaths::writableLocation( QStandardPaths::HomeLocation) + "/pictalk/" + QString(CONFIG_FILENAME), QSettings::IniFormat);
    settings.beginGroup("WebServer");
    WebService *ws = new WebService(&a);
    HttpListener* webserver = new HttpListener( &settings, ws, &a);
    control.setWebservice( ws );
    (void) webserver ;

    control.setRadio( radio );
    control.start();

    MainWindow *w = new MainWindow();
    w->connect( ws, SIGNAL(mtuneTo(qint64)), w, SLOT(SLOT_ExternalSetFrequency(qint64)), Qt::QueuedConnection );
    w->setRadio( radio );
    splash.finish(w);
    w->show();

    return a.exec();
}
