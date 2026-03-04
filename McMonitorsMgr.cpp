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
#include "McMonitorsMgr.h"
#include "McWindowMgr.h"

list <McMonitorsMgr*> McMonitorsMgr::monitorList;
McMonitorsMgr *McMonitorsMgr::mainMonitor = NULL;
size_t McMonitorsMgr::monitorCount = 0;
size_t McMonitorsMgr::effectiveMonitorCount = 0;
BOOL McMonitorsMgr::monitorsCreated = FALSE;
BOOL McMonitorsMgr::isTouchScreen = FALSE;

#define _REFERENCE_DPI 89.0

// Key track of information about multiple monitors & their DPI, location, etc.
// The static member keep a list of the McMonitorsMgr objects, one for each monitor.

McMonitorsMgr::McMonitorsMgr( HMONITOR _hMonitor )
{
	hMonitor = _hMonitor;

	MONITORINFO minfo = { 0 };
	McMonitorsMgr::myGetMonitorInfo( hMonitor, &minfo );

	monitorSize.set( &minfo.rcMonitor );
	monitorWorkSize.set( &minfo.rcWork );
	mdCenter.x = (monitorSize.left + monitorSize.right) / 2;
	mdCenter.y = (monitorSize.top + monitorSize.bottom) / 2;
	aspect = ((float) monitorSize.getWidth( )) / ((float) monitorSize.getHeight( ));
	if (minfo.dwFlags&MONITORINFOF_PRIMARY) // This is the main monitor
	{
		isMainMonitor = TRUE;
		mainMonitor = this;
	}
	else
		isMainMonitor = FALSE;

	typedef HRESULT( CALLBACK* gdfmType ) (HMONITOR, MONITOR_DPI_TYPE, UINT*, UINT*);
	typedef HRESULT( CALLBACK* gsfmType ) (HMONITOR, DEVICE_SCALE_FACTOR *);

	// Done this way since the functions are not in a lib until Windows 8.1


	HMODULE dllHandle = LoadLibrary( L"shcore.dll" );
	if (dllHandle)
	{
		gdfmType gdfmPtr = NULL;
		gdfmPtr = (gdfmType) GetProcAddress( dllHandle, "GetDpiForMonitor" );
		unsigned int xdpi = 0, ydpi = 0;
		if (NULL != gdfmPtr)
		{
			gdfmPtr( hMonitor, MDT_RAW_DPI, &xdpi, &ydpi );
			if (xdpi > 0)
			{
				monitorDpi = (float) xdpi;
				unsigned int axdpi = 0, aydpi = 0;
				gdfmPtr( hMonitor, MDT_EFFECTIVE_DPI, &axdpi, &aydpi );
				scaleFactor = monitorDpi / axdpi;
			}
		}
		if (!gdfmPtr || xdpi == 0)
		{
			HDC hdc = GetDC( GetDesktopWindow( ) );
			int mms = GetDeviceCaps( hdc, HORZSIZE );
			int pixels = GetDeviceCaps( hdc, HORZRES );
			monitorDpi = floor( (25.4F * pixels) / mms );
			ReleaseDC( GetDesktopWindow( ), hdc );
			scaleFactor = 1.1f;
		}

		FreeLibrary( dllHandle );

		HDC dc = GetDC( GetDesktopWindow( ) );
		int mmsx = GetDeviceCaps( dc, HORZSIZE );
		int mmsy = GetDeviceCaps( dc, VERTSIZE );

		double pscale = -161.806
			+ 0.876 * monitorDpi
			+ 0.309 * mmsx
			+ 0.758 * mmsy
			- 0.0013 * mmsx * mmsy;

		ReleaseDC( GetDesktopWindow( ), dc );
		scaleFactor = 100 * scaleFactor / ((float) pscale);
		if (scaleFactor > 1.2)
			scaleFactor = 1.0f;
	}

	if (monitorDpi < 1.0) monitorDpi = 96.0F;
	
	monitorObjectScale = (float)((1/scaleFactor + monitorDpi / _REFERENCE_DPI )/2.0);
	if (isMainMonitor)
	{
		WINDOWPLACEMENT wp = { 0 };
		McWindowMgr::myGetWindowPlacement( GetDesktopWindow( ), &wp );

		dtR.set( &wp.rcNormalPosition );

		mdOffset.x = monitorSize.left - dtR.left;
		mdOffset.y = monitorSize.top - dtR.top;

		if (dtR.getHeight( ) < 851) 
			scaleFactor *= 0.8F;

	}

	monitorList.push_back(this);

}

