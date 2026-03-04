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
#include "McZoomW.h"
#include "McBackingW.h"
#include "McThumbW.h"
#include "McLabelW.h"
#include "McMainW.h"
#include "McBgW.h"
#include "McWList.h"
#include "McWItem.h"
#include "McAnimationMgr.h"
#include "McPilesMgr.h"
#include "McPropsMgr.h"
#include "McModernAppMgr.h"
#include "McHotKeyMgr.h"
#include "McMonitorsMgr.h"
#include "McHookMgr.h"
#include "McIconMgr.h"
#include "McColorTools.h"
#include "McDesktopW.h"
#include "McWindowMgr.h"
#include "McDragMgr.h"

#pragma unmanaged
using namespace std;

#pragma comment( lib, "Dwmapi.lib" )	// Needed to use the dwm
#pragma comment( lib, "gdiplus.lib" )	// Needed to paint the background window and zoom window
#pragma comment( lib, "Crypt32.lib" )
#pragma comment( lib, "Shcore.lib" )
#pragma comment( lib, "MSCOREE.LIB" )
#pragma comment( lib, "winmm.lib" )

#include <Mmsystem.h>

#if MC_LOGGING
FILE*			MC::logFile = NULL; 
#endif
MC*				MC::mc=NULL;
McState			MC::state = MCS_Idle;
BOOL			MC::usingTaskview = FALSE;
HANDLE			MC::mutexHandle = NULL;
McWList*		MC::windowList = NULL;
McPropsMgr*		MC::properties = NULL;
float			MC::dpi	= 0.0;
float			MC::scale = 0.0;
HWND			MC::_eventHwnd = NULL;
BOOL			MC::isMessageBoxActive = FALSE;
BOOL			MC::mm = FALSE;
BOOL			MC::md = FALSE;
char			MC::username[100];
char			MC::trueUsername[100];
char			MC::computername[MAX_COMPUTERNAME_LENGTH+1];
INT				MC::WM_SHELLHOOKMESSAGE = -1;
INT				MC::WM_SHELLHOOKREPEAT = -1;
BOOL			MC::isWindowsAnimating = TRUE;
BOOL			MC::isDropShadowOn = FALSE;
McIconMgr*		MC::iconManager = NULL;
ULONGLONG		MC::mouseWheelTime = 0;
HWND			MC::pressHwnd;
BOOL			MC::tabletMode = FALSE;
BOOL			MC::variableRotation = TRUE;
DWORD			MC::processId = 0;
BOOL			MC::_incycle = FALSE;	// Flag indicating that we're _in the midst of displaying the thumbnail pile

#define _LISTEN_CLASS_NAME L"Emcee.UI.Listener"
#define _LISTEN_WINDOW_NAME L"ListenerW"

#define SIGN(x)  ( ((x)<0) ?- 1 : 1 )

int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int nCmdShow )
{
	size_t tInstance = reinterpret_cast<size_t>(hInstance);
	srand( (unsigned int)tInstance );

	timeBeginPeriod(1);

	MC::setProcessId( GetCurrentProcessId( ) );
	MC::CreateProperties( );

	// This mutex prevents multiple instances from running at the same time.
	// If another instance is running we'll post a message simulating its
	// hot key response.  Not pretty, but effective.

	wchar_t wuser[100];
	ExpandEnvironmentStrings( L"%USERNAME%", wuser, 100 );
	char user[100], trueUser[100];
	MC::wchar2char( wuser, trueUser );
	size_t i, len = strlen( trueUser );
	for (i = 0; i < len; i++)
		if (!isalnum( trueUser[i] ))
			user[i] = '_';
		else
			user[i] = trueUser[i];
	user[i] = 0;
	MC::setUserName( 100, user, trueUser );
	wchar_t hostname[MAX_COMPUTERNAME_LENGTH + 1];
	DWORD hnlen = MAX_COMPUTERNAME_LENGTH + 1;
	GetComputerName( hostname, &hnlen );
	MC::setComputerName( hostname );
	wchar_t mutexname[1000];
	MC::char2wchar( user, wuser );
	swprintf_s( mutexname, 1000, L"_Emcee_%s:%s", hostname, wuser );
	MC::mutexHandle = CreateMutex( NULL, TRUE, mutexname );
	if (GetLastError( ) == ERROR_ALREADY_EXISTS)
	{
		MC::mutexHandle = NULL;
		// Determine if we are in Windows 10 Tablet Mode
		HWND tmcw = FindWindowEx( NULL, NULL, _MODERN_TABLET_MODE, NULL );
		BOOL tm = FALSE;
		if (tmcw)
			tm = IsWindowVisible( tmcw );
		MC::CreateProperties( );
		McHotKeyMgr::createHotKeyData( );
		McHotKeyMgr::injectHotKey( );
		MC::Exit( 0 );
	}
#if MC_LOGGING
	MC::Log("\n**** Emcee (x64) started as %S\n", mutexname);
#endif
	SetProcessDpiAwareness( PROCESS_PER_MONITOR_DPI_AWARE );
	McMonitorsMgr::CreateMonitorInformation( );
	// Initialize COM
	CoInitializeEx( NULL, COINIT_APARTMENTTHREADED ); 
	McModernAppMgr::Initialize( );
	McHookMgr::initializeMouseHooks( );
	// Start GDI+ 
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	GdiplusStartup( &gdiplusToken, &gdiplusStartupInput, NULL );
	// Figure out color scheme
	McModernAppMgr::DetermineModernColors( );
	// Initialize Icon Manager .. only happens once at program startup,
	// but the icon manager periodically looks for newly installed Windows Store apps.
	MC::createIconManager( );
	// Determine if we're starting _in resident or one-time mode
	char command_buffer[2000];
	MC::wchar2char( pCmdLine, command_buffer );
	BOOL rc = !strstr( command_buffer, "-resident" );
	if (strstr( command_buffer, "-onetime" ))
		MC::_runcycles( );
	else
		MC::_runlistenloop( hInstance, rc );
	GdiplusShutdown( gdiplusToken );
	McHookMgr::releaseMouseHooks( );
	McModernAppMgr::Uninitialize( );
	McMonitorsMgr::DeleteMonitorInformation( );
	CoUninitialize( );
	MC::DeleteProperties( );
	MC::releaseIconManager( );

	timeEndPeriod(1);
	
	MC::Exit(0);
}


