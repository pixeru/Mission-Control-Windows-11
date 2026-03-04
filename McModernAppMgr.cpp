#include "stdafx.h"

#include "Emcee.h"
#include "McModernAppMgr.h"
#include "McMainW.h"
#include "McColorTools.h"
#include "McMonitorsMgr.h"
#include "McDragMgr.h"
#include "McWList.h"
#include "McHookMgr.h"
#include "McWindowMgr.h"

// Anything that acts directly (well, not directly since they are in a sandbox, so effectively),
// on windows store apps is contained here.  Since windows 8/10 provides very little in the way
// of support for desktop applications controlling windows store apps, much is accomplished
// using a little slight of hand.

#pragma unmanaged

/*////////////// PART A: IMPORTANT DEFINITIONS /////////////////////////////////*/

static BOOL CALLBACK _enumChildCB( HWND hwnd, LPARAM objPtr );

////////////// PART B: DECLARATION OF _ModerrnAppMgr and iVisibilitySubscriber //////////////////

#include <wrl\client.h>
#include <wrl\implements.h>

class iVisibilitySubscriber : 
    public Microsoft::WRL::RuntimeClass< 
        Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::RuntimeClassType::ClassicCom>, 
        IAppVisibilityEvents>
{ 
private:
	ULONG ref;
public: 
	iVisibilitySubscriber() { ref=0; }; 
	~iVisibilitySubscriber() {}; 

    // AppVisibilityOnMonitorChanged will be called when the desktop appears or completely 
	// disappears on a monitor 
    IFACEMETHODIMP AppVisibilityOnMonitorChanged(_In_ HMONITOR hMonitor, 
                                           MONITOR_APP_VISIBILITY previousAppVisibility, 
                                           MONITOR_APP_VISIBILITY currentAppVisibility); 
 
    // LauncherVisibilityChange will be called whenever the Start menu becomes visible or hidden 
	// but won't tell you where
    IFACEMETHODIMP LauncherVisibilityChange(BOOL currentVisibleState); 

}; 

class _ModerrnAppMgr
{

public:	// Actual implementation of McModernAppMgr methods

	iVisibilitySubscriber spSubscriber;
	BOOL hadImmersiveBackground;
	HWND modernHwnd;

	HRESULT Initialize( )
	{
		HRESULT hr = CoCreateInstance( CLSID_AppVisibility, nullptr,
			CLSCTX_INPROC_SERVER, IID_PPV_ARGS( &spAppVisibility ) );
		if (SUCCEEDED( hr ))
		{
			Microsoft::WRL::ComPtr<IAppVisibilityEvents> spSubscriber = Microsoft::WRL::Details::Make<iVisibilitySubscriber>( );
			spAppVisibility->Advise( spSubscriber.Get( ), &dwCookie );
		}
#if MC_DESKTOPS
		hr = CoCreateInstance( CLSID_VirtualDesktopManager, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS( &spVirtualDesktopManager ) );
#endif
		return hr;
	}

	void Uninitialize( )
	{
		spAppVisibility->Unadvise( dwCookie );
	}

	BOOL CloseApp( McWItem *item )
	{
		BOOL result = FALSE;
		if (item->getIsW10App( ))
		{
			PostMessage( item->getAppHwnd( ), WM_CLOSE, NULL, NULL );
			if ((!item->getIsCloaked( )) && MC::inTabletMode( ))
			result = TRUE;
		}

		return result;
	}

	wchar_t *getModernWindowName( )
	{
		return _MODERN_MODERN_APP;
	}

	HWND doFindModernWindowEx( HWND parent, HWND firstChild, wchar_t* wname, HWND *searchHwnd )
	{
		HWND hwnd;

		hwnd = FindWindowEx( parent, firstChild, _MODERN_MODERN10_APP, wname );

		while (hwnd)
		{
			modernHwnd = hwnd;
			Sleep( 0 );

			if (modernHwnd)
			{
				if (searchHwnd) *searchHwnd = hwnd;
				return modernHwnd;
			}

			hwnd = FindWindowEx( NULL, hwnd, _MODERN_MODERN10_APP, NULL );
		}

		return NULL;
	}

