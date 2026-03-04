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
#include "McBaseW.h"
#include "McWItem.h"
#include "McModernAppMgr.h"

#pragma unmanaged

class McZoomW;

class McMainW : public McBaseW<McMainW>
{
public:
	McMainW(  );
	~McMainW();
	void Activate( );
	void Deactivate( );
	PCWSTR  ClassName() const { return L"Emcee.MainW"; }
	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	DWORD runThread();
	void calcFrame( double value );
	void start();	
	void finish();	
	void startZoom( HWND, HWND, McRect *, McRect * );
	void onSettings();
	void onMinimizeAll( BOOL = TRUE, McWItem * = NULL, HMONITOR = NULL );
	void onMinimizeImmersiveApps( HMONITOR = NULL );
	void onNormalizeAll( HMONITOR = NULL);
	void onFinish();
	void onDestroy( );
	void setOnDisplayFocusItem( McWItem *_onDisplayFocusItem ) { onDisplayFocusItem = _onDisplayFocusItem; }
	void setStartButtonLocation( );
	void doQuitAndInject( WPARAM vkCode );
	void doTransitionOver( McState );
	McWItem *getActiveApp( ) { return activeApp; }
	void clearActiveApp( McWItem *_target = NULL );
	void setActiveApp( McWItem *_activeApp, HMONITOR _activeMonitor = NULL )
	{
		activeApp = _activeApp;
		if (activeApp == _APP_SHOW_DESKTOP)
			activeMonitor = _activeMonitor;
	}
	void initLastValue( ) {	lastValue = latestValue = -1;	}

private:
	
	McRect desktopRect;
	void onTransitionDown();
	void onTransitionUp();
	void onTransitionOver( McState = MCS_TransitionOver );
	void onDisplay();
    void onTouch( TC_Message * );
	void onMouseWheel( WPARAM, LPARAM );
	BOOL onLeftButtonUp( BOOL control, int x, int y );
	BOOL onMiddleButtonDown( int x, int y );
	void onPaint( );
	void setThumbnailDimensions();
	McWItem *activeApp;			
	HMONITOR activeMonitor;
	int		transitionCount;
	double lastValue;
	double latestValue;
	int fadingAlpha;
	McWItem* onDisplayFocusItem;
	HANDLE mwEvent;		
	HANDLE mwThread;
	DWORD  mwThreadId;
	int injectWinKey;

};