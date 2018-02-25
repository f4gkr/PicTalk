#include "rxdevice.h"
#include <QDebug>

RxDevice::RxDevice(QObject *parent):
    QObject(parent)
{
    fifo = new SampleFifo();

}

RxDevice::~RxDevice() {
    if( fifo != NULL )
        delete fifo ;
}

int RxDevice::getFifoSize() {
    return( fifo->getSize() );
}

int RxDevice::setRxGain(float db ) {
    Q_UNUSED(db);
    return(0);
}

float RxDevice::getRxGain() {
    return(0);
}

float RxDevice::getMinGain() {
    return( 0) ;
}

float RxDevice::getMaxGain() {
    return( 0) ;
}

void  RxDevice::SLOT_start() {
    qDebug() << "RxDevice::SLOT_start()" ;
    startAcquisition() ;
}

void  RxDevice::SLOT_stop() {
    qDebug() << "RxDevice::SLOT_stop()" ;
    stopAcquisition() ;
}


float RxDevice::getNoiseFloor()  {
    return( -100 );
}