MC::MC()
{
	if (mc) return;
	mc = this;
	runAgain = FALSE;
	mcAnimation = NULL;
	dpi = scale = 0.0;
	McPilesMgr::setNPiles(0);
	MC::setIsWindowsAnimating();
	MC::setIsDropShadowOn();
	SYSTEMTIME time;
	GetLocalTime(&time);
	// Read properties file (Emcee.cfg) if found
	properties->reread();
	// Some things might have changed, so reinitialize
	McModernAppMgr::DetermineModernColors();
	McMonitorsMgr::setEffectiveMonitorCount();
	MC::md = McMonitorsMgr::getMonitorCount() > 1;			// Does system have multiple mons.
	MC::mm = MC::getProperties()->getMultiMonitor() && md;	// Are we using multiple mons.
	McHookMgr::releaseMouseHooks();
	McHookMgr::initializeMouseHooks();
	for (list<McMonitorsMgr*>::iterator mi = McMonitorsMgr::monitorList.begin();
		mi != McMonitorsMgr::monitorList.end(); mi++)
	{
		McMonitorsMgr *monitor = *mi;
		if (MC::getProperties()->getMultiMonitor() || monitor->getIsMainMonitor())
			McRect *size = monitor->getMonitorSize();
	}
	// Create the wrapper to access window's animation interface, which
	// is efficient compared to simple threading implementations.
	mcAnimation = new McAnimationMgr();
	HRESULT hr = mcAnimation->Initialize();
	if ( !SUCCEEDED(hr) )
	{
		MC::Exit(95);
	}
	windowList = NULL;
}

void MC::start( )
{
	windowList = new McWList( );
	tc = new McTouchMgr( );
	// Group the windows into piles based on application name and
	// any grouping specified _in the properties file.   Within each
	// pile arrange the thumbnails _in a reasonable way.
	McWItem **dtItems = windowList->getDesktopItem( );
	int _nPiles = McPilesMgr::getNPiles( );
	for (int i = 0; i < McMonitorsMgr::getEffectiveMonitorCount( ); i++)
	{

		windowList->markVisibleW10Windows( dtItems[i]->getAppMonitor( ) );
		McPilesMgr *piles = new McPilesMgr( _nPiles, dtItems[i]->getAppMonitor( ) );
		windowList->positionBackground( dtItems[i], piles->getImmersiveAppOffset( ), piles->getImmersiveAppGap( ) );
		dtItems[i]->createDesktopWindow( );
		MC::setVariableRotation( );
		piles->createPiles( );
		MC::clearVariableRotation( );
		// Arrange the thumbnail piles _in the monitor space
		piles->layoutPiles( );
		_nPiles = piles->getNextPileNumber( );
		// Then map that information onto the positions for the individual
		// thumbnails.
		piles->positionThumbs( );
		McPilesMgr::setNPiles( _nPiles );
		if (dtItems[i]->getAppMonitor( ) == McMonitorsMgr::getMainMonitor( )->getHMONITOR( ))
		{
			// Tell main and bg windows if navigation arrows are required
			McRect leftArrowRect;
			if (piles->areArrowsNeeded( &leftArrowRect ))
				McWindowMgr::getBgW( )->setArrowsNeeded( &leftArrowRect );
			else
				McWindowMgr::getBgW( )->setArrowsNeeded( NULL );
		}
		delete piles;
	}
	McWindowMgr::getMainW( )->setStartButtonLocation( );
	// Create windows for each thumbnail
	windowList->sortWindows( );
	list<McWItem *> *itemList = windowList->getItemList( );

	windowList->createThumbWindows();
}

