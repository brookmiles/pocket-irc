#ifndef _CONVERTHSL_H_INCLUDED_
#define _CONVERTHSL_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif

void RGBtoHSL( COLORREF rgb, double *H, double *S, double *L );
COLORREF HSLtoRGB( double H, double S, double L );

#ifdef __cplusplus
} // extern "C"
#endif

#endif//_CONVERTHSL_H_INCLUDED_