	BOOL EnumAllApps( _In_ WNDENUMPROC lpEnumFunc, _In_ LPARAM lParam )
	{
		BOOL result = TRUE;

		HWND hwnd;
		HWND searchHwnd = NULL;

		// First window returned is most recent, followed by rest from
		// oldest to newest.  searchHwnd is the toplevel window found,
		// hwnd is the windows8 app

		while ((hwnd = doFindModernWindowEx( NULL, searchHwnd, NULL, &searchHwnd )))
		{
			if (!lpEnumFunc( searchHwnd, lParam ))
				return FALSE;
		}

		return TRUE;
	}

	BOOL GetAppInfo( HWND hwnd, DWORD processId, char *className, char *progName, int*isW10App, int maxLen )
	{
		char    _className[1000];
		char    _progName[1000];

		if (0 < GetClassNameA( hwnd, _className, 1000 ))
		{
			if (strstr( _className, _MODERN_MODERN10_APP_BYTE ))
			{
				strcpy_s( _className, 1000, _MODERN_MODERN10_APP_BYTE );
				if (isW10App) *isW10App = TRUE;
			}

			if (!strcmp( _className, _MODERN_SEARCH_RESULTS_BYTE ))
				sprintf_s( className, 1000, "explorer/%s", _className );
			else
				strcpy_s( className, 1000, _className );
		}
		else
			return FALSE;

		if (processId == 0)
			GetWindowThreadProcessId( hwnd, &processId );

		MC::GetWText( hwnd, _progName, TRUE );

		if (_progName[0] == 0) // If framed window doesn't have name get name of UI Window
		{
			HWND uiHwnd = FindWindowEx( hwnd, NULL, _MODERN_MODERN_APP, NULL );
			if (!uiHwnd)
			{
				if (!IsIconic( hwnd )) return FALSE;
				strcpy_s( _progName, 100, "(Bug in Windows)" );
			}
			else
				MC::GetWText( uiHwnd, _progName, TRUE );
			if (_progName[0] == 0)
				return FALSE;
		}

		sprintf_s( progName, maxLen, "%s", _progName );

		if (strstr( progName, "Search" ) && strstr( className, _MODERN_MODERN_APP_BYTE ))
			return FALSE;

		if (strstr( progName, "Search" ) && strstr( className, _MODERN_MODERN10_APP_BYTE ))
			return FALSE;

		return TRUE;
	}

	COLORREF GetAppBarColor( )
	{
		return GetPaletteColor( 5 );
	}

	COLORREF GetIconBgColor( )
	{
		return GetPaletteColor( 4 );
	}

	COLORREF GetPaletteColor( int idx )
	{
		return paletteArray[idx%paletteCount];
	}

	int GetPaletteCount( )
	{
		return paletteCount;
	}

	COLORREF GetBgColor( )
	{
		return bgColor;
	}

	COLORREF GetAutoColor( )
	{
		return autoColor;
	}

	COLORREF GetSettingsColor( )
	{
		return GetPaletteColor( 6 );
	}

	void RegistryBackground( )
	{
		HKEY rootKey;
		LONG res = RegOpenKeyEx( HKEY_CURRENT_USER,
			L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Accent",
			0, KEY_QUERY_VALUE, &rootKey );
		if (res == ERROR_SUCCESS)
		{
			BYTE values[100];
			DWORD size = 100;
			DWORD type = REG_BINARY;
			res = RegQueryValueEx( rootKey, L"AccentPalette", NULL, &type, (LPBYTE) &values, &size );

			paletteCount = size / 4;
			if (paletteArray) delete paletteArray;
			paletteArray = new COLORREF[paletteCount];
			for (int i = 0; i < paletteCount; i++)
			{
				int idx = i * 4;
				paletteArray[i] = RGB( values[idx], values[idx + 1], values[idx + 2] );
			}
			RegCloseKey( rootKey );
		}
		return;
	}

