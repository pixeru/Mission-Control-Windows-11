#include "stdafx.h"
#include "Emcee.h"
#include "McTouchMgr.h"
#include "McBaseW.h"
#include "McMonitorsMgr.h"

// Manage touch events.

McTouchMgr::McTouchMgr()
{
	myState = TCS_IDLE;

	if (MC::getMM( ))
		ourDpi = McMonitorsMgr::getMainMonitor( )->getDpi( );
	else
		ourDpi = 0;
	ignoreGesture = FALSE;
}

McTouchMgr::~McTouchMgr()
{
}

LRESULT McTouchMgr::handleGestureMessage(
	HWND			myHwnd,
	WPARAM			wParam,
	LPARAM			lParam )
{

	if (!MC::_incycle)	// This object may no longer exist
		return 0;

	GESTUREINFO gestureInfo;
	gestureInfo.cbSize = sizeof( GESTUREINFO );
	GetGestureInfo( (HGESTUREINFO)lParam, &gestureInfo );

	if (gestureInfo.dwID == GID_BEGIN || gestureInfo.dwID == GID_END)
	{
		ignoreGesture = FALSE;
		myState = TCS_IDLE;
		return DefWindowProc( myHwnd, WM_GESTURE, wParam, lParam );
	}

	
	if ( !ignoreGesture )
		switch (gestureInfo.dwID)
		{
			case GID_ZOOM:
			case GID_PAN:
			case GID_ROTATE:
				onComplexGesture( myHwnd, gestureInfo.dwID, &gestureInfo );
				break;
			case GID_TWOFINGERTAP:
				if ( gestureInfo.dwFlags & GF_END )
					onTwoFingerTap( myHwnd, &gestureInfo );
				break;
			case GID_PRESSANDTAP:
				onPressAndTap( myHwnd, &gestureInfo );
				break;
			default:
				break;
		}

	CloseGestureInfoHandle( (HGESTUREINFO)lParam );
	return S_OK;
}

void McTouchMgr::generateCallback( HWND myHwnd, TC_Gesture tcGesture, int x, int y )
{
	touchMessage.tcGesture = tcGesture;
	touchMessage.tcPoint.x = x;
	touchMessage.tcPoint.y = y;
	SendMessage( myHwnd, WM_USER, WMMC_TOUCH, (LPARAM)&touchMessage );
}

void McTouchMgr::onPressAndTap( HWND myHwnd, GESTUREINFO *gi )
{
	if ( gi->dwFlags & GF_BEGIN )
	{
		generateCallback( myHwnd, TCG_PRESS_AND_TAP, gi->ptsLocation.x, gi->ptsLocation.y );
		myState = TCS_WAITING;
	}
	if ( gi->dwFlags & GF_END )
		myState = TCS_IDLE;
} 

void McTouchMgr::onTwoFingerTap( HWND myHwnd, GESTUREINFO *gi )
{
	generateCallback( myHwnd, TCG_2F_TAP, touchMessage.tcPoint.x, touchMessage.tcPoint.y );
}

void McTouchMgr::onComplexGesture( HWND myHwnd, UINT dwID, GESTUREINFO *gi )
{
	// handle one of the complex gestures
	int dwFlags = gi->dwFlags&(GF_BEGIN|GF_END);

	if ( dwFlags == GF_END )
		ignoreGesture = TRUE;

	if ( (dwFlags & GF_BEGIN) && (myState == TCS_IDLE))
	{
		originalPoint.x=gi->ptsLocation.x;
		originalPoint.y=gi->ptsLocation.y;

		if ( originalPoint.x == 0 && originalPoint.y == 0 ) 
			return;

		if (!MC::getMM( ))
			ourDpi = 
				McMonitorsMgr::getMonitor( MonitorFromPoint( originalPoint, MONITOR_DEFAULTTONEAREST ) )->getDpi( );

		switch( dwID )
		{
		case GID_ROTATE:
			startValue = GID_ROTATE_ANGLE_FROM_ARGUMENT( gi->ullArguments );
			break;
		case GID_ZOOM:
			startValue = ((double)(gi->ullArguments))/ourDpi;
			break;
		case GID_PAN:
			startValue = 0.0;
			break;
		default:
			return;
		}

		myState = TCS_STARTED;
	}

	double fx, fy;

	if ( (dwFlags & GF_END) && (myState != TCS_STARTED ) )
	{
		myState = TCS_IDLE;
		return;
	}
	if ( ( dwFlags == 0 || (dwFlags&GF_END) ) && myState == TCS_STARTED )
	{

		double currentValue, changeValue, changeMagnitude;

		switch( dwID )
		{
		case GID_ROTATE:
			currentValue = GID_ROTATE_ANGLE_FROM_ARGUMENT( gi->ullArguments );
			break;
		case GID_ZOOM:
			currentValue = ((double)(gi->ullArguments))/ourDpi;
			break;
		case GID_PAN:
			fx = (double)(gi->ptsLocation.x - originalPoint.x);
			fy = (double)(gi->ptsLocation.y - originalPoint.y);
			currentValue = sqrt( fx*fx + fy*fy )/ourDpi;
			break;
		default:
			return;
		}

		changeValue = currentValue - startValue;
		changeMagnitude = fabs( changeValue );

		BOOL trigger = ( changeMagnitude > 
				((dwID == GID_ROTATE) ? 0.25 :
					( dwID == GID_PAN ) ? 0.1 :
										    0.075 )) ;
		if ( trigger ) 
		{
			startValue = currentValue;
			myState = TCS_WAITING;

			switch( dwID )
			{
			case GID_ROTATE:
				generateCallback( myHwnd, TCG_ROTATE, originalPoint.x, originalPoint.y );
				break;
			case GID_ZOOM:
				generateCallback( myHwnd,
					(FSIGN( changeValue )<0)?TCG_PINCH:TCG_SPREAD,
					originalPoint.x, originalPoint.y );
				break;
			case GID_PAN:
				{
					McRect *r = (McMonitorsMgr::getMainMonitor( )->getMonitorSize( ));
					double fxdist = fabs( fx );
					double fydist = fabs( fy );
					TC_Gesture gesture;

					if ( fxdist/((double)r->getWidth()) < fydist/((double)r->getHeight()) )
					{
						if ( fy > 0.0 )
							gesture = TCG_2F_SWIPE_DOWN;
						else
							gesture = TCG_2F_SWIPE_UP;
					}
					else
					{
						if ( fx > 0 )
							gesture = TCG_2F_SWIPE_RIGHT;
						else
							gesture = TCG_2F_SWIPE_LEFT;
					}

					generateCallback( myHwnd, gesture, originalPoint.x, originalPoint.y );
				}
				break;
			default:
				break;
			}
		}
	}
}

	  
