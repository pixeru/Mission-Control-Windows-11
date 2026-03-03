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
#include "McFadingW.h"
#include "McWindowMgr.h"

using namespace Gdiplus;

#pragma unmanaged

// Because of a bug (or feature) introduced in W10 1607, displaying DWM Thumbnails with partial opacity no 
// longer works reliably.   So this is alternative code to do partial transparency by displaying all
// partially opaque thumbnails on this, the Fading window, a layered window whose opacity is
// varied during some transitions.    The Fading window is positioned under the Main window but above the
// Background window.

McFadingW::McFadingW( )
{
	isActivated = FALSE;
}

McFadingW::~McFadingW( )
{
	Deactivate( );
	DestroyWindow( getHwnd( ) );
}

void McFadingW::Activate( )
{
	if (!isActivated)
	{
		SetWindowText( getHwnd( ), L"FadingW" );
		isActivated = TRUE;
	}
}

void McFadingW::Deactivate( )
{
	if (isActivated)
	{
		isActivated = FALSE;
		ShowWindow( getHwnd( ), SW_HIDE );
		SetWindowText( getHwnd( ), L"<idle>" );
	}
}

LRESULT McFadingW::HandleMessage( UINT uMsg, WPARAM wParam, LPARAM lParam )
{

	if (uMsg == WM_SYSCOMMAND)
		return 0;
	
	switch (uMsg)
	{
	case WM_CREATE:

		return 0;

	default:
		return DefWindowProc( m_hwnd, uMsg, wParam, lParam );

	}
	return TRUE;
}


