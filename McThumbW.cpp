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
#include "Emcee.h"
#include "McMainW.h"
#include "McBgW.h"
#include "McThumbW.h"
#include "McZoomW.h"
#include "McLabelW.h"
#include "McWList.h"
#include "McWItem.h"
#include "McModernAppMgr.h"
#include "McDragMgr.h"
#include "McWindowMgr.h"
#include "McPilesMgr.h"

// Window showing thumbnail for a single window either desktop or windows store

#pragma unmanaged
McThumbW::McThumbW( )
{
	labels = NULL;
	rawLabels = NULL;
	owner = NULL;
	haveMouse = FALSE;
}

McThumbW::~McThumbW( )
{
	Deactivate( );
	DestroyWindow( getHwnd() );
}

void McThumbW::Activate( McWItem *_owner )
{
	if (!isActivated)
	{
		if (labels) delete labels;
		if (rawLabels) delete rawLabels;
		labels = rawLabels = NULL;
		haveMouse = FALSE;
		owner = _owner;
		createLabel( );
		WCHAR b[1000];
		wsprintf( b, L"%S", owner->getProgName( ) );
		SetWindowText( getHwnd( ), b );
		isActivated = TRUE;
	}
}

void McThumbW::Deactivate( )
{
	if (isActivated)
	{
		ShowWindow( getHwnd( ), SW_HIDE );
		SetWindowText( getHwnd( ), L"<idle>" );
		if (!isActivated)
			return;
		if (labels) delete labels;
		if (rawLabels) delete rawLabels;
		rawLabels = labels = NULL;
		McWindowMgr::freeThumbW( this );
		isActivated = FALSE;
	}
}
 
