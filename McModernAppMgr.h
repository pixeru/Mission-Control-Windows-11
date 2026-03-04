#pragma once
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

#pragma unmanaged

class McWItem;

class _ModerrnAppMgr;

#define _APP_SHOW_DESKTOP ((McWItem *)(-1))

#define _MODERN_MODERN_APP_BYTE				"Windows.UI.Core.CoreWindow"		// The magic window class
#define _MODERN_MODERN10_APP_BYTE			"ApplicationFrameWindow"			// DT W10 App
#define _MODERN_IMMERSIVE_BACKGROUND_BYTE	"ImmersiveBackgroundWindow"			// Solid backing to IA
#define _MODERN_IMMERSIVE_SPLASH_BYTE		"ImmersiveSplashScreenWindowClass"	// Splash Screen IA
#define _MODERN_IMMERSIVE_LAUNCHER_BYTE		"ImmersiveLauncher" 				// Start Screen
#define _MODERN_IMMERSIVE_GUTTER_BYTE		"ImmersiveGutter" 					// Gutter between snapped IAs
#define _MODERN_SEARCH_RESULTS_BYTE			"SearchResultsView"					// Search Results IA
#define _MODERN_LOCK_SCREEN_BYTE			"Windows Default Lock Screen"		// Something to do with hair?
#define _MODERN_SEARCHPANE_BYTE				"SearchPane"						// We hold this to be self-evident.
#define _MODERN_GHOST_WINDOW_BYTE			"ModernGhostWindow"					// Ooh, Scary!
#define _MODERN_NATIVE_HWND_HOST_BYTE		"NativeHWNDHost"
#define _MODERN_PROJECT_EDGE_BYTE			"Microsoft Edge"					// Icon display name
#define _MODERN_EDGE_SHORT_BYTE				"Microsoft Edge"					// Search string
#define _MODERN_EDGE_LONG_BYTE				"Microsoft.Windows.Edge"			// Alt. Search string
#define _MODERN_PREDICTION					"PredictionWnd"
#define _MODERN_GET_STARTED_BYTE			"Get Started"
#define _MODERN_START_SCREEN_BYTE			"Start"
#define _MODERN_ACTION_CENTER_BYTE			"Action center"
#define _MODERN_TABLET_MODE_BYTE			"TabletModeCoverWindow"
#define _MODERN_SHELL_BYTE					"Shell_"							// Various shell windows
#define _MODERN_VIEWFRAME_BYTE				"MultitaskingViewFrame"				// Holds Taskview, sometimes window
#define _MODERN_TASKVIEW_BYTE				"Task View"
#define _MODERN_PROJECT_BYTE				"Task Project"
#define _MODERN_WORKER_BYTE					"MSCTFIME UI"
#define _MODERN_LEN_SHELL					6
#define _MODERN_IMMERSIVE_BYTE				"Immersive"
#define _MODERN_LEN_IMMERSIVE				9
#define _MODERN_TABLET_MODE_BYTE			"TabletModeCoverWindow"			// Special background for tablet mode
#define _MODERN_IS_EDGE( pn ) (strstr( pn, _MODERN_EDGE_SHORT_BYTE ) || strstr( pn, _MODERN_EDGE_LONG_BYTE ))
#define _MODERN_MODERN_APP					TEXT(_MODERN_MODERN_APP_BYTE)	// Immersive App
#define _MODERN_MODERN10_APP				TEXT(_MODERN_MODERN10_APP_BYTE )
#define _MODERN_IMMERSIVE_BACKGROUND		TEXT(_MODERN_IMMERSIVE_BACKGROUND_BYTE)	// Solid backing to IA
#define _MODERN_IMMERSIVE_LAUNCHER			TEXT(_MODERN_IMMERSIVE_LAUNCHER_BYTE)	// Start Screen
#define _MODERN_IMMERSIVE_GUTTER			TEXT(_MODERN_IMMERSIVE_GUTTER_BYTE)	// Gutter between snapped IAs
#define _MODERN_SEARCH_RESULTS				TEXT(_MODERN_SEARCH_RESULTS_BYTE)	// Search Results IA
#define _MODERN_PROJECT_EDGE				TEXT(_MODERN_PROJECT_EDGE_BYTE)	
#define _MODERN_WORKER						TEXT(_MODERN_WORKER_BYTE )
#define _MODERN_VIEWFRAME					TEXT(_MODERN_VIEWFRAME_BYTE)	
#define _MODERN_TASKVIEW					TEXT(_MODERN_TASKVIEW_BYTE)
#define _MODERN_TABLET_MODE					TEXT(_MODERN_TABLET_MODE_BYTE)
#define _MODERN_GET_STARTED					TEXT(_MODERN_GET_STARTED_BYTE)
#define _MODERN_START_SCREEN				TEXT(_MODERN_START_SCREEN_BYTE)

class McModernAppMgr
{
public:

	static HRESULT	Initialize();
	static void		Uninitialize();
	static BOOL		CloseApp( McWItem * );   
	static BOOL		MoveApp( McWItem *, McMonitorsMgr *, McMonitorsMgr *, McRect * );
	static BOOL		GetAppInfo( HWND, DWORD, char *, char *, int *, int );
	static BOOL		EnumAllApps( WNDENUMPROC lpEnumFunc, LPARAM lParam );
	static void		DetermineModernColors();	// Obtain the colors
	static COLORREF	GetAppBarColor( );			// Return the colors
	static COLORREF GetIconBgColor( );
	static COLORREF GetSettingsColor( );
	static COLORREF GetPaletteColor( int );
	static int		GetPaletteCount( );
	static COLORREF GetAutoColor( );
	static COLORREF GetBgColor( );
	static void		RemoveLauncher( );				
	static void     RemoveAnimatedSystemWindows(  );	
	static void		ClickMouseAt( long x, long y, BOOL useLB = FALSE );
	static BOOL		tabletModeDividerExists( HWND* = NULL );
	static _ModerrnAppMgr *getInterface() { return _ModernAppProxy; }
	static BOOL		CheckLockScreen( );
	static void		InjectAnyKey( WORD = 0, WORD = 0 );
	static void		InjectWinKey( WORD = 0, WORD = 0 );
	static BOOL		MatchesModernClassname( char *classname );
	static HWND     GetUnderlyingModernWindow( HWND );
#if MC_DESKTOPS
	static BOOL McModernAppMgr::IsWindowOnCurrentDesktop( HWND hwnd );
	static void McModernAppMgr::GetWindowDesktop( HWND hwnd, GUID *desktopID );
#endif
	static BOOL		IsLauncherVisible( );

private:

	static _ModerrnAppMgr* _ModernAppProxy;

};