MC::~MC()
{
	// Fake Exit : Restores task bar if currently supressed
	Exit( _DO_NOT_EXIT );
	// Clean up
	if ( windowList ) delete windowList;
	if ( mcAnimation ) delete mcAnimation;
	// Get rid of the remaining windows
	McWindowMgr::getBgW( )->Deactivate( );
	McWindowMgr::getMainW( )->Deactivate( );
	McWindowMgr::getFadingW( )->Deactivate( );
	McWindowMgr::getBackingW( )->Deactivate( );
	McWindowMgr::getZoomW( )->Deactivate( );
	McWindowMgr::getLabelW( )->Deactivate( );
	if ( tc) delete tc;
	mc = NULL;
	state = MCS_Idle;
}

McMainW *MC::getMainWindowDelegate( )
{
	return McWindowMgr::getMainW( );
}

void	MC::setVanishingPoint( McRect *thumb, McRect *vanish, BOOL isOnAppBar, BOOL isCloaked, BOOL obscuredApp, POINT *mdCenter )
{ 
	// Minimized and cloaked windows have to vanish somewhere, and the animation
	// into the windows taskbar is something I've never liked.   So, like
	// old soldiers they just fade away.    Windows obscured by full screen windows appear
	// to grow from the screen center, which reduces graphics overhead.
	if (obscuredApp)
	{
		vanish->top = vanish->bottom = mdCenter->y;
		vanish->left = vanish->right = mdCenter->x;
	}
	else if (isCloaked)
	{
		if (MC::inTabletMode( ))
		{
			vanish->top = vanish->bottom = mdCenter->y;
			vanish->left = vanish->right = mdCenter->x;
		}
		else
			vanish->set( thumb );
	}
	else
	{
		if (isOnAppBar)
		{
			vanish->top = vanish->bottom = thumb->bottom;
			vanish->left = thumb->left;
			vanish->right = thumb->right;
		}
		else
		{
			vanish->left = vanish->right = (thumb->left + thumb->right) / 2;
			vanish->top = vanish->bottom = (thumb->top + thumb->bottom) / 2;
		}
	}
}

BOOL MC::openAppFile( char *_filename, char *_permissions, FILE **file, char *rPath, char *rDirectory, BOOL logit )
{
	// OPEN APPLICATION FILE IN THE %LOCALAPPDATA% DIRECTORY
	// IF WE'RE OPENING IT FOR READING & THE DIRECTORY DOESN'T
	// EXIST, OPEN THE DEFAULT FILE IN THE (PROGRAM FILES) DIRECTORY
	// AND RETURN TRUE, WHICH WILL CAUSE THE DEFAULT VALUES TO BE
	// WRITTEN INTO A FILE IN THE DATA DIRECTORY.
	// OTHER WISE RETURN FALSE;
	BOOL amReading = _permissions[0]=='r';
	FILE *f = NULL;
	// Get path to local %LOCALAPPDATA%
	wchar_t wfileroot[1000];
	ExpandEnvironmentStrings( L"%LOCALAPPDATA%", wfileroot, 1000 );
	// get path to %LOCALAPPDATA%\Emcee
	wchar_t wdirectory[1000];
	swprintf_s( wdirectory, 1000, L"%s\\%s", wfileroot, L"Emcee" );
	char directory[1000];
	wchar2char( wdirectory, directory );
	if ( rDirectory )
		strcpy_s( rDirectory, 1000, directory);
	// See if the directory exists
	DWORD dwAttrib = GetFileAttributes( wdirectory );
	BOOL dirExists =	dwAttrib != INVALID_FILE_ATTRIBUTES &&
						( dwAttrib & FILE_ATTRIBUTE_DIRECTORY );
	char filename[1000];
	if ( !dirExists )	// Directory doesn't exist
	{
		SECURITY_ATTRIBUTES secAttrib;
		secAttrib.nLength = sizeof( SECURITY_ATTRIBUTES );
		secAttrib.lpSecurityDescriptor = NULL;
		secAttrib.bInheritHandle = TRUE;
		CreateDirectory( wdirectory, &secAttrib );
	}
	sprintf_s( filename, 1000, "%s\\%s", directory, _filename );
	errno_t err = fopen_s( file, filename, (const char *)_permissions );
#if MC_LOGGING
	//if ( logit )		MC::Log("***Result %d from Attempting to open %s\n", err,filename);
#endif
	if ( err && amReading )
	{
		char buff[1000];
		GetEnvironmentVariableA( "ProgramFiles", buff, 1000 );
		strcat_s( buff, 1000, "\\" );
		err = sprintf_s( filename, 1000, "%sEmcee\\default_%s", buff,_filename );
		err = fopen_s( file, filename, (const char *)_permissions );
#if MC_LOGGING
		//if ( logit )			MC::Log("***Result %d from Attempting to open %s\n", err,filename);
#endif
	}

	if (err)
		if ( file ) *file = NULL;
	if ( rPath )
		strcpy_s( rPath, 1000, filename );
	return (err == 0);
}