LRESULT McThumbW::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	McWList *wl = MC::getWindowList();
	McWItem *wi = this->getOwner( );
	if (wl && wi && uMsg == WM_SYSCOMMAND )
			return 0;
	
	switch (uMsg)
	{

	case WM_MOUSEWHEEL:

		if (!McMonitorsMgr::getIsTouchscreen( ) || MC::getIsMessageBoxActive( ) || !isActivated)
			return DefWindowProc( getHwnd( ), uMsg, wParam, lParam );
		if (McWindowMgr::getZoomW( ))
			if (McWindowMgr::getZoomW( )->isZoomed( ))
				SendMessage( McWindowMgr::getZoomW( )->getHwnd( ),
				WM_MOUSEWHEEL, wParam, lParam );
			else
				if (owner->getThumbRect( )->contains( GET_X_LPARAM( lParam ), GET_Y_LPARAM( lParam ) ))
					onMouseWheel( wParam, lParam );
				else
					if (MC::getState( ) == MCS_Display && VK_IS_KEY_PRESSED( VK_CONTROL ))
						PostMessage( McWindowMgr::getMainW( )->getHwnd( ), WM_MOUSEWHEEL, wParam, lParam );

		return DefWindowProc( m_hwnd, uMsg, wParam, lParam );

	case WM_GESTURE:

		if (MC::getIsMessageBoxActive( ) || !isActivated)
			return DefWindowProc( getHwnd( ), uMsg, wParam, lParam );
		MC::getTouchController( )->handleGestureMessage( getHwnd( ), wParam, lParam );
		return 0;

	case WM_SETFOCUS:
		if (MC::getIsMessageBoxActive( ) || !isActivated)
			return DefWindowProc( getHwnd( ), uMsg, wParam, lParam );
		{
			wl->setFocusItem( owner );
			HWND fg = GetForegroundWindow( );
			if (fg != getHwnd( ))
			{
				LockSetForegroundWindow( LSFW_UNLOCK );
				SetForegroundWindow( getHwnd( ) );
			}
		
			//InvalidateRect( getHwnd( ), NULL, FALSE );
			//RedrawWindow( getHwnd( ), NULL, NULL, RDW_INTERNALPAINT );
			if (!McWindowMgr::getLabelW( )->getPaused( ))
				McWindowMgr::getLabelW( )->Show( this );
		}
		return 0;

	case WM_KILLFOCUS:

		if (MC::getIsMessageBoxActive( ) || !isActivated)
			return DefWindowProc( getHwnd( ), uMsg, wParam, lParam );
		wl->setFocusItem( NULL );

		//InvalidateRect( getHwnd( ), NULL, FALSE );
		//RedrawWindow( getHwnd( ), NULL, NULL, RDW_INTERNALPAINT );//RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW );
		McWindowMgr::getLabelW( )->Hide( );
		return 0;

	case WM_PAINT:
		// Painting occurs during animations -- main window positions the thumbnail for
		// the desktop background thumbnail.
		return DefWindowProc( m_hwnd, uMsg, wParam, lParam );

	case WM_MOUSEMOVE:

		if (MC::getIsMessageBoxActive( ) || !isActivated)
			return DefWindowProc( getHwnd( ), uMsg, wParam, lParam );
		// If we have multiple monitors this might be a window being dragged to another monitor.

		if ((wParam&MK_LBUTTON) &&
			McDragMgr::processEvent( getHwnd( ), DragMove, GET_X_LPARAM( lParam ), GET_Y_LPARAM( lParam ) ))
			return 0;

		if (!haveMouse)
		{
			// If the mouse just entered the window, set up to get a hover event if it
			// is still over the window in 200 ms.

			TRACKMOUSEEVENT tme;
			tme.cbSize = sizeof( TRACKMOUSEEVENT );
			tme.dwFlags = TME_LEAVE | TME_HOVER;
			tme.hwndTrack = getHwnd( );
			tme.dwHoverTime = McWindowMgr::getHoverTime( );
			TrackMouseEvent( &tme );
			haveMouse = TRUE;
		}
		return DefWindowProc( getHwnd( ), uMsg, wParam, lParam );

	case WM_MOUSEHOVER:				// Move to top of pile

		if (MC::getIsMessageBoxActive( ) || !isActivated)
			return DefWindowProc( getHwnd( ), uMsg, wParam, lParam );
		if (MC::getMM( ) && McDragMgr::isDragging( )) return 0;

		if (haveMouse)
		{
			MC::getWindowList( )->changeFocus( owner );
			McWindowMgr::getLabelW( )->Resume( );
		}
		return DefWindowProc( getHwnd( ), uMsg, wParam, lParam );

	case WM_MOUSELEAVE:

		if (MC::getIsMessageBoxActive( ) || !isActivated)
			return DefWindowProc( getHwnd( ), uMsg, wParam, lParam );
		if (MC::getMM( ) && McDragMgr::isDragging( )) return 0;

		haveMouse = FALSE;

		return DefWindowProc( getHwnd( ), uMsg, wParam, lParam );

	case WM_LBUTTONDOWN:

		if (MC::getIsMessageBoxActive( ) || !isActivated)
			return DefWindowProc( getHwnd( ), uMsg, wParam, lParam );

		McWindowMgr::getLabelW( )->Hide( );
		McDragMgr::processEvent( getHwnd( ), DragButtonDown, GET_X_LPARAM( lParam ), GET_Y_LPARAM( lParam ), owner );

		MC::recordHwnd( getHwnd( ) );
		setLbInfo( GET_X_LPARAM( lParam ), GET_Y_LPARAM( lParam ) );
		return DefWindowProc( getHwnd( ), uMsg, wParam, lParam );

	case WM_LBUTTONUP:

		if (MC::getIsMessageBoxActive( ) || !isActivated)
			return DefWindowProc( getHwnd( ), uMsg, wParam, lParam );

		if (McDragMgr::processEvent( getHwnd( ), DragButtonUp, GET_X_LPARAM( lParam ), GET_Y_LPARAM( lParam ) ))
			return 0;

		if (!checkLbInfo( GET_X_LPARAM( lParam ), GET_Y_LPARAM( lParam ) ))
			return DefWindowProc( getHwnd( ), uMsg, wParam, lParam );
		clearLbInfo( );

		if (!MC::matchHwnd( getHwnd( ) ))
			return DefWindowProc( getHwnd( ), uMsg, wParam, lParam );

		onSelectWindow( (wParam&MK_CONTROL) != NULL );
		return DefWindowProc( getHwnd( ), uMsg, wParam, lParam );

	case WM_RBUTTONDOWN:	// Do zoom
		if (MC::getIsMessageBoxActive( ) || !isActivated)
			return DefWindowProc( getHwnd( ), uMsg, wParam, lParam );
		onZoom( (wParam&MK_CONTROL) != NULL );
		return DefWindowProc( getHwnd( ), uMsg, wParam, lParam );

	case WM_MBUTTONDOWN:	// Move to bottom of the Z-Order.

		if (MC::getIsMessageBoxActive( ) || !isActivated)
			return DefWindowProc( getHwnd( ), uMsg, wParam, lParam );
		if (((wParam&MK_CONTROL) == NULL))
			PostMessage( McWindowMgr::getMainW( )->getHwnd( ), WM_KEYDOWN, VK_KILLKEY, TRUE );
		else
			if (owner->getIsOnAppBar( ))
				immersiveAppMoveToBottom( );
			else
				onMoveToBottom( MCS_TransitionDesk );
		return DefWindowProc( getHwnd( ), uMsg, wParam, lParam );

	case WM_KEYUP:
	case WM_CHAR:
	case WM_DEADCHAR:
	case WM_SYSDEADCHAR:
	case WM_SYSKEYUP:
	case WM_UNICHAR:

		if (MC::getIsMessageBoxActive( ) || !isActivated)
			return DefWindowProc( getHwnd( ), uMsg, wParam, lParam );
		break;

	case WM_SYSKEYDOWN:
	case WM_KEYDOWN:

		if (MC::getIsMessageBoxActive( ) || !isActivated)
			return DefWindowProc( getHwnd( ), uMsg, wParam, lParam );

		if (wParam == VK_SHIFT || wParam == VK_CONTROL || wParam == VK_MENU)
			break;

		if (McWindowMgr::getZoomW( ))
			if (McWindowMgr::getZoomW( )->isZoomed( ))
			{
				SendMessage( McWindowMgr::getZoomW( )->getHwnd( ),
					WM_KEYDOWN, wParam, lParam );
				return 0;
			}

		if (wParam == VK_KILLKEY)
			PostMessage( McWindowMgr::getMainW( )->getHwnd( ), WM_KEYDOWN, VK_KILLKEY, NULL );
		else
			MC::getWindowList( )->keyPress( wParam );
		return DefWindowProc( getHwnd( ), uMsg, wParam, lParam );

	case WM_USER:

		if (MC::getIsMessageBoxActive( ) || !isActivated)
			return DefWindowProc( getHwnd( ), uMsg, wParam, lParam );
		switch (wParam)
		{
		case WMMC_FOCUS:
			MC::getWindowList( )->changeFocus( owner );
			MC::getWindowList( )->nextPile( 1 );
			MC::getWindowList( )->changeFocus( owner );
			break;
		case WMMC_SELECT:
			onSelectWindow( lParam != NULL );
			break;
		case WMMC_TOUCH:
			onTouch( (TC_Message *) lParam );
			break;
		default:
			break;
		}
		return 0;

	default:
		return DefWindowProc( m_hwnd, uMsg, wParam, lParam );
	}



	return TRUE;
}

