// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Mission Control for Windows 11 by pixeru
// Source code originally by Emcee (https://sourceforge.net/projects/mcsoft/)
// Extensive development by pixeru due to discontinued development of original project.
// Copyright (c) pixeru. All rights reserved.
#include "stdafx.h"
#include "Emcee.h"
#include "McMainW.h"
#include "McBgW.h"
#include "McThumbW.h"
#include "McZoomW.h"
#include "McLabelW.h"
#include "McBackingW.h"
#include "McWList.h"
#include "McWItem.h"
#include "McAnimationMgr.h"
#include "McModernAppMgr.h"
#include "McWindowMgr.h"
#include "McPilesMgr.h"
#include "McDragMgr.h"

// Zoom Window is used to display thumbnails of zoomed thumbs.

#pragma unmanaged
McZoomW::McZoomW()
{
	myAlpha = 0;
	oldFocusItem = NULL;
	mode = MC_ZOOM_NONE;
	zwEvent = CreateEvent( NULL, TRUE, FALSE, TEXT("zoom thread" ));
	zwThread = CreateThread( NULL, 0, McBaseW::ThreadProc, this, 0, &zwThreadId );
	isSelected = NULL;
	inAnimation = FALSE;
	isZoomingAgain = FALSE;
}

McZoomW::~McZoomW()
{
	Deactivate( );
	DestroyWindow( getHwnd() );
	TerminateThread( zwThread, 0 );
	CloseHandle( zwEvent );
	CloseHandle( zwThread );
}

void McZoomW::Activate( )
{
	if (!isActivated)
	{
		myAlpha = 0;
		mode = MC_ZOOM_NONE;
		isSelected = isZoomingAgain = FALSE;
		SetWindowText( getHwnd( ), L"ZoomW" );
		isActivated = TRUE;
	}
}

void McZoomW::Deactivate( )
{
	if (isActivated)
	{
		ShowWindow( getHwnd( ), SW_HIDE );
		SetWindowText( getHwnd( ), L"<idle>" );
		isActivated = FALSE;
	}
}

