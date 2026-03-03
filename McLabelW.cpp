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
#include "McThumbW.h"
#include "McLabelW.h"
#include "McWItem.h"
#include "McColorTools.h"
#include "McModernAppMgr.h"
#include "McMonitorsMgr.h"
#include "McMainW.h"
#include "McWindowMgr.h"

#pragma unmanaged

// Special window to put a label on the "active" thumbnail.

using namespace Gdiplus;

BOOL McLabelW::labelFontStale = FALSE;

McLabelW::McLabelW()
{
	isActivated = FALSE;

	thumbWindow = NULL;

	paused = FALSE;
	
	monitor = NULL;
	disabled = FALSE;

	labelFont = NULL;

	mouseWasOverX = FALSE;
	mouseButtonDown = FALSE;
	bgColor = NULL;
	solidBrush = textBrush = NULL;

	haveMouse = FALSE;
	isVisible = FALSE;

	MC::loadImage( IDI_BLACKX, &blackImage );
	MC::loadImage( IDI_REDX, &redImage );

	Gdiplus::Color *textColor = new Gdiplus::Color( 255, 255, 255, 255 );
	textBrush = new SolidBrush( *textColor );
	delete textColor;

	xsize = GetSystemMetrics( SM_CYICON );
	labelPadding = 1;
	isize = xsize;

}

McLabelW::~McLabelW()
{
	Deactivate( );
	if ( labelFont )
		DeleteObject( labelFont );
	if (bgColor) delete bgColor;
	if (solidBrush) delete solidBrush;
	if (textBrush) delete textBrush;
	if (blackImage) delete blackImage;
	if (redImage) delete redImage;
	DestroyWindow( getHwnd() );
}

void McLabelW::Activate( )
{
	if (!isActivated)
	{
		disabled = MC::getProperties( )->getDisableLabels( );
		SetWindowText( getHwnd( ), L"LabelW" );
		isActivated = TRUE;
	}
}

void McLabelW::Deactivate( )
{
	if (isActivated)
	{
		paused = FALSE;
		mouseWasOverX = FALSE;
		mouseButtonDown = FALSE;
		Hide( );
		SetWindowText( getHwnd( ), L"<idle>" );
		isActivated = FALSE;
	}
}



void McLabelW::NewLabelFontSize()
{
	labelFontStale = TRUE;
}

void McLabelW::Show( McThumbW *_thumbWindow )
{
	if (paused || disabled || !getHwnd( ) || MC::getState()!=MCS_Display) return;

	if (thumbWindow == _thumbWindow && isVisible)
		return;

	if (thumbWindow)						
		Hide( );

	thumbWindow = _thumbWindow;
	windowItem = thumbWindow->getOwner( );

	mouseWasOverX = FALSE;
	mouseButtonDown = FALSE;

	McMonitorsMgr *oldMonitor = monitor;
	if (MC::getMM( ) && !windowItem->getIsOnAppBar( ))
		monitor = McMonitorsMgr::getMonitor( windowItem->getAppMonitor( ) );
	else
		monitor = McMonitorsMgr::getMainMonitor( );

	if (monitor != oldMonitor)
		NewLabelFontSize( );

	if (McLabelW::labelFontStale)
	{
		labelFontStale = FALSE;
		if (labelFont)
		{
			DeleteObject( labelFont );
			labelFont = NULL;
		}
	}

	if (!labelFont )
	{
		WCHAR *fontName = L"Segoe UI";
		LOGFONT lf = { 0 };
		lf.lfHeight = (int) floor( monitor->getObjectScale( )*(MC::getProperties( )->getLabelFontSize( ) - 4) );
		lf.lfWeight = FW_MEDIUM;
		lf.lfQuality = CLEARTYPE_QUALITY;
		lf.lfOutPrecision = OUT_TT_PRECIS;
		lf.lfPitchAndFamily = VARIABLE_PITCH | FF_SWISS;
		wcscpy_s( lf.lfFaceName, LF_FACESIZE,
			fontName );

		labelFont = CreateFontIndirect( &lf );
	}

	COLORREF oldRGB;

	if (bgColor)
	{
		oldRGB = bgColor->ToCOLORREF( );
	}

	COLORREF c = 
		windowItem->getIsCloaked() && !MC::inTabletMode() ?
		   McModernAppMgr::GetPaletteColor( McModernAppMgr::GetPaletteCount()-1 ) : McModernAppMgr::GetPaletteColor(3);

	if (!bgColor || (oldRGB != c))
	{
		if (bgColor)
			delete bgColor;
		if (solidBrush)
			delete solidBrush;

		bgColor = new Gdiplus::Color( 255, GetRValue( c ), GetGValue( c ), GetBValue( c ) );
		solidBrush = new SolidBrush( *bgColor );
	}

	doMeasurements( );

	SetWindowPos( getHwnd( ), HWND_TOP, labelDimensions.left, labelDimensions.top,
		labelDimensions.getWidth( )/*+(thumbWindow->getOwner()->getIsOnAppBar()?1:0)*/, labelDimensions.getHeight( ), SWP_SHOWWINDOW );

	isVisible = TRUE;
}