void MC::reportError( BOOL dialog, char *msg, HWND hwnd )
{
	if ( !dialog )
	{
	wchar_t wbuff[1000];
	char    cbuff[1000];
	FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		wbuff, 1000, NULL );
	wchar2char( wbuff, cbuff );
	}
	if ( dialog )
	{
		wchar_t wmsg[1000];
		char2wchar( msg, wmsg );
		MessageBeep( MB_OK );
		MC::doMessageBox( hwnd, wmsg, L"Emcee Error ...", MB_OK|MB_ICONSTOP );
	}
}

BOOL MC::HandleShutdownMessage( char *location, UINT uMsg, WPARAM, LPARAM lParam )
{
	char *message = "<Unknown WM_>";
	BOOL killit = TRUE;
	if ( uMsg == WM_QUERYENDSESSION )
		message = "WM_QUERYENDSESSION";
	else if ( uMsg == WM_ENDSESSION )
	{
		message = "WM_ENDSESSION";
		MC::Exit( 666 );
	}
	return TRUE;
}

int MC::doMessageBox( HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType )
{
	isMessageBoxActive = TRUE;
	int outcome = MessageBox( hWnd, lpText, lpCaption, uType|MB_SYSTEMMODAL|MB_DEFBUTTON1|MB_TOPMOST );
	isMessageBoxActive = FALSE;
	return outcome;
}

void MC::setIsDropShadowOn( )
{
	// In windows 10, drop shadow is a setting the user controls, and we match it here.   If drop 
	// shadow is off then thumbnails get a 1 pixel black boarder.
	HKEY rootKey;
	LONG lresult;
	lresult = RegOpenKeyEx(
		HKEY_CURRENT_USER,
		TEXT( "Control Panel\\Desktop" ),
		0,
		KEY_QUERY_VALUE, &rootKey );
	if (lresult != ERROR_SUCCESS)
	{
		MC::isWindowsAnimating = TRUE;
		return;
	}
	BYTE value[8];
	DWORD size = 8*sizeof( BYTE );
	lresult = RegQueryValueEx( rootKey, TEXT( "UserPreferencesMask" ), NULL, NULL, 
		(LPBYTE) value, &size );
	if (lresult != ERROR_SUCCESS)
	{
		MC::isDropShadowOn = TRUE;
		RegCloseKey( rootKey );
		return;
	}
	MC::isDropShadowOn = ((value[2]&0x4) != 0);
	RegCloseKey( rootKey );
}

void MC::setIsWindowsAnimating( )
{
	// It matters if the user has animation enabled, but only for timing coordination
	// with explorer's actions.
	HKEY rootKey;
	LONG lresult;
	lresult = RegOpenKeyEx(
		HKEY_CURRENT_USER,
		TEXT( "Control Panel\\Desktop\\WindowMetrics" ),
		0,
		KEY_QUERY_VALUE, &rootKey );
	if (lresult != ERROR_SUCCESS)
	{
		MC::isWindowsAnimating = TRUE;
		return;
	}
	wchar_t value[256] = { 0 };
	DWORD size = sizeof(256*sizeof(wchar_t));
	lresult = RegQueryValueEx( rootKey, TEXT( "MinAnimate" ), NULL, NULL, (LPBYTE) &value, &size );
	if (lresult != ERROR_SUCCESS)
	{
		MC::isWindowsAnimating = TRUE;
		RegCloseKey( rootKey );
		return;
	}
	MC::isWindowsAnimating = (value[0] != L'0');
	RegCloseKey( rootKey );
}

// A window is created here because it can receive messages
// posted by other processes.   That is the mechanism used
// _in -resident mode to let a second instance signal that
// the it was started and thus mc display cycle should be run.
// It is a message only window.

