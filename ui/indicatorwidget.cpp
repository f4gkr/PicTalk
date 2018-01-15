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
#include "indicatorwidget.h"
#include <qpainter.h>
#include "qwt/qwt_dial_needle.h"
#include "qwt/qwt_round_scale_draw.h"

IndicatorWidget::IndicatorWidget( QString name, QWidget *parent) : QwtDial(parent)
{
    m_Value = 0 ;
    myName = name ;
    m_maxValue = 100 ;
    m_alarmValue = m_maxValue * 1.1 ;
    m_minValue = 0 ;

     setLowerBound( m_minValue );
     setUpperBound( m_maxValue);
     setMode( QwtDial::RotateNeedle ) ;
     setFrameShadow( QwtDial::Sunken );
     QwtRoundScaleDraw *scaleDraw = new QwtRoundScaleDraw();
     scaleDraw->setSpacing( 8 );
     scaleDraw->enableComponent( QwtAbstractScaleDraw::Backbone, false );
     scaleDraw->setTickLength( QwtScaleDiv::MinorTick, 0 );
     scaleDraw->setTickLength( QwtScaleDiv::MediumTick, 4 );
     scaleDraw->setTickLength( QwtScaleDiv::MajorTick, 8 );
     setScaleDraw( scaleDraw );

     setWrapping( false );
     setReadOnly( true );

     setOrigin( 135.0 );
     setScaleArc( 0.0, 270.0 );

     QwtDialSimpleNeedle *needle = new QwtDialSimpleNeedle(
                 QwtDialSimpleNeedle::Arrow, true, Qt::red,
                 QColor( Qt::gray ).light( 130 ) );
     setNeedle( needle );
}

IndicatorWidget::IndicatorWidget(
                 QString name,
                 double min_value,
                 double max_value,
                 QString unit,
                 QWidget *parent) :
    QwtDial(parent)
{
    m_Value = 0 ;
    myName = name ;
    m_maxValue = max_value ;
    m_alarmValue = m_maxValue * 1.1 ;
    m_minValue = min_value ;
    m_Unit = unit ;

    setMode( QwtDial::RotateNeedle ) ;
    setFrameShadow( QwtDial::Sunken );
    setLowerBound( qMin(m_minValue, m_maxValue));
    setUpperBound( qMax(m_minValue, m_maxValue) );
    QwtRoundScaleDraw *scaleDraw = new QwtRoundScaleDraw();

    scaleDraw->setSpacing( 8 );
    scaleDraw->enableComponent( QwtAbstractScaleDraw::Backbone, false );
    scaleDraw->setTickLength( QwtScaleDiv::MinorTick, 0 );
    scaleDraw->setTickLength( QwtScaleDiv::MediumTick, 4 );
    scaleDraw->setTickLength( QwtScaleDiv::MajorTick, 8 );
    setScaleDraw( scaleDraw );

    setWrapping( false );
    setReadOnly( true );

    setOrigin( 135.0 );
    setScaleArc( 0.0, 270.0 );

    QwtDialSimpleNeedle *needle = new QwtDialSimpleNeedle(
                QwtDialSimpleNeedle::Arrow, true, Qt::red,
                QColor( Qt::gray ).light( 130 ) );
    setNeedle( needle );
}

void IndicatorWidget::drawScaleContents( QPainter *painter,
    const QPointF &center, double radius ) const
{
    QRectF rect( 0.0, 0.0, 2.0 * radius, 2.0 * radius - 10.0 );
    rect.moveCenter( center );

    const QColor color = palette().color( QPalette::Text );
    painter->setPen( color );

    const int flags = Qt::AlignBottom | Qt::AlignHCenter;
    painter->drawText( rect, flags, myName );
}

void IndicatorWidget::SLOT_updateValue(float newValue ) {
    if( newValue == m_Value ) return ;

    m_Value = newValue ;
    if( m_Value > m_maxValue ) m_Value = m_maxValue ;
    if( m_Value < m_minValue ) m_Value = m_minValue ;

    setValue( m_Value ) ;
}
