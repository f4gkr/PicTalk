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
#include "spectrumplot.h"
#include "qwt/qwt_plot.h"
#include "qwt/qwt_math.h"
#include "qwt/qwt_scale_engine.h"
#include "qwt/qwt_symbol.h"
#include "qwt/qwt_plot_grid.h"
#include "qwt/qwt_plot_marker.h"
#include "qwt/qwt_plot_curve.h"
#include "qwt/qwt_legend.h"
#include "qwt/qwt_text.h"
#include "qwt/qwt_picker_machine.h"
#include "common/datatypes.h"

#define PI (3.14159265358979323846)
#define SPECTRUM_FFT_SIZE (2048)

#define SPLOT_DEBUG (0)
SpectrumPlot::SpectrumPlot(QWidget *parent, bool withGrid) :
    QwtPlot(parent)
{
    int k ;

    setAutoFillBackground( true );
    s_center_frequency = 0 ;
    s_bw = 1e6 ;
    min_scale = -90 ;
    max_scale = -40 ;
    show_maxhold = true ;
    show_peak = true ;

    // spectrum calculation
    semspectrum = new QSemaphore(1);
    last_fft_size = SPECTRUM_FFT_SIZE ;


    spectrum = (double *)malloc( last_fft_size * sizeof( double ));
    max_hold = (double *)malloc( last_fft_size * sizeof( double ));
    frequencies = (double *)malloc( last_fft_size * sizeof( double ));
    for( k=0 ; k < last_fft_size ; k++ ) {
             spectrum[k] = 0 ;
             max_hold[k] = -200 ;
             frequencies[k] = 0 ;
    }

    // legend
    //QwtLegend *legend = new QwtLegend;
    //insertLegend( legend, QwtPlot::BottomLegend );
    setAxisScale( QwtPlot::xBottom, -s_bw , s_bw);
    // grid
    if( withGrid ) {
    QwtPlotGrid *grid = new QwtPlotGrid;
        grid->enableXMin( true );
#ifndef _WIN32
        grid->setMajorPen( QPen( Qt::white, 0, Qt::DotLine ) );
        grid->setMinorPen( QPen( Qt::gray, 0 , Qt::DotLine ) );
#endif
        grid->attach( this );
    }
    // curves
    curve = new QwtPlotCurve( "Amplitude" );
    curve->setRenderHint( QwtPlotItem::RenderAntialiased );
    curve->setPen( QPen( Qt::yellow ) );
    //curve->setLegendAttribute( QwtPlotCurve::LegendShowLine );
    curve->setYAxis( QwtPlot::yLeft );
    curve->setZ( 90 );
    curve->attach( this );

    curve_maxhold = new QwtPlotCurve( "Max" );
    curve_maxhold->setRenderHint( QwtPlotItem::RenderAntialiased );
    curve_maxhold->setPen( QPen( Qt::white ) );
    //curve_maxhold->setLegendAttribute( QwtPlotCurve::LegendShowLine );
    curve_maxhold->setYAxis( QwtPlot::yLeft );
    curve_maxhold->setZ( 95 );
    curve_maxhold->attach( this );

    // axes
    //setAxisTitle( QwtPlot::xBottom, "Freq [MHz]" );
    //setAxisTitle( QwtPlot::yLeft, " Amplitude[dB]" );
    //setAxisScale( QwtPlot::yLeft, min_scale, max_scale );
    setAxisMaxMajor( QwtPlot::xBottom, 6 );
    setAxisMaxMinor( QwtPlot::xBottom, 10 );

    mark_max = new QwtPlotMarker();
    mark_max->setLineStyle( QwtPlotMarker::HLine );
    mark_max->setLabelAlignment( Qt::AlignRight | Qt::AlignBottom );
    mark_max->setLinePen( QPen( QColor( 200, 150, 0 ), 0, Qt::DashDotLine ) );
    mark_max->setSymbol( new QwtSymbol( QwtSymbol::Diamond,
                QColor( Qt::yellow ), QColor( Qt::green ), QSize( 7, 7 ) ) );
    mark_max->attach( this );


    mark_rx = new QwtPlotMarker();
    mark_rx->setLabel( QString::fromLatin1( "RX A" ) );
    mark_rx->setLabelAlignment( Qt::AlignLeft | Qt::AlignBottom );
    mark_rx->setLabelOrientation( Qt::Vertical );
    mark_rx->setLineStyle( QwtPlotMarker::VLine );
//#ifndef _WIN32
    mark_rx->setLinePen( QColor( 200, 10, 10 ), 0, Qt::DashLine );
//#endif
    mark_rx->setXValue( 0 );
    mark_rx->attach( this );

    sub_rx = new QwtPlotMarker();
    sub_rx->setLabel( QString::fromLatin1( "RX B" ) );
    sub_rx->setLabelAlignment( Qt::AlignLeft | Qt::AlignBottom );
    sub_rx->setLabelOrientation( Qt::Vertical );
    sub_rx->setLineStyle( QwtPlotMarker::VLine );
//#ifndef _WIN32
    sub_rx->setLinePen( QColor( 200, 10, 10 ), 0, Qt::DashLine );
//#endif
    sub_rx->setXValue( 0 );
    sub_rx->attach( this );

    d_picker = new QwtPlotPicker( QwtPlot::xBottom, QwtPlot::yLeft,
                                  QwtPlotPicker::CrossRubberBand, QwtPicker::AlwaysOn,
                                  this->canvas() );

    d_picker->setStateMachine( new QwtPickerClickPointMachine() ); //new QwtPickerDragPointMachine() );
    d_picker->setRubberBandPen( QColor( Qt::green ) );
    d_picker->setRubberBand( QwtPicker::CrossRubberBand );
    d_picker->setTrackerPen( QColor( Qt::white ) );

    connect( d_picker, SIGNAL( selected( const QPointF & ) ),
             this, SLOT( SLOT_selected( const QPointF & ) ) );

    setCanvasBackground( QColor( Qt::darkBlue ) );
    if( SPLOT_DEBUG ) qDebug() << "SpectrumPlot::SpectrumPlot done" ;
}

