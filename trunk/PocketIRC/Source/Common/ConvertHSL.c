#include <windows.h>

#include "ConvertHSL.h"

void RGBtoHSL( COLORREF rgb, double *H, double *S, double *L )
{
	double delta;
	double r = (double)GetRValue(rgb)/255;
	double g = (double)GetGValue(rgb)/255;
	double b = (double)GetBValue(rgb)/255;
	double cmax = max(r,max(g,b));
	double cmin = min(r,min(g,b));
	*L=(cmax+cmin)/2.0;
	if(cmax==cmin) {
		*S = 0;
		*H = 0; // it's really undefined
	} else {
		if(*L < 0.5)
			*S = (cmax-cmin)/(cmax+cmin);
		else
			*S = (cmax-cmin)/(2.0-cmax-cmin);
		delta = cmax - cmin;
		if(r==cmax)
			*H = (g-b)/delta;
		else if(g==cmax)
			*H = 2.0 +(b-r)/delta;
		else
			*H = 4.0+(r-g)/delta;
		*H /= 6.0;
		if(*H < 0.0)
			*H += 1;
	}
}

static double HuetoRGB(double m1, double m2, double h )
{
	if( h < 0 ) h += 1.0;
	if( h > 1 ) h -= 1.0;
	if( 6.0*h < 1 )
		return (m1+(m2-m1)*h*6.0);
	if( 2.0*h < 1 )
		return m2;
	if( 3.0*h < 2.0 )
		return (m1+(m2-m1)*((2.0/3.0)-h)*6.0);
	return m1;
}

COLORREF HSLtoRGB( double H, double S, double L )
{
	double r,g,b;
	double m1, m2;
	
	if(S==0) {
		r=g=b=L;
	} else {
		if(L <=0.5)
			m2 = L*(1.0+S);
		else
			m2 = L+S-L*S;
		m1 = 2.0*L-m2;
		r = HuetoRGB(m1,m2,H+1.0/3.0);
		g = HuetoRGB(m1,m2,H);
		b = HuetoRGB(m1,m2,H-1.0/3.0);
	} 
	return RGB((BYTE)(r*255),(BYTE)(g*255),(BYTE)(b*255));
}