	void DetermineModernColors( )
	{
		RegistryBackground( );
		COLORREF rgb = GetSettingsColor( );
		int blue = (rgb & 0xFF0000) >> 16;
		int green = (rgb & 0x00FF00) >> 8;
		int red = rgb & 0x0000FF;
		int h, s, l;
		McColorTools::getHSLfromRGB( red, green, blue, &h, &s, &l );
		s /= 2;
		l = min( 255, (3 * l) );
		McColorTools::getRGBfromHSL( h, s, l, &red, &green, &blue );
		autoColor = RGB( red, green, blue );
		bgColor = MC::getProperties( )->getDtAutoColor( ) ? autoColor : MC::getProperties( )->getDtBgColor( );
		hadImmersiveBackground = TRUE;
	}

	BOOL IsWindowVisible( HWND hwnd, McRect *_windowRect = NULL )
	{
		McRect windowRect;
		if (_windowRect)
			windowRect.set( _windowRect );
		else
		{
			WINDOWINFO wp = { 0 };
			wp.cbSize = sizeof( WINDOWINFO );
			McWindowMgr::myGetWindowInfo( hwnd, &wp );
			windowRect.set( &wp.rcClient );
		}
		POINT p;
		p.x = (windowRect.left + windowRect.right) / 2;
		p.y = 20;
		HWND hwnd1 = WindowFromPoint( p );
		if (hwnd1)
			hwnd1 = GetAncestor( hwnd1, GA_ROOT );

		return (hwnd1 == hwnd);
	}

	BOOL IsLauncherVisible( )
	{
		BOOL isVisible = FALSE;
		spAppVisibility->IsLauncherVisible( &isVisible );

		return isVisible != 0;
	}

	void RemoveAnimatedSystemWindows(  )
	{
		long sleepTime = 10;

		sleepTime = RemoveLauncher( );

		HWND hwnd = FindWindow(_MODERN_VIEWFRAME, NULL);
		if (hwnd)
		{
			McModernAppMgr::InjectAnyKey(VK_ESCAPE);
			Sleep( max(sleepTime, 250) );
		}
		else
		{
			McMonitorsMgr *monitor = McMonitorsMgr::getMainMonitor();
			POINT sPoint;
			McRect *mSize = monitor->getMonitorSize();
			sPoint.x = mSize->right - 10;
			sPoint.y = mSize->top + mSize->getHeight() - 100;
			hwnd = WindowFromPoint(sPoint);
			HWND parent = GetAncestor(hwnd, GA_ROOT);
			if (parent) hwnd = parent;
			if (hwnd)
			{
				char className[1000];
				char progName[1000];

				GetClassNameA(hwnd, className, 1000);
				GetWindowTextA(hwnd, progName, 1000);

				if (strstr(className, _MODERN_MODERN_APP_BYTE))
				{
					if (strstr(progName, _MODERN_START_SCREEN_BYTE))
						sleepTime = max(sleepTime, 449);

					else if (strstr(progName, _MODERN_ACTION_CENTER_BYTE))
						ClickMouseAt(mSize->left + 1, mSize->top + 100);

					else if (strstr(progName, _MODERN_TASKVIEW_BYTE))
					{
						sleepTime = max(sleepTime, 250);
						McModernAppMgr::InjectAnyKey(VK_ESCAPE);
					}

					else if (strstr(progName, _MODERN_PROJECT_BYTE))
						McModernAppMgr::InjectAnyKey(VK_ESCAPE);
				}
				else
					if (strstr(className, _MODERN_SHELL_BYTE))
					{
						ClickMouseAt(0, 0, TRUE);
						sleepTime = max(sleepTime, 500);
					}
			}
		}

		if (sleepTime > 0)
		{
			Sleep( sleepTime );
			return;
		}

	}

	long RemoveLauncher( )
	{
		if (IsLauncherVisible( ))
		{
			InjectWinKey( );
			return MC::inTabletMode( ) ? 300 : 300;
		}
		else
			return 0;
	}

#if MC_DESKTOPS
	BOOL IsWindowOnCurrentDesktop( HWND hwnd )
	{
		BOOL result;
		spVirtualDesktopManager->IsWindowOnCurrentVirtualDesktop( hwnd, &result );
		return result;
	}

