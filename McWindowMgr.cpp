#include "stdafx.h"
#include "Emcee.h"
#include "McWindowMgr.h"
#include "McMonitorsMgr.h"

#pragma unmanaged

// Maintain a pool of windows that can be used during invocations - this reduces cpu usage
// But, over time, unused windows in the pool are discarded - this reduces memory usage

McBgW *McWindowMgr::bgW = NULL;
McBackingW *McWindowMgr::backingW = NULL;
McLabelW *McWindowMgr::labelW = NULL;
McZoomW *McWindowMgr::zoomW = NULL;
McMainW *McWindowMgr::mainW = NULL;
McFadingW *McWindowMgr::fadingW = NULL;

int McWindowMgr::hoverTime = 50;

list<McDesktopW*> McWindowMgr::usedDesktopWList;
list<McDesktopW*> McWindowMgr::freeDesktopWList;

list<McThumbW*> McWindowMgr::usedThumbWList;
list<McThumbW*> McWindowMgr::freeThumbWList;

list<McButtonW*> McWindowMgr::usedButtonWList;
list<McButtonW*> McWindowMgr::freeButtonWList;

UINT_PTR McWindowMgr::timerId = 0;

BOOL McWindowMgr::myGetWindowInfo(HWND hwnd, WINDOWINFO *wi)
{
	if (!wi) return FALSE;
	wi->cbSize = sizeof(WINDOWINFO);
	BOOL result = GetWindowInfo(hwnd, wi);
	if (result)
	{
		wi->rcWindow.right--;
		wi->rcWindow.bottom--;
		wi->rcClient.right--;
		wi->rcClient.bottom--;
	}
	return result;
}

BOOL McWindowMgr::myGetWindowRect(HWND hwnd, RECT *r)
{
	if (!r) return FALSE;
	BOOL result = GetWindowRect(hwnd, r);
	if (result)
	{
		r->right--;
		r->bottom--;
	}
	return result;
}

BOOL McWindowMgr::myGetWindowPlacement(HWND hwnd, WINDOWPLACEMENT *pm)
{
	if (!pm) return FALSE;
	pm->length = sizeof(WINDOWPLACEMENT);
	BOOL result = GetWindowPlacement(hwnd, pm);
	if (result)
	{
		pm->ptMaxPosition.x--;
		pm->ptMaxPosition.y--;
		pm->rcNormalPosition.right--;
		pm->rcNormalPosition.bottom--;
	}
	return result;
}

BOOL McWindowMgr::mySetWindowPlacement(HWND hwnd, WINDOWPLACEMENT *placement)
{
	if (!placement) return FALSE;
	WINDOWPLACEMENT wp;
	memcpy(&wp, placement, sizeof(WINDOWPLACEMENT));
	wp.ptMaxPosition.x++;
	wp.ptMaxPosition.y++;
	wp.rcNormalPosition.right++;
	wp.rcNormalPosition.bottom++;
	return SetWindowPlacement(hwnd, &wp);
}

void CALLBACK McWindowMgr::timerProc( HWND, UINT, UINT_PTR, DWORD )
{
	if (freeThumbWList.size( ) > 0)
	{
		McThumbW *tw = *(freeThumbWList.begin());
		size_t size = freeThumbWList.size( );
		freeThumbWList.remove( tw );
		delete tw;
	}
}

McBgW *McWindowMgr::allocateBgW( )
{

	if (timerId == 0)
		timerId = SetTimer( NULL, _WM_TIMER_ID, _WM_TIMER_DURATION, timerProc );

	McRect *ds = McMonitorsMgr::getMainMonitor( )->getDesktopSize( );
	if (!bgW)
		bgW = new McBgW( );

	bgW->Activate( );

	return bgW;
}

McBackingW *McWindowMgr::allocateBackingW( )
{
	if (!backingW)
	{
		backingW = new McBackingW( );
		BOOL hwAccel = MC::getProperties()->getHardwareAcceleration();
		backingW->Create( L"BackingW", NULL, hwAccel );

		long xsettings = GetWindowLong( backingW->getHwnd( ), GWL_EXSTYLE );
		SetWindowLong( backingW->getHwnd( ), GWL_EXSTYLE, xsettings | WS_EX_LAYERED );
	}
	backingW->Activate( );
	return backingW;
}

McFadingW *McWindowMgr::allocateFadingW( )
{
	if (!fadingW)
	{
		fadingW = new McFadingW( );
		BOOL hwAccel = MC::getProperties()->getHardwareAcceleration();
		fadingW->Create( L"FadingW", NULL, hwAccel );

		long xsettings = GetWindowLong( fadingW->getHwnd( ), GWL_EXSTYLE );
		SetWindowLong( fadingW->getHwnd( ), GWL_EXSTYLE, xsettings | WS_EX_LAYERED );
	}
	fadingW->Activate( );
	return fadingW;
}

