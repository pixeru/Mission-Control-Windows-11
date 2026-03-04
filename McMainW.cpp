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
#include "McModernAppMgr.h"
#include "McMainW.h"
#include "McBgW.h"
#include "McThumbW.h"
#include "McZoomW.h"
#include "McLabelW.h"
#include "McWList.h"
#include "McWItem.h"
#include "McAnimationMgr.h"
#include "McMonitorsMgr.h"
#include "McHotKeyMgr.h"
#include "McDragMgr.h"
#include "McHookMgr.h"
#include "McWindowMgr.h"
#include "McPilesMgr.h"

#pragma unmanaged

// McMainW is the topmost background window behind thumbnails in the thumbnail view.
// It detects mouse, touch and kb events and also controls the behavior of the background window.
// A thread is used to wait for animations to complete.

McMainW::McMainW( ) : McBaseW()
{
	mwEvent = CreateEvent( NULL, TRUE, FALSE, TEXT("transition animation thread" ));
	mwThread = CreateThread( NULL, 0, McBaseW::ThreadProc, this, 0, &mwThreadId );
}

McMainW::~McMainW()
{
	DestroyWindow( getHwnd() );
	TerminateThread( mwThread,0 );
	CloseHandle( mwThread );
	CloseHandle( mwEvent );
}

void McMainW::Activate( )
{
	if (!isActivated)
	{
		desktopRect.set( McMonitorsMgr::getMainMonitor( )->getDesktopSize( ) );
		injectWinKey = 0;
		activeApp = NULL;
		onDisplayFocusItem = NULL;
		transitionCount = 0;
		setStartButtonLocation( );
		PostMessage( getHwnd( ), WM_SETCURSOR, (WPARAM) GetDesktopWindow( ), NULL );
		WCHAR b[1000];
		DWORD size = 1000;
		GetComputerName( b, &size );
		SetWindowText( getHwnd( ), b );
		isActivated = TRUE;
	}
}

void McMainW::Deactivate( )
{
	if (isActivated)
	{
		ShowWindow( getHwnd( ), SW_HIDE );
		SetWindowText( getHwnd( ), L"<idle>" );
		isActivated = FALSE;
	}
}


void McMainW::start( )
{
	EnableWindow( getHwnd( ), TRUE );
	SetActiveWindow( getHwnd( ) );

	PostMessage( getHwnd(), WM_USER, WMMC_TRANSITIONDOWN, NULL );
}

