#pragma once
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
#include "McBaseW.h"
#include "McMonitorsMgr.h"

#define MC_ZOOM_NONE 0
#define MC_ZOOM_UP 1
#define MC_ZOOM_DOWN 2
#define MC_ZOOM_AGAIN 3
#define MC_ZOOM_ZOOMED 4
#define MC_ZOOM_PANIC 5

#define _ZOOM_ALPHA_VALUE 96

#pragma unmanaged

class McWItem;

class McZoomW : public McBaseW<McZoomW>
{
public:

	McZoomW();
	~McZoomW();

	void Activate( );
	void Deactivate( );

	PCWSTR ClassName() const { return L"Emcee.ZoomW"; }
	LRESULT HandleMessage( UINT uMsg, WPARAM wParam, LPARAM lParam );
	void doZoom( int mode );
	DWORD runThread();
	void calcFrame( double value );
	int getAlpha( )
	{
		if (mode == MC_ZOOM_AGAIN)
			return _ZOOM_ALPHA_VALUE;
		else
			return myAlpha; 
	}
	void clearList() { thumbOwners.clear(); }
	void addItem( McWItem *item ) { thumbOwners.push_back( item ); }
	BOOL buildSuperZoom( int pileNumber, HMONITOR );
	BOOL isZoomed() { return (mode == MC_ZOOM_ZOOMED);  }
	BOOL isZooming() { return zooming; }

	BOOL isItZoomed( HWND appHwnd );
	void initLastValue( ) { lastValue = latestValue = -1; }


private:
	int mode;
	int myAlpha;
	McWItem *oldFocusItem;
	McWItem *isSelected;
	McWItem *isZoomingAgain;
	LPARAM lParamS;
	list<McWItem *>thumbOwners;
	HTHUMBNAIL thumbnail;
	HWND sourceWindow;
	HWND thumbWindow;
	BOOL inAnimation;
	HANDLE zwEvent;	
	HANDLE zwThread;
	HANDLE zwMutex;
	DWORD  zwThreadId;
	double lastValue;
	double latestValue;
	BOOL	zooming;
	void runZoom();
	void onPaint( );
    void onTouch( TC_Message * );
	void onMouseWheel( WPARAM, LPARAM );
	BOOL doReZoom( int, int );
	void doKillFromZoom( int, int );
};