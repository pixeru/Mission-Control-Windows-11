#include "stdafx.h"
#include "McDragMgr.h"
#include "McThumbW.h"
#include "McWList.h"
#include "McMonitorsMgr.h"
#include "McMainW.h"
#include "McWindowMgr.h"

// McDragMgr is an entirely static class used to manage dragging windows around on multiple
// monitors.

// Static declared variables

MC_DRAG_STATE McDragMgr::state = DragStateIdle;
POINT McDragMgr::startingPoint;
POINT McDragMgr::lastPoint;
HWND McDragMgr::dragHwnd;
McRect McDragMgr::startingThumb;
McRect McDragMgr::currentThumb;
McWItem *McDragMgr::dragItem;

BOOL McDragMgr::isDragging( )
{
	return state == DragStateDragging;
}

BOOL McDragMgr::isIdle( )
{
	return state == DragStateIdle;
}

BOOL McDragMgr::processEvent( HWND posHwnd, MC_DRAG_EVENT eventType, long x, long y, McWItem *item )
{
	return FALSE; // Disabled dragging permanently because it interferes with clicking

	if ( eventType == DragButtonDown )
	{
		dragItem = item;
		dragHwnd = item->getThumbHwnd( );
	}

	RECT or;
	McWindowMgr::myGetWindowRect( posHwnd, &or );
	x += or.left;
	y += or.top;

	switch (eventType)
	{
	case DragButtonDown:
		return onButtonDown( x, y );
	case DragMove:
		return onDrag( x, y );
	case DragButtonUp:
		return onButtonUp( x, y );
	default:
		return FALSE;
	}
}

BOOL McDragMgr::onButtonDown( long x, long y )
{
	state = DragStatePressed;
	startingThumb.set( dragItem->getThumbRect( ) );
	startingPoint.x = lastPoint.x = x;
	startingPoint.y = lastPoint.y = y;
	return TRUE;
}

BOOL McDragMgr::onDrag( long x, long y )
{
	if (state == DragStateIdle)
		return FALSE;

	if ( !dragItem->getIsMoveable() )
		return FALSE;

	maybeRedraw( x, y );

	return TRUE;
}

BOOL McDragMgr::onButtonUp( long x, long y )
{
	if (!isDragging( ))
	{
		state = DragStateIdle;
		return FALSE;
	}

	maybeRedraw( x, y );

	state = DragStateIdle;

	HMONITOR originalMonitor = dragItem->getAppMonitor( );
	HMONITOR currentMonitor = MonitorFromWindow( dragHwnd, MONITOR_DEFAULTTONEAREST );
	
	if (currentMonitor == originalMonitor)
		restoreWindow( );
	else
		moveWindow( originalMonitor, currentMonitor );

	return TRUE;
}

void McDragMgr::restoreWindow( )
{
	dragItem->getThumbRect( )->set( &currentThumb );
	PostMessage( McWindowMgr::getMainW( )->getHwnd( ), WM_USER, WMMC_REINIT, NULL );
}

BOOL McDragMgr::fitToWindow( McRect *wRect, McRect *mRect, 
	POINT *newLocation, SIZE *newSize, BOOL *newIsMax )
{
	// Determines if wRect fits in the monitor rect mRect.
	// if not, finds adjusted size and returns newLocation and newSize
	// if newLocation and newSize fill the monitor that newIsMax is set to TRUE
	// otherwise newIsMax is set to FALSE;

	// returns TRUE if adjustments were needed

	McRect windowRect( wRect );
	BOOL result = !mRect->encloses( wRect );

	if ( result )
	{
		// overlaps in some way ...
		// first move window up and left to be on screen
		int xoffset = min( 0, mRect->right - windowRect.right );
		int yoffset = min( 0, mRect->bottom - windowRect.bottom );
		windowRect.offset( xoffset, yoffset );

		// now move it down and right to be on screen
		xoffset = max( 0, mRect->left - windowRect.left );
		yoffset = max( 0, mRect->top - windowRect.top );
		windowRect.offset( xoffset, yoffset );

		// now adjust bottom and right so it fits
		if (windowRect.right > mRect->right) windowRect.right = mRect->right;
		if (windowRect.bottom > mRect->bottom) windowRect.bottom = mRect->bottom;

		newLocation->x = windowRect.left;
		newLocation->y = windowRect.top;
		newSize->cx = windowRect.getWidth( );
		newSize->cy = windowRect.getHeight( );
	}

	*newIsMax = windowRect.equals( mRect );
	return result;
}

void McDragMgr::addHiddenEdge( McRect *wRect, SIZE *borders, BOOL isMax )
{
	wRect->left -= borders->cx;
	wRect->right += borders->cx;
	if ( isMax )
		wRect->top -= borders->cy;
	wRect->bottom += borders->cy;
}

