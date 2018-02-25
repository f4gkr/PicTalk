#include "parameterswindow.h"
#include <QMessageBox>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSettings>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include "common/constants.h"

ParametersWindow::ParametersWindow(QWidget *parent) : QDialog(parent)
{
    setWindowTitle( tr("Your station"));

    GlobalConfig& gc = GlobalConfig::getInstance() ;

    QPalette Pal(palette());
    Pal.setColor(QPalette::Background, QColor(0x30,0x30,0x20,0xFF));

    QVBoxLayout *layout = new QVBoxLayout( this );
    layout->setSpacing(1);
    layout->setAlignment( Qt::AlignTop );

    QWidget *top_band = new QWidget;
    top_band->setAutoFillBackground(true);

    QHBoxLayout *tb_layout = new QHBoxLayout;
    tb_layout->setAlignment( Qt::AlignLeft );
    tb_layout->setContentsMargins( 0,0,0,0);

    tb_layout->addWidget( new QLabel(tr("Settings for data upload")));

    top_band->setLayout(tb_layout);
    layout->addWidget( top_band );
    QWidget *licenseSettings = new QWidget();
    QVBoxLayout *cvlic = new QVBoxLayout();
    cvlic->setAlignment( Qt::AlignLeft );
    cvlic->setSpacing(1);
    cvlic->addWidget( new QLabel(tr("License")));

    QHBoxLayout *hl = new QHBoxLayout();
    hl->addWidget( new QLabel(tr("Your callsign:")));
    sCallSign = new QLineEdit();
    sCallSign->setText( gc.CALLSIGN );
    sCallSign->setToolTip( tr("Enter your Call Sign "));
    hl->addWidget( sCallSign );
    QWidget *wsemail = new QWidget();
    wsemail->setLayout( hl );
    cvlic->addWidget( wsemail );


    hl = new QHBoxLayout();
    hl->addWidget( new QLabel(tr("Your latitude:")));
    sLatitude = new QLineEdit();
    sLatitude->setText( gc.mLatitude );
    sLatitude->setToolTip( tr("Example : 48.604N"));
    hl->addWidget( sLatitude );
    QWidget *wsLat = new QWidget();
    wsLat->setLayout( hl );
    cvlic->addWidget( wsLat );

    hl = new QHBoxLayout();
    hl->addWidget( new QLabel(tr("Your longitude:")));
    sLongitude = new QLineEdit();
    sLongitude->setText( gc.mLongitude );
    sLongitude->setToolTip( tr("Example : 1.792E"));
    hl->addWidget( sLongitude );
    QWidget *wsLon = new QWidget();
    wsLon->setLayout( hl );
    cvlic->addWidget( wsLon );

    licenseSettings->setLayout( cvlic );
    layout->addWidget(licenseSettings);

    QWidget *btnPanel = new QWidget();
    QHBoxLayout *hbtn = new QHBoxLayout();
    hbtn->setAlignment( Qt::AlignLeft );
    hbtn->setSpacing(1);
    QPushButton *saveButton = new QPushButton(tr("Save my settings"));
    hbtn->addWidget( saveButton );
    QPushButton *closeButton = new QPushButton(tr("Cancel"));
    hbtn->addWidget( closeButton );
    btnPanel->setLayout( hbtn );

    layout->addWidget(btnPanel);
    connect( closeButton, SIGNAL(clicked()), this, SLOT(close()));
    connect( saveButton, SIGNAL(clicked()), this, SLOT(SLOT_save()));
}

void ParametersWindow::SLOT_save() {
    bool valid = true ;
    GlobalConfig& gc = GlobalConfig::getInstance() ;
    gc.CALLSIGN = sCallSign->text() ;

    gc.mLatitude = sLatitude->text() ;
    gc.mLongitude = sLongitude->text() ;

    if( !gc.mLatitude.endsWith('N') && !gc.mLatitude.endsWith('S')) {
        valid = false ;
    }
    if( !gc.mLongitude.endsWith('E') && !gc.mLongitude.endsWith('W')) {
        valid = false ;
    }
    if( !valid ) {
        QMessageBox msgBox;
        msgBox.setWindowTitle( VER_PRODUCTNAME_STR );
        msgBox.setText("ERROR:  Your lat/lon values must end with letters (N/S or E/W)");
        msgBox.setStandardButtons(QMessageBox::Yes );
        msgBox.exec() ;
        return ;
    }

    if( !QDir( QStandardPaths::writableLocation( QStandardPaths::HomeLocation) + "/" + QString(DATAFOLDER) ).exists() ) {
        QDir().mkpath(QStandardPaths::writableLocation( QStandardPaths::HomeLocation) + "/" + QString(DATAFOLDER) ) ;
    }
    QSettings settings(QStandardPaths::writableLocation( QStandardPaths::HomeLocation) +"/" + QString(DATAFOLDER) +
                       "/" + QString(CONFIG_FILENAME), QSettings::IniFormat);

    settings.beginGroup("Station");
    settings.setValue( "mLatitude", gc.mLatitude );
    settings.setValue( "mLongitude", gc.mLongitude );
    settings.setValue( "CALLSIGN", gc.CALLSIGN );
    settings.endGroup();
    close();
}