LRESULT McMainW::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_SYSCOMMAND)
		return 0;

	if ( MC::getState() == MCS_Settings )
		return DefWindowProc(m_hwnd, uMsg, wParam, lParam);

    switch (uMsg)
    {

	case WM_MOUSEWHEEL:		// Only happens when passed on from active thumb window
		if ( !McMonitorsMgr::getIsTouchscreen() || MC::getState() != MCS_Display || MC::getIsMessageBoxActive( ))
			return DefWindowProc( getHwnd( ), uMsg, wParam, lParam );
		if (McWindowMgr::getZoomW() )
			if (McWindowMgr::getZoomW( )->isZoomed( ))
				SendMessage( McWindowMgr::getZoomW( )->getHwnd( ),
				WM_MOUSEWHEEL, wParam, lParam );
		onMouseWheel( wParam, lParam );
		return S_OK;

	case WM_GESTURE:
		MC::getTouchController()->handleGestureMessage( getHwnd(), wParam, lParam );
		return 0;

	case WM_QUERYENDSESSION:
	case WM_ENDSESSION:

		return MC::HandleShutdownMessage( "MainW", uMsg, wParam, lParam );

	case WM_CREATE:
		break;

	case WM_DESTROY:

		ShowWindow( getHwnd(), SW_HIDE );
		return 0;

    case WM_PAINT:
		onPaint( );
        return 0;

	case WM_HOTKEY:
		if ( MC::getState() == MCS_Display )
		{
			McZoomW *zw = McWindowMgr::getZoomW( );
			if ( zw )
				if (zw->isZoomed( ) || zw->isZooming( ))
					return 0;
			McHotKeyMgr::popHotKey( getHwnd() );
			onFinish();
			return 0;
		}
		return DefWindowProc(getHwnd(), uMsg, wParam, lParam);

	case WM_RBUTTONDOWN:
		break;

	case WM_LBUTTONDOWN:

		setLbInfo( GET_X_LPARAM( lParam ),GET_Y_LPARAM( lParam ));
		MC::recordHwnd( getHwnd( ) );
		return DefWindowProc(getHwnd(), uMsg, wParam, lParam);

	case WM_LBUTTONUP:
		if ( !checkLbInfo(GET_X_LPARAM( lParam ),GET_Y_LPARAM( lParam )))
				return DefWindowProc(getHwnd(), uMsg, wParam, lParam);
		clearLbInfo();

		if (!MC::matchHwnd( getHwnd( ) ))
			return DefWindowProc( getHwnd( ), uMsg, wParam, lParam );

		if ( MC::getState() == MCS_Display || MC::getState() == MCS_TransitionDown || MC::getState() == MCS_TransitionOver )
			if (onLeftButtonUp( (wParam&MK_CONTROL)!=0, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)))
				return 0;

		return DefWindowProc(getHwnd(), uMsg, wParam, lParam);

	case WM_MBUTTONDOWN:

		if (MC::getState( ) == MCS_Display)
			if (onMiddleButtonDown( GET_X_LPARAM( lParam ), GET_Y_LPARAM( lParam ) ))
				return 0;

		return DefWindowProc( getHwnd( ), uMsg, wParam, lParam );


	case WM_MOUSEMOVE:

		if (MC::getMM( ))
		{
			if ((wParam&MK_LBUTTON))
				McDragMgr::processEvent( getHwnd( ), DragMove, GET_X_LPARAM( lParam ), GET_Y_LPARAM( lParam ) );
		}
		break;

	case WM_SYSKEYDOWN:
	case WM_KEYDOWN:

 		if (wParam == VK_SHIFT || wParam == VK_CONTROL || wParam == VK_MENU || wParam == VK_LWIN || wParam == VK_RWIN )
			break;

		MC::getWindowList()->keyPress( wParam, lParam );
		break;

	case WM_USER:

		switch( wParam )
		{
		case WMMC_TRANSITIONDOWN:

			if ( MC::getState() == MCS_Idle )
			{
				onTransitionDown();
				return 0;
			}
			break;

		case WMMC_LAUNCHER:

			if (MC::getState( ) == MCS_Display)
			{
				Sleep( MC::getIsWindowsAnimating()?500:100 );
				onTransitionUp( );
			}

			return 0;

		case WMMC_TRANSITIONUP:

			if (  !MC::getIsMessageBoxActive() )
			{
				onTransitionUp();
				return 0;
			}
			break;

		case WMMC_TRANSITIONOVER:

			onTransitionOver( (McState)lParam );
			return 0;

		case WMMC_DISPLAY:

			onDisplay();
			return 0;

		case WMMC_SHOWLABEL:

			if ( McWindowMgr::getLabelW() )
				McWindowMgr::getLabelW( )->Resume( );
			if ( MC::getWindowList() )
				if ( MC::getWindowList()->getFocusItem() )
					PostMessage( MC::getWindowList( )->getFocusItem( )->getThumbHwnd( ),
						WM_SETFOCUS, NULL, NULL );
			break;

		case WMMC_DESTROY:

			onDestroy();
			return 0;

		case WMMC_REINIT:

			MC::getWindowList()->reinitLayout( MCS_TransitionDesk );
			break;

		case WMMC_TOUCH:
			onTouch( (TC_Message *)lParam );
			return 0;
		case WMMC_DOSETTINGS:
			onSettings( );
			return S_OK;
		case WMMC_DOSTART:
			doQuitAndInject( VK_LWIN );
			return S_OK;
		case WMMC_DOTASKVIEW:
			doQuitAndInject( VK_TAB );
			return S_OK;
		case WMMC_SHIFTLEFT:
			MC::getWindowList( )->onLeftArrowClick( );
			return S_OK;
		case WMMC_SHIFTRIGHT:
			MC::getWindowList( )->onRightArrowClick( );
			return S_OK;
		default:
			break;
		}
		return 0;

    default:
        return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
    }

    return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
}

void McMainW::onMouseWheel( WPARAM wParam, LPARAM lParam )
{
	ULONGLONG timeNow = GetTickCount64( );
	if (timeNow - MC::getWheelTime( ) < 400)
	{
		MC::setWheelTime( timeNow );
		return;
	}

	MC::setWheelTime( timeNow );

	int fwKeys = GET_KEYSTATE_WPARAM( wParam );
	int zDelta = GET_WHEEL_DELTA_WPARAM( wParam );

	TC_Message tcMessage( TCG_NONE, GET_X_LPARAM( lParam ), GET_Y_LPARAM( lParam ) );

	if (fwKeys&MK_CONTROL)
	{
		tcMessage.tcGesture = (zDelta < 0) ? TCG_PINCH : TCG_SPREAD;
		onTouch( &tcMessage );
	}

}

