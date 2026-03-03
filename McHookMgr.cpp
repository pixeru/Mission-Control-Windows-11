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

#include "McHookMgr.h"
#include "McMonitorsMgr.h"
#include "McModernAppMgr.h"
#include "McMainW.h"
#include "McWindowMgr.h"


#pragma unmanaged
using namespace std;

// PROCEDURES FOR HANDLING MOUSE HOOKS, WHICH ARE USED FOR HOT CORNER ACTIVATION

McRect *		McHookMgr::hcCornerRect = NULL;
McRect *		McHookMgr::hcCornerAreaRect = NULL;
UINT_PTR		McHookMgr::hcTimerId = 0;
HHOOK			McHookMgr::mouseHook = NULL;
BOOL			McHookMgr::watchCorner = FALSE;
myHookState		McHookMgr::mouseHookState = myHookIdle;
BOOL			McHookMgr::wasInCorner = FALSE;
BOOL			McHookMgr::wasOutsideCornerArea = FALSE;
int				McHookMgr::hcMonitorCount = 0;
int				McHookMgr::hcStartIndex = 0;

#define _HC_TIMER_ID 0x32766

BOOL McHookMgr::hcCornerContains( long x, long y )
{
	for (int i = 0; i<hcMonitorCount; i++)
		if (hcCornerRect[i].contains( x, y ))
			return TRUE;
	return FALSE;
}

BOOL McHookMgr::hcIsOutsideCornerArea( long x, long y )
{
	for (int i = 0; i < hcMonitorCount; i++)
		if (hcCornerAreaRect[i].contains( x, y ))
			return FALSE;
	return TRUE;
}

void McHookMgr::initializeMouseHooks( )
{
	if (mouseHook)
		return;

	wasOutsideCornerArea = FALSE;

	mouseHookState = myHookIdle;

	watchCorner = MC::getProperties( )->getMouseCornerActivate( );
	if (!watchCorner)
		return;

	McHotKeyMgr::createHotKeyData( );
	hcMonitorCount = (int)McMonitorsMgr::monitorList.size( );
	hcStartIndex = 0;

	hcCornerRect = new McRect[hcMonitorCount];
	hcCornerAreaRect = new McRect[hcMonitorCount];

	int hotCornerPadding = 0;

	int corner = MC::getProperties( )->getMouseCornerActivate( );

	for (list<McMonitorsMgr*>::iterator mi = McMonitorsMgr::monitorList.begin( );
		mi != McMonitorsMgr::monitorList.end( ); mi++)
	{
		McMonitorsMgr *monitor = *mi;

		if (hcCornerRect)
		{
			McRect mSize(monitor->getMonitorSize());
			McRect *size = &mSize;
			int hotCornerSizeHoriz = 4;
			int hotCornerSizeVert = 2;
			if (McMonitorsMgr::getIsTouchscreen( ))
			{
				hotCornerSizeVert = hotCornerSizeHoriz = (int) max( 10, floor( monitor->getDpi( ) * 0.125 ) );
				hotCornerPadding = 2 * max( hotCornerSizeHoriz, hotCornerSizeVert );
			}
			else
				hotCornerPadding = 3 * max( hotCornerSizeHoriz, hotCornerSizeVert );

			McRect cr;
			if (corner)
			{
				if (corner & MCA_UPPERLEFT)
					cr.set
					( size->left, monitor->getMonitorSize( )->top, size->left + hotCornerSizeHoriz,
					size->top + hotCornerSizeVert );
				else
					if (corner & MCA_UPPERRIGHT)
						cr.set
						( size->right - hotCornerSizeHoriz, size->top,
						size->right, size->top + hotCornerSizeVert );
					else
						if (corner & MCA_LOWERRIGHT)
							cr.set
							( size->right - hotCornerSizeHoriz, size->bottom - hotCornerSizeVert,
							size->right, size->bottom );
				hcCornerRect[hcStartIndex].set( &cr );
				hcCornerAreaRect[hcStartIndex].set( cr.left - hotCornerPadding, cr.top - hotCornerPadding,
					cr.right + hotCornerPadding, cr.bottom + hotCornerPadding );
			}
			else
			{
				cr.set( 0, 0, 0, 0 );
				hcCornerAreaRect[hcStartIndex].set( -100000, -100000, 100000, 100000);
			}
		}

		hcStartIndex++;
	}

	mouseHook = SetWindowsHookEx(
		WH_MOUSE_LL, mouseHookProc, GetModuleHandle( NULL ), 0 );

}

