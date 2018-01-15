//////////////////////////////////////////////////////////////////////
// datatypes.h: Common data type declarations
//
// History:
//	2010-09-15  Initial creation MSW
//	2011-03-27  Initial release
//	2013-07-28  Added single/double precision math macros
//////////////////////////////////////////////////////////////////////
#ifndef DATATYPES_H
#define DATATYPES_H
#include <math.h>

//comment out to use single precision math
//#define USE_DOUBLE_PRECISION
#include <QtGlobal>

//define single or double precision reals and complex types
typedef float tSReal;
typedef double tDReal;

typedef struct _sCplx
{
    tSReal re;
    tSReal im;
}tSComplex;

typedef struct _dCplx
{
    tDReal re;
    tDReal im;
}tDComplex;

typedef struct _isCplx
{
    int re;
    int im;
}tStereo16;

#ifdef USE_DOUBLE_PRECISION
 #define TYPEREAL tDReal
 #define TYPECPX	tDComplex
#else
 #define TYPEREAL tSReal
 #define TYPECPX	tSComplex
#endif

#ifdef USE_DOUBLE_PRECISION
 #define MSIN(x) sin(x)
 #define MCOS(x) cos(x)
 #define MPOW(x,y) pow(x,y)
 #define MEXP(x) exp(x)
 #define MFABS(x) fabs(x)
 #define MLOG(x) log(x)
 #define MLOG10(x) log10(x)
 #define MSQRT(x) sqrt(x)
 #define MATAN(x) atan(x)
 #define MFMOD(x,y) fmod(x,y)
 #define MATAN2(x,y) atan2(x,y)
#else
 #define MSIN(x) sinf(x)
 #define MCOS(x) cosf(x)
 #define MPOW(x,y) powf(x,y)
 #define MEXP(x) expf(x)
 #define MFABS(x) fabsf(x)
 #define MLOG(x) logf(x)
 #define MLOG10(x) log10f(x)
 #define MSQRT(x) sqrtf(x)
 #define MATAN(x) atanf(x)
 #define MFMOD(x,y) fmodf(x,y)
 #define MATAN2(x,y) atan2f(x,y)
#endif

#define TYPESTEREO16 tStereo16
#define TYPEMONO16 int

//#define K_2PI (8.0*atan(1))	//maybe some compilers are't too smart to optimize out
#define K_2PI (2.0 * 3.14159265358979323846)
#define K_PI (3.14159265358979323846)
#define K_PI4 (K_PI/4.0)
#define K_PI2 (K_PI/2.0)
#define K_3PI4 (3.0*K_PI4)


inline TYPECPX cmult( TYPECPX a, TYPECPX b ) {
    TYPECPX w ;
    w.re = a.re*b.re - a.im*b.im ;
    w.im = a.re*b.im + a.im*b.re ;
    return( w );
}

inline TYPECPX cadd( TYPECPX a, TYPECPX b ) {
    TYPECPX w ;
    w.re = a.re + b.re  ;
    w.im = a.im + b.im ;
    return( w );
}

inline TYPECPX csub( TYPECPX a, TYPECPX b ) {
    TYPECPX w ;
    w.re = a.re - b.re  ;
    w.im = a.im - b.im ;
    return( w );
}

inline TYPECPX cmulscal( TYPEREAL a, TYPECPX b ) {
    TYPECPX w ;
    w.re = a * b.re  ;
    w.im = a * b.im ;
    return( w );
}


inline TYPEREAL cmodulus( TYPECPX a ) {
    return( a.re*a.re + a.im*a.im );
}

inline TYPECPX conj( TYPECPX a ) {
    TYPECPX w ;
    w.re = a.re ;
    w.im = -a.im ;
    return( w );
}

inline TYPECPX cinverse( TYPECPX a ) {
    TYPECPX w ;
    TYPEREAL m = cmodulus(a);
    w.re = a.re / m ;
    w.im = -a.im / m ;
    return( w );
}

inline TYPECPX cone() {
    TYPECPX w ;
    w.im = 0 ;
    w.re = 1 ;
    return( w );
}

inline float min( float x, float y ) {
    if( x <= y ) return( x ) ;
    return( y );
}

inline float max( float x, float y ) {
    if( x >= y ) return( x ) ;
    return( y );
}


#endif // DATATYPES_H
