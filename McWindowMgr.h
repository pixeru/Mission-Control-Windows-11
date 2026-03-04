#pragma once
#include "stdafx.h"
#include "Emcee.h"
#include "McBaseW.h"
#include "McBgW.h"
#include "McBackingW.h"
#include "McLabelW.h"
#include "McDesktopW.h"
#include "McZoomW.h"
#include "McMainW.h"
#include "McThumbW.h"
#include "McButtonW.h"
#include "McFadingW.h"

#define _WM_TIMER_ID		0x32765
#define _WM_TIMER_DURATION	(900*1000) /* 15 Minutes */

class McWindowMgr
{
public:

	static McBgW*		allocateBgW( );
	static McBackingW*	allocateBackingW( );
	static McLabelW*	allocateLabelW( );
	static McZoomW*		allocateZoomW( );
	static McMainW*		allocateMainW( );
	static McFadingW*	allocateFadingW( );
	static McDesktopW*	allocateDesktopW( McWItem *item, McRect *rect );
	static void			freeDesktopW( McDesktopW * );
	static McThumbW*	allocateThumbW( McWItem *item );
	static void			freeThumbW( McThumbW * );
	static McButtonW*	allocateButtonW( WCHAR *, McBgW *bgW, int w, int h, Bitmap *i1, Bitmap *i2, WORD cmd );
	static void			freeButtonW( McButtonW * );
	static void			destroyAllWindows( ) {} 
	static void FadeWindowIn( HWND, int = 255, int = 11 );
	static void FadeWindowOut( HWND, int = 11 );
	static BOOL myGetWindowInfo(HWND, WINDOWINFO*);
	static BOOL myGetWindowRect(HWND, RECT*);
	static BOOL myGetWindowPlacement(HWND, WINDOWPLACEMENT*);
	static BOOL mySetWindowPlacement(HWND, WINDOWPLACEMENT*);

private:

	static McBgW*		bgW;
	static McBackingW*	backingW;
	static McLabelW*	labelW;
	static McZoomW*		zoomW;
	static McMainW*		mainW;
	static McFadingW*	fadingW;
	static list<McDesktopW*>usedDesktopWList;
	static list<McDesktopW*>freeDesktopWList;
	static list<McThumbW*>usedThumbWList;
	static list<McThumbW*>freeThumbWList;
	static list<McButtonW*>usedButtonWList;
	static list<McButtonW*>freeButtonWList;
	static int hoverTime;
	static UINT_PTR timerId;
	static void CALLBACK timerProc( HWND, UINT, UINT_PTR, DWORD );

public:

	static McBgW*		getBgW( )		{ if (bgW && bgW->isActivated) return bgW; else return NULL; }
	static McBackingW*	getBackingW( )	{ if (backingW && backingW->isActivated) return backingW; else return NULL; }
	static McLabelW*	getLabelW( )	{ if (labelW && labelW->isActivated) return labelW; else return NULL; }
	static McZoomW*		getZoomW( )		{ if (zoomW && zoomW->isActivated) return zoomW; else return NULL; }
	static McMainW*		getMainW( )		{ if (mainW && mainW->isActivated) return mainW; else return NULL; }	// Get desktop z-order of a window
	static McFadingW*	getFadingW( )	{ if (fadingW && fadingW->isActivated) return fadingW; else return NULL; }
	static int getZorder( HWND appHwnd );
	static int getHoverTime( )
	{
		return 1;
	}
	static void setHoverTime( int ht )
	{
	}
};