LRESULT McZoomW::HandleMessage( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if (uMsg == WM_SYSCOMMAND)
		return 0;
	
	switch ( uMsg )
	{
	case WM_CREATE:
	
		return 0;

	case WM_MOUSEWHEEL:

		if (McMonitorsMgr::getIsTouchscreen( ) )
			onMouseWheel( wParam, lParam );
		return DefWindowProc(m_hwnd, uMsg, wParam, lParam);

	case WM_GESTURE:
		MC::getTouchController()->handleGestureMessage( getHwnd(), wParam, lParam );
		return 0;

	case WM_MOUSEMOVE:
		break;

	case WM_KEYDOWN:
		if (inAnimation)
			return 0;
		if (wParam == VK_RETURN)
		{
			isSelected = NULL;
			lParamS = NULL;
		}
		else if ( wParam != VK_Z_KEY && wParam != VK_ESCAPE )
			return 0;
		doZoom( MC_ZOOM_DOWN ); 
		return 0;

	case WM_LBUTTONDOWN:
		setLbInfo( GET_X_LPARAM( lParam ),GET_Y_LPARAM( lParam ));
		MC::recordHwnd( getHwnd( ) );
		return DefWindowProc(getHwnd(), uMsg, wParam, lParam);

	case WM_LBUTTONUP:
		if (inAnimation)
			return 0;
		if ( !checkLbInfo(GET_X_LPARAM( lParam ),GET_Y_LPARAM( lParam )))
				return DefWindowProc(getHwnd(), uMsg, wParam, lParam);
		clearLbInfo();

		if ( !MC::matchHwnd( getHwnd() ) )
			return DefWindowProc( getHwnd( ), uMsg, wParam, lParam );

		{
			int Left = GET_X_LPARAM( lParam );
			int Top  = GET_Y_LPARAM( lParam );
			for ( list<McWItem *>::iterator it = thumbOwners.begin(); it != thumbOwners.end(); it++ )
			{
				if ( (*it)->getCurRect()->contains( Left, Top ) )
				{
					isSelected = *it;
					lParamS = wParam&MK_CONTROL;
					break;
				}
			}
			doZoom( MC_ZOOM_DOWN);
			return 0;
		}

	case WM_MBUTTONDOWN:
		if (inAnimation)
			return 0;
 		if ( (wParam & MK_CONTROL) == NULL ) 
			doKillFromZoom( GET_X_LPARAM( lParam ), GET_Y_LPARAM( lParam ) );
		return 0;

	case WM_RBUTTONDOWN:

		// transition down, get rid of thumbnail, hide window
		if (inAnimation)
			return 0;
		if (thumbOwners.size( ) == 1 || (wParam&MK_CONTROL))
			doZoom( MC_ZOOM_DOWN );
		else
		{
			BOOL hit = doReZoom( GET_X_LPARAM( lParam ), GET_Y_LPARAM( lParam ) );
			if (!hit)
			{
				PostMessage( getHwnd( ), WM_LBUTTONDOWN, wParam, lParam );
				PostMessage( getHwnd( ), WM_LBUTTONUP, wParam, lParam );
			}
		}
		
		return 0;

	case WM_USER:

		// Restore thumbnail view after zoom completed
		{
			if (wParam == WMMC_TOUCH)
				onTouch( (TC_Message *) lParam );
			else if (wParam == WMMC_HIDE)
			{
				MC::setState( MCS_Display );
				mode = MC_ZOOM_NONE;
				McWItem *destroyItem = NULL;

				for (list<McWItem*>::reverse_iterator it = thumbOwners.rbegin( ); it != thumbOwners.rend( ); it++)
				{
					(*it)->clearTempThumbnail( );
					if ((*it)->getVisibility( ))
					{
						(*it)->setTempThumbnail( );
						(*it)->registerThumbnail( FALSE, WIL_THUMB_WINDOW );
						(*it)->showThumbnail(  );
						(*it)->placeInPile( );
					}
					else
					{
						destroyItem = *it;
					}
				}
				

				ShowWindow( McWindowMgr::getBackingW( )->getHwnd( ), SW_HIDE );

				if (oldFocusItem)
				{
					if ( oldFocusItem->getVisibility() )
						SendMessage( oldFocusItem->getThumbWindow( )->getHwnd( ), WM_SETFOCUS, 0, 0 );
				}

				McWindowMgr::FadeWindowOut( getHwnd( ) );
				ShowWindow( getHwnd( ), SW_HIDE ); 

				for (list<McWItem*>::reverse_iterator it = thumbOwners.rbegin( ); it != thumbOwners.rend( ); it++)
				{
					if ((*it)->getTempThumbnail( ))
						(*it)->unregisterThumbnail( (*it)->getTempThumbnail( ) );
				}

				clearList( );

				oldFocusItem = NULL;

				if (destroyItem)
				{
					MC::getWindowList( )->killApp( destroyItem );
					return 0;
				}

				if (isSelected)
				{
					PostMessage( isSelected->getThumbWindow( )->getHwnd( ), WM_USER, WMMC_SELECT, lParamS );
					isSelected = NULL;
					lParamS = NULL;
				}

				if (MC::getWindowList( ))
					if (MC::getWindowList( )->getFocusItem( ))
						McWindowMgr::getLabelW()->Show( MC::getWindowList( )->getFocusItem( )->getThumbWindow( ) );

				isZoomingAgain = NULL;
			}
		}
		return 0;

	case WM_DESTROY:
		return 0;

	case WM_PAINT:

		if ( mode != MC_ZOOM_NONE )
			onPaint( );

		return 0;

	default:
		return DefWindowProc( m_hwnd, uMsg, wParam, lParam );
		
	}

	return TRUE;
}

void McZoomW::doKillFromZoom( int Left, int Top )
{
	McWItem *found = NULL;
	for ( list<McWItem *>::iterator it = thumbOwners.begin(); it != thumbOwners.end(); it++ )
	{
		if ( (*it)->getCurRect()->contains( Left, Top ) )
		{
			found = *it;
			found->unregisterThumbnail();
			break;
		}
	}

	if (found)
		found->setVisibility( FALSE, FALSE );

	doZoom( MC_ZOOM_DOWN );
}

BOOL McZoomW::doReZoom( int Left, int Top )
{
	isZoomingAgain = NULL;
	for ( list<McWItem *>::iterator it = thumbOwners.begin(); it != thumbOwners.end(); it++ )
	{
		if ( (*it)->getCurRect()->contains( Left, Top ) )
		{
			isZoomingAgain = *it;
			break;
		}
	}
	BOOL outcome = TRUE;
	if (isZoomingAgain)
		doZoom( MC_ZOOM_AGAIN );
	else
		outcome = FALSE;
	return outcome;
}

void McZoomW::onMouseWheel( WPARAM wParam, LPARAM lParam )
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

	// Convert to one of our touch gestures

	if ( fwKeys&MK_CONTROL )
		tcMessage.tcGesture = (zDelta<0) ? TCG_PINCH : TCG_SPREAD;
	else
		tcMessage.tcGesture = (zDelta<0) ? TCG_2F_SWIPE_UP : TCG_2F_SWIPE_DOWN;

	onTouch( &tcMessage );
}

