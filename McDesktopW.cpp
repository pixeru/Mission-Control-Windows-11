#include "stdafx.h"

#include "Emcee.h"
#include "McDesktopW.h"
#include "McWItem.h" 
#include "McMainW.h"
#include "McBgW.h"
#include "McWindowMgr.h"

// Window showing thumbnail for a single window either desktop or windows store

#pragma unmanaged

McDesktopW::McDesktopW()
{
	owner = NULL;
	isActivated = FALSE;
}

McDesktopW::~McDesktopW( )
{
	Deactivate( );
	DestroyWindow( getHwnd( ) );
}

void McDesktopW::Activate( McWItem *_owner, McRect *_rect )
{
	if (!isActivated)
	{
		owner = _owner;
		ourRect.set( _rect );
		WCHAR b[1000];
		wsprintf( b, L"DT 0x%x", _owner->getAppMonitor( ) );
		SetWindowText( getHwnd( ), b );
		isActivated = TRUE;
	}
}

void McDesktopW::Deactivate( )
{
	if (isActivated)
	{
		ShowWindow( getHwnd( ), SW_HIDE );
		SetWindowText( getHwnd( ), L"<idle>" );
		if (!isActivated) return;
		owner = NULL;
		isActivated = FALSE;
		McWindowMgr::freeDesktopW( this );
	}
}

LRESULT McDesktopW::HandleMessage( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if (uMsg == WM_SYSCOMMAND)
		return 0;
	
	return DefWindowProc( m_hwnd, uMsg, wParam, lParam );
}

void McDesktopW::setPosition( BOOL show, McRect *rect )
{
	if (rect)
		ourRect.set( rect );

	if ( show )
		SetWindowPos( getHwnd( ), GetNextWindow( McWindowMgr::getBgW()->getHwnd( ), GW_HWNDPREV ),
			ourRect.left, ourRect.top, ourRect.getWidth( ), ourRect.getHeight( ),
				SWP_NOACTIVATE | SWP_SHOWWINDOW );
}
