// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Mission Control for Windows 11 by pixeru
// Source code originally by Emcee (https://sourceforge.net/projects/mcsoft/)
// Extensive development by pixeru due to discontinued development of original project.
// Copyright (c) pixeru. All rights reserved.
#include "stdafx.h"
#include "Emcee.h"
#include "McBackingW.h"
#include "McZoomW.h"
#include "McWindowMgr.h"

using namespace Gdiplus;

#pragma unmanaged

// Window to create the "darkening" effect during a zoom.

McBackingW::McBackingW()
{
	isActivated = FALSE;
}

McBackingW::~McBackingW()
{
	Deactivate( );
	DestroyWindow( getHwnd() );
}

void McBackingW::Activate( )
{
	if (!isActivated)
	{
		SetWindowText( getHwnd( ), L"BackingW" );
		isActivated = TRUE;
	}
}

void McBackingW::Deactivate( )
{
	if (isActivated)
	{
		isActivated = FALSE;
		ShowWindow( getHwnd( ), SW_HIDE );
		SetWindowText( getHwnd( ), L"<idle>" );
	}
}

LRESULT McBackingW::HandleMessage( UINT uMsg, WPARAM wParam, LPARAM lParam )
{

	if (uMsg == WM_SYSCOMMAND)
		return 0;

	switch ( uMsg )
	{
	case WM_CREATE:
	
		return 0;

	case WM_PAINT:
		if (isActivated)
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint( m_hwnd, &ps );
			if (McWindowMgr::getZoomW())
			{
				SetLayeredWindowAttributes(getHwnd(), 0, McWindowMgr::getZoomW()->getAlpha(), LWA_ALPHA);
			}
			McRect rr;
			if (MC::getMM( ))
			{
				rr.set( McMonitorsMgr::getMainMonitor( )->getDesktopSize( ) );
				rr.offset( -rr.left, -rr.top );
			}
			else
				rr.set( (McMonitorsMgr::getMainMonitor( )->getMonitorSize( )) );
			FillRect( hdc,
				&rr,
				(HBRUSH) GetStockObject( BLACK_BRUSH ) );
			EndPaint( m_hwnd, &ps );
			return 0;
		}

	default:
		return DefWindowProc( m_hwnd, uMsg, wParam, lParam );
		
	}
	return TRUE;
}