McLabelW *McWindowMgr::allocateLabelW( )
{
	if (!labelW)
	{
		labelW = new McLabelW( );
		BOOL hwAccel = MC::getProperties()->getHardwareAcceleration();
		labelW->Create( L"LabelW", NULL, hwAccel );
		long xsettings = GetWindowLong( labelW->getHwnd( ), GWL_EXSTYLE );
		SetWindowLong( labelW->getHwnd( ), GWL_EXSTYLE, xsettings | WS_EX_LAYERED );
	}
	labelW->Activate( );
	return labelW;
}

McZoomW *McWindowMgr::allocateZoomW( )
{
	if (!zoomW)
	{
		zoomW = new McZoomW( );
		BOOL hwAccel = MC::getProperties()->getHardwareAcceleration();
		zoomW->Create( L"ZoomW", NULL, hwAccel );

		DWORD dwPanWant = GC_PAN_WITH_SINGLE_FINGER_VERTICALLY;
		DWORD dwPanBlock = GC_PAN_WITH_GUTTER | GC_PAN_WITH_INERTIA | GC_PAN_WITH_SINGLE_FINGER_HORIZONTALLY;

		GESTURECONFIG gc [] =
		{
			{ GID_TWOFINGERTAP, GC_TWOFINGERTAP, 0 },
			{ GID_PRESSANDTAP, GC_PRESSANDTAP, 0 },
			{ GID_ZOOM, GC_ZOOM, 0 },
			{ GID_ROTATE, GC_ROTATE, 0 },
			{ GID_PAN, dwPanWant, dwPanBlock }
		};

		UINT uiGcs = 5;

		long xsettings = GetWindowLong( zoomW->getHwnd( ), GWL_EXSTYLE );
		SetWindowLong( zoomW->getHwnd( ), GWL_EXSTYLE, xsettings | WS_EX_LAYERED );

		SetGestureConfig( zoomW->getHwnd( ), 0, uiGcs, gc, sizeof( GESTURECONFIG ) );
	}

	zoomW->Activate( );
	return zoomW;
}

McMainW *McWindowMgr::allocateMainW( )
{
	if (!mainW)
	{
		mainW = new McMainW( );
		BOOL hwAccel = MC::getProperties()->getHardwareAcceleration();
		mainW->Create( L"MainW", NULL, hwAccel );

		DWORD dwPanWant = 0;
		DWORD dwPanBlock = GC_PAN_WITH_SINGLE_FINGER_VERTICALLY | GC_PAN_WITH_GUTTER | GC_PAN_WITH_INERTIA | GC_PAN_WITH_SINGLE_FINGER_HORIZONTALLY;

		GESTURECONFIG gc [] =
		{
			{ GID_TWOFINGERTAP, GC_TWOFINGERTAP, 0 },
			{ GID_PRESSANDTAP, GC_PRESSANDTAP, 0 },
			{ GID_PAN, dwPanWant, dwPanBlock },
			{ GID_ZOOM, GC_ZOOM, 0 },
			{ GID_ROTATE, GC_ROTATE, 0 }
		};

		UINT uiGcs = 5;
		SetGestureConfig( mainW->getHwnd( ), 0, uiGcs, gc, sizeof( GESTURECONFIG ) );
	}

	mainW->Activate( );
	return mainW;
}

McButtonW *McWindowMgr::allocateButtonW( WCHAR *_name, McBgW *bgW, int w, int h, Bitmap *i1, Bitmap *i2, WORD cmd )
{
	McButtonW *bw = NULL;

	if (freeButtonWList.size( ) == 0)
	{
		bw = new McButtonW( );
		BOOL hwAccel = MC::getProperties()->getHardwareAcceleration();
		bw->Create( L"ButtonW", L"ActionWindow", hwAccel );
		freeButtonWList.push_back( bw );
	}

	bw = *(freeButtonWList.begin( ));

	freeButtonWList.remove( bw );
	usedButtonWList.push_back( bw );

	bw->preActivate( _name, bgW, w, h, i1, i2, cmd );

	return bw;
}

void McWindowMgr::freeButtonW( McButtonW *bw )
{
	freeButtonWList.push_back( bw );
	usedButtonWList.remove( bw );
}

