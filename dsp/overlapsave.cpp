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
#include <QDebug>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "overlapsave.h"

#define DEBUG_MATLAB 0
#define OLAS_DEBUG (0)
#define INPLACE (1)

 int OverlapSave::FILTER_KERNEL_SIZE = 3200 ;



OverlapSave::OverlapSave(int inSampleRate, int outSampleRate, QObject *parent) : QObject(parent)
{
    int k ;
    m_inSampleRate = inSampleRate ;
    m_outSampleRate = outSampleRate ;
    k = 2 ;
    while( m_inSampleRate/k > outSampleRate )
        k++ ;

    DecimFactor = k-- ;
    m_outSampleRate = m_inSampleRate / DecimFactor ;
    Nrequired = FILTER_KERNEL_SIZE ;
    if( OLAS_DEBUG ) qDebug() << "inSR" <<m_inSampleRate << " DecimFactor=" << DecimFactor << " outSR=" << m_outSampleRate << " wanted was " << outSampleRate;
    H = calc_filter(m_inSampleRate,0,m_inSampleRate/(DecimFactor*2.0), Nrequired, 55.0, &NTaps) ;
    if( OLAS_DEBUG ) qDebug() << "NTaps=" << NTaps <<  Nrequired ;

    if( DEBUG_MATLAB ) {
        FILE* f = fopen( "filter_taps.dat", "wb");
        fwrite( H,sizeof(double),NTaps,f);
        fclose(f);
    }
    plan = NULL ;
    plan_rev = NULL ;
    fft_size = 0 ;
    fftin = NULL ;

    _H = NULL ;
    _H0 = NULL ;
    datas_in = NULL ;
    data_size = 0 ;
    wr_pos = 0 ;
    mix_offset = 0 ;
    m_center_freq = 0 ;
    mix_phase = 0 ;
    ttl_put = ttl_read = 0 ;
    apply_postmixer = false ;
    m_band_start = 0 ;
    m_band_bins = 0 ;
}

void OverlapSave::reset() {
    if( OLAS_DEBUG ) {
        qDebug() << "OverlapSave::reset()" ;
        qDebug() << " out_wpos=" << out_wpos ;
        qDebug() << " rd_pos=" << rd_pos ;
        qDebug() << " wr_pos=" << wr_pos ;
        qDebug() << " decim_r_pos=" << decim_r_pos ;
    }
    out_wpos = rd_pos =  0 ;
    wr_pos = 0 ; //overlap ;
    memset( (void *)datas_in, 0, data_size * sizeof(TYPECPX));
    decim_r_pos = 0 ;
    wr_pos = overlap ;
    mix_phase = 0 ;
    apply_postmixer = false ;
    if( OLAS_DEBUG )
        qDebug() << "ttl_put=" << ttl_put << " ttl_read=" << ttl_read ;
}

int OverlapSave::getOLASOutSampleRate() {
    return( m_outSampleRate );
}

float OverlapSave::getCenterOfWindow() {
    return( m_center_freq );
}

void OverlapSave::setCenterOfWindow( float freq ) {

    fftwf_complex tmp[fft_size] ;

    if( OLAS_DEBUG ) qDebug() << "OverlapSave::setCenterOfWindow() freq = " << freq ;
    float bin_width = m_inSampleRate / fft_size ;

    if( fabs(freq) < bin_width ) {
        memcpy( (void *)_H, (void *)_H0, fft_size*sizeof(fftwf_complex) ) ;
        m_center_freq = 0 ;
        mix_phase = 0 ;
        mix_offset = 0 ;
        apply_postmixer = false ;
        m_band_start = fft_size - (int)floorf( m_outSampleRate/(2*bin_width) );
        return ;
    }

    m_center_freq = freq ;

    int decal = (int)floorf( freq / bin_width );
    mix_phase = 0 ;
    mix_offset = -freq/m_inSampleRate*2*M_PI ; //
    apply_postmixer = true ;

    for( int i=0 ; i < fft_size ; i++ ) {
        int idx_dest = (i + decal) % fft_size ;
        if( idx_dest < 0 ) {
            idx_dest = fft_size + idx_dest ;
        }
        tmp[idx_dest][0] = _H0[i][0];
        tmp[idx_dest][1] = _H0[i][1];
    }

    memcpy( (void *)_H, (void *)tmp, fft_size*sizeof(fftwf_complex) ) ;
    m_band_start = (int)floorf( ( freq - m_outSampleRate/2 )/bin_width) ;
    if( m_band_start < 0 ) {
        m_band_start += fft_size/2 ;
    }
    // sauvegarder le filtre pour debug matlab
    if( DEBUG_MATLAB ) {
        FILE* fw = fopen( "filter_taps_dec.dat", "wb");
        fwrite( _H,sizeof(fftwf_complex),fft_size,fw);
        fclose(fw);
    }

}

