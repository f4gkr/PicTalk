#ifndef RXDEVICE_H
#define RXDEVICE_H
#include <QObject>
#include <stdint.h>
#include <qglobal.h>
#include "common/tuningpolicy.h"
#include "common/datatypes.h"
#include "common/samplefifo.h"

class RxDevice: public QObject
{
    Q_OBJECT
public:    
    explicit RxDevice(QObject *parent = 0);
    ~RxDevice();

    virtual void close() = 0 ;

    virtual int getDeviceCount() = 0 ;
    virtual char* getHardwareName()= 0;
    virtual int startAcquisition() = 0;
    virtual int stopAcquisition() = 0;

    virtual int setRxSampleRate(uint32_t sampling_rate ) = 0;
    virtual uint32_t getRxSampleRate() = 0;

    virtual int setRxCenterFreq( TuningPolicy *freq_hz ) = 0 ;
    virtual qint64 getRxCenterFreq() = 0 ;

    // this gives the physical limits (hardware limits) of the receiver by itself, without any RFE
    virtual qint64 getMin_HWRx_CenterFreq() = 0  ;
    virtual qint64 getMax_HWRx_CenterFreq() = 0   ;

    virtual int getFifoSize();
    virtual SampleFifo *getFIFO() { return( fifo ); }

    virtual bool deviceHasSingleGainStage() { return( true ) ; }
    virtual QWidget* getDisplayWidget() {return(NULL) ; }

    virtual int setRxGain(float db );
    virtual float getRxGain();
    virtual float getMinGain() ;
    virtual float getMaxGain();

public slots:
    void SLOT_start();
    void SLOT_stop();

protected:
    SampleFifo *fifo ;

};

#endif // RXDEVICE_H