void SpectrumPlot::SLOT_setCurrentRXFreq(double rx_freq , int channel)
{
    if( channel == 0 ) {
        mark_rx->setXValue( rx_freq );
    } else {
        sub_rx->setXValue( rx_freq );
    }
}


void SpectrumPlot::SLOT_selected(const QPointF &pos )
{
    double f = pos.x();
    qint64 fcenter = s_center_frequency ;
    qint64 delta   = (qint64)( f*1e6 - fcenter );
    if( SPLOT_DEBUG ) qDebug() << "SpectrumPlot::SLOT_selected" << fcenter << delta ;
    emit NewDemodFreq(fcenter,delta);
}


void SpectrumPlot::showPeak( double freq, double amplitude )
{
    QString label;
    label.sprintf( "%03.1fdB  %0.3fMHz", (float)amplitude, (float)freq );


    QwtText text( label );
    text.setFont( QFont( "Helvetica", 10, QFont::Bold ) );
    text.setColor( QColor( 200, 150, 0 ) );

    mark_max->setValue( freq, amplitude );
    mark_max->setLabel( text );
}

void SpectrumPlot::setNewParams( double center_frequency, double bw )
{
    //semspectrum->acquire(1);
    if( center_frequency >= 0 )
        s_center_frequency = center_frequency ;
    if( bw > 0 )
        s_bw = bw ;
    //semspectrum->release(1);
    //qDebug() << "SpectrumPlot::setNewParams( double center_frequency="<< s_center_frequency << ", double bw = "<< s_bw << ")" ;

    setAutoReplot( false );
    setAxisScale( QwtPlot::xBottom, (s_center_frequency - s_bw/2)/1e6 , (s_center_frequency + s_bw/2)/1e6);
    setAutoReplot( true );
}


void SpectrumPlot::razMaxHold() {
    int k ;

    semspectrum->acquire(1);
    for( k=0 ; k < last_fft_size ; k++ ) {
             max_hold[k] = -200 ;
    }
    semspectrum->release(1);

}

void SpectrumPlot::setMaxHoldVisible( bool show ) {
    if( this->show_maxhold == show ) {
        return ;
    }

    if( this->show_maxhold ) {
        // remove the plot
        curve_maxhold->detach();
        this->show_maxhold = false ;
    } else {
        curve_maxhold->attach( this );
        this->show_maxhold = true ;
    }

}


void SpectrumPlot::setMinMaxScales( float min, float max ) {
     min_scale = min ;
     max_scale = max ;
     if( SPLOT_DEBUG ) qDebug() << "SpectrumPlot::setMinMaxScales" << min << max ;
     setAxisScale( QwtPlot::yLeft, min_scale, max_scale );
}

void SpectrumPlot::setPowerTab(qint64 center_frequency, double *power_dB, int length, double bw) {
     int k,i ;
     double pow_max, pow_min, pos_max ;
     double f_low = center_frequency - bw/2 ;
     double f_step= bw / length ;

     semspectrum->acquire(1);
     if( length != last_fft_size ) {
         last_fft_size = length ;
         free( spectrum );
         free( max_hold );
         spectrum = (double *)malloc( last_fft_size * sizeof( double ));
         max_hold = (double *)malloc( last_fft_size * sizeof( double ));
         if( SPLOT_DEBUG ) qDebug() << "realloc frequencies"  ;
         free( frequencies );
         frequencies = (double *)malloc( last_fft_size * sizeof( double ));

         if( SPLOT_DEBUG ) qDebug() << "init values"  ;
         for( k=0 ; k < last_fft_size ; k++ ) {
                  spectrum[k] = 0 ;
                  max_hold[k] = -200 ;
         }
         if( SPLOT_DEBUG ) qDebug() << "realloc done for new size " <<  last_fft_size ;
     }

     s_center_frequency = center_frequency ;
     s_bw = bw ;

     pow_max = -200 ;
     pow_min = +200 ;
     pos_max = frequencies[length/2] ;

     for( i=0 ; i < length ; i++ ) {

         spectrum[i] = power_dB[i] ;
         frequencies[i] = (f_low + i*f_step)/1e6 ;

         if( spectrum[i] > pow_max ) {
             pow_max = spectrum[i] ;
             pos_max = frequencies[i] ;
         }
         if( spectrum[i] < pow_min ) {
             pow_min = spectrum[i] ;
         }

         if( spectrum[i] > max_hold[i]) {
             max_hold[i] = (9*max_hold[i] + spectrum[i])/10.0;
         } else {
             if( max_hold[i] > -200 ) {
                 max_hold[i] -= .1 ;
             }
         }
     }

     if( pow_min < min_scale ) pow_min = min_scale ;
     semspectrum->release(1);
     setAutoReplot( false );
     setAxisScale( QwtPlot::xBottom, (center_frequency - bw/2)/1e6 , (center_frequency + bw/2)/1e6);
     setAutoReplot( true );

     if( show_peak )
         showPeak( pos_max, pow_max);

     curve->setSamples( frequencies, spectrum, length );
     if( show_maxhold ) {
         curve_maxhold->setSamples( frequencies, max_hold, length );
     }   
//     if( pow_max > max_scale ) {
//         setMinMaxScales( pow_min, pow_max ) ;
//     }
}

void SpectrumPlot::resizeEvent( QResizeEvent *event )
{
    QwtPlot::resizeEvent( event );
}
