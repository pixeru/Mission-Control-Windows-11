#pragma once
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
