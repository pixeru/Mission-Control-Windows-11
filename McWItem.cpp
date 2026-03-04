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
#include <gdiplus.h>
#include "Emcee.h"
#include "McModernAppMgr.h"
#include "McWList.h"
#include "McWItem.h"
#include "McBgW.h"
#include "McMainW.h"
#include "McThumbW.h"
#include "McDesktopW.h"
#include "McZoomW.h"
#include "McMonitorsMgr.h"
#include "McIconMgr.h"
#include "McWindowMgr.h"
#include "McPilesMgr.h"

// Object containing information about a single running window -- desktop app, windows store app or desktop.

#pragma unmanaged
McWItem::McWItem(
	HWND	_appHwnd,
	HMONITOR _appMonitor,
	DWORD	_appPid,
	DWORD	_appTid,
	BOOL	_isDesktop,
	BOOL	_isCloaked,
	BOOL	_isMinimized,
	BOOL	_isMaximized,
	BOOL	_isW10App,
	BOOL	_isEdgeSnapped,
	char *	_className,
	char*	_processName,
	SIZE*	_borders,
	RECT*	_paddedRect,
	RECT*	_normalRect,
	RECT*	_contentRect,
	HWND	_iconShell
	)
{
	useAlpha = FALSE;
	alpha	 = 255;

	isNew = FALSE;
	isWindowObscured = FALSE;

	animateLocation = WIL_NO_WINDOW;
	animateRect.set( 0, 0, 0, 0 );

	appHwnd = _appHwnd;
	appMonitor = _appMonitor;
	appPid = _appPid;
	appTid = _appTid;

	isDesktop = _isDesktop;
	isCloaked = _isCloaked;
	isMinimized = _isMinimized;
	isMaximized = _isMaximized;
	isW10App = FALSE;
	isEdgeSnapped = _isEdgeSnapped;
	isW10App = _isW10App;
	className = new string( _className );
	processName = new string( _processName );

	modernHwnd = 0;
	setRotation( );

	setBorders( _borders );

	paddedRect.set( _paddedRect );

	normalRect.set( _normalRect );
	currentRect.set( _normalRect );
	contentRect.set( _contentRect );

	if (isDesktop)
		iconShell = _iconShell;
	else
		iconShell = NULL;

	isVisible = TRUE;
	wasVisible = TRUE;
	becomesInvisible = FALSE;

	onMainMonitor = FALSE;
	McMonitorsMgr *monitor = McMonitorsMgr::getMonitor( appMonitor );
	if ( monitor )
		if (monitor->getIsMainMonitor( ))
			onMainMonitor = TRUE;

	// Calculate Z-Order for this window

	if (isDesktop)
		zOrder = 100000;
	else
		zOrder = (isCloaked?10000:(isMinimized)?5000:0) + McWindowMgr::getZorder( appHwnd );

	pileNumber = 0;

	// Initialize local variables

	onAppBar = FALSE;

	if (MC::getProperties()->getUseAppBar( ) && !MC::inTabletMode() )
			if (isW10App && isMaximized )
				onAppBar = TRUE;

	thumbnail = tempThumbnail = NULL;
	iconThumb = NULL;
	thumbLocation = WIL_NO_WINDOW;
	thumbWindow = NULL;
	desktopWindow = NULL;
	thumbHwnd = 0;

	haveFocus = FALSE;

	iconBitmap = NULL;
	iconColor = NULL;
	hicon = NULL;
	if (getIsAPackage( ))
	{
		// do find by display name
		McIconMgr *mgr = MC::getIconMgr( );
		WCHAR wbuff[1000];
		if (strstr( getProgName( ), "Weather for Life" ))
			MC::char2wchar( "AccuWeather", wbuff );
		else
			MC::char2wchar( (char *) getProgName( ), wbuff );

		if (!mgr->getAppLogo( wbuff, &iconBitmap, &iconColor ))
			MC::loadBitmap( IDI_MODERN, &iconBitmap );
	}
	else
		hicon = McIconMgr::GetWindowIcon( getAppHwnd( ) );
}