void McLabelW::Hide( )
{
	if (paused) return;
	mouseWasOverX = FALSE;
	mouseButtonDown = FALSE;
	if (!thumbWindow)
		return;
	thumbWindow = NULL;
	
	ShowWindow( getHwnd(), SW_HIDE );
	isVisible = FALSE;
}

void McLabelW::doMeasurements( )
{
	McWItem *wItem = thumbWindow->getOwner( );

	McRect mr( (monitor->getMonitorSize( )) );
	POINT *offsets = monitor->getOffsets( );

	McRect wr( wItem->getThumbRect( ) );

	WCHAR *labels = thumbWindow->getLabels( TRUE );

	int   maxWidth = 0;

	lineHeight = 0;

	HDC dc = GetDC( thumbWindow->getHwnd( ) );
	SelectObject( dc, labelFont );

	labelPadding = (int) floor( 0.5 + monitor->getDpi( ) / 32.0 );

	int labelWidth = wr.getWidth( )-(thumbWindow->getOwner()->getIsOnAppBar()?1:2);

	displayMask = _MCLABEL_DISPLAYALL;
	BOOL haveIcon = thumbWindow->getOwner( )->haveIcon( );
	if (labelWidth < 5 * labelPadding + 3 * xsize)
	{
		displayMask &= ~_MCLABEL_DISPLAYTEXT;
		if (labelWidth < 3 * labelPadding + 2 * xsize)
			displayMask &= ~_MCLABEL_DISPLAYICON;
	}
	else
	{
		displayMask |= _MCLABEL_DISPLAYTEXT;
		int maxLen =
			labelWidth - 4 * labelPadding - xsize - isize;
		int minlen = (int) floor( .5 + .90 * maxLen );
		// Figure out the label size in pixels

		int labelLen = 0;

		textWidth = (float)maxLen;

		Gdiplus::RectF layout( 0.0, 0.0, 10000, 10000 );
		Gdiplus::RectF bRect( 0.0, 0.0, 0.0, 0.0 );
		Gdiplus::Font font( dc );
		Gdiplus::Graphics g( dc );

		g.MeasureString( labels, -1, &font, layout, &bRect );
		size_t reduction = wcslen( labels ) - 2;
		while (bRect.Width >= maxLen && reduction > 0)
		{
			WCHAR *ol = labels;
			WCHAR c = ol[reduction];
			ol[reduction] = 0;
			labels = new WCHAR[reduction + 4];
			wcscpy_s( labels, reduction + 4, ol );
			wcscat_s( labels, reduction + 4, L"..." );
			ol[reduction] = c;
			delete ol;
			reduction -= 2;
			g.MeasureString( labels, -1, &font, layout, &bRect );
		}

		lineHeight = (int) max( lineHeight, floor( 1.0 + bRect.Height ) );
		maxWidth = (int) max( maxWidth, floor( 1.0 + bRect.Width ) );
		labelLen = (int) floor( bRect.Width );

		size_t newSize = wcslen( labels );

		// Create the newLabels

		WCHAR *newLabels = new WCHAR[1 + newSize];
		wcscpy_s( newLabels, 1 + newSize, labels );
		delete labels;

		// Now build the new labels

		thumbWindow->setLabels( newLabels );
		delete newLabels;

	}

	int labelHeight = max( labelPadding + lineHeight, 2 * labelPadding + xsize );
	int labelX, labelY;

	if (labelWidth > wr.getWidth( ))
		labelX = wr.left + (wr.getWidth( ) - labelWidth) / 2;
	else
		labelX = wr.left;
	if (wItem->getIsOnAppBar( ))
		labelY = wr.top - labelHeight;
	else
		labelY = wr.top;
	if (labelX < mr.left)
		labelX = mr.left + labelPadding;

	vCenterOffset = (int) (max( 0, labelHeight - lineHeight ) / 2.0);

	if (thumbWindow->getOwner()->getIsOnAppBar()) labelY--;
	labelDimensions.set( labelX, labelY, labelX + labelWidth, labelY + labelHeight);


	xRect.set( 0, 0, xsize, xsize);
	xRect.offset( labelWidth - labelPadding - xsize - 1, (labelDimensions.getHeight( ) - xsize - 1) / 2 );// labelPadding );

	ReleaseDC( thumbWindow->getHwnd( ), dc );
}