void McMainW::onTouch( TC_Message *tcm )
{
	switch ( tcm->tcGesture )
	{
	case TCG_2F_TAP:
		if (MC::getState( ) == MCS_Display)
		{
			POINT *offsets = McMonitorsMgr::getMainMonitor( )->getOffsets();
			int x = tcm->tcPoint.x - offsets->x;
			int y = tcm->tcPoint.y - offsets->y;

			McWItem **dtItems = MC::getWindowList( )->getDesktopItem( );

			for (int i = 0; i < McMonitorsMgr::getEffectiveMonitorCount( ); i++)
			{
				if (dtItems[i]->getNormalRect( )->contains( x, y ))
					onNormalizeAll( dtItems[i]->getAppMonitor( ) );
				Sleep( 250 );
			}
		}
		break;
	case TCG_PRESS_AND_TAP:
		if ( MC::getState( ) == MCS_Display )
		{
			POINT *offsets = McMonitorsMgr::getMainMonitor( )->getOffsets( );
			int x = tcm->tcPoint.x - offsets->x;
			int y = tcm->tcPoint.y - offsets->y;

			McWItem **dtItems = MC::getWindowList( )->getDesktopItem( );

			for (int i = 0; i < McMonitorsMgr::getEffectiveMonitorCount( ); i++)
			{
				if (dtItems[i]->getNormalRect( )->contains( x, y ))
					onMinimizeImmersiveApps( dtItems[i]->getAppMonitor( ) );
			}
		}
		break;
	case TCG_2F_SWIPE_DOWN:
		onFinish();
		break;
	case TCG_PINCH:
		break;
	case TCG_SPREAD:
		break;

	default:
		break;
	}
}

void McMainW::doQuitAndInject( WPARAM vkKey )
{
	injectWinKey = (int)vkKey;
	onFinish( );
}

BOOL  McMainW::onLeftButtonUp( BOOL control, int x, int y )
{
	McWList *windowList = MC::getWindowList();
	list<McWItem *> *itemList = windowList->getItemList( );

	if ( MC::getState() == MCS_TransitionDown || MC::getState() == MCS_TransitionOver || MC::getState() == MCS_Display )
	{
		for (list<McWItem *>::reverse_iterator rit = itemList->rbegin(); rit != itemList->rend(); ++rit)
		{
			McWItem *item = *rit;
			if (item->getCurRect()->contains(x, y))
			{
				if (item->getThumbWindow())
				{
					item->getThumbWindow()->onSelectWindow( control );
					return TRUE;
				}
			}
		}
	}

	McWItem **dtItems = windowList->getDesktopItem( );
	POINT *offsets = McMonitorsMgr::getMainMonitor( )->getOffsets( );
	x -= offsets->x;
	y -= offsets->y;

	for (int i = 0; i < McMonitorsMgr::getEffectiveMonitorCount( ); i++)
	{
		if (dtItems[i]->getNormalRect( )->contains( x, y ))
		{
			if (dtItems[i]->getThumbRect( )->contains( x, y ))
			{
				onMinimizeAll( TRUE, NULL, dtItems[i]->getAppMonitor() );
				return TRUE;
			}
			else if (control)
			{
				onNormalizeAll( dtItems[i]->getAppMonitor( ) );
				return TRUE;
			}
			else
				onFinish( );
		}
		else if (!MC::getMM() )
			onFinish( );
	}

	return FALSE;
}

BOOL McMainW::onMiddleButtonDown( int x, int y )
{
	McWItem **dtItems = MC::getWindowList( )->getDesktopItem( );
	POINT *offsets = McMonitorsMgr::getMainMonitor( )->getOffsets( );
	x -= offsets->x;
	y -= offsets->y;

	for (int i = 0; i < McMonitorsMgr::getEffectiveMonitorCount( ); i++)
	{
		if (dtItems[i]->getNormalRect( )->contains( x, y ))
		{
			if (dtItems[i]->getThumbRect( )->contains( x, y ))
			{
				onMinimizeImmersiveApps( dtItems[i]->getAppMonitor( ) );
				return TRUE;
			}
		}
	}

	return FALSE;
}

void McMainW::onTransitionUp()
{
	MC::setState(MCS_TransitionUp);
	McWindowMgr::getBgW( )->hideButtons( );

	// Hide each subwindow, detach the thumbnail and reattach it
	// to the mainwindow.   Then go to the thread to finish

	McWList *windowList = MC::getWindowList();

	windowList->initializeRects( MCS_TransitionUp );

	HWND main = McWindowMgr::getMainW()->getHwnd();
	list <McWItem *> *itemList = windowList->getItemList( );
	for (ITEM_IT it = itemList->begin( ); it != itemList->end( ); it++)
	{
		McWItem *item = *it;
		HTHUMBNAIL oldThumb = item->getThumbnail();
		item->registerThumbnail(FALSE,WIL_MAIN_WINDOW, TRUE );
		item->showThumbnail();
		item->closeThumbWindow();
		if ( oldThumb )
			item->unregisterThumbnail( oldThumb );
	}

	// Do the same thing to the desktop windows

	for (int i = 0; i < McMonitorsMgr::getEffectiveMonitorCount( ); i++)
	{
		McWItem *dtItem = windowList->getDesktopItem( )[i];
		HTHUMBNAIL oldThumb = dtItem->getThumbnail( );
		HTHUMBNAIL oldIconThumb = dtItem->getIconThumb( );
		dtItem->registerThumbnail( FALSE, WIL_BG_WINDOW );
		dtItem->showThumbnail( );
		dtItem->closeDesktopWindow( );
		if ( oldThumb )
			dtItem->unregisterThumbnail( oldThumb );
		if (oldIconThumb)
			dtItem->unregisterThumbnail( oldIconThumb );
	}

	MC::setState( MCS_TransitionUp );

	double startval = 0.0;
	if (lastValue > 0.0 && lastValue < 1.0)
		startval = 1.0 - lastValue;

	if (startval < 0.0) startval = 0.0;

	calcFrame( startval );
	MC::getMC( )->getAnimationMgr( )->doAnimate( getHwnd( ), startval, 1.2, 1.0 );
}