McWItem::~McWItem()
{
	if ( thumbnail ) unregisterThumbnail();
	disposeThumbWindow();
	disposeDesktopWindow( );
	if (thumbWindow) McWindowMgr::freeThumbW( thumbWindow );
	delete className;
	delete processName;
}

HRESULT McWItem::registerThumbnail( wiThumbLocation _thumbLocation )
{
	return  registerThumbnail( TRUE, _thumbLocation, FALSE );
}

HRESULT McWItem::registerThumbnail( wiThumbLocation _thumbLocation, BOOL _useAlpha )
{
	return registerThumbnail( TRUE, _thumbLocation, _useAlpha );
}

HRESULT McWItem::registerThumbnail( BOOL _unreg, wiThumbLocation _thumbLocation )
{
	return registerThumbnail( _unreg, _thumbLocation, FALSE );
}

HRESULT McWItem::registerThumbnail( BOOL _unreg, wiThumbLocation _thumbLocation, BOOL _useAlpha )
{
	useAlpha = _useAlpha; 

	if (_unreg )
	{
		if (thumbnail != NULL)
		{
			DwmUnregisterThumbnail( thumbnail );
			thumbnail = NULL;
		}
		if (iconThumb)
		{
			DwmUnregisterThumbnail( iconThumb );
			thumbnail = NULL;
		}

	}

	if ( !thumbWindow && !isDesktop )
		return GENERIC_HRESULT_FAILURE;

	HWND _displayHwnd;

	switch ( _thumbLocation )
	{
	case WIL_MAIN_WINDOW:

		if ( needsTransparency() )
			_displayHwnd = McWindowMgr::getFadingW( )->getHwnd( );
		else
			_displayHwnd = McWindowMgr::getMainW( )->getHwnd( );

		break;

	case WIL_ZOOM_WINDOW:

		_displayHwnd = McWindowMgr::getZoomW()->getHwnd();
		break;
		
	case WIL_THUMB_WINDOW:
		_displayHwnd = thumbWindow->getHwnd( );
		break;

	case WIL_BG_WINDOW:

		_displayHwnd = McWindowMgr::getBgW( )->getHwnd( );
		break;

	case WIL_DT_WINDOW:
		_displayHwnd = desktopWindow->getHwnd( );
		break;

	default:

		return GENERIC_HRESULT_FAILURE;
	}
		
	thumbLocation = _thumbLocation;

	if (isVisible)
	{
		BOOLEAN attribute = TRUE;
		HRESULT hr = S_OK;
		hr = DwmRegisterThumbnail( _displayHwnd, appHwnd, &thumbnail );
		if (iconShell)
			hr = DwmRegisterThumbnail( _displayHwnd, iconShell, &iconThumb );
		if (hr != S_OK)
			if (!getIsOnAppBar( ))
				isVisible = FALSE;

		return hr;
	}

	return S_OK;
}

void McWItem::unregisterThumbnail( HTHUMBNAIL _thumbnail )
{
	if ( _thumbnail )
		DwmUnregisterThumbnail( _thumbnail );
	else
	{
		if (thumbnail)
		{
			DwmUnregisterThumbnail( thumbnail );
			thumbnail = NULL;
			thumbLocation = WIL_NO_WINDOW;
		}
		if (iconThumb)
		{
			DwmUnregisterThumbnail( iconThumb );
			iconThumb = NULL;
		}
	}
}

