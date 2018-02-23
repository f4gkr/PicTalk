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

#include "airspywidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>

AirspyWidget::AirspyWidget(airspy_device *device, QWidget *parent) : QWidget(parent)
{
    m_device = device ;

    QVBoxLayout *crlayout = new QVBoxLayout;
    crlayout->setContentsMargins( 1,1,1,1);
    crlayout->setAlignment(Qt::AlignLeft | Qt::AlignTop  );

    lnaGain = new gkDial(4,tr("LNA Gain"));
    lnaGain->setScale(0,16);
    lnaGain->setValue(6);
    crlayout->addWidget(lnaGain);

    mixerGain = new gkDial(4,tr("Mixer Gain"));
    mixerGain->setScale(0,16);
    mixerGain->setValue(6);
    crlayout->addWidget(mixerGain);

    vgaGain = new gkDial(4,tr("VGA Gain"));
    vgaGain->setScale(0,16);
    vgaGain->setValue(6);
    crlayout->addWidget(vgaGain);

    setLayout(crlayout);

    connect( lnaGain, SIGNAL(valueChanged(int)),
             this, SLOT(SLOT_setLNAGain(int)) );
    connect( mixerGain, SIGNAL(valueChanged(int)),
             this, SLOT(SLOT_setMIXGain(int)) );
    connect( vgaGain, SIGNAL(valueChanged(int)),
             this, SLOT(SLOT_setVGAGain(int)) );
}

void AirspyWidget::SLOT_setLNAGain(int g) {
    airspy_set_lna_gain( m_device, g );
}

void AirspyWidget::SLOT_setVGAGain(int g) {
    airspy_set_vga_gain( m_device, g );
}

void AirspyWidget::SLOT_setMIXGain(int g) {
    airspy_set_mixer_gain( m_device, g);
}