void McMainW::onTransitionDown()
{
	// Attach the thumbnails to the main or fading window.

	McWList *windowList = MC::getWindowList();

	list<McWItem *> *itemList = windowList->getItemList( );

	if (MC::inTabletMode( ))
	{
		BOOL flag = FALSE;
		for (ITEM_IT it = itemList->begin( ); it != itemList->end( ); it++)
			if (!(*it)->getIsMinimized( ))
			{
				flag = TRUE;
				break;
			}

		if (!flag)
			for (ITEM_IT it = itemList->begin( ); it != itemList->end( ); it++)
			{
				McWItem *item = (*it);
				if (item->getIsMinimized( ))
				{
					PostMessage( item->getAppHwnd( ), WM_SYSCOMMAND, SC_RESTORE, NULL );
					item->setIsMinimized( FALSE );
					Sleep( 500 );
					break;
				}
			}
	}

#if MC_DESKTOPS
	BOOL haveCloaked = FALSE;
	BOOL haveUnCloaked = FALSE;
	for (ITEM_IT it = itemList->begin( ); it != itemList->end( ); it++)
	{
		if ((*it)->getIsCloaked( )) haveCloaked = TRUE;
		else haveUnCloaked = TRUE;
	}

	if ( haveUnCloaked || ( haveCloaked && MC::getProperties()->getShowAllDesktops() ) )
		calcFrame( 0.0 );
#endif


	McRect *r = McMonitorsMgr::getMainMonitor( )->getDesktopSize( );

	SetLayeredWindowAttributes( McWindowMgr::getFadingW()->getHwnd(), 0, 0, LWA_ALPHA );
	windowList->initializeRects(MCS_TransitionDown);

	MC::setState(MCS_TransitionDown);

	SetWindowPos(McWindowMgr::getBgW()->getHwnd(), HWND_TOP,
		r->left, r->top, r->getWidth(), r->getHeight(),
		SWP_SHOWWINDOW);
	SetWindowPos(McWindowMgr::getFadingW()->getHwnd(), HWND_TOP,
		r->left, r->top, r->getWidth(), r->getHeight(),
		SWP_SHOWWINDOW);
	SetWindowPos(getHwnd(), HWND_TOP,
		r->left, r->top, r->getWidth(), r->getHeight(),
		SWP_SHOWWINDOW);

	McHotKeyMgr::pushHotKey( getHwnd());
	SetForegroundWindow(getHwnd());

	McWindowMgr::getBgW()->Show(McWindowMgr::getFadingW()->getHwnd());

	for (ITEM_IT it = itemList->begin( ); it != itemList->end( ); it++)
	{

		(*it)->registerThumbnail( FALSE, WIL_MAIN_WINDOW, TRUE );
		(*it)->showThumbnail();
	}

	McWindowMgr::getBgW( )->lowerButtons( );

	for (int i = 0; i < McMonitorsMgr::getEffectiveMonitorCount(); i++)
	{
		windowList->getDesktopItem()[i]->registerThumbnail(WIL_BG_WINDOW);
		windowList->getDesktopItem()[i]->showThumbnail();
	}

	SetLayeredWindowAttributes(McWindowMgr::getBgW()->getHwnd(), 1 + McModernAppMgr::GetBgColor(), 0, LWA_COLORKEY);

	MC::getMC( )->getAnimationMgr( )->doAnimate( getHwnd( ), 0.0, 1.0 );
}

void McMainW::doTransitionOver( McState transition )
{
	transitionCount++;
	PostMessage( getHwnd( ), WM_USER, WMMC_TRANSITIONOVER, transition );
}

