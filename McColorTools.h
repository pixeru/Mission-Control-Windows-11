#pragma once
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// YOU MAY USE THIS CODE AS YOU WISH FOR PERSONAL PURPOSES ONLY.
// YOU MAY NOT USE THIS CODE IN PART OR IN WHOLE IN A COMMERCIAL PRODUCT
// WITHOUT THE EXPRESS WRITTEN PERMISSION OF EMCEE APP SOFTWARE
// 9622 SANDHILL ROAD, MIDDLETON, WI 53562
// Copyright (c) Emcee App Software. All rights reserved
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