HRESULT McWItem::showThumbnail()
{
	if (!thumbnail)
		return GENERIC_HRESULT_FAILURE;

	if ( !isVisible )
		return S_OK;

	DWM_THUMBNAIL_PROPERTIES props = { 0 };
	props.dwFlags = /*DWM_TNP_VISIBLE |*/ DWM_TNP_RECTDESTINATION | DWM_TNP_RECTSOURCE;
	props.fVisible = TRUE;

	BOOL leftHanded = MC::getWindowList()->getLeftHanded();
	McWItem **dItems = MC::getWindowList( )->getDesktopItem( );

	McRect rect( &currentRect );
	BOOL canAbort = TRUE;
	switch( thumbLocation )
	{
	case WIL_ZOOM_WINDOW:
		break;
	case WIL_MAIN_WINDOW:
		rect.offset( McMonitorsMgr::getMainMonitor( )->getOffsets( ) );
		break;
	case WIL_BG_WINDOW:
		rect.offset( McMonitorsMgr::getMainMonitor( )->getOffsets( ) );
		rect.offset( 1, 1 );
		break;
	case WIL_DT_WINDOW:
		canAbort = FALSE;
	case WIL_THUMB_WINDOW:
		rect.set( &thumbRect );
		rect.offset( -rect.left, -rect.top );
		break;

	default:
		return GENERIC_HRESULT_FAILURE;
	}

	if (rect.getWidth( ) < 0) rect.right = rect.left + 1;
	if (rect.getHeight( ) < 0) rect.bottom = rect.top + 1;

	if (animateLocation == thumbLocation && animateRect.equals( &rect ) && canAbort)  // Abort if this is a duplicate
		return S_OK;

	animateLocation = thumbLocation;
	animateRect.set( &rect );

	props.rcDestination = rect;
	props.rcSource = contentRect;

	if (thumbLocation == WIL_MAIN_WINDOW && getIsOnAppBar( ) && getIsMinimized( ) )
	{
		McRect tmpContent( contentRect );
		double tmpAspect;

		if (rect.getHeight( ) > 0)
		{
			tmpAspect = rect.getHeight( ) / (DOUBLE) rect.getWidth( );
			tmpContent.bottom = tmpContent.top + (LONG) floor( .5 + tmpAspect * tmpContent.getWidth( ) );
			props.rcSource = tmpContent;
			rect.offset( 1, 1 );
			props.rcDestination = rect;
		}
	}
	else 
		if (isMinimized && rect.getHeight()>0 && contentRect.getHeight()>0 )
		{
			McRect tmpContent( contentRect );

			double sAspect = rect.getWidth( ) / ((DOUBLE) rect.getHeight( ));
			double cAspect = tmpContent.getWidth( ) / ((DOUBLE) tmpContent.getHeight( ));

			if (cAspect > sAspect) 
				tmpContent.right = tmpContent.left + (int)floor( 0.5 + sAspect*tmpContent.getHeight( ) );
			else
				tmpContent.bottom = tmpContent.top + (int)floor( 0.5 + tmpContent.getWidth( ) / sAspect );

			sAspect = rect.getWidth( ) / ((DOUBLE) rect.getHeight( ));
			cAspect = tmpContent.getWidth( ) / ((DOUBLE) tmpContent.getHeight( ));


			props.rcSource = tmpContent;
		}

	HWND i = this->appHwnd;
	HRESULT hr = DwmUpdateThumbnailProperties( thumbnail, &props );
	if (hr != S_OK)
	{
		if (!isDesktop) MC::simulateCloseWindow( getAppHwnd( ) );
	}
	
	if (iconThumb)
		hr = DwmUpdateThumbnailProperties(iconThumb, &props);
	
	return hr;
}

void McWItem::createThumbWindow()
{
	if ( thumbWindow || isDesktop ) return;
	thumbWindow = McWindowMgr::allocateThumbW( this );
	thumbHwnd = thumbWindow->getHwnd( );
}

void McWItem::disposeThumbWindow( )
{
	if (thumbWindow && !isDesktop)
	{
		thumbWindow->Deactivate( );
		thumbWindow = NULL;
		thumbHwnd = 0;
	}
}

void McWItem::createDesktopWindow( )
{
	if (!isDesktop) return;
	if (!desktopWindow)
		desktopWindow = McWindowMgr::allocateDesktopW( this, &thumbRect );
	desktopWindow->setPosition( FALSE, &thumbRect );
}

