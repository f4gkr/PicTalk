//==========================================================================================
// + + +   This Software is released under the "Simplified BSD License"  + + +
// Copyright 2014 F4GKR Sylvain AZARIAN . All rights reserved.
// SPECIAL THANKS TO F6BHI FOR LETTING ME TESTING THE FUNCUBE
// This code contains portions from QtHID
//  Copyright (C) 2010  Howard Long, G6LVB
//  CopyRight (C) 2011  Alexandru Csete, OZ9AEC
//                      Mario Lorenz, DL5MLO
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

/***************************************************************************
 *  This file is part of Qthid.
 *
 *  Copyright (C) 2010  Howard Long, G6LVB
 *  CopyRight (C) 2011  Alexandru Csete, OZ9AEC
 *                      Mario Lorenz, DL5MLO
 *
 *  Qthid is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Qthid is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Qthid.  If not, see <http://www.gnu.org/licenses/>.
 *
 ***************************************************************************/

const unsigned short _usVID=0x04D8;  /*!< USB vendor ID. */
const unsigned short _usPID=0xFB56;  /*!< USB product ID. */
const unsigned short _usPID2=0xFB31;  /*!< USB product ID. */
//-----------------------------------------------------------------


#include "funcube.h"
#include "fcdhidcmd.h"
#include <QDebug>
#include <QWidget>
#include <unistd.h>
#include "fcdwidget.h"

#define FCD_DEBUG (1)

FUNCube::FUNCube(bool pro) {
    hardwareName = (char *)malloc( 64 * sizeof( char ));
    sprintf( hardwareName, "%s", "FUNcube");
    audio_input = NULL ;
    sampling_rate = 0 ;
    // disable gain combo in gkSDR - we have a panel for settings
    gainMax = 0 ;
    gainMin = 0 ;
    fcd = NULL ;
    ppm_error = 0 ;
    gui = NULL ;

    // Test if we have a fcd device here
     _PID = _usPID ;
     if( pro ) {
         _PID = _usPID2 ;
     }
     hid_device *phd = fcdOpen();
     if( phd == NULL ) {
          return ;
     }
     fcdBlReset();
     fcdClose(phd);

    //audio_input = new AudioInput( "hw:CARD=HD,DEV=0", 96000*2 ) ;
    audio_input = new AudioInput( "FUNcube", 96000*2 ) ;


    sampling_rate = audio_input->getSampleRate() ;
    if( sampling_rate < 0 ) {
        delete audio_input ;
        audio_input = NULL ;
        return ;
    }
}

int FUNCube::getDeviceCount() {
    if( audio_input == NULL )
        return(0);
    return(1);
}

void FUNCube::close() {
    if( fcd != NULL ) {
        fcdClose(fcd);
    }
    if( audio_input != NULL )
        audio_input->stopAudio();
    if( gui != NULL )
        delete gui ;
}


char* FUNCube::getHardwareName() {
    return( hardwareName );
}

int FUNCube::startAcquisition() {
    if( audio_input == NULL ) {
        qDebug() << "FUNCube::prepareRXEngine() audio_input == NULL" ;
        return(0);
    }

    if( FCD_DEBUG ) qDebug() << "FUNCube::prepareRXEngine() audio_input->startAudio()" ;
    if( !audio_input->startAudio()) {
        return(0);
    }
    return(1);
}

int FUNCube::stopAcquisition() {
    if( audio_input == NULL )
        return(0);
    audio_input->stopAudio();
    return(1);
}

int FUNCube::setRxCenterFreq(TuningPolicy *freq_hz ) {
    FCD_MODE_ENUM rc ;

    // FCD has 1Khz resolution
    centerFreq = (freq_hz->rx_hardware_frequency)/1000 ;
    if( FCD_DEBUG ) qDebug() << "FUNCube::setRxCenterFreq() -> " << centerFreq << " KHz" ;
    rc = fcdAppSetFreqkHz( (int)centerFreq );
    if( rc == FCD_MODE_APP) {
        if( FCD_DEBUG ) qDebug() << "FUNCube::setRxCenterFreq() OK " ;
        return(1);
    }
    return(0);
}

qint64 FUNCube::getRxCenterFreq() {
    // FCD has 1Khz resolution
    return( centerFreq*1000 );
}

// this gives the physical limits (hardware limits) of the receiver by itself, without any RFE
qint64 FUNCube::getMin_HWRx_CenterFreq() {
    return(64e6) ;
}

qint64 FUNCube::getMax_HWRx_CenterFreq() {
    return(1700e6);
}


int FUNCube::setRxSampleRate( unsigned int sample_rate ) {
    Q_UNUSED(sample_rate);
    return(0);
}


uint32_t FUNCube::getRxSampleRate() {
    return( sampling_rate );
}