LRESULT McLabelW::HandleMessage( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if (uMsg == WM_SYSCOMMAND)
		return 0;
	
	switch( uMsg )
	{

	case WM_MOUSEMOVE:
	{
		if (!isActivated) break;
		if (!haveMouse)
		{
			TRACKMOUSEEVENT tme;
			tme.cbSize = sizeof( TRACKMOUSEEVENT );
			tme.dwFlags = TME_LEAVE;
			tme.hwndTrack = getHwnd( );
			TrackMouseEvent( &tme );
			haveMouse = TRUE;
		}
		BOOL mouseIsOverX = xRect.contains( GET_X_LPARAM( lParam ), GET_Y_LPARAM( lParam ) );
		if (mouseIsOverX != this->mouseWasOverX)
		{
			mouseButtonDown = FALSE;
			mouseWasOverX = mouseIsOverX;
			displayMask = _MCLABEL_DISPLAYX;
			InvalidateRect( getHwnd( ), &xRect, FALSE );
		}
	}
		return 0;

	case WM_MOUSELEAVE:
		haveMouse = FALSE;
		if (mouseWasOverX)
		{
			mouseButtonDown = FALSE;
			mouseWasOverX = FALSE;
			displayMask = _MCLABEL_DISPLAYX;
			InvalidateRect( getHwnd( ), &xRect, FALSE );
		}
		return 0;

	case WM_LBUTTONDOWN:

		if (thumbWindow && isActivated)
		{
			mouseButtonDown = TRUE;
			if ( !thumbWindow->getOwner( )->getIsOnAppBar( ))
			{
				if (!mouseWasOverX)
					SendMessage( thumbWindow->getHwnd( ), uMsg, wParam, lParam );
			}
			else
				MC::recordHwnd( thumbWindow->getHwnd( ) );
		}
		break;

	case WM_LBUTTONUP:
		if (mouseButtonDown && isActivated)
		{
			if (mouseWasOverX)
			{
				PostMessage( McWindowMgr::getMainW( )->getHwnd( ), WM_KEYDOWN, VK_KILLKEY, TRUE );
				break;
			}
			else if ( thumbWindow )
				SendMessage( thumbWindow->getHwnd( ), uMsg, wParam, lParam );
			mouseButtonDown = FALSE;
			break;
		}

	case WM_MOUSEWHEEL:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:	
	case WM_CHAR:
	case WM_DEADCHAR:
	case WM_SYSDEADCHAR:
	case WM_SYSKEYUP:
	case WM_UNICHAR:
	case WM_SYSKEYDOWN:
	case WM_KEYDOWN:
		if (thumbWindow)
			if (!thumbWindow->getOwner( )->getIsOnAppBar( ))
				SendMessage( thumbWindow->getHwnd( ), uMsg, wParam, lParam );
		break;

	case WM_PAINT:

		if ( isActivated && thumbWindow )
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint( getHwnd(), &ps );
			Graphics g( hdc );
			g.SetCompositingMode( CompositingModeSourceOver );
			g.SetCompositingQuality( CompositingQualityHighQuality );
			g.SetInterpolationMode( InterpolationModeHighQualityBicubic );

			Rect rectangle( 0, 0, labelDimensions.getWidth( ), labelDimensions.getHeight( ) );
			g.FillRectangle( solidBrush, rectangle );
			g.FillRectangle(solidBrush, rectangle);
			SelectFont( hdc, labelFont );
			Gdiplus::Font font(hdc);
			PointF origin;
			RectF layout;
			WCHAR *labels = thumbWindow->getLabels( FALSE );

			if ( displayMask & _MCLABEL_DISPLAYTEXT )
				{
				origin.X =
					(float) 2*labelPadding+isize;
				origin.Y = (float) (vCenterOffset);

				g.DrawString( labels, -1, &font, origin, textBrush );
				}

			delete labels;

			if (displayMask & _MCLABEL_DISPLAYX)
				g.DrawImage( mouseWasOverX ? redImage : blackImage,
					xRect.left, xRect.top, 
					xRect.getWidth( )-1, xRect.getHeight( )-1 );

			if (displayMask& _MCLABEL_DISPLAYICON && windowItem->haveIcon() )
			{
				int x, y;
				if (_MCLABEL_DISPLAYX)
				{
					x = labelPadding;
					y = (labelDimensions.getHeight( ) - xsize) / 2;
				}
				else
				{
					x = (labelDimensions.getWidth( ) - xsize) / 2;
					y = (labelDimensions.getHeight( ) - xsize) / 2;
				}
				windowItem->DrawIcon( hdc, x, y, xRect.getWidth( ), xRect.getHeight( ) );
				
			}

			EndPaint( getHwnd(), &ps );
			displayMask = 0;

			return S_OK;
		}
		break;

	case WM_ERASEBKGND:
		return 0;

	case WM_USER:

		McWindowMgr::FadeWindowIn( getHwnd( ), thumbWindow->getOwner()->getIsOnAppBar() ? 255 : 240 );
		return 0;

	default:
		break;
	}
	return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
}