void McWItem::disposeDesktopWindow( )
{
	if (isDesktop && desktopWindow)
	{
		desktopWindow->Deactivate( );
		desktopWindow = NULL;
	}
}

void McWItem::closeDesktopWindow( )
{
	if (desktopWindow && isDesktop) 
		ShowWindow( desktopWindow->getHwnd( ), SW_HIDE );
}

void McWItem::closeThumbWindow()
{
	if ( thumbWindow && !isDesktop ) 
		ShowWindow( thumbWindow->getHwnd(), SW_HIDE );
}

void McWItem::showDesktopWindow( )
{
	if (desktopWindow)
		desktopWindow->setPosition( TRUE );
}

void McWItem::showThumbWindow()
{
	if ( !isDesktop ) 
		showThumbWindow( HWND_TOP);
}

void McWItem::showThumbWindow( HWND hwnd )
{
	int offs = 0;
	if ( !getIsOnAppBar() )
		offs = 1;
	if (thumbWindow && (!isDesktop) && isVisible)
		SetWindowPos( thumbWindow->getHwnd( ), hwnd,
			thumbRect.left-offs, thumbRect.top-offs, 
			
			thumbRect.getWidth()+offs, thumbRect.getHeight()+offs,
			(hwnd == HWND_TOP ? SWP_SHOWWINDOW : SWP_NOACTIVATE | SWP_SHOWWINDOW ) );
}

void McWItem::placeInPile(BOOL reorder)
{
	if ( isDesktop ) return;

	McWList *windowList = MC::getWindowList();
	McWItem *item = this;
	while ( TRUE )
	{
		item = windowList->getItemAbove/*getPileItemAbove*/( item );// was getPileItem
		if ( !item ) break;
		if ( item->isVisible || getIsOnAppBar( ) ) break;
	}
	if ( item != NULL )
	{
		if ( reorder ) windowList->insertItemBefore( this, item );
		showThumbWindow( item->getThumbHwnd() );
	}
	else
	{
		if ( reorder ) windowList->insertItemOnTop( this );
		showThumbWindow();
	}
}

void McWItem::loseFocus()
{
	if ( haveFocus )
		placeInPile( TRUE );
	haveFocus = FALSE;
}

void McWItem::pushToBottom( )
{
	if (isDesktop) return;

	if (getIsOnAppBar( ))
		McWindowMgr::getMainW( )->clearActiveApp( this );

	if (MC::inTabletMode( ) && !isCloaked)
	{
		McWItem *newItem = NULL;
		list<McWItem *> *itemList = MC::getWindowList( )->getItemList( );
		for (list<McWItem *>::reverse_iterator rit = itemList->rbegin( ); rit != itemList->rend( ); rit++)
		{
			McWItem* rit_item = *rit;
			if (rit_item != this)
			{
				newItem = rit_item;
				MC::getWindowList( )->changeFocus( newItem );
				break;
			}
		}

		if (newItem)
		{
			newItem->getThumbWindow( )->onSelectWindow( FALSE, FALSE );
			BOOL t = TRUE;
			DwmSetWindowAttribute( getAppHwnd( ), DWMWA_CLOAK, &t, sizeof( t ) );
			setIsCloaked( TRUE );
		}
	}
	else
	{
		SetWindowPos( getAppHwnd( ), HWND_BOTTOM, paddedRect.left, paddedRect.top, paddedRect.getWidth( ), paddedRect.getHeight( ),
			SWP_NOACTIVATE );
		if (isMinimized)
			PostMessage( getAppHwnd( ), WM_SYSCOMMAND, SC_MINIMIZE, NULL );
	}

	BOOL bottomOnly = (isCloaked || isMinimized);
	McWItem *bottomItem = NULL;
	list<McWItem *> *itemList = MC::getWindowList( )->getItemList( );
	for (list<McWItem *>::iterator it = itemList->begin( ); it != itemList->end( ); it++)
	{
		McWItem *item = *it;
		BOOL iBottomOnly = (item->isCloaked || item->isMinimized);
		if (bottomOnly == iBottomOnly)
		{
			bottomItem = item;
			break;
		}
	}
	if (!bottomItem)
		return;
	zOrder = bottomItem->getZorder( ) + 1;
	placeInPile( TRUE );
}