OverlapSave::~OverlapSave() {
    if( OLAS_DEBUG ) qDebug() << "OverlapSave::~OverlapSave()" ;
    if( fftin != NULL )
        fftwf_free(fftin);

    if( _H != NULL )
        fftwf_free(_H);
    if( _H0 != NULL )
        fftwf_free(_H0);

    if( plan != NULL ) {
        fftwf_destroy_plan( plan );
    }
    if( plan_rev != NULL ) {
        fftwf_destroy_plan( plan_rev );
    }
    if( datas_in != NULL )
        free(datas_in);
    if( datas_in != NULL )
        free(datas_out);
}

void OverlapSave::configure( int maxInSize , int use_fft_size ) {

    if( OLAS_DEBUG ) qDebug() << "OverlapSave::configure" <<
                maxInSize  <<
                use_fft_size ;



    if( use_fft_size != fft_size ) {
        if( fftin != NULL )
            fftwf_free(fftin);


        if( _H != NULL )
            fftwf_free(_H);
        if( _H0 != NULL )
            fftwf_free(_H0);

        if( plan != NULL ) {
            fftwf_destroy_plan( plan );
        }
        if( plan_rev != NULL ) {
            fftwf_destroy_plan( plan_rev );
        }

        fft_size = use_fft_size;
        _H = (fftwf_complex *)fftwf_malloc(sizeof(fftwf_complex) * fft_size);
        _H0 = (fftwf_complex *)fftwf_malloc(sizeof(fftwf_complex) * fft_size);
        fftin = (fftwf_complex *)fftwf_malloc(sizeof(fftwf_complex) * fft_size);

        if( OLAS_DEBUG ) qDebug() << "OverlapSave::configure() preparing FFT plans... please wait" ;
        plan = fftwf_plan_dft_1d(fft_size, fftin, fftin, FFTW_FORWARD, FFTW_ESTIMATE );
        plan_rev = fftwf_plan_dft_1d(fft_size, fftin, fftin, FFTW_BACKWARD, FFTW_ESTIMATE );
        if( OLAS_DEBUG ) qDebug() << "OverlapSave::configure() preparing FFT plans ok" ;

        data_size = (maxInSize + fft_size + NTaps );
        if( OLAS_DEBUG ) qDebug() << "OverlapSave::configure() using data_size=" << data_size ;
        // recopier coeffs du filtre dans entree FFT
        for( int i=0; i < NTaps ; i++ ) {
             fftin[i][0] = H[i] * 1.0/ fft_size ;
             fftin[i][1] = 0 ;
        }
        // padder à 0 jusqu'a FFT_SIZE
        for( int i=NTaps; i < fft_size ; i++ ) {
             fftin[i][0] = 0 ;
             fftin[i][1] = 0 ;
        }
        // calculer une bonne fois pour toute la FFT du filtre
        fftwf_execute( plan );
        // conserver la valeur
        memcpy( (void *)_H, (void *)fftin, fft_size*sizeof(fftwf_complex) ) ;
        memcpy( (void *)_H0, (void *)fftin, fft_size*sizeof(fftwf_complex) ) ;
    }

    if( datas_in != NULL )
        free(datas_in);
    if( datas_in != NULL )
        free(datas_out);

    datas_in = (TYPECPX*)malloc( data_size * sizeof(TYPECPX));
    if( datas_in == NULL ) {
        qDebug() << "OverlapSave::configure() Critical error : cannot malloc datas_in "  ;
        return ;
    }
    datas_out = (TYPECPX*)malloc( data_size * sizeof(TYPECPX));
    if( datas_out == NULL ) {
        qDebug() << "OverlapSave::configure() Critical error : cannot malloc datas_out"  ;
        return ;
    }


    overlap = NTaps - 1 ;
    decim_r_pos = 0 ;
    out_wpos = rd_pos =  0 ;
    wr_pos = overlap ;
    memset( (void *)datas_in, 0, data_size * sizeof(TYPECPX));
    apply_postmixer = false ;

    float bin_width = m_inSampleRate / fft_size ;
    // estimate the fft bins of our band.
    // Warning... using fftw -> positive spectrum is [0...N/2[ and negative [N/2...N]
    m_band_start = fft_size - (int)floorf( m_outSampleRate/(2*bin_width) ); ;
    m_band_bins  = (int)floorf( m_outSampleRate / bin_width );

    if( OLAS_DEBUG ) qDebug() << "OverlapSave::configure() finished" ;
}