	void GetWindowDesktop( HWND hwnd, GUID *desktopID )
	{
		if (S_OK != spVirtualDesktopManager->GetWindowDesktopId( hwnd, desktopID ))
		{
			desktopID->Data1 = 0;
			desktopID->Data2 = 0;
			desktopID->Data3 = 0;
		}

	}
#endif

	void ClickMouseAt( long x, long y, BOOL useLB=FALSE )
	{
		POINT cursorPos;
		GetCursorPos( &cursorPos );

		McRect *mRect = McMonitorsMgr::getMainMonitor( )->getDesktopSize( );

		long AxCurrent = ( 65535 * (cursorPos.x - mRect->left) ) / mRect->getWidth( );
		long AxClick =	 ( 65535 * (x - mRect->left          ) ) / mRect->getWidth( );
		long AyCurrent = ( 65535 * (cursorPos.y - mRect->top ) ) / mRect->getHeight( );
		long AyClick =   ( 65535 * (y - mRect->top           ) ) / mRect->getHeight( );

		INPUT input[4];

		for (int i = 0; i < 4; i++)
		{
			input[i].type = INPUT_MOUSE;
			input[i].mi.time = 0;
			input[i].mi.mouseData = 0;
			input[i].mi.dwExtraInfo = NULL;
			input[i].mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_VIRTUALDESK;
			switch (i)
			{
			case 0:
				input[i].mi.dx = AxClick;
				input[i].mi.dy = AyClick;
				input[i].mi.dwFlags |= MOUSEEVENTF_MOVE;
				break;
			case 1:
				input[i].mi.dx = AxClick;
				input[i].mi.dy = AyClick;
				input[i].mi.dwFlags |= useLB ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_MIDDLEDOWN;
				break;
			case 2:
				input[i].mi.dx = AxClick;
				input[i].mi.dy = AyClick;
				input[i].mi.dwFlags |= useLB ? MOUSEEVENTF_LEFTUP : MOUSEEVENTF_MIDDLEUP;
				break;
			case 3:
				input[i].mi.dx = AxCurrent;
				input[i].mi.dy = AyCurrent;
				input[i].mi.dwFlags |= MOUSEEVENTF_MOVE;
				break;
			default:
				break;
			}
		}
		SendInput( 2, input, sizeof( INPUT ) );
		SendInput( 2, &input[2], sizeof( INPUT ) );
	}

	void InjectWinKey( WORD key1 = 0, WORD key2 = 0 )
	{
		// Inject a key combo including the windows key

		McHookMgr::releaseKbHooks( );

		INPUT injected[6];
		for (int i = 0; i < 6; i++)
		{
			injected[i].type = INPUT_KEYBOARD;
			injected[i].ki.dwFlags = 0;
			injected[i].ki.time = 0;;
		}

		int idx = 0;
		injected[idx++].ki.wVk = VK_LWIN;

		if (key1)	injected[idx++].ki.wVk = key1;

		if (key2) injected[idx++].ki.wVk = key2;

		injected[idx].ki.wVk = VK_LWIN;
		injected[idx++].ki.dwFlags = KEYEVENTF_KEYUP;

		if (key2)
		{
			injected[idx].ki.wVk = key2;
			injected[idx++].ki.dwFlags = KEYEVENTF_KEYUP;
		}

		if (key1)
		{
			injected[idx].ki.wVk = key1;
			injected[idx++].ki.dwFlags = KEYEVENTF_KEYUP;
		}

		SendInput( idx, injected, sizeof( INPUT ) );
		McHookMgr::initializeKbHooks( );
	}