void FUNCube::setIQCorrectionFactors( TYPEREAL gainI, TYPEREAL gainQ,
                                      TYPEREAL crossGain ) {
    audio_input->setIQCorrectionFactors( gainI, gainQ, crossGain);
}

void FUNCube::getIQCorrectionFactors( TYPEREAL *gainI,
                                      TYPEREAL *gainQ,
                                      TYPEREAL *crossGain ) {
    audio_input->getIQCorrectionFactors( gainI, gainQ, crossGain);
}

int FUNCube::getFifoSize() {
    if( audio_input == NULL ) {
        return( 0 );
    }
    SampleFifo* f = audio_input->getFifo() ;
    return( f->getSize() ) ;
}

SampleFifo *FUNCube::getFIFO() {
    if( audio_input == NULL ) {
        return( NULL );
    }
    return( audio_input->getFifo() );
}



void FUNCube::setppmError( double ppmError )  {
    this->ppm_error = ppmError ;
}

double FUNCube::getppmError()  {
    return(ppm_error);
}


/***************************************************************************
 *  This file is part of Qthid.
 *
 *  Copyright (C) 2010  Howard Long, G6LVB
 *  CopyRight (C) 2011  Alexandru Csete, OZ9AEC
 *                      Mario Lorenz, DL5MLO
 *
 *  Qthid is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Qthid is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Qthid.  If not, see <http://www.gnu.org/licenses/>.
 *
 ***************************************************************************/
hid_device *FUNCube::fcdOpen(void)
{
    struct hid_device_info *phdi=NULL;
    hid_device *phd=NULL;
    char *pszPath=NULL;

    phdi=hid_enumerate(_usVID, _PID);
    if (phdi==NULL)
    {
        return NULL; // No FCD device found
    }

    pszPath=strdup(phdi->path);
    if (pszPath==NULL)
    {
        return NULL;
    }

    hid_free_enumeration(phdi);
    phdi=NULL;

    if ((phd=hid_open_path(pszPath)) == NULL)
    {
        free(pszPath);
        pszPath=NULL;

        return NULL;
    }

    free(pszPath);
    pszPath=NULL;

    return phd;
}

/** \brief Close FCD HID device. */
void FUNCube::fcdClose(hid_device *phd)
{
    hid_close(phd);
}



/** \brief Set FCD frequency with kHz resolution.
  * \param nFreq The new frequency in kHz.
  * \return The FCD mode.
  *
  * This function sets the frequency of the FCD with 1 kHz resolution. The parameter
  * nFreq must already contain any necessary frequency correction.
  *
  * \sa fcdAppSetFreq
  */
FCD_MODE_ENUM FUNCube::fcdAppSetFreqkHz(int nFreq)
{
    hid_device *phd=NULL;
    unsigned char aucBufIn[65];
    unsigned char aucBufOut[65];

    phd = fcdOpen();

    if (phd == NULL)
    {
        return FCD_MODE_NONE;
    }

    // Send an App reset command
    aucBufOut[0] = 0; // Report ID, ignored
    aucBufOut[1] = FCD_CMD_APP_SET_FREQ_KHZ;
    aucBufOut[2] = (unsigned char)nFreq;
    aucBufOut[3] = (unsigned char)(nFreq>>8);
    aucBufOut[4] = (unsigned char)(nFreq>>16);
    hid_write(phd, aucBufOut, 65);
    memset(aucBufIn, 0xCC, 65); // Clear out the response buffer
    hid_read(phd, aucBufIn, 65);

    if (aucBufIn[0]==FCD_CMD_APP_SET_FREQ_KHZ && aucBufIn[1]==1)
    {
        fcdClose(phd);
        phd = NULL;

        return FCD_MODE_APP;
    }

    fcdClose(phd);
    phd = NULL;

    return FCD_MODE_BL;
}




/** \brief Write FCD parameter (e.g. gain or filter)
  * \param u8Cmd The command byte / parameter ID, see FCD_CMD_APP_SET_*
  * \param pu8Data The parameter value to be written
  * \param u8len Length of pu8Data in bytes
  * \return One of FCD_MODE_NONE, FCD_MODE_APP or FCD_MODE_BL (see description)
  *
  * This function can be used to set the value of a parameter in the FCD for which there is no
  * high level API call. It gives access to the low level API of the FCD and the caller is expected
  * to be aware of the various FCD commands, since they are required to be supplied as parameter
  * to this function.
  *
  * The return value can be used to determine the success or failure of the command execution:
  * - FCD_MODE_APP : Reply from FCD was as expected (nominal case).
  * - FCD_MODE_BL : Reply from FCD was not as expected.
  * - FCD_MODE_NONE : No FCD was found
  */