void McMainW::onTransitionOver( McState transition )
{
	// Attach the thumbnails to the main window.

	if (!transitionCount)
		return;
	transitionCount = 0;

	McLabelW *l = McWindowMgr::getLabelW();
	l->Hide();
	l->Pause( );

	McWList *windowList = MC::getWindowList();

	windowList->initializeRects( transition );

	McWindowMgr::getBgW( )->lowerButtons( );

	SetForegroundWindow( getHwnd() );
	list<McWItem *> *itemList = windowList->getItemList( );
	for (ITEM_IT it = itemList->begin( ); it != itemList->end( ); it++)
	{
		McWItem *item = *it;
		if (transition == MCS_TransitionOver ||
			(transition == MCS_TransitionImmersiveApp && item->getIsOnAppBar( )) ||
			(transition == MCS_TransitionDesk && !item->getIsOnAppBar( )))
		{
			HTHUMBNAIL oldThumb = item->getThumbnail( );
			item->registerThumbnail( FALSE, WIL_MAIN_WINDOW, TRUE );
			item->showThumbnail( );
			item->closeThumbWindow( );
			if (oldThumb) item->unregisterThumbnail( oldThumb );
		}
	}

	if (transition != MCS_TransitionImmersiveApp)
	{

		for (int i = 0; i < McMonitorsMgr::getEffectiveMonitorCount( ); i++)
		{
			McWItem *dtItem = windowList->getDesktopItem( )[i];
			if (!dtItem->getStartRect( )->equals( dtItem->getEndRect( ) ))
			{
				HTHUMBNAIL oldThumb = dtItem->getThumbnail( );
				HTHUMBNAIL oldIconThumb = dtItem->getIconThumb( );
				dtItem->registerThumbnail( FALSE, WIL_BG_WINDOW );
				dtItem->showThumbnail( );
				dtItem->closeDesktopWindow( );
				if (oldThumb) dtItem->unregisterThumbnail( oldThumb );
				if (oldIconThumb) dtItem->unregisterThumbnail( oldIconThumb );
			}
		}
	}

	MC::setState(transition);

	MC::getMC()->getAnimationMgr()->doAnimate( getHwnd(), 0.0, 1.0 );
}

void McMainW::onDestroy( )
{

	// called after the animation back to desktop view is completed.

	McHotKeyMgr::popHotKey( getHwnd( ) );

	HWND hwnd = McWindowMgr::getBgW( )->getHwnd( );
	McWList *windowList;
	list<McWItem *> *itemList;

	windowList = MC::getWindowList( );
	itemList = windowList->getItemList( );
	for (ITEM_IT it = itemList->begin( ); it != itemList->end( ); it++)
	{
		HTHUMBNAIL oldThumb = (*it)->getThumbnail( );
		if (!(*it)->needsTransparency( ))
		{
			(*it)->registerThumbnail( FALSE, WIL_BG_WINDOW );
			(*it)->showThumbnail( );
		}
		if (oldThumb)
			(*it)->unregisterThumbnail( oldThumb );
	}

	ShowWindow( getHwnd( ), SW_HIDE );
	ShowWindow( McWindowMgr::getBackingW( )->getHwnd( ), SW_HIDE );
	McWindowMgr::getBgW( )->Hide( );

	windowList=MC::getWindowList();

	for (int i = 0; i < McMonitorsMgr::getEffectiveMonitorCount( ); i++)
		windowList->getDesktopItem( )[i]->unregisterThumbnail();
	itemList = windowList->getItemList( );
	for (ITEM_IT it = itemList->begin( ); it != itemList->end( ); it++)
		(*it)->unregisterThumbnail();

	MC::setState( MCS_Finishing );

	if (activeApp)
	{

		if (activeApp != _APP_SHOW_DESKTOP)
		{
			HWND dividerHwnd = 0;
			if (McModernAppMgr::tabletModeDividerExists( &dividerHwnd ))
			{
				if (activeApp->getIsCloaked( ))
				{
					WINDOWPLACEMENT wp = { 0 };
					McWindowMgr::myGetWindowPlacement( dividerHwnd, &wp );
					McModernAppMgr::ClickMouseAt(
						(wp.rcNormalPosition.left + wp.rcNormalPosition.right) / 2,
						(wp.rcNormalPosition.top + wp.rcNormalPosition.bottom) / 2, TRUE );
					Sleep( 250 );
					SwitchToThisWindow( activeApp->getAppHwnd( ), FALSE );
				}
			}
			else
			{
				activeApp->raiseApp( );
				if (activeApp->getIsCloaked( ) && activeApp->getIsMinimized( ))
				{
					if (activeApp->getIsMaximized( ))
					{
						McRect *r = McMonitorsMgr::getMonitor( activeApp->getAppMonitor( ) )->getMonitorSize( );
						SetWindowPos( activeApp->getAppHwnd( ), HWND_TOP, r->left, r->top, r->getWidth( ), r->getHeight( ), SWP_SHOWWINDOW | SWP_ASYNCWINDOWPOS );
					}
					else
						PostMessage( activeApp->getAppHwnd( ), WM_SYSCOMMAND, SC_RESTORE, NULL );
				}
			}
		}
	}

	if (injectWinKey)
	{
		if (injectWinKey != VK_LWIN)
		{
			if ( injectWinKey == VK_TAB)
				MC::usingTaskview = TRUE;
			McModernAppMgr::InjectWinKey( injectWinKey );
		}
		else
			McModernAppMgr::InjectWinKey( );
		injectWinKey = 0;
	}
}

