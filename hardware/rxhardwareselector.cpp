#include "rxhardwareselector.h"
#include "common/constants.h"
#include "hardware/funcube/funcube.h"
#include "hardware/rtlsdr.h"
#include "hardware/miricscpp.h"
#include "airspydevice.h"

RxHardwareSelector::RxHardwareSelector(QObject *parent) : QObject(parent)
{

}

RxDevice *RxHardwareSelector::getReceiver() {
    FUNCube *fcdboard = NULL ;
    RTLSDR *dongle = NULL ;
    MiricsSDR *rsp = NULL ;
    AirspyDevice *spy = NULL ;

    spy = new AirspyDevice();
    if( spy->getDeviceCount() > 0 ) {
        return(spy);
    }
    delete spy ;

    rsp = new MiricsSDR(0);
    if( rsp->getDeviceCount() > 0 ) {
        rsp->setRxSampleRate(1536*1000);
        return( rsp );
    }
    delete rsp ;
    rsp = NULL ;
/**
Is there is a bug with the FUNCUBE implementation ?
**/
    // test FUNcube pro
    fcdboard = new FUNCube(true);
    if( fcdboard->getDeviceCount() > 0 ) {
        return(fcdboard);
    }
    delete fcdboard ;

    // test normal (non pro)
    fcdboard = new FUNCube(false);
    if( fcdboard->getDeviceCount() > 0 ) {
        return(fcdboard);
    }
    delete fcdboard ;
    fcdboard = NULL ;

    dongle = new RTLSDR(0);
    if( dongle->getDeviceCount() > 0 ) {
        if( dongle->setRxSampleRate( SYMBOL_RATE * 100 ) == 0 ) { // sampling rate is .960 MHz
            return(dongle);
        }
    }
    delete dongle ;
    dongle = NULL ;
    return(NULL);
}
