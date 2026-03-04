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
#pragma unmanaged

#include "Emcee.h"
#include "McWItem.h"

using namespace std;

enum MC_DRAG_EVENT
{
	DragButtonDown,
	DragMove,
	DragButtonUp
};

enum MC_DRAG_STATE
{
	DragStateIdle,
	DragStatePressed,
	DragStateDragging
};

#define MC_DRAG_MINPIX 2.828

class McDragMgr
{
public:
	static BOOL processEvent( HWND, MC_DRAG_EVENT, long x, long y, McWItem* = NULL ); 
											     // Returns TRUE if Event is Consumed
	static BOOL isDragging( );
	static BOOL isIdle( );
	static void restoreWindow( );

private:

	static	BOOL onButtonDown( long x, long y );
	static	BOOL onDrag( long x, long y );
	static	BOOL onButtonUp( long x, long y );
	static	BOOL fitToWindow( McRect *wRect, McRect *mRect, 
			POINT *newLocation, SIZE *newSize, BOOL *newIsMax );
	static	void addHiddenEdge( McRect *wRect, SIZE *borders, BOOL isMax );
	static	void maybeRedraw( long x, long y );
	static	void moveWindow( HMONITOR, HMONITOR );
	static MC_DRAG_STATE state;
	static POINT startingPoint;
	static POINT lastPoint;
	static HWND  dragHwnd;
	static HWND  pressHwnd;
	static McRect startingThumb;
	static McRect currentThumb;
	static McWItem *dragItem;
};