void McMainW::onDisplay( )
{
	// Called after an animation to a thumbnail view has completed.

	McState oldState = MC::getState( );
	MC::setState( MCS_Display );

	McWindowMgr::getLabelW( )->Pause( );

	McWList *windowList = MC::getWindowList( );

	list<McWItem *> *itemList = windowList->getItemList( );

	// To avoid flicker as we transition from thumbnail display on the main
	// window to display on the thumbnail windows some care is needed.

	// First, position the thumb windows BENEATH the main or fading window

	for (ITEM_IT it = itemList->begin( ); it != itemList->end( ); it++)
	{
		McWItem *item = *it;
		if (oldState == MCS_TransitionOver || oldState == MCS_TransitionDown ||
			(oldState == MCS_TransitionImmersiveApp && item->getIsOnAppBar( )) ||
			(oldState == MCS_TransitionDesk && !item->getIsOnAppBar( )))
		{
			item->adjustVisibility( );

			HTHUMBNAIL oldThumb = item->getThumbnail( );
			item->registerThumbnail( FALSE, WIL_THUMB_WINDOW );
			item->showThumbnail( );
			item->showThumbWindow( McWindowMgr::getFadingW( )->getHwnd( ) );

			if (oldThumb)
				item->unregisterThumbnail( oldThumb );
		}
		else
			item->showThumbWindow( getHwnd( ) );
	}

	if (oldState < MCS_TransitionImmersiveApp)
		McWindowMgr::getBgW( )->doRepaint( TRUE );

	// Now reposition the (now clear) main window under the thumb windows

	McRect *r = McMonitorsMgr::getMainMonitor( )->getDesktopSize( );
	if (windowList->getFirstVisibleItem( ) != NULL)
	{
		SetWindowPos( McWindowMgr::getFadingW()->getHwnd(), GetNextWindow( McWindowMgr::getBgW( )->getHwnd( ), GW_HWNDPREV ),
			r->left, r->top, r->getWidth( ), r->getHeight( ), SWP_NOACTIVATE );
		SetWindowPos( getHwnd( ), GetNextWindow( McWindowMgr::getFadingW( )->getHwnd( ), GW_HWNDPREV ),
			r->left, r->top, r->getWidth( ), r->getHeight( ), SWP_NOACTIVATE );
	}

	// Now move the desktop thumbnails to to desktop windows, position the desktop windows under the
	// main window, and remove the desktop thumbnails from the background window.

	for (int i = 0; i < McMonitorsMgr::getEffectiveMonitorCount( ); i++)
	{
		McWItem *dtItem = windowList->getDesktopItem( )[i];
		HTHUMBNAIL oldThumb = dtItem->getThumbnail( );
		HTHUMBNAIL oldIconThumb = dtItem->getIconThumb( );
		dtItem->registerThumbnail( FALSE, WIL_DT_WINDOW );
		dtItem->showThumbnail( );
		dtItem->showDesktopWindow( );
		if ( oldThumb )
			dtItem->unregisterThumbnail( oldThumb );
		if (oldIconThumb)
			dtItem->unregisterThumbnail( oldIconThumb );
	}

	if (onDisplayFocusItem == (McWItem *)-1 )
		onDisplayFocusItem = windowList->getFirstVisibleAppbarItem( );

	if ( !onDisplayFocusItem )
		onDisplayFocusItem = windowList->getLastVisibleAppbarItem();

	if (!onDisplayFocusItem)
		onDisplayFocusItem = windowList->getLastVisibleItem( );

	if ( onDisplayFocusItem )
	{
		if (onDisplayFocusItem != (McWItem *) -1)
			PostMessage( onDisplayFocusItem->getThumbHwnd( ), WM_USER, WMMC_FOCUS, NULL );
		onDisplayFocusItem = NULL;
	}

	PostMessage( McWindowMgr::getBgW( )->getHwnd( ), WM_USER, WMMC_BGCHANGED, NULL );
	PostMessage( getHwnd( ), WM_USER, WMMC_SHOWLABEL, NULL );

}