BOOL McHookMgr::hcAnalyzeWindows( POINT *p )
{
	HWND hwnd = GetForegroundWindow( );
	McWList *wl = MC::getWindowList( );
	DWORD threadId, processId;
	threadId = GetWindowThreadProcessId( hwnd, &processId );
	WINDOWINFO info = {};
	McWindowMgr::myGetWindowInfo( hwnd, &info );

	DWORD style = info.dwStyle;

	McRect mRect;
	mRect.set( &info.rcWindow );
	BOOL contains = mRect.contains( p->x, p->y );
	BOOL fullscreen = TRUE;

	if (contains)
	{
		HMONITOR windowMon = MonitorFromWindow( hwnd, MONITOR_DEFAULTTONEAREST );
		HMONITOR appMon = MonitorFromPoint( *p, MONITOR_DEFAULTTONEAREST );
		if (windowMon == appMon)
		{
			McMonitorsMgr *monitor = McMonitorsMgr::getMonitor( windowMon );
			fullscreen =
				(mRect.getWidth( ) >= monitor->getMonitorSize( )->getWidth( )) &&
				(mRect.getHeight( ) >= monitor->getMonitorSize( )->getHeight( ));
		}
	}

	if (!(contains && fullscreen))
		return FALSE;

	char className[1000];
	if (0 == GetClassNameA( hwnd, className, 1000 ))
		sprintf_s( className, sizeof( className ), "<CN UNKNOWN>" );

	if (!strcmp( className, _MODERN_MODERN_APP_BYTE ) ||
		!strcmp( className, _MODERN_IMMERSIVE_BACKGROUND_BYTE ) ||
		!strcmp( className, _MODERN_IMMERSIVE_LAUNCHER_BYTE ) ||
		!strcmp( className, _MODERN_SEARCH_RESULTS_BYTE ))
		return FALSE;

	HANDLE handle = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId );

	char progName[1000];
	char  *pNamePtr = progName;
	if (0 < GetProcessImageFileNameA( handle, progName, 1000 ))
	{
		size_t i, j = strlen( progName );
		for (i = 0; i < j; i++)
		{
			if (progName[i] == '\\') pNamePtr = &progName[i + 1];
			if (progName[i] == '.') progName[i] = 0;
		}
		strcpy_s( progName, sizeof( progName ), pNamePtr );
	}
	else
		strcpy_s( progName, sizeof( progName ), className );

	CloseHandle( handle );

	BOOL exclude = MC::getProperties( )->getIsExcluded( progName );

	return exclude;
}

void CALLBACK McHookMgr::hcTimerProc( HWND, UINT, UINT_PTR, DWORD )
{
	KillTimer( NULL, hcTimerId );
	hcTimerId = 0;

	if (mouseHookState != myHookWaiting)
		return;

	mouseHookState = myHookIdle;

	if (MC::_incycle)
		McHotKeyMgr::injectHotKey( );

	POINT p;
	GetCursorPos( &p );
	if (hcCornerContains( p.x, p.y ))
	{
		if (!hcAnalyzeWindows( &p ))
			MC::_runcycles( );
	}
}

LRESULT CALLBACK McHookMgr::mouseHookProc( int nCode, WPARAM wParam, LPARAM lParam )
{

	if (0 <= nCode)	// legit hook call
	{
		MSLLHOOKSTRUCT  *s = reinterpret_cast<MSLLHOOKSTRUCT *>(lParam);

		POINT pp;
		GetCursorPos( &pp );
		POINT *p = &pp;
		BOOL isInCorner;

		switch (wParam)
		{
		case WM_MOUSEMOVE:
			isInCorner = hcCornerContains( p->x, p->y );

			// Only act if the cursor has been outside of the expanded corner area since last time 
			// it was in the corner area.
		
			if (isInCorner && wasOutsideCornerArea)
			{
				wasOutsideCornerArea = FALSE;
				wasInCorner = isInCorner;
				if (!isInCorner)
					break;
				if (watchCorner && (mouseHookState == myHookIdle) && isInCorner)
				{
					mouseHookState = myHookWaiting;

					// React more quickly for touch to corner than to mouse
					hcTimerId = SetTimer( NULL, _HC_TIMER_ID, 
						s->flags & LLMHF_INJECTED ? 10 : 100, hcTimerProc );
				}
				isInCorner = wasInCorner;
			}
			else
				if (!wasOutsideCornerArea)
					wasOutsideCornerArea = hcIsOutsideCornerArea( p->x, p->y );
			break;

		default:

			break;
		}
	}

	return CallNextHookEx( NULL, nCode, wParam, lParam );
}