void McThumbW::onMouseWheel( WPARAM wParam, LPARAM lParam )
{
	ULONGLONG timeNow = GetTickCount64();
	if ( timeNow - MC::getWheelTime() < 400 )
	{
		MC::setWheelTime( timeNow );
		return;
	}

	MC::setWheelTime( timeNow );

	// Process the event
	
	int fwKeys = GET_KEYSTATE_WPARAM( wParam );
	int zDelta = GET_WHEEL_DELTA_WPARAM( wParam );

	TC_Message tcMessage(TCG_NONE, GET_X_LPARAM( lParam ), GET_Y_LPARAM( lParam ) );

	if ( fwKeys&MK_CONTROL )
		tcMessage.tcGesture = (zDelta<0) ? TCG_PINCH : TCG_SPREAD;
	else
		tcMessage.tcGesture = (zDelta<0) ? TCG_2F_SWIPE_UP : TCG_2F_SWIPE_DOWN;

	onTouch( &tcMessage );
}


void McThumbW::onTouch( TC_Message *tcm )
{
	if ( MC::getState() != MCS_Display )
		return;

	switch( tcm->tcGesture )
	{
	case TCG_2F_TAP:
		// Push the thumb to the bottom, desktop app to bottom of z-order
		if (!owner->getIsOnAppBar( ))
			onMoveToBottom( MCS_TransitionDesk );
		break;
	case TCG_PRESS_AND_TAP:
		// Kill
		MC::getWindowList()->changeFocus( owner );
		PostMessage( McWindowMgr::getMainW()->getHwnd(), WM_KEYDOWN, VK_KILLKEY, TRUE );
		break;
	case TCG_2F_SWIPE_DOWN:
	case TCG_2F_SWIPE_UP:
		// Zoom Thumbnail
		onZoom( FALSE );
		break;
	case TCG_2F_SWIPE_LEFT:
		if (owner->getIsOnAppBar( ))
			MC::getWindowList( )->onRightArrowClick( );
		break;
	case TCG_2F_SWIPE_RIGHT:
		if (owner->getIsOnAppBar( ))
			MC::getWindowList()->onLeftArrowClick();
		break;
	case TCG_SPREAD:
		// Zoom entire pile

		onZoom( TRUE );
		break;
	default:
		break;
	}
}

void McThumbW::immersiveAppMoveToBottom()
{
	onMoveToBottom( MCS_TransitionImmersiveApp );
}

void McThumbW::onMoveToBottom( McState transition )
{
	McWList *wl = MC::getWindowList();
	McWItem *itemBelow = wl->getItemBelow( owner );
	if ( itemBelow == NULL )
	 return;
	owner->loseFocus();
	owner->pushToBottom();
	if ( transition != MCS_TransitionNone )
		MC::getWindowList()->reinitLayout( transition );
}

