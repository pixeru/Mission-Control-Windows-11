#pragma once
#include "stdafx.h"
#include "Emcee.h"

// Class used to manipulate colors

class McColorTools
{
public:

	static int getRGBfromHSL( int h, int s, int l, int *r, int *g, int *b );
	static void getHSLfromRGB( int r, int g, int b, int *h, int *s, int *l );
	static int deadenColor( int *r, int *g, int *b, BOOL = FALSE );
	static int getComplement( int r_in, int g_in, int b_in, int *r_out, int *g_out, int *b_out );
	static int getTextIntensity( int r, int g, int b );
	static int getLuminosity( int r, int g, int b );

private:

	static int getContrast( int h, int s, int l );

};