McDesktopW *McWindowMgr::allocateDesktopW( McWItem *item, McRect *rect )
{
	McDesktopW *dt = NULL;

	if (freeDesktopWList.size( ) == 0)
	{
		dt = new McDesktopW( );
		BOOL hwAccel = MC::getProperties()->getHardwareAcceleration();
		dt->Create( L"DesktopW", L"MonitorWindow", hwAccel );

		HWND hwnd = dt->getHwnd( );

		LONG newStyle = WS_POPUP;
		SetWindowLongPtr( hwnd, GWL_STYLE, newStyle );

		if ( hwAccel )
		{
			MARGINS marg = { -1 };
			int policy = DWMNCRP_ENABLED;
			DwmSetWindowAttribute( hwnd, DWMWA_NCRENDERING_POLICY, &policy, sizeof( int ) );
			DwmExtendFrameIntoClientArea( hwnd, &marg );
		}

		freeDesktopWList.push_back( dt );
	}

	dt = *(freeDesktopWList.begin( ));

	freeDesktopWList.remove( dt );
	usedDesktopWList.push_back( dt );

	dt->Activate( item, rect );
	
	return dt;
}

void McWindowMgr::freeDesktopW( McDesktopW *dt )
{
	freeDesktopWList.push_back( dt );
	usedDesktopWList.remove( dt );
}

McThumbW *McWindowMgr::allocateThumbW( McWItem *item )
{
	McThumbW *tw = NULL;

	if (freeThumbWList.size( ) == 0)
	{
		tw = new McThumbW( );
		BOOL hwAccel = MC::getProperties()->getHardwareAcceleration();
		tw->Create( L"ThumbW", L"AppWindow", hwAccel );

		HWND hwnd = tw->getHwnd( );

		DWORD dwPanWant = GC_PAN;
		DWORD dwPanBlock = GC_PAN_WITH_GUTTER | GC_PAN_WITH_INERTIA;

		if (item->getIsOnAppBar( ))
			dwPanWant |= (GC_PAN_WITH_SINGLE_FINGER_HORIZONTALLY | GC_PAN_WITH_SINGLE_FINGER_HORIZONTALLY);
		else
			dwPanBlock |= (GC_PAN_WITH_SINGLE_FINGER_HORIZONTALLY | GC_PAN_WITH_SINGLE_FINGER_HORIZONTALLY);

		GESTURECONFIG gc [] =
		{
			{ GID_TWOFINGERTAP, GC_TWOFINGERTAP, 0 },
			{ GID_PRESSANDTAP, GC_PRESSANDTAP, 0 },
			{ GID_ZOOM, GC_ZOOM, 0 },
			{ GID_ROTATE, GC_ROTATE, 0 },
			{ GID_PAN, dwPanWant, dwPanBlock }
		};

		UINT uiGcs = 5;

		SetGestureConfig( hwnd, 0, uiGcs, gc, sizeof( GESTURECONFIG ) );

		LONG newStyle = WS_POPUP | WS_BORDER;
		SetWindowLongPtr( hwnd, GWL_STYLE, newStyle );

		if ( hwAccel )
		{
			MARGINS marg = { -1 };
			int policy = DWMNCRP_ENABLED;
			DwmSetWindowAttribute( hwnd, DWMWA_NCRENDERING_POLICY, &policy, sizeof( int ) );
			DwmExtendFrameIntoClientArea( hwnd, &marg );
		}

		freeThumbWList.push_back( tw );
	}

	tw = *(freeThumbWList.begin( ));

	freeThumbWList.remove( tw );
	usedThumbWList.push_back( tw );
	tw->Activate( item );

	return tw;

}

void McWindowMgr::freeThumbW( McThumbW *tw )
{
	freeThumbWList.push_back( tw );
	usedThumbWList.remove( tw );
}

void McWindowMgr::FadeWindowOut( HWND hwnd, int sleep )
{
	for (int i = 224; i > 0; i -= 32)
	{
		SetLayeredWindowAttributes( hwnd, NULL, i, LWA_ALPHA );
		Sleep( sleep );
	}
	SetLayeredWindowAttributes( hwnd, NULL, 0, LWA_ALPHA );
}

void McWindowMgr::FadeWindowIn( HWND hwnd, int maxAlpha, int delay )
{

	if (maxAlpha < 0) maxAlpha = 0;
	if (maxAlpha > 255) maxAlpha = 255;

	int incr = maxAlpha / 8;

	for (int i = incr-1; i <= maxAlpha; i += incr)
	{
		SetLayeredWindowAttributes( hwnd, NULL, i, LWA_ALPHA );
		Sleep( delay );
	}
	SetLayeredWindowAttributes( hwnd, NULL, maxAlpha, LWA_ALPHA );
}

int McWindowMgr::getZorder( HWND appHwnd )
{
	int	zOrder = 0;
	for (HWND h = appHwnd; h; h = GetWindow( h, GW_HWNDPREV ), zOrder++);
	return zOrder;
}
