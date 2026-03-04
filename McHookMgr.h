#pragma once
#include "Emcee.h"

#pragma managed( push, off )

enum myHookState
{
	myHookIdle,		// waiting for mouse push
	myHookWaiting,	// waiting for hot corner timer to fire
	myHookTabDown	// unused
};

class McHookMgr
{
public:

	static void initializeMouseHooks( );
	static void releaseMouseHooks( );
	static void initializeKbHooks( );
	static void releaseKbHooks( );

private: // methods

	static LRESULT CALLBACK keyboardHookProc( int nCode, WPARAM wParam, LPARAM lParam ); 
	static LRESULT CALLBACK mouseHookProc( int nCode, WPARAM wParam, LPARAM lParam );
	static void CALLBACK hcTimerProc( HWND, UINT, UINT_PTR, DWORD );
	static BOOL hcCornerContains( long x, long y );
	static BOOL hcIsOutsideCornerArea( long x, long y );
	static BOOL hcAnalyzeWindows( POINT *p );

private: // data

	static BOOL watchCorner;
	static int hcMonitorCount;
	static BOOL wasInCorner;
	static BOOL wasOutsideCornerArea;
	static myHookState mouseHookState;
	static HHOOK mouseHook;
	static McRect *hcCornerRect;
	static McRect *hcCornerAreaRect;
	static int hcStartIndex;
	static UINT_PTR hcTimerId;
	static HHOOK keyboardHook;
	static myHookState keyboardHookState;

};

#pragma managed( pop )