void McMainW::onMinimizeAll( BOOL terminate, McWItem *except, HMONITOR _monitor )
{
	// Minimize all desktop windows and lower all windows store apps on _monitor, or on
	// all monitors if _monitor is 0;   If terminate is TRUE then return to the background to
	// await a hot key or hot corner event

	McWList *windowList = MC::getWindowList( );
	list<McWItem *> *itemList = windowList->getItemList( );

	if (MC::inTabletMode( ))
	{
		for (ITEM_IT it = itemList->begin( ); it != itemList->end( ); it++)
		{
			McWItem *item = *it;
			if (!item->getIsMinimized( ))
			{
				item->setIsMinimized( TRUE );
			}
		}
		doQuitAndInject( VK_LWIN );
		return;
	}

	setActiveApp( _APP_SHOW_DESKTOP, MC::getMM()?_monitor:0 );


	for (ITEM_IT it = itemList->begin( ); it != itemList->end( ); it++)
	{
		McWItem *item = *it;
		if (!_monitor || !MC::getMM( ) || (MC::getMM( ) && _monitor == item->getAppMonitor( )))
			if ( (except != item) && !item->getIsCloaked() )
			{
				PostMessage( item->getAppHwnd( ), WM_SYSCOMMAND, SC_MINIMIZE, NULL );
				item->setIsMinimized( TRUE );
			}
	}

	if ( terminate )
		onFinish();
}

void McMainW::onMinimizeImmersiveApps( HMONITOR _monitor )
{
	setActiveApp( _APP_SHOW_DESKTOP, MC::getMM( ) ? _monitor : 0 );
	onFinish( );
}

void McMainW::onFinish()
{
	MC::setState( MCS_TransitionUp );
	PostMessage( getHwnd(), WM_USER, WMMC_TRANSITIONUP, NULL );
}

void McMainW::onNormalizeAll( HMONITOR _monitor )
{
	if (MC::inTabletMode( ))
	{
		onFinish( );
		return;
	}
	MC::setState( MCS_Finishing );
	McWList *windowList = MC::getWindowList();

	// normalize (show) any minimized desktop windows and be done.

	list<McWItem*> *itemList = windowList->getItemList( );
	if (itemList->size( ) > 0)
	{
		int count = 0;
		McThumbW** thumbs = new McThumbW*[itemList->size( )];
		for (ITEM_IT it = itemList->begin( ); it != itemList->end( ); it++)
		{
			McWItem *item = *it;
			thumbs[count] = NULL;
			if (item->getIsMinimized( ) && !(item->getIsW10App()&&item->getIsMaximized()) )
				if (!_monitor || !MC::getMM( ) || (MC::getMM( ) && item->getAppMonitor( ) == _monitor))
					thumbs[count++] = item->getThumbWindow();
		}
		for ( int i=0;i<count;i++ )
			if ( thumbs[i] )
				if (!thumbs[i]->getOwner( )->getIsCloaked( ))
					thumbs[i]->onSelectWindow( FALSE, FALSE );

		delete thumbs;
	}

	onFinish();
}


void McMainW::onPaint(  )
{
	static int	oldState = -1;
	static BOOL doClear = 0;
	// Painting consists of displaying the in-transition thumbnails in the right place
	// during an animation.
	if (lastValue < 0 || lastValue == latestValue)
		return;
	latestValue = lastValue;

	McWList *windowList = MC::getWindowList();
	list<McWItem*> *itemList = windowList->getItemList( );

	McState state = MC::getState( );

	switch( state )
	{
	case MCS_Idle:

		return;

	case MCS_TransitionOver:
	case MCS_TransitionUp:
	case MCS_TransitionDown:
	{
		for (int i = 0; i < McMonitorsMgr::getEffectiveMonitorCount(); i++)
			windowList->getDesktopItem()[i]->showThumbnail();
		for (ITEM_IT it = itemList->begin(); it != itemList->end(); it++)
		{
			(*it)->showThumbnail();
			
		}
		SetLayeredWindowAttributes( McWindowMgr::getFadingW( )->getHwnd( ), 0, this->fadingAlpha, LWA_ALPHA );
		break;
	}

	case MCS_TransitionImmersiveApp:
		{
			for (ITEM_IT it = itemList->begin( ); it != itemList->end( ); it++)
			if ((*it)->getIsOnAppBar( ))
					(*it)->showThumbnail();
		}
		break;

	case MCS_TransitionDesk:
		{
			for (ITEM_IT it = itemList->begin( ); it != itemList->end( ); it++)
			if (!(*it)->getIsOnAppBar( ))
					(*it)->showThumbnail();
		}
		break;

	default:
		break;
	}

}