int OverlapSave::put( TYPECPX *insamples, int length ) {
    long buff_len = 0 ;
    long end = wr_pos + length ;
    if( end >= data_size ) {
        qDebug() << "OverlapSave::put() too long"  ;
        return(INPUT_TOO_LONG); // too long
    }
    ttl_put += length ;
    TYPECPX *ptw = datas_in ;
    ptw += wr_pos ;
    memcpy( (void *)ptw, (void *)insamples, length * sizeof(TYPECPX));
    wr_pos = end ;
    buff_len = wr_pos - rd_pos ;
    if( buff_len < fft_size ) {
        return(NEED_MORE_DATA);
    }
    //qDebug() << "OverlapSave::put() apply_postmixer=" << apply_postmixer ;
    step1();
    return(GET_DATA_OUT);
}

// decaler le pointeur d'entrée de X échantillons à Fech in, > ou < 0
// recalage temporel - synchro de plusieurs streams...
// exemple : Fin à 10 MHz : 1 ech = 100 nS
bool OverlapSave::shiftInTimeDomain(int samples) {
     int tmp = wr_pos + samples ;
     if( tmp < 0 ) return( false );
     if( tmp >= data_size ) return( false );
     wr_pos = tmp ;
     return( true );
}

// decaler le pointeur d'entrée de X échantillons à Fech out, > ou < 0
// recalage temporel - synchro de plusieurs streams...
// exemple : Fout à 100 KHz : 1 ech = 10 µS
bool OverlapSave::shiftOutTimeDomain(int samples) {
    long tmp = out_wpos + samples ;
    if( tmp < 0 ) return( false );
    if( tmp >= data_size ) return( false );
    out_wpos = tmp ;
    return( true );
}

bool OverlapSave::shiftOutPhase( float angleRadians ) {
    mix_phase += angleRadians ;
    return( true );
}

void OverlapSave::razOutPhase() {
    mix_phase = 0 ;
}

void OverlapSave::shiftOutputCenterFrequency( float offset_hz ) {
    mix_offset += offset_hz/m_inSampleRate*2*M_PI ;
}

// Faire la FFT d'un bloc de signal d'entree
// puis avancer de la longeur de la FFT - la longeur du filtre
void OverlapSave::step1() {
    long buff_len = wr_pos - rd_pos ;
    TYPECPX* r = datas_in ;


    while( buff_len >= fft_size) {
        // recopier dans buffer d'entree FFT les données temporelle 'in'
        memcpy( (void *)fftin, (void *)datas_in, fft_size * sizeof(TYPECPX));
        // calculer la FFT du signal
        fftwf_execute( plan );



        // repositionner le buffer interne : décalage
        buff_len = wr_pos - fft_size + overlap ;
        r += fft_size - overlap ;
        if( buff_len > 0 )
            memcpy( (void *)datas_in, (void *)r, buff_len*sizeof(TYPECPX));
        rd_pos = 0 ;
        wr_pos = buff_len ;

        step2();
        r = datas_in ;
    }
}



// faire le produit de Kroeneker entre la FFT du signal entrant
// et la FFT du filtre
/* revenir dans le domaine temporel et faire le mixage
 * Attention... on fait le mix pendant la décimation donc on doit
 * prendre les bonnes valeurs de l'OL aux bons instants (ceux où on décime)
 * laisser les double ! sinon beaucoup de bruit à cause des fonctions sin/cos
 * permet de gagner du temps : on ne mixe que les données sorties, pas tout
 */
void OverlapSave::step2() {
    // float power ;
    double tmp_phase ;
    long i,p ;

    if( out_wpos >= data_size ) {
        return ;
    }

    // estimation puissance dans la sous bande utile
//    power = 0 ;
//    for( i=0 ; i < m_band_bins ; i++ ) {
//         p = (m_band_start+i) % fft_size ;
//         float I = fftin[p][0] ;
//         float Q = fftin[p][1] ;
//         power += sqrtf( I*I + Q*Q );
//    }
//    power /= m_band_bins ;

    // produit fft(filtre) .* fft( signal )
    for( i=0 ; i < fft_size ; i++ ) {
        float a = fftin[i][0];
        float b = fftin[i][1];
        float c = _H[i][0] ;
        float d = _H[i][1] ;
        fftin[i][0] = a*c - b*d ;
        fftin[i][1] = a*d + b*c ;
    }

    // ifft( fft(filtre) .* fft( signal ) )
    fftwf_execute( plan_rev );


    // mix + decim en une seule fois
    p = decim_r_pos % (fft_size - overlap);
    p += overlap ;
    while( (p < fft_size ) && (out_wpos < data_size)) {
       // laisser les doubles !
        if( apply_postmixer ) {
            double a = (double)fftin[p][0]; // fftout
            double b = (double)fftin[p][1];
            tmp_phase = mix_phase + p*mix_offset ;
            if( fabs(tmp_phase) > 2*M_PI )
                        tmp_phase -= ((tmp_phase>0) ? 2*M_PI : -2*M_PI);
            double c = cos(tmp_phase) ;
            double d = sin(tmp_phase) ;

            datas_out[out_wpos].re = (float)(a*c - b*d) ; //fftout[p][0] ;
            datas_out[out_wpos].im = (float)(a*d + b*c) ; //fftout[p][1] ;
        } else {
            datas_out[out_wpos].re = fftin[p][0];
            datas_out[out_wpos].im = fftin[p][1] ;
        }
        out_wpos++ ;
        p += DecimFactor ;
        decim_r_pos += DecimFactor ;
    }

    // remettre à jour le compteur de phase pour les mixages suivants
    if( apply_postmixer ) {
        mix_phase += (fft_size-overlap)*mix_offset ;
        if( fabs(mix_phase) > 2*M_PI )
                mix_phase -= ((mix_phase>0) ? 2*M_PI : -2*M_PI);
    }


}