void McZoomW::onTouch( TC_Message *tcm )
{
	switch( tcm->tcGesture )
	{
	case TCG_PINCH:
	case TCG_2F_TAP:
	case TCG_2F_SWIPE_DOWN:
		// leave zoom
		doZoom( MC_ZOOM_DOWN );
		break;
	case TCG_SPREAD:
		if ( thumbOwners.size() > 1 )
			doReZoom( tcm->tcPoint.x, tcm->tcPoint.y );
		break;
	case TCG_2F_SWIPE_UP:
		// zoom in on one of the zoomed piles.
		if (thumbOwners.size( ) > 1)
			doReZoom( tcm->tcPoint.x, tcm->tcPoint.y );
		else
			doZoom( MC_ZOOM_DOWN );
		break;
	case TCG_PRESS_AND_TAP:
		// kill an app
		doKillFromZoom( tcm->tcPoint.x, tcm->tcPoint.y );
		break;
	default:
		break;

	}
}

BOOL McZoomW::isItZoomed( HWND appHwnd )
{
	for (list<McWItem *>::iterator it = thumbOwners.begin( ); it != thumbOwners.end( ); it++)
	{
		if ((*it)->getAppHwnd( ) == appHwnd)
		{
			return TRUE;
		}
	}
	return FALSE;
}

void McZoomW::onPaint()
{		
	if (lastValue < 0 || lastValue == latestValue)
		return;
	latestValue = lastValue;
	for ( list<McWItem *>::iterator it = thumbOwners.begin(); it != thumbOwners.end(); it++ )
		(*it)->showThumbnail();
}

void McZoomW::doZoom( int _mode )
{
	if (_mode == MC_ZOOM_PANIC)
	{
		mode = MC_ZOOM_NONE;
		ShowWindow( McWindowMgr::getBackingW( )->getHwnd( ), SW_HIDE );
		ShowWindow( getHwnd( ), SW_HIDE );
	}

	SetLayeredWindowAttributes( getHwnd( ), NULL, 255, LWA_ALPHA );

	if (MC::getIsMessageBoxActive( )) return;
	if ( _mode == MC_ZOOM_UP )
	{
		oldFocusItem = MC::getWindowList( )->getFocusItem( );
		if (mode != MC_ZOOM_NONE)
			return;
	}
	else  
	{
		if (mode != MC_ZOOM_ZOOMED)
		{
			PostMessage( getHwnd( ), WM_USER, WMMC_HIDE, _mode );
			return;
		}
	}

	if (mode == MC_ZOOM_DOWN)
	{
		int i;
		i = 0;
	}

	McLabelW *l = McWindowMgr::getLabelW();
	l->Hide();
	l->Pause( );

	mode = _mode;

	list<McWItem *>::iterator it;
	McRect *dtR = McMonitorsMgr::getMainMonitor( )->getDesktopSize( );
	POINT p;

	p.x = -dtR->left;
	p.y = -dtR->top;

	for ( it = thumbOwners.begin(); it != thumbOwners.end(); it++ )
	{
		McWItem *item = *it;
		switch( mode )
		{
		case MC_ZOOM_UP:
		case MC_ZOOM_DOWN:
			item->setStartRect( item->getThumbRect( ) );
			item->setEndRect( item->getZoomRect( ) );
			if (MC::getMM( ))
			{
				item->getStartRect( )->offset( &p );
				item->getEndRect( )->offset( &p );
			}
			item->setCurRect( item->getStartRect( ) );
			break;

		case MC_ZOOM_AGAIN:
			item->setStartRect( item->getZoomRect( ) );
			if (isZoomingAgain == item)
			{
				item->calculateZoomedPosition( );
				item->setEndRect( item->getZoomRect( ) );
			}
			else
				item->setEndRect( item->getThumbRect( ) );
			if (MC::getMM( ))
			{
				item->getEndRect( )->offset( &p );
				item->getStartRect( )->offset( &p );
			}
			break;
		}
	}

	McRect *r = NULL;

	if ( mode == MC_ZOOM_UP )
	{

		r = MC::getMM( ) ? McMonitorsMgr::getMainMonitor( )->getDesktopSize( ) :
			McMonitorsMgr::getMainMonitor()->getMonitorSize() ;

		if ( thumbOwners.size() > 1 )
			SetWindowPos( getHwnd( ), GetNextWindow( McWindowMgr::getMainW( )->getHwnd( ), GW_HWNDPREV ),
				r->left, r->top, r->getWidth( ), r->getHeight( ), SWP_NOACTIVATE | SWP_SHOWWINDOW );
		else
			SetWindowPos( getHwnd( ), HWND_TOP,
			r->left, r->top, r->getWidth( ), r->getHeight( ), SWP_SHOWWINDOW );
		
		for ( it = thumbOwners.begin(); it != thumbOwners.end(); it++ )
		{
			McWItem *item = *it;
			HTHUMBNAIL oldT = item->getThumbnail();
			item->registerThumbnail( FALSE, WIL_ZOOM_WINDOW );
			item->showThumbnail();
			item->closeThumbWindow();
			if ( oldT ) item->unregisterThumbnail( oldT );
		}

	}
	else if ( mode == MC_ZOOM_AGAIN )
	{
		for (it = thumbOwners.begin( ); it != thumbOwners.end( ); it++)
		{
			McWItem *item = *it;
			if (item != isZoomingAgain)
			{
				HTHUMBNAIL oldT = item->getThumbnail( );
				item->registerThumbnail( FALSE, WIL_MAIN_WINDOW );
				item->showThumbnail( );
				if (oldT) item->unregisterThumbnail( oldT );
			}
		}
		SetForegroundWindow( getHwnd() );
	}

	MC::setState( MCS_Zoom );

	if (mode == MC_ZOOM_UP)
	{
		SetWindowPos( McWindowMgr::getBackingW( )->getHwnd( ), HWND_TOP,
			r->left, r->top, r->getWidth( ), r->getHeight( ), SWP_SHOWWINDOW );
		SetWindowPos( getHwnd( ), HWND_TOP,
			r->left, r->top, r->getWidth( ), r->getHeight( ), SWP_SHOWWINDOW );
	}
	else
		if (mode == MC_ZOOM_DOWN)
		{
			calcFrame( 0 );
		}

	inAnimation = TRUE;
	MC::getMC()->getAnimationMgr()->doAnimate( getHwnd(), (mode!=MC_ZOOM_DOWN)?1.0: 0.0, 1 );
}


