#include "stdafx.h"
#include "McColorTools.h"

// Utilities for manipulating colors

int McColorTools::getRGBfromHSL( int xh, int xs, int xl, int *xr, int *xg, int *xb )
{
	double h,v;
	double r,g,b;

	if ( xh == 255 )
		h=254.0/255.0;
	else
		h = xh/255.0;
	double s = xs/255.0;
	double l = xl/255.0;
 
    r = l;   // default to gray
    g = l;
    b = l;
    v = (l <= 0.5) ? (l * (1.0 + s)) : (l + s - l * s);
    if (v > 0)
    {
		double m;
        double sv;
		int sextant;
        double fract, vsf, mid1, mid2;
 
        m = l + l - v;
        sv = (v - m ) / v;
        h *= 6.0;
        sextant = (int)h;
        fract = h - sextant;
        vsf = v * sv * fract;
        mid1 = m + vsf;
        mid2 = v - vsf;
        switch (sextant)
        {
         case 0:
            r = v;
            g = mid1;
            b = m;
            break;
		case 1:
			r = mid2;
			g = v;
			b = m;
			break;
		case 2:
			r = m;
			g = v;
			b = mid1;
			break;
		case 3:
			r = m;
			g = mid2;
			b = v;
			break;
		case 4:
			r = mid1;
			g = m;
			b = v;
			break;
		case 5:
			r = v;
			g = m;
			b = mid2;
			break;
		}
	}

	*xr = (int)floor(.5+r*255);
	*xg = (int)floor(.5+g*255);
	*xb = (int)floor(.5+b*255);

	return getContrast( xh, xs, xl );
}

void McColorTools::getHSLfromRGB( int xr, int xg, int xb, int *xh, int *xs, int *xl )
{
	double r = xr/255.0;
	double g = xg/255.0;
	double b = xb/255.0;
	double v;
	double m;
	double vm;
	double r2, g2, b2;
 
	double h = 0; // default to black
	double s = 0;
	double l = 0;
	v = max(r,g);
	v = max(v,b);
	m = min(r,g);
	m = min(m,b);
	l = (m + v) / 2.0;
	if (l > 0.0)
	{
		vm = v - m;
		s = vm;
		if (s > 0.0)
		{
			s /= (l <= 0.5) ? (v + m ) : (2.0 - v - m) ;
            r2 = (v - r) / vm;
            g2 = (v - g) / vm;
            b2 = (v - b) / vm;
            if ( fabs(r-v)<0.0001)
                  h = (fabs(g-m)<0.0001 ? 5.0 + b2 : 1.0 - g2);
            else if (fabs(g-v)<0.0001)
                  h = (fabs(b-m)<0.0001 ? 1.0 + r2 : 3.0 - b2);
            else
                  h = (fabs(r-m)<0.0001 ? 3.0 + g2 : 5.0 - r2);
            h /= 6.0;
		}
	}

	*xh = (int)floor(.5+255*h);
	*xs = (int)floor(.5+255*s);
	*xl = (int)floor(.5+255*l);
}

int McColorTools::getComplement( int r_in, int g_in, int b_in, int *r_out, int *g_out, int *b_out )
{
	int h,s,l;

	getHSLfromRGB( r_in, g_in, b_in, &h, &s, &l );
	h += 85;
	if ( h >255 ) h-=255;
	if ( s < 10 )
	{
		s = 155;
		h=rand()%253+1;
		l=80;
	}
	return getRGBfromHSL( h,s,l,r_out,g_out,b_out );
}

int McColorTools::getTextIntensity( int r, int g, int b )
{
	int h,s,l;
	getHSLfromRGB( r, g, b, &h, &s, &l );
	return getContrast( h, s, l );
}

int McColorTools::getLuminosity( int r, int g, int b )
{
	int h, s, l;
	getHSLfromRGB( r, g, b, &h, &s, &l );
	return l;
}

int McColorTools::deadenColor( int *r, int *g, int *b, BOOL always )
{
	int h,s,l;
	getHSLfromRGB( *r, *g, *b, &h, &s, &l );
	if ( always || l > 90 || s > 225 )
	{
		s = (4*s)/5;
		l = min((3*l)/5,90);
	}
	return getRGBfromHSL( h,s,l,r,g,b );
}

#define _MC_COL_THRESH 140

int McColorTools::getContrast( int h, int s, int l )
{
	return l>_MC_COL_THRESH? 0 : 255;
}