DWORD McMainW::runThread()
{
	// Thread wait for activation, which comes after an animation

	while ( TRUE )
	{
		DWORD hr = WaitForSingleObject( mwEvent, INFINITE );
		switch ( hr )
		{
		case WAIT_OBJECT_0:
			break;
		default:
			return 0;
		}

		ResetEvent( mwEvent );

		switch( MC::getState() )
		{
		case MCS_TransitionUp:

			PostMessage( getHwnd(), WM_USER, WMMC_DESTROY, NULL );
			break;
		case MCS_TransitionDesk:
		case MCS_TransitionImmersiveApp:
		case MCS_TransitionOver:
		case MCS_TransitionDown:
			while (lastValue < .9999)
				Sleep( 25 );

			if ( MC::getState() == MCS_TransitionDown ) McWindowMgr::setHoverTime( 250 );

			PostMessage( getHwnd(), WM_USER, WMMC_DISPLAY, NULL );
			break;
		default:
			break;
		}

	}
}


void McMainW::calcFrame( double _value )
{
	// Called during animation, figures out how big each thumbnail should be and where.

	lastValue = _value;

	double value = _value;

	McWList *windowList = MC::getWindowList();

	switch (MC::getState( ))
	{
	case MCS_TransitionOver:
	case MCS_TransitionUp:
	case MCS_TransitionDown:
		{
			McWItem **dtItems = windowList->getDesktopItem( );
			for (int i = 0; i < McMonitorsMgr::getEffectiveMonitorCount( ); i++)
			{
				dtItems[i]->getCurRect()->interpolate(
					dtItems[i]->getEndRect(),
					dtItems[i]->getStartRect(),
					value);
					dtItems[i]->getCurRect( )->offset( -1, -1 );
			}
		}
	}

	double alphaval = value;
	switch ( MC::getState() )
	{
	case MCS_TransitionDown:
		break;
	case MCS_TransitionUp:
		alphaval = 1.0-value;
		break;
	case MCS_TransitionOver:
	case MCS_TransitionImmersiveApp:
	case MCS_TransitionDesk:
		alphaval = 1.0;
		break;
	default:
		alphaval = value;
		break;
	}

	if (fabs( _value ) < 0.00001) _value = 0.0;
	if (fabs( 1.0 - _value ) < 0.00001) _value = 1.0;
	list<McWItem *> *itemList = windowList->getItemList( );
	fadingAlpha = (int) (255.0*(pow(alphaval,3)) );

	for (ITEM_IT it = itemList->begin( ); it != itemList->end( ); it++)
	{
		McWItem *item = *it;
	
		item->getCurRect()->interpolate(
			item->getEndRect(),
			item->getStartRect(),
			value );
	}
	if (MC::getState() != MCS_Idle && (value > .9999))
	{
		SetEvent(mwEvent);
	}
}

#pragma managed
SETTINGS_DISPOSITION doSettings(list<string>*);
#pragma unmanaged

void McMainW::onSettings()
{
	// Bring up the settings editor

	MC::setState( MCS_Settings );

	McLabelW *l = McWindowMgr::getLabelW( );
	l->Hide( );

	SETTINGS_DISPOSITION disposition = doSettings(MC::getWindowList()->getDesktopAppList());
	MC::setState( MCS_Display );

	switch (disposition)
	{
	case settingsAccept:
		if ( !McModernAppMgr::IsLauncherVisible() )
			MC::getMC( )->setRunAgain( TRUE );
		MC::getProperties( )->write( );
		McWindowMgr::getMainW( )->clearActiveApp( );

	case settingsOther:
		if (!McModernAppMgr::IsLauncherVisible( ))
			onFinish( );
		break;
	case settingsCancel:
		if (MC::getWindowList( ))
			if (MC::getWindowList( )->getFocusItem( ))
				l->Show( MC::getWindowList( )->getFocusItem( )->getThumbWindow( ) );
			else
				McWindowMgr::getBgW( )->raiseButtons( );
		break;
	}

	if (McModernAppMgr::IsLauncherVisible( ))
		PostMessage( McWindowMgr::getMainW( )->getHwnd( ), WM_USER, WMMC_LAUNCHER, NULL );
}

void McMainW::clearActiveApp( McWItem *_target )
{
	if ( _target==NULL || _target == activeApp )
		{
			activeApp = NULL;
		}
	}

void McMainW::setStartButtonLocation( )
{
	BOOL haveAppBar = FALSE;
	McWList *wl = MC::getWindowList( );
	if ( wl )
		for (ITEM_IT it = wl->getItemList()->begin( ); it != wl->getItemList()->end( ); it++)
		{
			if ((*it)->getIsOnAppBar( ))
			{
				haveAppBar = TRUE;
				break;
			}
		}


	{
		McRect mR( (McMonitorsMgr::getMainMonitor( )->getMonitorSize( )) );
		long bottom = mR.bottom;
		long padding = 5;
		if (haveAppBar > 0)
		{
			bottom += (wl->getImmersiveAppBlank() - wl->getImmersiveappOffset( ));
			padding = wl->getImmersiveappGap( );
		}

	}
}



