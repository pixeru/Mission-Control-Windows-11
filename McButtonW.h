#pragma once
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Mission Control for Windows 11 by pixeru
// Source code originally by Emcee (https://sourceforge.net/projects/mcsoft/)
// Extensive development by pixeru due to discontinued development of original project.
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