void McThumbW::onSelectWindowTM( BOOL doFinish )
{
	McWindowMgr::getMainW( )->setActiveApp( owner );
	if ( doFinish )
		McWindowMgr::getMainW( )->onFinish( );
}

void McThumbW::onSelectWindow( BOOL isCtrl, BOOL doFinish )
{

	if (McModernAppMgr::tabletModeDividerExists( ))
	{
		onSelectWindowTM( doFinish );
		return;
	}
	if (isCtrl && !MC::inTabletMode() )
		McWindowMgr::getMainW( )->onMinimizeAll( FALSE, owner, owner->getAppMonitor( ) );

	BOOL wasMinimized = owner->getIsMinimized( );
	if (!wasMinimized && !owner->getIsCloaked( ))
	{
		McRect rect;
		WINDOWINFO info;
		McWindowMgr::myGetWindowInfo( owner->getAppHwnd( ), &info );
		rect.set( &info.rcWindow ); 
		SetWindowPos( owner->getAppHwnd( ), HWND_TOP,
			rect.left, rect.top,
			rect.getWidth( ), rect.getHeight( ), SWP_SHOWWINDOW );
	}
	else if (owner->getIsCloaked( ) && doFinish)
	{
		if (wasMinimized)
		{
			McRect windowrect;
			McWindowMgr::myGetWindowRect( owner->getAppHwnd( ), &windowrect );
			owner->setNormalRect( &windowrect );
		}
		McWindowMgr::getMainW( )->setActiveApp( owner );
		McWindowMgr::getMainW( )->onFinish( );
		return;
	}

	owner->raiseToTop( owner->getIsW10App( ) );

	if (wasMinimized && owner->getIsEdgeSnapped( ))
		owner->setIsMinimized( TRUE );

	owner->setStartRect( owner->getNormalRect( ) );
	MC::getWindowList( )->changeFocus( owner );

	if (!isCtrl)
	{
		McWindowMgr::getMainW( )->setActiveApp( owner );
		owner->setIsWindowObscured( FALSE );
		if (wasMinimized)
		{
			McRect wr;
			McWindowMgr::myGetWindowRect( owner->getAppHwnd( ), &wr );
			WINDOWPLACEMENT pm;
			McWindowMgr::myGetWindowPlacement( owner->getAppHwnd( ), &pm );
			
			owner->setIsWindowObscured( FALSE );
			if (!owner->getIsMaximized( ))
			{
				MC::doShowWindow( owner->getAppHwnd( ), SW_SHOW, owner->getIsW10App( ) );
			}
		}
	}
	else
		owner->raiseApp( );
	if (doFinish)
		McWindowMgr::getMainW( )->onFinish( );
	else
		owner->setIsCloaked( FALSE );
}


void McThumbW::immersiveAppMoveToTop()
{
	owner->raiseToTop();
	MC::getWindowList()->reinitLayout( MCS_TransitionImmersiveApp );
}

void McThumbW::onZoom( BOOL isSuper )
{
	// Request to animate to zoom view, either just this window or if isSuper all windows.

	McZoomW *zw = McWindowMgr::getZoomW();

	if (zw->isZoomed( ))
		return;

	if ( isSuper && McWindowMgr::getZoomW() )
		isSuper = McWindowMgr::getZoomW( )->buildSuperZoom( owner->getPileNumber( ), 
			owner->getIsOnAppBar() || !MC::getMM() ?
				McMonitorsMgr::getMainMonitor()->getHMONITOR() :
				owner->getAppMonitor() );

	if ( !isSuper )
	{
		zw->clearList();
		MC::getWindowList()->changeFocus( owner );
		owner->calculateZoomedPosition();
		zw->addItem( owner );
	}

	zw->doZoom( MC_ZOOM_UP );
}

void McThumbW::createLabel( )
{
	if (rawLabels) return; 

	char cbuff[1000];

	MC::GetWText( owner->getAppHwnd( ), cbuff );
	if (strlen( cbuff ) < 1)
			if (owner->getIsW10App( ))
				sprintf_s( cbuff, 1000, "*%s", owner->getProgName( ) );
			else
				sprintf_s( cbuff, 1000, "%s - pid: %d", owner->getProgName( ), owner->getAppPid( ) );

	int count = 0;
	char *cp = strchr( cbuff, '&' );
	while (cp && 10 > ++count)
	{
		*cp = '+';
		count++;
		cp = strchr( cbuff, '&' );
	}

	labels = new WCHAR[strlen( cbuff ) + 1];
	rawLabels = new WCHAR[strlen( cbuff ) + 1];
	MC::char2wchar( cbuff, labels );
	MC::char2wchar( cbuff, rawLabels );
}