	void InjectAnyKey( WORD key1, WORD key2 = 0 )
	{
		//Inject a key combo up to 2 keys
		INPUT injected[4];
		for (int i = 0; i < 4; i++)
		{
			injected[i].type = INPUT_KEYBOARD;
			injected[i].ki.dwFlags = 0;
			injected[i].ki.time = 0;
		}

		int idx = 0;

		injected[idx++].ki.wVk = key1;

		if (key2) injected[idx++].ki.wVk = key2;

		if (key2)
		{
			injected[idx].ki.wVk = key2;
			injected[idx++].ki.dwFlags = KEYEVENTF_KEYUP;
		}


		injected[idx].ki.wVk = key1;
		injected[idx++].ki.dwFlags = KEYEVENTF_KEYUP;

		SendInput( idx, injected, sizeof( INPUT ) );
	}

	BOOL CheckLockScreen( )
	{
		HWND hwnd = WindowFromPoint( *(McMonitorsMgr::getMainMonitor( )->getMonitorCenter( )) );
		HWND parent = GetAncestor( hwnd, GA_ROOT );
		if (parent) hwnd = parent;
		char WindowName[1000];
		GetWindowTextA( hwnd, WindowName, 1000 );
		if (!strcmp( WindowName, _MODERN_LOCK_SCREEN_BYTE ) )
		{
			GetClassNameA( hwnd, WindowName, 1000 );
			if (!strcmp( WindowName, _MODERN_MODERN_APP_BYTE ))
			{
				MessageBeep( MB_ICONASTERISK );
				return TRUE;
			}
		}
		return FALSE;
	}

	_ModerrnAppMgr( )
	{
		paletteArray = NULL;
		paletteCount = 0;
		autoColor = 0;
		bgColor = 0;
		hadImmersiveBackground = FALSE;
	}

	~_ModerrnAppMgr( )
	{
	}

	HWND GetUnderlyingModernWindow( HWND hwnd )
	{
		modernHwnd = NULL;
		EnumChildWindows( hwnd, _enumChildCB, (LPARAM)this );
		return modernHwnd;
	}

	void notifyOfLauncher( )
	{
		if (McWindowMgr::getMainW( )  )
			PostMessage( McWindowMgr::getMainW( )->getHwnd( ), WM_USER, WMMC_LAUNCHER, NULL );
	}

