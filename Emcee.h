#pragma once
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Mission Control for Windows 11 by pixeru
// Source code originally by Emcee (https://sourceforge.net/projects/mcsoft/)
// Extensive development by pixeru due to discontinued development of original project.
#include "stdafx.h"

// If compiling under Visual Studio 2013 or earlier versions, the IVirtualDesktopManager interface
// is not availble without some trickery to get the Windows 10 SDK working with the older VS Version.
// If you don't want to do that, set MC_DESKTOPS to FALSE to turn off Emcee's multiple desktop functionality.

#define MC_DESKTOPS TRUE
#define MC_LOGGING FALSE

#define _EMCEE_VERSION "ShareWare:1.7 (x64)" /* Version */

#include "resource.h"
#include "McRect.h"
#include "McPropsMgr.h"
#include "McTouchMgr.h"

// Various Useful Defines

#define GENERIC_HRESULT_FAILURE (0x80000000|ERROR_BAD_COMMAND)
#define VK_KILLKEY	0x4B	/*K*/
#define VK_A_KEY	0x41	/*A*/
#define VK_B_KEY	0x42	/*B*/
#define VK_D_KEY	0x44	/*D*/
#define VK_H_KEY	0x48	/*H*/
#define VK_L_KEY	0x4C	/*L*/
#define VK_M_KEY	0x4D	/*M*/
#define VK_N_KEY	0x4E	/*N*/
#define VK_P_KEY	0x50	/*P*/
#define VK_Q_KEY	0x51	/*Q*/
#define VK_R_KEY	0x52	/*R*/
#define VK_S_KEY	0x53	/*S*/
#define VK_T_KEY	0x54	/*T*/
#define VK_U_KEY	0x55	/*U*/
#define VK_X_KEY	0x58	/*Y*/
#define VK_Z_KEY	0x5A	/*Z*/
#define VK_IS_KEY_PRESSED( key ) ( GetKeyState(key)&0x80 )
#define VK_IS_KEY_TOGGLED( key ) ( GetKeyState(key)&0x01 )

#pragma warning( disable : 4793 )

#pragma managed(push,off)

// Special (locally used) exit code

#define _DO_NOT_EXIT		-999

// Various default sizes

#define _DEFAULT_ICON_SIZE 40.0f
#define _DEFAULT_ICON_PADDING (_DEFAULT_ICON_SIZE+10.0f)
#define _DEFAULT_LABEL_WIDTH 100.0f

// Results returned from doSettings(). Here instead of McSettingsEditor.h to avoid managed/unmanaged conflicts

enum SETTINGS_DISPOSITION
{
	settingsAccept = 1,
	settingsCancel = 2,
	settingsOther = 3
};

// PROGRAM STATE

enum McState
{
	MCS_Idle,					// Initializing
	MCS_TransitionDown,			// going from desktop to thumbnails
	MCS_Display,				// navigating thumbnails
	MCS_Settings,				// doing settings
	MCS_TransitionUp,			// Going from thumbnails to desktop
	MCS_Zoom,					// Zooming
	MCS_TransitionOver,			// going from thumbnails to thumbnails
	MCS_TransitionImmersiveApp,	// moving only app bar thumbnails
	MCS_TransitionDesk,			// opposite of the above
	MCS_TransitionNone,			// no animation needed
	MCS_Finishing				// all done final transition
};

// USER MESSAGES, delivered as wParam portion of posted WM_USER message

#define WMMC_DOFLUSH		0x0000	// Flush Message Queue
#define WMMC_TRANSITIONDOWN	0x0001	// Enter State MCS_TransitionDown
#define WMMC_TRANSITIONUP	0x0002	// Restore desktop
#define WMMC_TRANSITIONOVER 0x0003	// Move thumbnails
#define WMMC_DISPLAY		0x0004	// Run Display Mode
#define WMMC_HIDE			0x0005	// Hide zoom window
#define WMMC_FOCUS			0x0006	// Take focus
#define WMMC_DESTROY		0x0007  // Done, Fini.
#define WMMC_SELECT			0x0008	// Select a thumb window
#define WMMC_BGCHANGED	    0x0009	// McModernAppMgr bg color changed
#define WMMC_LAUNCHER		0x000A	// Launcher visibility event
#define WMMC_STARTMC		0x000C	// Allows external app to force thumb view
#define WMMC_ENDMC			0x000D	// Allows external app to end thumb view
#define WMMC_REINIT         0x000E	// Do a full reinit of thumbnail view
#define WMMC_TOUCH          0x000F  // TOUCH messge
#define WMMC_DOSETTINGS		0x0010	// Do Settings
#define WMMC_DOSTART		0x0011	// Do Start Menuu
#define WMMC_DOTASKVIEW		0x0012	// Do Task View
#define WMMC_SHIFTLEFT		0x0013	// Shift Left on App Bar
#define WMMC_SHIFTRIGHT		0x0014	// Shift Right on App Bar
#define WMMC_SHOWLABEL      0x0015	// Enable Labels
#define WMMC_EXIT			0x00FF	// Exiting