FCD_MODE_ENUM FUNCube::fcdAppSetParam(unsigned char u8Cmd, unsigned char *pu8Data, unsigned char u8len)
{
    hid_device *phd=NULL;
    unsigned char aucBufOut[65];
    unsigned char aucBufIn[65];


    phd = fcdOpen();

    if (phd == NULL)
    {
        return FCD_MODE_NONE;
    }

    aucBufOut[0]=0; // Report ID, ignored
    aucBufOut[1]=u8Cmd;
    memcpy(aucBufOut+2, pu8Data,u8len);
    hid_write(phd,aucBufOut,65);

    /* we must read after each write in order to empty FCD/HID buffer */
    memset(aucBufIn,0xCC,65); // Clear out the response buffer
    hid_read(phd,aucBufIn,65);

    /* Check the response, if OK return FCD_MODE_APP */
    if (aucBufIn[0]==u8Cmd && aucBufIn[1]==1) {
        fcdClose(phd);
        phd = NULL;

        return FCD_MODE_APP;
    }

    /* Response did not contain the expected bytes */
    fcdClose(phd);
    phd = NULL;

    return FCD_MODE_BL;

}


/** \brief Read FCD parameter (e.g. gain or filter)
  * \param u8Cmd The command byte / parameter ID, see FCD_CMD_APP_GET_*
  * \param pu8Data TPointer to buffer where the parameter value(s) will be written
  * \param u8len Length of pu8Data in bytes
  * \return One of FCD_MODE_NONE, FCD_MODE_APP or FCD_MODE_BL (see description)
  *
  * This function can be used to read the value of a parameter in the FCD for which there is no
  * high level API call. It gives access to the low level API of the FCD and the caller is expected
  * to be aware of the various FCD commands, since they are required to be supplied as parameter
  * to this function.
  *
  * The return value can be used to determine the success or failure of the command execution:
  * - FCD_MODE_APP : Reply from FCD was as expected (nominal case).
  * - FCD_MODE_BL : Reply from FCD was not as expected.
  * - FCD_MODE_NONE : No FCD was found
  */
FCD_MODE_ENUM FUNCube::fcdAppGetParam(unsigned char u8Cmd, unsigned char *pu8Data, unsigned char u8len)
{
    hid_device *phd=NULL;
    unsigned char aucBufOut[65];
    unsigned char aucBufIn[65];

    phd = fcdOpen();
    if (phd == NULL)
    {
        return FCD_MODE_NONE;
    }

    aucBufOut[0]=0; // Report ID, ignored
    aucBufOut[1]=u8Cmd;
    hid_write(phd,aucBufOut,65);

    memset(aucBufIn,0xCC,65); // Clear out the response buffer
    hid_read(phd,aucBufIn,65);
    /* Copy return data to output buffer (even if cmd exec failed) */
    memcpy(pu8Data,aucBufIn+2,u8len);

    /* Check status bytes in returned data */
    if (aucBufIn[0]==u8Cmd && aucBufIn[1]==1) {
        fcdClose(phd);
        phd = NULL;

        return FCD_MODE_APP;
    }

    /* Response did not contain the expected bytes */
    fcdClose(phd);
    phd = NULL;

    return FCD_MODE_BL;
}

/** \brief Reset FCD to application mode.
 * \return FCD_MODE_NONE
 *
 * This function is used to switch the FCD from bootloader mode
 * into application mode.
 */
FCD_MODE_ENUM FUNCube::fcdBlReset(void)
{
    hid_device *phd=NULL;
    //    unsigned char aucBufIn[65];
    unsigned char aucBufOut[65];

    phd = fcdOpen();

    if (phd == NULL)
    {
        return FCD_MODE_NONE;
    }

    // Send an BL reset command
    aucBufOut[0] = 0; // Report ID, ignored
    aucBufOut[1] = FCD_CMD_BL_RESET;
    hid_write(phd, aucBufOut, 65);

    /** FIXME: hid_read() will hang due to a pthread_cond_wait() never returning.
      It seems that the read_callback() in hid-libusb.c will never receive any
      data during the reconfiguration. Since the same logic works in the native
      windows application, it could be a libusb thing. Anyhow, since the value
      returned by this function is not used, we may as well jsut skip the hid_read()
      and return FME_NONE.
      Correct switch from BL to APP mode can be observed in /var/log/messages (linux)
      (when in bootloader mode the device version includes 'BL')
      */
    /*
       memset(aucBufIn,0xCC,65); // Clear out the response buffer
       hid_read(phd,aucBufIn,65);
       if (aucBufIn[0]==FCDCMDBLRESET && aucBufIn[1]==1)
       {
       FCDClose(phd);
       phd=NULL;
       return FME_BL;
       }
       FCDClose(phd);
       phd=NULL;
       return FME_APP;
       */

    fcdClose(phd);
    phd = NULL;

    return FCD_MODE_NONE;

}

QWidget* FUNCube::getDisplayWidget() {
    if( gui == NULL ) {
        FCDWidget *wid = new FCDWidget( this );
        gui = (QWidget*)wid ;
    }
    return( gui );
}