	BOOL MoveApp( McWItem *item, McMonitorsMgr *originalMonitor, McMonitorsMgr *newMonitor, McRect *thumb )
	{
		// Move a windows store app from one monitor to another monitor, if possible.

		McRect originalRect( item->getNormalRect( ) );
		McRect oMonRect( originalMonitor->getMonitorSize( ) );
		McRect nMonRect( newMonitor->getMonitorSize( ) );

		int offsetx = nMonRect.left - oMonRect.left;
		int offsety = nMonRect.top - oMonRect.top;

		HMONITOR oMonitor = MonitorFromWindow( item->getAppHwnd( ), MONITOR_DEFAULTTONEAREST );

		McRect newRect;

		int error = 0;

		// Windows are moved by injecting WIN/shift/-> or WIN/shift/<- keys.   On some systems, if numlock is set,
		// these injected keys are mapped to numbers on the key pad, so require Numlock be off.
		// Moving them is also unreliable if "animate windows when maximizing and minimizing" is on,
		// so require it be off.

		if (VK_IS_KEY_TOGGLED( VK_NUMLOCK ))
			error = 1;
		else if (MC::getIsWindowsAnimating( ))
			error = 2;

		if (error)
		{
			wchar_t wbuff[1000];
			if (error == 1)
				swprintf_s( wbuff, 1000, L"\n\n\nThe window for %S, a \"Modern\" application can "
				L"not be moved to a different monitor, given current keyboard Settings."
				L"\n\nTurn off NUM-LOCK and try again.", item->getProgName( ) );
			else
				swprintf_s( wbuff, 1000, L"\n\n\nThe window for this \"Modern\" application "
				L"can not be moved to a different monitor, given current system Settings."
				L"\n\nTurn of \"Animate windows when minimizing and maximizing\" in\n\n"
				L"Control Panel->System->Advanced System Settings->Visual Effects." );
			MessageBeep( MB_ICONERROR );
			MC::doMessageBox( item->getThumbHwnd( ),
				wbuff,
				L"Window Move Error",
				MB_OK | MB_ICONERROR );
			return FALSE;
		}

		int key = 0;
		int distance = 1;
		int leftd = 1;
		int rightd = 1;

		if (McMonitorsMgr::getMonitorCount( ) > 2)
		{
			int leftd, rightd;
			if (newMonitor->getOrdinal( ) < originalMonitor->getOrdinal( ))
			{
				leftd = originalMonitor->getOrdinal( ) - newMonitor->getOrdinal( );
				rightd = (int)(newMonitor->getOrdinal( ) + McMonitorsMgr::getMonitorCount( ) - originalMonitor->getOrdinal( ));
			}
			else
			{
				rightd = newMonitor->getOrdinal( ) - originalMonitor->getOrdinal( );
				leftd = (int)(originalMonitor->getOrdinal( ) + McMonitorsMgr::getMonitorCount( ) - newMonitor->getOrdinal( ));
			}
		}

		if (rightd < leftd)
		{
			key = VK_RIGHT;
			distance = rightd;
		}
		else if (leftd < rightd)
		{
			key = VK_LEFT;
			distance = leftd;
		}
		else
			key = rand( ) % 2 == 1 ? VK_RIGHT : VK_LEFT;

		int altkey = (key == VK_LEFT) ? VK_RIGHT : VK_LEFT;

		WINDOWINFO wi;

		SetWindowPos( item->getAppHwnd( ), HWND_TOPMOST, originalRect.left, originalRect.top, originalRect.getWidth( ), originalRect.getHeight( ),
			SWP_SHOWWINDOW );
		Sleep( 100 );

		BOOL success = TRUE;
		HMONITOR nMonitor = NULL;

		// Move it distance monitors to right or left
		for (int i = 0; i < distance; i++)
		{
			// try it with Win/Shift/arrow

			InjectWinKey( VK_SHIFT, key );
			Sleep( 50 );
			McWindowMgr::myGetWindowInfo( item->getAppHwnd( ), &wi );
			newRect.set( &wi.rcWindow );
			nMonitor = MonitorFromWindow( item->getAppHwnd( ), MONITOR_DEFAULTTONEAREST );

			// Case below detects moving snapped window onto a snapped display.   The
			// Image hovers over the inter-app gutter until it is placed with a 
			// Win/Arrow and an Enter.

			if (nMonitor == oMonitor)
			{
				InjectWinKey( altkey );
				Sleep( 50 );
				InjectAnyKey( VK_RETURN );
				Sleep( 100 );
				McWindowMgr::myGetWindowInfo( item->getAppHwnd( ), &wi );
				newRect.set( &wi.rcWindow );
				nMonitor = MonitorFromWindow( item->getAppHwnd( ), MONITOR_DEFAULTTONEAREST );
				if (nMonitor == oMonitor)
					success = FALSE;
			}
			if (!success)
				break;
		}

		if (!success)
			return FALSE;

		// Now make sure the position and size information the moved window are correct.

		McMonitorsMgr *monitor = McMonitorsMgr::getMonitor( nMonitor );

		item->setIsMinimized( FALSE );
		item->setVisibility( TRUE, FALSE );
		item->setAppMonitor( nMonitor );
		item->setNormalRect( &newRect );
		newRect.offset( -newRect.left, -newRect.top );
		item->getContentRect( )->set( &newRect );
		item->getThumbRect( )->set( thumb );
		item->setCurRect( thumb );
		MC::getWindowList( )->setFocusItem( item );

		return TRUE;
	}

public:

	HWND			_activeAppHwnd;

	COLORREF		autoColor;
	COLORREF		bgColor;
	COLORREF*		paletteArray;
	int				paletteCount;

	DWORD			dwCookie;

	BOOL			_includeLauncher;
	BOOL			_isFullScreen;

	IAppVisibility *spAppVisibility;
#if MC_DESKTOPS
	IVirtualDesktopManager *spVirtualDesktopManager;
#endif
};

// IMPLEMENT CALLBACKS HERE