void MC::_runlistenloop( HINSTANCE hInstance, BOOL rc )
{
	WNDCLASS wc = { 0 };
	wc.lpfnWndProc = _windowproc;
	wc.hInstance = hInstance;
	wc.lpszClassName = _LISTEN_CLASS_NAME;
	RegisterClass( &wc );
	// We create the window as a frameless message window
	MC::_eventHwnd = CreateWindow(
		_LISTEN_CLASS_NAME,
		_LISTEN_WINDOW_NAME,
		WS_CHILD,
		0, 0, 0, 0,
		HWND_MESSAGE,
		NULL,
		hInstance,
		NULL );
	if (!McHotKeyMgr::pushHotKey( MC::_eventHwnd ))
		MC::Exit( 99 );
	WM_SHELLHOOKMESSAGE = RegisterWindowMessage( TEXT( "SHELLHOOK" ) );
	WM_SHELLHOOKREPEAT = RegisterWindowMessage( TEXT( "SHELLHOOKR" ) );
	// We're going to monitor shell window messages to know if an existing window goes
	// away or a new window appears -- this only matters when Emcee is active and showing
	// thumbnails.
	RegisterShellHookWindow( _eventHwnd );
	
	NOTIFYICONDATA nid = { sizeof(nid) };
	nid.hWnd = MC::_eventHwnd;
	nid.uID = 1001;
	nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP | NIF_INFO;
	nid.uCallbackMessage = WM_TRAYICON;
	nid.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MC));
	wcscpy_s(nid.szTip, L"Mission Control");
	wcscpy_s(nid.szInfo, L"Mission Control is now running. The Win+Tab shortcut has been replaced to activate it.");
	wcscpy_s(nid.szInfoTitle, L"Mission Control Started");
	nid.dwInfoFlags = NIIF_INFO;
	Shell_NotifyIcon(NIM_ADD, &nid);

	McHookMgr::initializeKbHooks();

	// if (rc) _runcycles( );
	MSG msg = {};
	BOOL bRet;
	while (0 != (bRet = GetMessage( &msg, NULL, 0, 0 )))
	{
		if (bRet == -1) 
			break;
		TranslateMessage(&msg);
#if MC_LOGGING
		//MC::Log("(outer) %04x %04x %08x\n", msg.message, msg.wParam, msg.lParam);
#endif
		DispatchMessage( &msg );
	}
	DeregisterShellHookWindow( _eventHwnd );

	NOTIFYICONDATA nid_del = { sizeof(nid_del) };
	nid_del.hWnd = MC::_eventHwnd;
	nid_del.uID = 1001;
	Shell_NotifyIcon(NIM_DELETE, &nid_del);

	McHookMgr::releaseKbHooks();

	// If this ever happens something has gone amiss.
	DestroyWindow( MC::_eventHwnd );
	MC::_eventHwnd = NULL;
}

void MC::simulateCloseWindow( HWND hwnd )
{
	PostMessage( _eventHwnd, WM_SHELLHOOKMESSAGE, HSHELL_WINDOWDESTROYED, (LPARAM)hwnd );
}

// Some rigamarole to delay processing of an event for half a second to let other stuff finish.
static HWND _local_hwnd;
static LPARAM _local_lParam;
static UINT _local_uMsg;
static WPARAM _local_wParam;

 static DWORD CALLBACK _local_ThreadProc( LPVOID )
{
	Sleep( 500 );
	PostMessage( _local_hwnd, /*_local_uMsg*/MC::WM_SHELLHOOKREPEAT, _local_wParam, _local_lParam );
	return 0;
}

static void doProcessDelay( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	_local_hwnd = hwnd;
	_local_lParam = lParam;
	_local_uMsg = uMsg;
	_local_wParam = wParam;
	DWORD threadId;
	CreateThread( NULL, 0, _local_ThreadProc, NULL, 0, &threadId );
}


#pragma managed
SETTINGS_DISPOSITION doSettings(list<string>*);
#pragma unmanaged