int OverlapSave::get( TYPECPX *data , int max_read, int min_read) {
    long remain = out_wpos ;


    if( remain == 0 ) {
        return(NO_OUT_DATA);
    }

    if( min_read > 0 ) {
        if( min_read > remain ) {
            return( NO_OUT_DATA );
        }
    }

    if( max_read > remain ) {
        max_read = remain ;
    }

    memcpy( (void *)data, datas_out, max_read * sizeof(TYPECPX));
    remain -= max_read ;
    if( remain > 0 ) {
        TYPECPX* ptr = datas_out ;
        ptr += max_read ;
        memcpy( (void *)datas_out, (void *)ptr, remain * sizeof(TYPECPX));
        out_wpos = remain ;
    } else {
        out_wpos = 0 ;
    }
    ttl_read += max_read ;
    return( max_read );
}

void OverlapSave::pushback( TYPECPX* data, int count ) {
    int i = 0 ;
    while( count-- && (out_wpos < data_size) ) {
        datas_out[out_wpos++] = data[i++] ;
    }
}

//#define K_PI (3.14159265358979323846)
//%  Fs=Sampling frequency
//%      * Fa=Low freq ideal cut off (0=low pass)
//%      * Fb=High freq ideal cut off (Fs/2=high pass)
//%      * Att=Minimum stop band attenuation (>21dB)
double *OverlapSave::calc_filter(double Fs, double Fa, double Fb, int M, double Att , int *Ntaps) {
    int k, j ;

    int len = 2*M + 1;
    double Alpha, C,x,y ;
    double *_H, *W ;

    _H = (double *)malloc( len * sizeof( double ));
    Alpha = .1102*(Att-8.7);

    // compute kaiser window of length 2M+1
    W = (double *)malloc( len * sizeof(double));
    if( W == NULL ) {
        qDebug() << "malloc error ~1" ;
        return(NULL);
    }
    C = bessi0(Alpha*M_PI);

    for( k=0 ; k < len ; k++ ) {
        y = k*1.0/M - 1.0 ;
        x = M_PI*Alpha*sqrt( 1 - y*y) ;
        W[k] = bessi0(x)/C ;
        //printf("h(%d)=%0.8f;\n", k+1, W[k]);
    }

    k = 0 ;
    for( j=-M ; j <= M ; j++ ) {
        if( j == 0 ) {
            _H[k] = 2*(Fb-Fa)/Fs;
        } else {
            _H[k] = 1/(K_PI*j)*(sin(2*M_PI*j*Fb/Fs)-sin(2*M_PI*j*Fa/Fs))*W[k];
        }
        k++ ;
    }

    *Ntaps = len ;
//    for( k=0 ; k < len ; k++ ) {
//        printf("h(%d)=%0.8f;\n", k+1, H[k]*1000000);
//    }
    free(W);
    return( _H );
}

double OverlapSave::bessi0( double x )
/*------------------------------------------------------------*/
/* PURPOSE: Evaluate modified Bessel function In(x) and n=0.  */
/*------------------------------------------------------------*/
{
   double ax,ans;
   double y;


   if ((ax=fabs(x)) < 3.75) {
      y=x/3.75,y=y*y;
      ans=1.0+y*(3.5156229+y*(3.0899424+y*(1.2067492
         +y*(0.2659732+y*(0.360768e-1+y*0.45813e-2)))));
   } else {
      y=3.75/ax;
      ans=(exp(ax)/sqrt(ax))*(0.39894228+y*(0.1328592e-1
         +y*(0.225319e-2+y*(-0.157565e-2+y*(0.916281e-2
         +y*(-0.2057706e-1+y*(0.2635537e-1+y*(-0.1647633e-1
         +y*0.392377e-2))))))));
   }
   return ans;
}
