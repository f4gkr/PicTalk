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
#ifndef SPECTRUMPLOT_H
#define SPECTRUMPLOT_H
#include <QWidget>
#include <QSemaphore>
#include <QDebug>
#include "qwt/qwt_plot.h"
#include "qwt/qwt_plot_curve.h"
#include "qwt/qwt_plot_marker.h"
#include "qwt/qwt_picker_machine.h"
#include "qwt/qwt_plot_picker.h"


#include "common/datatypes.h"

class SpectrumPlot : public QwtPlot
{
     Q_OBJECT

public:
     SpectrumPlot(QWidget *parent = NULL, bool withGrid=false);
     ~SpectrumPlot() {
         //qDebug() << "SpectrumPlot::~SpectrumPlot" ;
     }

     void showPeak( double freq, double amplitude ) ;

     void setNewParams( double center_frequency, double bw=0 );
     void setPowerTab( qint64 center_frequency, double *power_dB, int length, double bw=0 );

     void setMaxHoldVisible( bool show );
     void setMinMaxScales( float min, float max );
     void razMaxHold();
     void setShowPeak( bool show ) { show_peak = show ; }

     double getMinScale() { return( min_scale ); }
     double getMaxScale() { return( max_scale ); }

signals:
     void NewDemodFreq(qint64 freq, qint64 delta);


public slots:
     void SLOT_setCurrentRXFreq( double rx_freq , int channel );

private Q_SLOTS:
    void SLOT_selected(const QPointF &pos);

protected:
    virtual void resizeEvent( QResizeEvent * );

private:
    QwtPlotCurve *curve ;
    QwtPlotCurve *curve_maxhold ;
    QwtPlotMarker *mark_max ;

    QwtPlotMarker *mark_rx ;
    QwtPlotMarker *sub_rx ;
    QwtPlotPicker *d_picker ;

    bool show_maxhold ;
    bool show_peak ;

    double s_center_frequency ;
    double s_bw ;
    double max_scale ;
    double min_scale ;


    QSemaphore *semspectrum;

    double *spectrum ;
    double *max_hold ;
    double *frequencies ;
    int last_fft_size;
};

#endif // SPECTRUMPLOT_H
