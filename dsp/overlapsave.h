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
#ifndef OVERLAPSAVE_H
#define OVERLAPSAVE_H

#include <QObject>
#include <fftw3.h>
#include "common/datatypes.h"

#define INPUT_TOO_LONG (-1)
#define NEED_MORE_DATA (-2)
#define GET_DATA_OUT (1)
#define NO_OUT_DATA (-1)
#define NOT_ENOUGH_DATA (-1)



class OverlapSave : public QObject
{
    Q_OBJECT
public:
    explicit OverlapSave(int inSampleRate, int outSampleRate, QObject *parent = 0);
    ~OverlapSave();

    void reset();

    int getOLASOutSampleRate() ;
    int getOLASInSampleRate() { return( m_inSampleRate ) ; }

    void configure(int maxInSize , int use_fft_size );
    int put(TYPECPX *insamples, int length );
    int get(TYPECPX *data , int max_read=-1, int min_read=-1);
    void pushback( TYPECPX* data, int count );

    void setCenterOfWindow( float freq );
    float getCenterOfWindow();

    bool shiftInTimeDomain( int samples );
    bool shiftOutTimeDomain( int samples );
    bool shiftOutPhase( float angleRadians );
    void razOutPhase();
    void shiftOutputCenterFrequency( float offset_hz );

    static int FILTER_KERNEL_SIZE ;

signals:

public slots:
private:
    int m_inSampleRate ;
    int m_outSampleRate;
    int DecimFactor ;
    int Nrequired ;
    double *H ;
    int NTaps ;
    int fft_size ;
    float m_center_freq ;
    int m_band_start ;
    int m_band_bins ;
    double mix_offset ;
    double mix_phase ;
    bool apply_postmixer;
    fftwf_complex * fftin,  *_H, *_H0 ; // *fftout, *fft_conv, *fft_filtered, ;
    fftwf_plan plan ;
    fftwf_plan plan_rev ;

    TYPECPX *datas_in ;
    TYPECPX *datas_out;
    long data_size ;
    long wr_pos ;
    long rd_pos ;
    long overlap ;
    long out_wpos ;
    quint64 decim_r_pos ;
    long ttl_put ;
    long ttl_read ;

    void step1();
    void step2();

    double bessi0( double x );
    double *calc_filter(double Fs, double Fa, double Fb, int M, double Att, int *Ntaps ) ;
};

#endif // OVERLAPSAVE_H
