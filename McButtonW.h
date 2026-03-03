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
#include "McBaseW.h"
#include "McBgW.h"

#pragma unmanaged

class McButtonW : public McBaseW<McButtonW>
{
public:

	McButtonW( );
	~McButtonW( );
	void Show( int x, int y );
	void Lower( );
	void Raise( );
	void Hide( );
	void preActivate( WCHAR *, McBgW *, int, int, Bitmap *, Bitmap *, WORD );
	void Activate( COLORREF bgc, COLORREF bghc = 0 );
	void Deactivate( );
	LRESULT HandleMessage( UINT, WPARAM, LPARAM );

private:

	void doRepaint( );
	PCWSTR	ClassName( ) const { return L"Emcee.ButtonW"; };
	DWORD runThread( ) { return 0; }

public:
private:

	BOOL hasMouse;
	BOOL isVisible;
	Size buttonSize;
	Point buttonPos;
	Bitmap *Bitmap1;
	Bitmap *Bitmap2;
	McBgW *bgWindow;
	SolidBrush *bgBrush;
	SolidBrush *bghBrush;
	WORD command;
	WCHAR buttonName[100];
};