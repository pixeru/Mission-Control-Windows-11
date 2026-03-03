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

#pragma managed(push, off)

#define FSIGN(x) ( (x<0.0)?-1:(x>0.0)?1:0 )

// Objects recieving touch messages pass them here for consolidation and
// interpretation.   They receive back messages when a gesture is completed.

// Who passes in touch message

enum TC_CallerType
{
	TCC_Unknown,
	TCC_Thumb,
	TCC_Main,
	TCC_Zoom
};

// Our state

enum TC_State
{
	TCS_IDLE,		// waiting for a gesture to begin
	TCS_STARTED,	// A complex gesture has started but we don't know which one yet
	TCS_WAITING		// Waiting for a gesture to complete
};

// Messages delivered back to objects

enum TC_Gesture
{
    TCG_NONE,
    TCG_2F_SWIPE_LEFT,
    TCG_2F_SWIPE_RIGHT,
    TCG_2F_SWIPE_UP,
    TCG_2F_SWIPE_DOWN,
    TCG_2F_TAP,
    TCG_PINCH,
    TCG_SPREAD,
	TCG_ROTATE,
    TCG_PRESS_AND_TAP
};

// The package we use to inform objects of an actionable gesture

struct TC_Message
{
	TC_Gesture	tcGesture;
    POINT       tcPoint;
	TC_Message()
	{
		tcGesture = TCG_NONE;
		tcPoint.x = tcPoint.y = 0;
	}
    TC_Message( TC_Gesture _tcGesture, int x, int y )
	{
        tcGesture = _tcGesture;
        tcPoint.x = x;
        tcPoint.y = y;
	}
	void set( TC_Gesture _tcGesture, int x, int y )
	{
		tcGesture = _tcGesture;
		tcPoint.x = x;
		tcPoint.y = y;
	}
};

class McTouchMgr
{
public:		// Methods

	McTouchMgr();
	~McTouchMgr();

	LRESULT handleGestureMessage(
        HWND            myHwnd,
		WPARAM			wParam,
		LPARAM			lParam );

private:	// Methods

	void clearState();

    void onComplexGesture( HWND, UINT, GESTUREINFO* );
	void onPressAndTap( HWND, GESTUREINFO* );
	void onTwoFingerTap( HWND, GESTUREINFO * );
    void generateCallback( HWND, TC_Gesture, int x, int y );

private:	// Data
	TC_State		myState;
    POINT           originalPoint;
    double          startValue;
    int             lastSign;
	WPARAM			originalGesture;
	TC_Gesture		cbGesture;
	TC_Message		touchMessage;
	float			ourDpi;
	BOOL			ignoreGesture;
};

#pragma managed(pop)