IFACEMETHODIMP  iVisibilitySubscriber::AppVisibilityOnMonitorChanged(
										   _In_ HMONITOR hMonitor, 
                                           MONITOR_APP_VISIBILITY previousAppVisibility, 
                                           MONITOR_APP_VISIBILITY currentAppVisibility)
{
	// Never called under windows 10
	return S_OK;
}
 
    // LauncherVisibilityChange will be called whenever the Start menu becomes visible or hidden 
IFACEMETHODIMP  iVisibilitySubscriber::LauncherVisibilityChange(BOOL currentVisibleState)
{
	if ( currentVisibleState )
		if ( McModernAppMgr::getInterface() ) McModernAppMgr::getInterface()->notifyOfLauncher();
	return S_OK;
}

////////////// PART C: FORWRD DECLARED STATIC CALLBACK FUNCTION /////////////

static BOOL CALLBACK _enumChildCB( HWND hwnd, LPARAM objPtr )
{
	_ModerrnAppMgr *me = (_ModerrnAppMgr*) objPtr;
	wchar_t className[1000];
	GetClassName( hwnd, className, 1000 );
	if (!wcscmp( className, _MODERN_MODERN_APP ))
	{
		if (me) me->modernHwnd = hwnd;
		return FALSE;
	}
	return TRUE;
}

////////////// PART D: IMPLENETATION OF McModernAppMgr Methods //////////////////////

_ModerrnAppMgr*	McModernAppMgr::_ModernAppProxy = NULL;

HRESULT McModernAppMgr::Initialize()
{
	if ( McModernAppMgr::_ModernAppProxy )
		return S_OK;

	_ModernAppProxy = new _ModerrnAppMgr();

	return _ModernAppProxy->Initialize();
}

void McModernAppMgr::Uninitialize()
{
	if ( !McModernAppMgr::_ModernAppProxy )
		return;
	_ModernAppProxy->Uninitialize();
	delete _ModernAppProxy;
	_ModernAppProxy = NULL;
}

BOOL McModernAppMgr::CloseApp( McWItem *item )
{
	if ( _ModernAppProxy ) return _ModernAppProxy->CloseApp( item );
	else return FALSE;
}

BOOL McModernAppMgr::EnumAllApps( WNDENUMPROC lpEnumFunc, LPARAM lParam )
{

	if ( _ModernAppProxy ) return _ModernAppProxy->EnumAllApps( lpEnumFunc, lParam );
	else 
		return FALSE;
}

BOOL McModernAppMgr::GetAppInfo( HWND hwnd, DWORD processId, char *className, char *progName, int *isW10App, int maxLen )
{

	if ( _ModernAppProxy ) return _ModernAppProxy->GetAppInfo( hwnd, processId, className, progName, isW10App, maxLen );
	else
		return FALSE;
}

COLORREF McModernAppMgr::GetAppBarColor()
{

	if ( _ModernAppProxy ) return _ModernAppProxy->GetAppBarColor();
	else
		return 0;
}

COLORREF McModernAppMgr::GetIconBgColor( )
{

	if (_ModernAppProxy) return _ModernAppProxy->GetIconBgColor( );
	else
		return 0;
}

COLORREF McModernAppMgr::GetPaletteColor( int idx )
{

	if (_ModernAppProxy) return _ModernAppProxy->GetPaletteColor( idx );
	else
		return 0;
}

COLORREF McModernAppMgr::GetAutoColor( )
{
	if (_ModernAppProxy) return _ModernAppProxy->GetAutoColor( );
	else
		return 0;
}

COLORREF McModernAppMgr::GetBgColor( )
{
	if (_ModernAppProxy) return _ModernAppProxy->GetBgColor( );
	else
		return 0;
}

int McModernAppMgr::GetPaletteCount( )
{
	if (_ModernAppProxy) return _ModernAppProxy->GetPaletteCount( );
	else
		return 0;
}


COLORREF McModernAppMgr::GetSettingsColor( )
{
	if (_ModernAppProxy) return _ModernAppProxy->GetSettingsColor( );
	else
		return 0;
}
void McModernAppMgr::DetermineModernColors()
{
	if ( _ModernAppProxy ) _ModernAppProxy->DetermineModernColors();
}