void McDragMgr::moveWindow( HMONITOR originalMonitor, HMONITOR newMonitor )
{
	McMonitorsMgr *nmon = McMonitorsMgr::getMonitor( newMonitor );
	McMonitorsMgr *omon = McMonitorsMgr::getMonitor( originalMonitor );

	McRect paddedRect( dragItem->getPaddedRect( ) );			// Original App Window
	McRect normalRect( dragItem->getNormalRect( ) );			// Original Used Window

	long mxoffset = nmon->getMonitorSize( )->left - omon->getMonitorSize( )->left;	// Monitor corner offset
	long myoffset = nmon->getMonitorSize( )->top - omon->getMonitorSize( )->top;	// Monitor y offset

	SIZE *borders = dragItem->getBorders( );

	POINT newLocation;
	SIZE newSize;
	BOOL newIsMax;

	WINDOWPLACEMENT wp;
	McWindowMgr::myGetWindowPlacement( dragItem->getAppHwnd( ), &wp );

	McRect *rcNormalPosition = (McRect*) &wp.rcNormalPosition;

	WINDOWINFO wi;
	McWindowMgr::myGetWindowInfo( dragItem->getAppHwnd( ), &wi );

	McRect *rcWindow = (McRect*) &wi.rcWindow;

	// Offset the positions in dragItem

	paddedRect.offset( mxoffset, myoffset );
	normalRect.offset( mxoffset, myoffset );

	// WW need to make sure it fits on the new screen here and adjust padded and normal

	BOOL changed = fitToWindow(
		&normalRect, nmon->getMonitorSize( ),
		&newLocation, &newSize, &newIsMax );

	if (changed)
	{
		normalRect.set( 0, 0, newSize.cx, newSize.cy );
		normalRect.offset( newLocation.x, newLocation.y );
	}

	if (newIsMax && dragItem->getIsW10App( ) && !dragItem->getIsMaximized( ))
	{
		newIsMax = FALSE;
		normalRect.left += 10;
		normalRect.top += 20;
		normalRect.right -= 20;
		normalRect.bottom -= 20;
	}
	else
		newIsMax |= dragItem->getIsMaximized( );

	if (newIsMax)
	{
		if (dragItem->getIsMinimized( ))
			wp.flags |= WPF_RESTORETOMAXIMIZED;
		normalRect.set( nmon->getMonitorSize( ) );
	}

	paddedRect.set( &normalRect );
	addHiddenEdge( &paddedRect, borders, FALSE );

	// Change the normal position in wp

	rcNormalPosition->set( &paddedRect );

	paddedRect.set( &normalRect );
	addHiddenEdge( &paddedRect, borders, newIsMax );

	// If not minimized, move the window, if minimized refresh the thumbnail

	BOOL moveSucceeded = McWindowMgr::mySetWindowPlacement( dragItem->getAppHwnd( ), &wp );
	Sleep( 100 );

	if (moveSucceeded && !dragItem->getIsMinimized( ))
	{
		moveSucceeded = 0 != SetWindowPos( dragItem->getAppHwnd( ),
			NULL,
			paddedRect.left, paddedRect.top,
			paddedRect.getWidth( ), paddedRect.getHeight( ),
			SWP_NOACTIVATE | SWP_NOZORDER );
	}

	if (moveSucceeded)
	{

		if (dragItem->getIsMinimized( ))
		{
			PostMessage( dragItem->getAppHwnd( ), WM_SYSCOMMAND, SC_RESTORE, NULL );
			Sleep( 100 );;
			PostMessage( dragItem->getAppHwnd( ), WM_SYSCOMMAND, SC_MINIMIZE, NULL );
			Sleep( 100 );
		}

		HMONITOR xmon = MonitorFromWindow( dragHwnd, MONITOR_DEFAULTTONEAREST );
		if (xmon == newMonitor)
		{
			dragItem->setAppMonitor( newMonitor );
			dragItem->setPaddedRect( &normalRect );
			dragItem->setNormalRect( &normalRect );
			dragItem->getContentRect( )->set( 0, 0, normalRect.getWidth( ), normalRect.getHeight( ) );
			dragItem->getContentRect( )->offset( borders->cx, newIsMax ? borders->cy : 0 );
			dragItem->setIsMaximized( newIsMax );
		}
	}

	dragItem->getThumbRect( )->set( &currentThumb );

	if (!moveSucceeded)
		MessageBeep( MB_ICONINFORMATION );

	PostMessage( McWindowMgr::getMainW( )->getHwnd( ), WM_USER, WMMC_REINIT, NULL );

}

void McDragMgr::maybeRedraw( long x, long y )
{
	// Redraw the window during drag if it has moved far enough to matter.
	long xdelta = lastPoint.x - x;
	long ydelta = lastPoint.y - y;

	if (MC_DRAG_MINPIX <= sqrt( xdelta*xdelta + ydelta*ydelta ))
	{
		lastPoint.x = x;
		lastPoint.y = y;
		currentThumb.set( &startingThumb );
		currentThumb.offset( lastPoint.x - startingPoint.x, lastPoint.y - startingPoint.y );
		SetWindowPos( dragHwnd, HWND_TOP,
			currentThumb.left, currentThumb.top, currentThumb.getWidth( ), currentThumb.getHeight( ), SWP_SHOWWINDOW );
		state = DragStateDragging;
	}
}