McMonitorsMgr::~McMonitorsMgr(  )
{
}

BOOL McMonitorsMgr::monitorCallback(HMONITOR hmon, HDC, LPRECT, LPARAM)
{
	McMonitorsMgr* mcm = new McMonitorsMgr(hmon);
	return TRUE;
}

bool McMonitorsMgr::_mcomp( McMonitorsMgr *first, McMonitorsMgr *second )
{
	if (first->getMonitorSize( )->bottom <= second->getMonitorSize( )->top)
		return TRUE;
	if (first->getMonitorSize( )->top >= second->getMonitorSize( )->bottom)
		return FALSE;
	return (first->getMonitorSize( )->right <= second->getMonitorSize( )->left);
}

void McMonitorsMgr::CreateMonitorInformation( BOOL doLog )
{
	if (monitorsCreated)
		DeleteMonitorInformation();
	monitorCount = 0;
	mainMonitor = NULL;
	EnumDisplayMonitors(NULL, NULL, McMonitorsMgr::monitorCallback, NULL);
	monitorCount = effectiveMonitorCount = monitorList.size();
	int targetTouch = NID_MULTI_INPUT & NID_READY;
	int touchInfo = GetSystemMetrics( SM_DIGITIZER );
	if ( ((touchInfo & targetTouch) == targetTouch ) && 5<=GetSystemMetrics( SM_MAXIMUMTOUCHES ))
		isTouchScreen = TRUE;

	monitorList.sort( _mcomp );

	

	int o = 0;
	if ( monitorCount > 1 )
		for (list<McMonitorsMgr *>::iterator mi = monitorList.begin( ); mi != monitorList.end( ); mi++)
		{
			McMonitorsMgr *monitor = *mi;
			(*mi)->ordinal = o++;
		}
	monitorsCreated = TRUE;
}

void McMonitorsMgr::setEffectiveMonitorCount( )
{
	// Effective monitor count is how many monitors we display thumbs on -- may be 1 even
	// on a multimonitor system if user specifies in Settings editor.

	if (MC::getProperties( )->getMultiMonitor( ))
		effectiveMonitorCount = monitorCount;
	else
		effectiveMonitorCount = 1;
}

void McMonitorsMgr::DeleteMonitorInformation()
{
	if (!monitorsCreated)
		return;

	while (monitorList.size() > 0)
	{
		McMonitorsMgr* m = (McMonitorsMgr *)monitorList.front();
		monitorList.remove(m);
		delete m;
	}
	monitorsCreated = FALSE;
}

McMonitorsMgr* McMonitorsMgr::getMonitor( HMONITOR _monitor )
{
	for (list<McMonitorsMgr *>::iterator mi = monitorList.begin( ); mi != monitorList.end( ); mi++)
	{
		McMonitorsMgr *monitor = *mi;
		if (_monitor == monitor->getHMONITOR( ))
			return monitor;
	}
	return NULL;
}

BOOL McMonitorsMgr::myGetMonitorInfo(HMONITOR mon, MONITORINFO *mi)
{
	if (!mi) return FALSE;
	mi->cbSize = sizeof(MONITORINFO);
	BOOL result = GetMonitorInfo(mon, mi);
	if (result)
	{
		mi->rcMonitor.right--;
		mi->rcMonitor.bottom--;
		mi->rcWork.right--;
		mi->rcWork.bottom--;
	}
	return result;
}