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

    GlobalConfig& cnf = GlobalConfig::getInstance() ;

    spy = new AirspyDevice();
    if( spy->getDeviceCount() > 0 ) {
        return(spy);
    }
    delete spy ;

    rsp = new MiricsSDR(cnf.device_index);
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

    dongle = new RTLSDR(cnf.device_index);
    if( dongle->getDeviceCount() > 0 ) {
        if( dongle->setRxSampleRate( SYMBOL_RATE * 100 ) == 0 ) { // sampling rate is .960 MHz
            return(dongle);
        }
    }
    delete dongle ;
    dongle = NULL ;
    return(NULL);
}