void McHookMgr::releaseMouseHooks( )
{
	mouseHookState = myHookIdle;
	wasOutsideCornerArea = FALSE;
	
	if (mouseHook)
	{
		UnhookWindowsHookEx( mouseHook );
		mouseHook = NULL;
	}

	if (hcCornerRect)
	{
		delete []	hcCornerRect;
		delete []	hcCornerAreaRect;
		hcCornerRect = hcCornerAreaRect = NULL;
	}

}

// PROCEDURES FOR KEYBOARD HOOKS, WHICH ARE USED FOR MONITORING FOR
// WINDOW KEY PRESS IF START MENU IS USED IN WINDOWS 10+


HHOOK McHookMgr::keyboardHook = NULL;
myHookState McHookMgr::keyboardHookState = myHookIdle;


LRESULT McHookMgr::keyboardHookProc( int nCode, WPARAM wParam, LPARAM lParam )
{
	if (0 <= nCode)
	{
		if (!MC::_incycle)
			return CallNextHookEx( NULL, nCode, wParam, lParam );

		KBDLLHOOKSTRUCT *kbHookStruct = reinterpret_cast<KBDLLHOOKSTRUCT *> (lParam);

		int code = kbHookStruct->vkCode;

		if (code == VK_LWIN || code == VK_RWIN)
		{
		}
		else

		switch( code )
		{
		case VK_OEM_COMMA:
		case VK_A_KEY:
		case VK_H_KEY:
		case VK_B_KEY:
		case VK_D_KEY:
		case VK_LWIN:
		case VK_RWIN:
		case VK_TAB:
		case VK_T_KEY:
		case VK_L_KEY:
		case VK_P_KEY:
		case VK_Q_KEY:
		case VK_R_KEY:
		case VK_S_KEY:
		case VK_U_KEY:
		case VK_X_KEY:
				break;

		default:

			keyboardHookState = myHookIdle;
			return CallNextHookEx( NULL, nCode, wParam, lParam );

		}

		switch (wParam)
		{
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:

			if (code == VK_LWIN || code == VK_RWIN)
			{
				
				if (keyboardHookState == myHookIdle)
				{
					keyboardHookState = myHookWaiting;
				}
				else
					keyboardHookState = myHookIdle;
			}
			else
			{	
				if (keyboardHookState == myHookWaiting)
				{
					if (McWindowMgr::getMainW( ))
					{
						switch (code)
						{
						case VK_H_KEY:
						case VK_A_KEY:
							PostMessage( McWindowMgr::getMainW( )->getHwnd( ), WM_USER, WMMC_DESTROY, NULL );
							break;
						default:
							McWindowMgr::getMainW( )->onFinish( );
							break;
						}
					}
				}
				keyboardHookState = myHookIdle;
			}

			return CallNextHookEx( NULL, nCode, wParam, lParam );;

		case WM_KEYUP:
		case WM_SYSKEYUP:
			if (code == VK_LWIN || code == VK_RWIN || code == VK_MENU )
				keyboardHookState = myHookIdle;
			return CallNextHookEx( NULL, nCode, wParam, lParam );;

		default:
			break;
		}
	}
	return CallNextHookEx( NULL, nCode, wParam, lParam );
}

void McHookMgr::initializeKbHooks( )
{

	if (MC::inTabletMode( ))
		return;

	if (keyboardHook)
		return;

	keyboardHookState = myHookIdle;
	keyboardHook = SetWindowsHookEx(
		WH_KEYBOARD_LL, keyboardHookProc, GetModuleHandle( NULL ), 0 );
}

void McHookMgr::releaseKbHooks( )
{
	keyboardHookState = myHookIdle;

	if (keyboardHook)
	{
		UnhookWindowsHookEx( keyboardHook );
		keyboardHook = NULL;
	}

	keyboardHookState = myHookIdle;

}