LRESULT CALLBACK MC::_windowproc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	BOOL repeat = FALSE;
	if ( (uMsg == WM_SHELLHOOKMESSAGE) && 
		( wParam == 53 || wParam == 54 ) )
	{

		char cn[1000];
		char cnn[1000];
		BOOL error = FALSE;
		error = 0 >= GetClassNameA( (HWND) lParam, cn, 1000 );
		GetWindowTextA( (HWND) lParam, cnn, 1000 );
		if (error) return S_OK;
		if (strstr( cn, _MODERN_VIEWFRAME_BYTE ))
		{
			DWORD pid;
			DWORD tid = GetWindowThreadProcessId( (HWND) lParam, &pid );
			GetWindowTextA( (HWND) lParam, cn, 1000 );
			if (strcmp( cn, _MODERN_TASKVIEW_BYTE ))
				return S_OK;
			HMONITOR mon = MonitorFromWindow( (HWND) lParam, MONITOR_DEFAULTTONEAREST );
			BOOL main = TRUE;
			// Dont use McMonitorInfo because it may not exist yet
			MONITORINFO minfo;
			if (mon)
			{
				McMonitorsMgr::myGetMonitorInfo( mon, &minfo );
				main = minfo.dwFlags & MONITORINFOF_PRIMARY;
			}
			if (wParam == 53)
			{
				if (usingTaskview)
					return S_OK;
				return DefWindowProc( hwnd, uMsg, wParam, lParam );
			}
			else
				if (main)
					usingTaskview = FALSE;
			return S_OK;
		}
	}
	switch (uMsg)
	{
	case WM_TRAYICON:
		if (lParam == WM_LBUTTONUP || lParam == WM_RBUTTONUP)
		{
			POINT pt;
			GetCursorPos(&pt);
			HMENU hMenu = CreatePopupMenu();
			
			HKEY hKey;
			BOOL bRunAtStartup = FALSE;
			if (RegOpenKeyEx(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_READ, &hKey) == ERROR_SUCCESS)
			{
				if (RegQueryValueEx(hKey, L"MissionControl", NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
				{
					bRunAtStartup = TRUE;
				}
				RegCloseKey(hKey);
			}

			AppendMenu(hMenu, MF_STRING, 1003, L"Settings");
			AppendMenu(hMenu, MF_STRING | (bRunAtStartup ? MF_CHECKED : MF_UNCHECKED), 1002, L"Run at Startup");
			AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
			AppendMenu(hMenu, MF_STRING, 1001, L"Exit");
			
			SetForegroundWindow(hwnd);
			int cmd = TrackPopupMenu(hMenu, TPM_RETURNCMD | TPM_NONOTIFY, pt.x, pt.y, 0, hwnd, NULL);
			PostMessage(hwnd, WM_NULL, 0, 0);
			DestroyMenu(hMenu);
			
			if (cmd == 1001)
			{
				MC::Exit(0);
			}
			else if (cmd == 1002)
			{
				if (RegOpenKeyEx(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_WRITE, &hKey) == ERROR_SUCCESS)
				{
					if (bRunAtStartup)
					{
						RegDeleteValue(hKey, L"MissionControl");
					}
					else
					{
						WCHAR szPath[MAX_PATH];
						GetModuleFileName(NULL, szPath, MAX_PATH);
						RegSetValueEx(hKey, L"MissionControl", 0, REG_SZ, (BYTE*)szPath, (DWORD)(wcslen(szPath) + 1) * sizeof(WCHAR));
					}
					RegCloseKey(hKey);
				}
			}
			else if (cmd == 1003)
			{
				if (MC::getState() == MCS_Idle)
				{
					// If we're idle, we need to temporarily enter settings state and bring up the settings UI
					MC::setState(MCS_Settings);
					SETTINGS_DISPOSITION res = doSettings(NULL);
					MC::setState(MCS_Idle);
					
					// Reinitialize hooks if settings changed
					if (res == settingsAccept)
					{
						MC::getProperties()->write();
					}
					McHookMgr::releaseKbHooks();
					McHookMgr::initializeKbHooks();
				}
				else if (McWindowMgr::getMainW() != NULL)
				{
					// If the main window is active, send it the settings message to handle properly
					PostMessage(McWindowMgr::getMainW()->getHwnd(), WM_USER, WMMC_DOSETTINGS, 0);
				}
			}
		}
		return 0;
	case WM_QUERYENDSESSION:
	case WM_ENDSESSION:
		return MC::HandleShutdownMessage( "_windowProc", uMsg, wParam, lParam );
	case WM_CREATE:
		return 0;
	case WM_USER:
		if (wParam == WMMC_ENDMC)
		{
			if (_incycle && McWindowMgr::getMainW( ))
				McWindowMgr::getMainW( )->onFinish( );
		}
		else if (wParam == WMMC_STARTMC )
		{
			if (_incycle)
				return 0;
			MC::_runcycles( );
		}
		return 0;
	case WM_HOTKEY:
		// Turn off hotkey monitoring while running, and turn it back on when done.
		// While running a separate trap of the hot key is one way to restore the
		// normal desktop view.
		if (_incycle)
			return 0;
		McHotKeyMgr::popHotKey( hwnd );
		_runcycles( );
		if (!McHotKeyMgr::pushHotKey( hwnd ))
		{
			MC::Exit( 98 );
		}
		return 0;
	default:
		if (!MC::inTabletMode( ))
		{
			if (uMsg == WM_SHELLHOOKREPEAT)
			{
				uMsg = WM_SHELLHOOKMESSAGE;
				repeat = TRUE;
			}
			if (uMsg == WM_SHELLHOOKMESSAGE)
			{
				if ((wParam != HSHELL_WINDOWDESTROYED) && (wParam != HSHELL_WINDOWCREATED))
				{
					return DefWindowProc( hwnd, uMsg, wParam, lParam );
				}
				if (_incycle && McWindowMgr::getZoomW( ))
				{
					HWND mysteryAppHwnd = (HWND) lParam;
					char classname[1000];
					if (0 == GetClassNameA( mysteryAppHwnd, classname, 1000 ))
						classname[0] = 0;
					else
						if (strstr( classname, "Emcee." ) || strstr( classname, "GDI+ Hook" ))
							return S_OK;
					McState state = MC::getState( );
					if (wParam == HSHELL_WINDOWCREATED || wParam == HSHELL_WINDOWDESTROYED)
					{
						if (state != MCS_Display || McDragMgr::isDragging( ))
						{
							if (hwnd)
								doProcessDelay( hwnd, uMsg, wParam, lParam );
							return DefWindowProc( hwnd, uMsg, wParam, lParam );
						}
					}
					if (wParam == HSHELL_WINDOWCREATED)
					{
						if (!strcmp( classname, _MODERN_MODERN10_APP_BYTE ))
							return S_OK;
						if (!strcmp( classname, _MODERN_MODERN_APP_BYTE ))
						{
							mysteryAppHwnd = GetAncestor( mysteryAppHwnd, GA_PARENT );
							if (0 == GetClassNameA( mysteryAppHwnd, classname, 1000 ))
								classname[0] = 0;
							else
								if (strcmp( classname, _MODERN_MODERN10_APP_BYTE ))
								{
									if (!repeat)
										doProcessDelay( hwnd, uMsg, wParam, lParam );
									return S_OK;
								}
						}
						MC::getWindowList( )->handleNewWindow( mysteryAppHwnd );
					}
					else
						if (wParam == HSHELL_WINDOWDESTROYED)
						{
							if (McWindowMgr::getZoomW( )->isItZoomed( mysteryAppHwnd ))
							{
								MC::getMC( )->setRunAgain( TRUE );
								ShowWindow( McWindowMgr::getBackingW( )->getHwnd( ), SW_HIDE );
								ShowWindow( McWindowMgr::getZoomW( )->getHwnd( ), SW_HIDE );
								McWindowMgr::getMainW( )->onFinish( );
								return S_OK;
							}
							McWItem *item = NULL;
							McWList *windowList = MC::getWindowList( );
							if (windowList)
							{
								McWItem *item = NULL;
								if (item = MC::getWindowList( )->findItem( mysteryAppHwnd ))
								{
									MC::getWindowList( )->removeKilledApp( item, "HSHELL_WINDOWDESTROYED" );
								}
							}
						}
					return S_OK;
				}
			}
		}
		return DefWindowProc( hwnd, uMsg, wParam, lParam );
	}
}

void MC::_runcycles(  )
{
	if (_incycle)
		return;
	_incycle = TRUE;
	// Get rid of start menu, action center and the like.
	McModernAppMgr::RemoveAnimatedSystemWindows(  );
	McMonitorsMgr::CreateMonitorInformation( TRUE );
	// Determine if we are in Windows 10 Tablet Mode
	HWND tmcw = FindWindowEx( NULL, NULL, _MODERN_TABLET_MODE, NULL );
	if (!tmcw)
		tabletMode = FALSE;
	else
		tabletMode = IsWindowVisible( tmcw );
	BOOL runAgain = TRUE;
	while (runAgain)
	{
#if MC_LOGGING
		MC::Log("+++ _runcycle()\n");
#endif
		runAgain = _runcycle( );
#if MC_LOGGING
		MC::Log("--- _runcycle() complete result = %d\n", runAgain);
#endif
	}
	_incycle = FALSE;
}

BOOL MC::_runcycle( )
{
	MC *mc = MC::getMC( );	// Create the MC Controller
	// Get the key windows we need
	McWindowMgr::allocateBgW( );
	McWindowMgr::allocateFadingW( );
	McWindowMgr::allocateMainW();
	mc->start( );			// This triggers building list of windows,
	McWindowMgr::allocateBackingW( );
	McWindowMgr::allocateZoomW( );
	McWindowMgr::allocateLabelW( );
	// Git going.
	
	McWindowMgr::getMainW( )->start( );
	// Message loop
	// Note this message loop runs inside another message loop when
	// running _in resident mode.   Either elegant or a kludge, hard
	// to say which.
	MSG msg = { 0 };
	while ((MC::getState( ) != MCS_Finishing) && GetMessage( &msg, NULL, 0, 0 ))
	{
		TranslateMessage( &msg );
#if MC_LOGGING
		//MC::Log("(inner) %04x %04x %08x\n", msg.message, msg.wParam, msg.lParam);
#endif
		DispatchMessage( &msg );
	}
	BOOL rval = mc->getRunAgain( );
	MC::setState( MCS_Idle );
	delete mc;
	
	return rval;
}

void MC::_triggerHotKey( )
{
	McHotKeyMgr::createHotKeyData( );
	McHotKeyMgr::injectHotKey( );
}

// utility functions to read images and bitmaps

IStream * MC::CreateStreamOnResource( LPCTSTR lpName, LPCTSTR lpType )
{
	IStream * ipStream = NULL;
	HGLOBAL hgblResourceData = NULL;
	HRSRC hrsrc = FindResource( NULL, lpName, lpType );
	if (hrsrc)
	{
		DWORD dwResourceSize = SizeofResource( NULL, hrsrc );
		HGLOBAL hglbImage = LoadResource( NULL, hrsrc );
		if (hglbImage)
		{
			LPVOID pvSourceResourceData = LockResource( hglbImage );
			if (pvSourceResourceData)
			{
				hgblResourceData = GlobalAlloc( GMEM_MOVEABLE, dwResourceSize );
				if (hgblResourceData)
				{
					LPVOID pvResourceData = GlobalLock( hgblResourceData );
					if (pvResourceData)
					{
						CopyMemory( pvResourceData, pvSourceResourceData, dwResourceSize );
						GlobalUnlock( hgblResourceData );
						if (SUCCEEDED( CreateStreamOnHGlobal( hgblResourceData, TRUE, &ipStream ) ))
							hgblResourceData = NULL;
					}
				}
			}
		}
	}
	if (hgblResourceData)
		GlobalFree( hgblResourceData );
	return ipStream;
}

BOOL MC::loadImage( WORD idi, Image **iPointer )
{
	if (!iPointer) return FALSE;
	IStream *stream = MC::CreateStreamOnResource( MAKEINTRESOURCE( idi ), L"IMAGE" );
	(*iPointer) = new Image( stream );
	stream->Release( );
	return TRUE;
}

BOOL MC::loadBitmap( WORD idi, Bitmap **bPointer )
{
	if (!bPointer) return FALSE;
	Image *rawImage = NULL;
	MC::loadImage( idi, &rawImage );
	int width = rawImage->GetWidth( );
	int height = rawImage->GetHeight( );
	(*bPointer) = new Bitmap( width, height, PixelFormat32bppARGB );
	Graphics gr( *bPointer );
	gr.DrawImage( rawImage, 0, 0, (int) width, height );
	delete rawImage;
	return TRUE;
}

void MC::createIconManager( )
{
	if (!iconManager)
	{
		iconManager = new McIconMgr( );
		// Since the icon manager has to scan the registry then load image files
		// and icon bitmaps, it may take a little while
		while (!iconManager->getReady( ))
			Sleep( 10 );
	}
}

void MC::releaseIconManager( )
{
	if (iconManager)
	{
		iconManager->Stop( );
		delete iconManager;
	}
	iconManager = NULL;
}

void McRect::interpolate( McRect *start, McRect *finish, double value, BOOL funny )
{

	if (funny)
	{
		double v2 = value;
		v2 *= value;
		long smid = (start->left + start->right) / 2;
		long fmid = (finish->left + finish->right) / 2;
		long mid = (LONG) floor( .5 + v2*smid + (1.0 - v2)*fmid );
		long www = (LONG) floor( .5 + value*(start->getWidth( )) + (1.0 - value)*finish->getWidth( ) ) / 2;
		left = mid - www;
		right = mid + www;
	}
	else
	{
		left = (LONG) floor( .5 + value*start->left + (1.0 - value)*finish->left );
		right = (LONG) floor( .5 + value*start->right + (1.0 - value)*finish->right );
	}
	top = (LONG) floor( .5 + value*start->top + (1.0 - value)*finish->top );
	bottom = (LONG) floor( .5 + value*start->bottom + (1.0 - value)*finish->bottom );
}

unsigned int MC::GetWText( HWND hwnd, char *outBuff, BOOL shorten )
{
	WCHAR buff1[1000];
	int len = GetWindowText( hwnd, buff1, 1000 );
	WCHAR buff2[1000] = { 0 };
	int p1 = 0;
	int p2 = 0;
	while (buff1[p1] > 0)
	{
		if (buff1[p1] == 8206)
		{
			p1++;
		}
		buff2[p2++] = buff1[p1++];
	}
	WCHAR *b = buff2;
	if (shorten)
	{
		WCHAR *token = NULL;
		while (token = wcsstr( b, L" - " ))
			b = token + 3;
	}
	while (*b == L' ') b++;
	if (outBuff)
	{
		MC::wchar2char( b, outBuff );
		return (unsigned int)strlen( outBuff );
	}
	return 0;
}

#if MC_LOGGING
void MC::Log(char *fmt, ...)
{
	char cbuff[1000] = { 0 };
	va_list args;
	va_start(args, fmt);
	vsprintf_s(cbuff, 1000, fmt, args);
	va_end(args);

	char logName[1000];
	sprintf_s(logName, 1000, "Emcee.log");
	openAppFile(logName, "a", &MC::logFile, NULL, NULL, FALSE);

	if (MC::logFile)
	{
		if (cbuff[0])
			fprintf(logFile, cbuff);
		else
			fprintf(MC::logFile, "<line deliberately left blank>\n");
		fclose(MC::logFile);
		MC::logFile = NULL;
	}
}
#endif