void McModernAppMgr::RemoveLauncher()
{
	if (_ModernAppProxy)
		_ModernAppProxy->RemoveLauncher( );
}

void McModernAppMgr::InjectAnyKey( WORD key1, WORD key2 )
{
	if (_ModernAppProxy) _ModernAppProxy->InjectAnyKey( key1, key2 );
}

void McModernAppMgr::InjectWinKey( WORD key1, WORD key2 )
{
	if (_ModernAppProxy) _ModernAppProxy->InjectWinKey( key1, key2 );
}

BOOL McModernAppMgr::CheckLockScreen( )
{
	if (_ModernAppProxy)
		return _ModernAppProxy->CheckLockScreen( );
	return FALSE;
}

// Given how Windows IVisibility interface works, what this really tells
// you is if the desktop is fully obscured on the monitor.    There could
// be a snapped immersive app sharing space with the desktop and this function
// would return FALSE.

void McModernAppMgr::RemoveAnimatedSystemWindows(  )
{
	if (_ModernAppProxy) _ModernAppProxy->RemoveAnimatedSystemWindows(  );
}

BOOL McModernAppMgr::MoveApp( McWItem *item, McMonitorsMgr *originalMonitor, McMonitorsMgr *newMonitor, McRect *thumb )
{
	if (_ModernAppProxy) return _ModernAppProxy->MoveApp( item, originalMonitor, newMonitor, thumb );
	return FALSE;
}

HWND McModernAppMgr::GetUnderlyingModernWindow( HWND hwnd)
{
	if (_ModernAppProxy) return _ModernAppProxy->GetUnderlyingModernWindow( hwnd );
	else
		return 0;
}

BOOL McModernAppMgr::IsLauncherVisible( )
{
	if (_ModernAppProxy) return _ModernAppProxy->IsLauncherVisible( );
	return FALSE;
}

void McModernAppMgr::ClickMouseAt( long x, long y, BOOL useLB )
{
	if (_ModernAppProxy) _ModernAppProxy->ClickMouseAt( x, y, useLB );
}

#if MC_DESKTOPS
BOOL McModernAppMgr::IsWindowOnCurrentDesktop( HWND hwnd )
{
	if (_ModernAppProxy) return _ModernAppProxy->IsWindowOnCurrentDesktop( hwnd );
	else
		return TRUE;
}

void McModernAppMgr::GetWindowDesktop( HWND hwnd, GUID *desktopID )
{
	if (!desktopID) return;
	if (_ModernAppProxy) return _ModernAppProxy->GetWindowDesktop( hwnd, desktopID );
}
#endif

BOOL McModernAppMgr::MatchesModernClassname( char *classname )
{
	return
		!strncmp( classname, _MODERN_MODERN_APP_BYTE, 100  ) ||
		!strncmp( classname, _MODERN_MODERN10_APP_BYTE, 100 ) ||
		!strncmp( classname, _MODERN_SEARCHPANE_BYTE, 100  ) ||
		!strncmp( classname, _MODERN_SEARCH_RESULTS_BYTE, 100 ) ||
		!strncmp( classname, _MODERN_GHOST_WINDOW_BYTE, 100 ) ||
		!strncmp( classname, _MODERN_IMMERSIVE_BYTE, _MODERN_LEN_IMMERSIVE ) ||
		!strncmp( classname, _MODERN_PREDICTION, 100 ) || 
		!strncmp( classname, _MODERN_SHELL_BYTE, _MODERN_LEN_SHELL );

}

BOOL McModernAppMgr::tabletModeDividerExists( HWND *tmDivider )
{
	if (!MC::inTabletMode( )) 
		return FALSE;
	HWND hwnd = FindWindowEx( NULL, NULL, L"JointDivider", NULL );
	if (tmDivider) *tmDivider = hwnd;
	if (hwnd)
		return IsWindowVisible( hwnd );
	return FALSE;
}