void McWItem::takeFocus()
{
	if ( !haveFocus )
	{
		showThumbWindow();
		haveFocus = TRUE;
	}
}

void McWItem::raiseApp( )
{
	if (isMaximized)
		MC::doShowWindow( getAppHwnd( ), SW_MAXIMIZE, isW10App );
	else
		MC::doShowWindow( getAppHwnd( ), SW_SHOW, isW10App );
	LockSetForegroundWindow( LSFW_UNLOCK );
	SetForegroundWindow( getAppHwnd( ) );
	SetFocus( getAppHwnd( ) );
	SetForegroundWindow( getAppHwnd( ) );
	SetFocus( getAppHwnd( ) );
}

void McWItem::raiseToTop(BOOL dealWithCloaking, BOOL doZOrder)
{
	McWList *wi = MC::getWindowList();

	if ( isMinimized )
	{
		if (dealWithCloaking)
			isCloaked = FALSE;

		PostMessage( appHwnd, WM_SYSCOMMAND, SC_RESTORE, NULL );
		
		setIsMinimized( FALSE );
	}
	McWItem *topItem = *(wi->getItemList()->rbegin());
	if ( this != topItem )
	{
		zOrder = topItem->getZorder()-1;
		placeInPile(TRUE);
	}
}

void McWItem::setAlpha( double _alpha )
{
	alpha = (BYTE) (_alpha*255);
}

void McWItem::adjustVisibility()
{
	if ( this->becomesInvisible )
		isVisible = becomesInvisible = FALSE;
}

void McWItem::setVisibility( BOOL _isVisible, BOOL _becomesInvisible )
{
	becomesInvisible = _becomesInvisible;

	useAlpha = !getVisibility();
	wasVisible = isVisible;
	isVisible = _isVisible;
}

void McWItem::setImmersiveAppAlpha( double value )
{
	if ( !getIsW10App() ) return;
	if ( getVisibility() && !wasVisible )
		setAlpha( value );
	else if ( wasVisible && !getVisibility() )
		setAlpha( 1.0-value );
	else
		setAlpha( 1.0 );
}

void McWItem::calculateZoomedPosition( )
{
	McMonitorsMgr *m =
		getIsOnAppBar( ) || !MC::getMM( ) ?
		McMonitorsMgr::getMainMonitor( ) :
		McMonitorsMgr::getMonitor( getAppMonitor( ) );
	McRect *mRect = m->getMonitorSize( );
	McRect normal( getNormalRect( ) );

	if ( normal.getWidth() > mRect->getWidth() )
		normal.scale( ((double)mRect->getWidth())/normal.getWidth() );
	if ( normal.getHeight() > mRect->getHeight() )
		normal.scale( ((double)mRect->getHeight())/normal.getHeight() );

	McRect zoomRect;

	zoomRect.set(
		mRect->left + (mRect->getWidth( ) - normal.getWidth( )) / 2,
		mRect->top + (mRect->getHeight( ) - normal.getHeight( )) / 2,
		normal.getWidth( ),
		normal.getHeight( ) );
	zoomRect.right += zoomRect.left;
	zoomRect.bottom += zoomRect.top;

	setZoomRect( &zoomRect );
}

void McWItem::DrawIcon( HDC hdc, int x, int y, int width, int height )
{
	Graphics g( hdc );
	if (hicon)
		McIconMgr::DrawFromIcon( &g, hicon, x, y, width );
	else
		if (iconBitmap)
			McIconMgr::DrawFromBitmap( &g, iconColor, iconBitmap, x, y, width );
}
