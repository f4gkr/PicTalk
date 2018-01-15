//==========================================================================================
// + + +   This Software is released under the "Simplified BSD License"  + + +
// Copyright 2014 F4GKR Sylvain AZARIAN . All rights reserved.
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
#include "gkdial.h"
#include <math.h>
#include <QPalette>
#include <QDebug>

#define STATE_WAIT (0)
#define STATE_USERCHANGE (1)
#define STATE_INTERNALCHANGE (2)

gkDial::gkDial(int knobType, QString label,
               bool bottomText, QWidget *parent) :
    QWidget(parent)
{
    m_state  = STATE_WAIT ;
    d_knob = createKnob( knobType );
    d_knob->setKnobWidth( 40 );
//    QPalette Pal(palette());
//    Pal.setColor(QPalette::Background, Qt::black);
//    setAutoFillBackground(true);
//    setPalette(Pal);

//    QPalette PalKnob(palette());
//    PalKnob.setColor(QPalette::BrightText, Qt::yellow);
//    PalKnob.setColor(QPalette::Text, Qt::yellow);
//    d_knob->setPalette( PalKnob );

    QLabel *t_label = new QLabel(this);
    t_label->setText( label );
    t_label->setAlignment( Qt::AlignCenter | Qt::AlignTop);

    //t_label->setStyleSheet("QLabel { color : yellow; }");

    QVBoxLayout *layout = new QVBoxLayout( this );;
    layout->setSpacing( 0 );
    layout->setContentsMargins( 0, 0, 0, 0 );
    if( bottomText ) {
        layout->addWidget( d_knob );
        layout->addWidget( t_label );
    } else {
        layout->addWidget( t_label );
        layout->addWidget( d_knob );
    }

    QFrame *line = new QFrame(this);
    line->setFrameShape(QFrame::HLine); // Horizontal line
    line->setFrameShadow(QFrame::Sunken);
    line->setLineWidth(1);
    layout->addWidget(line);

    connect( d_knob, SIGNAL( valueChanged( double ) ),
             this, SLOT( setNum( double ) ) );
}

void gkDial::setScale( float min, float max ) {
    d_knob->setKnobStyle( QwtKnob::Flat );
    d_knob->setMarkerStyle( QwtKnob::Notch );
    d_knob->setScaleEngine( new QwtLinearScaleEngine(10) );
    //
    d_knob->setScale( min, max );
    int mminor = (int)( (fabs(max) - fabs(max))/15.0 );
    d_knob->setScaleStepSize( (float)mminor );
    d_knob->setScaleMaxMinor( (float)mminor );
    d_knob->setValue( min + (fabs(max)-fabs(min))/2);
    //d_knob->setStyleSheet("QLabel { color : yellow; }");
    d_knob->setTotalSteps( 100 );
    //d_knob->setSingleSteps( (float)mminor/5.0 );
}

void gkDial::setValue( float value ) {
    m_state = STATE_INTERNALCHANGE ;
    d_knob->setValue( value );
}

void gkDial::setValueAndWidget( float value ) {
    m_state = STATE_WAIT ;
    d_knob->setValue( value );
}

QwtKnob *gkDial::createKnob( int knobType ) const
{
    QwtKnob *knob = new QwtKnob();
    knob->setTracking( true );

    switch( knobType )
    {
    case 0:
    {
        knob->setKnobStyle( QwtKnob::Sunken );
        knob->setMarkerStyle( QwtKnob::Nub );
        knob->setWrapping( true );
        knob->setNumTurns( 1 );
        knob->setScaleStepSize( 10.0 );
        knob->setScale( 0, 100 );
        knob->setTotalSteps( 100 );
        break;
    }
    case 1:
    {
        knob->setKnobStyle( QwtKnob::Sunken );
        knob->setMarkerStyle( QwtKnob::Dot );
        break;
    }
    case 2:
    {
        knob->setKnobStyle( QwtKnob::Sunken );
        knob->setMarkerStyle( QwtKnob::Tick );
        knob->setTotalSteps( 100 );
        knob->setStepAlignment( false );
        //knob->setSingleSteps( 1 );
        knob->setPageSteps( 5 );

        break;
    }
    case 3:
    {
        knob->setKnobStyle( QwtKnob::Flat );
        knob->setMarkerStyle( QwtKnob::Notch );
        knob->setScaleEngine( new QwtLogScaleEngine() );
        knob->setScaleStepSize( 1.0 );
        knob->setScale( 0.1, 1000.0 );
        knob->setScaleMaxMinor( 10 );
        break;
    }
    case 4:
    {
        knob->setKnobStyle( QwtKnob::Raised );
        knob->setMarkerStyle( QwtKnob::Dot );
        knob->setWrapping( true );
        break;
    }
    case 5:
    {
        knob->setKnobStyle( QwtKnob::Styled );
        knob->setMarkerStyle( QwtKnob::Triangle );
        knob->setTotalAngle( 180.0 );
        knob->setScale( 100, -100 );
        break;
    }
    }

    return knob;
}


void gkDial::setNum( double v )
{
    if( m_state == STATE_INTERNALCHANGE ) {
        m_state = STATE_WAIT ;
        return ;
    }
    emit valueChanged( (int)v );
    m_state = STATE_WAIT ;

}