#define WM_TRAYICON         (WM_USER + 10)

// Classes referenced in this header only as pointers.

class McMainW;
class McBgW;
class McThumbW;
class McDesktopW;
class McZoomW;
class McLabelW;
class McBackingW;
class McWItem;
class McWList;
class McAnimationMgr;
class McPilesMgr;
class McMonitorsMgr;
class McIconMgr;
class McLabelW;
class Gdiplus::Image;
class Gdiplus::Bitmap;

// Provides two things: an MC object is created as part of being in thumbnail mode,
// and the MC class provides bunches of static utility methods.
// Either kludgy or elegant depending on mood.  Uninteresting historical reasons.

class MC
{

public:

	~MC( );

	static void		setVanishingPoint( McRect *start, McRect *vanish, BOOL isOnAppBar,
		BOOL isCloaked = FALSE,
		BOOL obscuredApp = FALSE,
		POINT *mdCenter = NULL );
	McAnimationMgr *getAnimationMgr( ) { return mcAnimation; }

	// PURE CONVENIENCE FUNCTIONS CODED IN HEADER

	void			setRunAgain( BOOL _runAgain ) { runAgain = _runAgain; }
	BOOL			getRunAgain( void ) { return runAgain; }
	void			start( );

private:

	MC( );
	McAnimationMgr*	mcAnimation;
	BOOL			runAgain;
	RECT			savedWorkArea;
	McTouchMgr*		tc;

public:

	// Globally accessible static methods

#if MC_LOGGING
	static void Log(char *fmt, ...);
#endif
	static unsigned int GetWText( HWND, char *, BOOL = FALSE );
	static IStream * CreateStreamOnResource( LPCTSTR, LPCTSTR );
	static BOOL loadImage( WORD idi, Gdiplus::Image **iPointer );
	static BOOL loadBitmap( WORD idi, Gdiplus::Bitmap **bPointer );
	static void createIconManager( );
	static void releaseIconManager( );
	static McIconMgr *getIconMgr( )
	{
		return iconManager;
	}

	static BOOL doShowWindow( HWND hwnd, int nCmdShow, BOOL w10 = FALSE )
	{
		if (w10 && nCmdShow != SW_RESTORE)
			return ShowWindowAsync( hwnd, nCmdShow );
		else
			return ShowWindow( hwnd, nCmdShow );
	}

	static void recordHwnd( HWND hwnd )
	{
		pressHwnd = hwnd;
	}

	static BOOL matchHwnd( HWND hwnd )
	{
		if (hwnd == pressHwnd) return TRUE;
		pressHwnd = NULL;
		return FALSE;
	}

	static McMainW *MC::getMainWindowDelegate( );

	static void Exit( int code )
	{
		if (code != _DO_NOT_EXIT)
		{
			exit( code );
		}
	}

	// Create MC object if necessary.   Otherwise return

	static MC* getMC( )
	{
		if (mc == 0)
			new MC( );
		return mc;
	}

	static McTouchMgr* getTouchController( )
	{
		MC *m = getMC( );
		return m->tc;
	}

	// Stupid shortcut conversions between char and wchar

	static WCHAR* char2wchar( char *cstr, WCHAR *wstr )
	{
		size_t cc = 0;
		mbstowcs_s( &cc, wstr, 1 + strlen( cstr ), cstr, _TRUNCATE );
		return wstr;
	}