void McZoomW::calcFrame( double value )
{
	lastValue = value;

	myAlpha = (int)((1.0-value)*_ZOOM_ALPHA_VALUE);
	for (list<McWItem*>::iterator it = thumbOwners.begin( ); it != thumbOwners.end( ); it++)
	{
		(*it)->getCurRect( )->interpolate( (*it)->getStartRect( ), (*it)->getEndRect( ), value, TRUE );
	}
	if ( ( mode == MC_ZOOM_DOWN ? 1.0-value : value ) < 0.0001 )
	{
		SetEvent(zwEvent);
	}
}

BOOL McZoomW::buildSuperZoom( int pileNumber, HMONITOR monitor )
{
	McWList *wl = MC::getWindowList();
	clearList();
	list<McWItem *> *itemList = wl->getItemList( );
	for (ITEM_IT it = itemList->begin( ); it != itemList->end( ); it++)
		if ( (*it)->getPileNumber() == pileNumber && (*it)->getVisibility() )
			addItem( *it );
	if ( thumbOwners.size() <= 1 )
		return FALSE;

	McPilesMgr *piles = new McPilesMgr( 0, monitor ); 
	piles->createZoomPiles( &thumbOwners );
	piles->layoutPiles( FALSE );
	piles->positionZooms();
	delete piles;

	return TRUE;
}

DWORD McZoomW::runThread( )
{
	while (TRUE)
	{
		DWORD hr = WaitForSingleObject( zwEvent, INFINITE );
		switch (hr)
		{
		case WAIT_OBJECT_0:
			break;
		default:
			return 0;
		}

		McWindowMgr::getLabelW( )->Resume( );

		if (mode == MC_ZOOM_UP || mode == MC_ZOOM_AGAIN)
		{

			Sleep( 25 ); // Very important to let thumbnail update finish
			if (isZoomingAgain)
			{
				isZoomingAgain->setStartRect( isZoomingAgain->getThumbRect( ) );
				clearList( );
				addItem( isZoomingAgain );
			}
			
			mode = MC_ZOOM_ZOOMED;
		}
		
		ResetEvent( zwEvent );

		if (mode == MC_ZOOM_DOWN)
		{
			if (isZoomingAgain)
			{
				clearList( );
				int pileNumber = isZoomingAgain->getPileNumber( );
				list<McWItem *> *itemList = MC::getWindowList()->getItemList( );
				for (ITEM_IT it = itemList->begin( ); it != itemList->end( ); it++)
					if ((*it)->getPileNumber( ) == pileNumber )
						addItem( *it );
				isZoomingAgain = NULL;
			}
			PostMessage( getHwnd( ), WM_USER, WMMC_HIDE, NULL );
			mode = MC_ZOOM_NONE;
		}

		inAnimation = FALSE;
		MC::setState( MCS_Display );

	}
	return 0;
}
