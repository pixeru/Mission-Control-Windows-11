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
#include "McButtonW.h"
#include "McMainW.h"
#include "McModernAppMgr.h"
#include "McWindowMgr.h"
#include "McPilesMgr.h"
#include "McWList.h"

#pragma unmanaged

McButtonW::McButtonW( )
{
	isVisible = FALSE;
	isActivated = FALSE;
	Bitmap1 = Bitmap2 = NULL;
	bgWindow = NULL;
	hasMouse = FALSE;
	bgBrush = NULL;
	bghBrush = NULL;
	command = 0;
}

McButtonW::~McButtonW()
{	
	hasMouse = FALSE;
	Hide( );
	if (Bitmap1)
		delete Bitmap1;
	if (Bitmap2)
		delete Bitmap2;
	if (bgBrush)
		delete bgBrush;
	if (bghBrush)
		delete bghBrush;
	isVisible = 0;
	bgWindow = NULL;
	Bitmap1 = Bitmap2 = NULL;

	DestroyWindow( getHwnd( ) );
}

void McButtonW::preActivate( WCHAR *_name, McBgW *bgW, int w, int h, Bitmap *i1, Bitmap *i2, WORD cmd )
{
	isVisible = FALSE;
	wcscpy_s( buttonName, 100, _name );
	bgWindow = bgW;
	buttonSize.Width = w;
	buttonSize.Height = h;
	if (Bitmap1) delete Bitmap1;
	Bitmap1 = i1;
	if (Bitmap2) delete Bitmap2;
	Bitmap2 = i2;
	bgBrush = bghBrush = NULL;
	command = cmd;
}

void McButtonW::Activate( COLORREF c, COLORREF hc )
{
	if (!isActivated)
	{
		Color *color = new Color( 255, GetRValue( c ), GetGValue( c ), GetBValue( c ) );
		if (bgBrush) delete bgBrush;
		bgBrush = new SolidBrush( *color );
		if (hc)
		{
			delete color;
			color = new Color( 255, GetRValue( hc ), GetGValue( hc ), GetBValue( hc ) );
		}
		if (bghBrush)
			delete bghBrush;
		bghBrush = new SolidBrush( *color );
		delete color;
		hasMouse = FALSE;
		SetWindowText( getHwnd( ), buttonName );
		isActivated = TRUE;
	}
}

void McButtonW::Deactivate( )
{
	if (isActivated)
	{
		Hide( );
		hasMouse = FALSE;
		if (bgBrush)
		{
			delete bgBrush;
			bgBrush = NULL;
		}
		if (bghBrush)
		{
			delete bghBrush;
			bghBrush = NULL;
		}
		SetWindowText( getHwnd( ), L"<idle>" );
		isActivated = FALSE;
	}
}

void McButtonW::Show( int x, int y )
{
	if (!bgWindow) return;
	if (!isActivated) return;
	buttonPos.X = x;
	buttonPos.Y = y;
	SetWindowPos( getHwnd( ), GetNextWindow( bgWindow->getHwnd( ), GW_HWNDPREV ),
		buttonPos.X, buttonPos.Y, buttonSize.Width, buttonSize.Height,
		SWP_NOACTIVATE |SWP_SHOWWINDOW );
	isVisible = TRUE;
	hasMouse = FALSE;
	doRepaint( );
}

void McButtonW::Lower( )
{
	// Put below the main window but above the bg window during animations
	if ( isVisible )
		SetWindowPos( getHwnd( ), GetNextWindow( bgWindow->getHwnd( ), GW_HWNDPREV ),
			buttonPos.X, buttonPos.Y, buttonSize.Width, buttonSize.Height, 
			SWP_NOACTIVATE | SWP_SHOWWINDOW );
}

void McButtonW::Raise( )
{
	// Put just above the main window but below all thumbnail windows

	if ( isVisible )
	SetWindowPos( getHwnd( ), GetNextWindow( McWindowMgr::getMainW( )->getHwnd( ), GW_HWNDPREV ),
	  buttonPos.X, buttonPos.Y, buttonSize.Width, buttonSize.Height, 
	  SWP_NOACTIVATE | SWP_SHOWWINDOW  );
}

void McButtonW::Hide( )
{
	if (isVisible)
	{
		ShowWindow( getHwnd( ), SW_HIDE );
		isVisible = FALSE;
	}
}

LRESULT McButtonW::HandleMessage( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if (uMsg == WM_SYSCOMMAND)
		return 0;

	switch (uMsg)
	{

	case WM_LBUTTONUP:
		if (command)
		{
			hasMouse = FALSE;
			PostMessage( McWindowMgr::getMainW( )->getHwnd( ), WM_USER, command, NULL );
			doRepaint( );
		}
		break;

	case WM_PAINT:
		if (isActivated)
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint( getHwnd( ), &ps );
			Graphics g( hdc );
			g.SetCompositingMode( CompositingModeSourceOver );
			g.SetCompositingQuality( CompositingQualityHighQuality );
			g.SetInterpolationMode( InterpolationModeHighQualityBicubic );

			Rect rectangle( 0, 0, buttonSize.Width, buttonSize.Height );
			if (hasMouse)
				g.FillRectangle( bghBrush, rectangle );
			else
				g.FillRectangle( bgBrush, rectangle );

			Bitmap *myBitmap = Bitmap1;
			if (hasMouse && Bitmap2)
				myBitmap = Bitmap2;
			g.DrawImage( myBitmap,
				0, 0,
				buttonSize.Width, buttonSize.Height );
			EndPaint( getHwnd( ), &ps );

			return S_OK;
		}
		break;

	case WM_MOUSEMOVE:

		if (MC::getIsMessageBoxActive( ))
			break;
		if (!hasMouse && isVisible )
		{
			// Mouse just entered the window
			TRACKMOUSEEVENT tme;
			tme.cbSize = sizeof( TRACKMOUSEEVENT );
			tme.dwFlags = TME_LEAVE;
			tme.hwndTrack = getHwnd( );
			tme.dwHoverTime = 50;
			TrackMouseEvent( &tme );
			hasMouse = TRUE;
			doRepaint( );
		}
		return S_OK;


	case WM_MOUSELEAVE:

		if (hasMouse && isVisible)
		{
			// Mouse just left
			hasMouse = FALSE;
			doRepaint( );
			return S_OK;
		}


	default:
		break;
	}
	return DefWindowProc( m_hwnd, uMsg, wParam, lParam );
}

void McButtonW::doRepaint( )
{
	InvalidateRect( getHwnd( ), NULL, TRUE );
	UpdateWindow( getHwnd( ) );
}