	static char *wchar2char( WCHAR *wstr, char *cstr )
	{
		size_t cc = 0;
		size_t len = wcslen( wstr );
		int res = wcstombs_s( &cc, cstr, len+2, wstr, len );
		if (res == EILSEQ)
		{
			for (int i = 0; i < len; i++)
				if (wstr[i]>255) wstr[i] = 32;
			res = wcstombs_s( &cc, cstr, len+2, wstr, len );
		}

		return cstr;
	}

	static void reportError( BOOL dialog, char *msg, HWND hwnd );

	static McState  getState( ) { return state; }
	static void 	setState( McState _state )
	{
		state = _state;
	}

	static ULONGLONG	getWheelTime( ) {
		return mouseWheelTime;
	}

	static void setWheelTime( ULONGLONG wt ) {
		mouseWheelTime = wt;
	}

#if MC_LOGGING
	static FILE*			logFile;
#endif

	static BOOL				getMM( )				{ return mm; }
	static BOOL				getMD( )				{ return md; }
	static McWList*			getWindowList( )		{ return windowList; }
	static McPropsMgr*		getProperties( )		{ return properties; }

	static BOOL				HandleShutdownMessage( char *, UINT, WPARAM, LPARAM );
	static int				doMessageBox( HWND, LPCTSTR, LPCTSTR, UINT );
	static BOOL				getIsMessageBoxActive( ){ return isMessageBoxActive; }
	static BOOL				inTabletMode( ) { return tabletMode; }
	static DWORD			getProcessId( ) { return processId; }
	static void				setProcessId( DWORD pid ) { processId = pid; }
	static char*			getComputerName( ) { return computername; }
	static wchar_t*			getComputerNameL( wchar_t *cn )
	{
		if (cn)
			char2wchar( computername, cn );
		return cn;
	}

	static void	setComputerName( wchar_t *cn )
	{
		wchar2char( cn, computername );
	}

	static char*			getUserName( ) { return username; }
	static char*			getTrueUserName( ) { return trueUsername; }
	static wchar_t*			getUserNameL( wchar_t *un )
	{
		if (un)
			char2wchar( username, un );
		return un;
	}
	static void setUserName( unsigned len, char *user, char *trueUser )
	{
		strcpy_s( username, len, user );
		strcpy_s( trueUsername, len, trueUser );
	}

	static void CreateProperties( ) { properties = new McPropsMgr( ); }
	static void DeleteProperties( ) { delete properties; }
	static BOOL getIsWindowsAnimating( ) { return isWindowsAnimating; }
	static void setIsWindowsAnimating( );
	static void setIsDropShadowOn( );
	static BOOL getisDropShadowOn( ){ return isDropShadowOn; }
	static char*getCompiledVersion( ) { return _EMCEE_VERSION; }
	static BOOL openAppFile( char *_filename, char *_permissions,
		FILE **file, char *rPath = NULL, char *rDirectory = NULL, BOOL logit = TRUE );
	static void simulateCloseWindow( HWND hwnd );
	static HWND _eventHwnd;
	static void setVariableRotation( )
	{
		variableRotation = TRUE;
	}
	static void clearVariableRotation( )
	{
		variableRotation = FALSE;
	}
	static BOOL getVariableRotation( )
	{
		return variableRotation;
	}

private:

	static MC*				mc;
	static McState			state;
	static BOOL				isMessageBoxActive;
	static HWND				pressHwnd;
	static ULONGLONG		mouseWheelTime;
	static McWList*			windowList;
	static McPropsMgr*		properties;
	static float			dpi;
	static float			scale;
	static BOOL				mm;
	static BOOL				md;
	static char				computername[MAX_COMPUTERNAME_LENGTH + 1];
	static char				username[100];
	static char				trueUsername[100];
	static INT				WM_SHELLHOOKMESSAGE;
	static BOOL				isWindowsAnimating;
	static BOOL				isDropShadowOn;
	static McIconMgr*		iconManager;
	static BOOL				tabletMode;
	static DWORD			processId;
	static BOOL				variableRotation;

public:

	static BOOL		_incycle;
	static INT		WM_SHELLHOOKREPEAT;
	static HANDLE	mutexHandle;
	static BOOL		usingTaskview;

private:

	static BOOL _runcycle( );
	static LRESULT CALLBACK _windowproc( HWND hwnd, UINT uMsg, WPARAM, LPARAM );

public:

	static void _runlistenloop( HINSTANCE, BOOL );
	static void _runcycles(  );
	static void _triggerHotKey( );

};

#pragma managed(pop)


