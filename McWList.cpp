#include "stdafx.h"

#include "Emcee.h"
#include "McWList.h"
#include "McWItem.h"
#include "McBgW.h"
#include "McMainW.h"
#include "McThumbW.h"
#include "McZoomW.h"
#include "McLabelW.h"
#include "McPropsMgr.h"
#include "McPilesMgr.h"
#include "McAnimationMgr.h"
#include "McMonitorsMgr.h"
#include "McIconMgr.h"
#include "McWindowMgr.h"
#include "McModernAppMgr.h"

#pragma unmanaged
// This class builds a list of application windows and creates a McWItem object
// for each one.   That class associates the application window with it's thumbnail
// display and it's thumbnail window.

McWList::McWList( )
{
	McMonitorsMgr *mainMonitor = McMonitorsMgr::getMainMonitor( );
	McRect mdT( mainMonitor->getMonitorSize( ) );
	mdT.offset( mainMonitor->getOffsets( ) );

	desktopItem = new McWItem*[McMonitorsMgr::getEffectiveMonitorCount( )];
	int i = 0;
	McMonitorsMgr *monitor1 = NULL;

	// Enumerate windows to  find the one which will yield a thumbnail image
	// of the desktop icons.  There is only one spanning the entire (possibly
	// multi-monitor desktop.

	HWND iconShell = NULL;
	EnumChildWindows( GetDesktopWindow( ), myDtIconSearchCallback, (LPARAM) &iconShell );

	for (list<McMonitorsMgr *>::iterator mi = McMonitorsMgr::monitorList.begin( ); mi != McMonitorsMgr::monitorList.end( ); mi++)
	{
		McMonitorsMgr *monitor = *mi;
		if (MC::getMM( ) || monitor->getIsMainMonitor( ))
		{
			McRect cRect( monitor->getMonitorSize( ) );
			cRect.offset( mainMonitor->getOffsets( ) ); 

			McRect r( monitor->getMonitorSize( ) );
			HWND shell = FindWindow( L"Progman", NULL );
			desktopItem[i] =
				new McWItem(
				shell, monitor->getHMONITOR( ), 0, 0, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, "_DTclass", "_DTprocess",
				NULL, monitor->getMonitorSize( ), &r, &cRect , iconShell  );
			i++;
		}
	}

	focusItem = NULL;

	// Knowing minWin allows us to rule out "invisible" convenience windows.

	minWin = GetSystemMetrics( SM_CYMIN );

	// Need border information for handling maximaized applications with frames (desktop)

	borderSize.x =GetSystemMetrics( SM_CXFIXEDFRAME );
	borderSize.y = GetSystemMetrics( SM_CYFIXEDFRAME );

	// Have at it.

	buildWindowList();
}

McWList::~McWList()
{
	if (desktopItem)
	{
		for (int i = 0; i < McMonitorsMgr::getEffectiveMonitorCount( ); i++ )
			delete desktopItem[i];
		delete desktopItem;
	}
	for ( ITEM_IT it=itemList.begin();it!=itemList.end();it++ )
		delete *(it);
}


void McWList::positionBackground( McWItem* dtItem, int _immersiveAppOffset, int _immersiveAppGap )
{
	McMonitorsMgr *monitor = McMonitorsMgr::getMonitor( dtItem->getAppMonitor( ) );

	float dpi = monitor->getDpi( );

	long hOffset;
	long vOffset;
	long blank = 0;
	if (monitor->getIsMainMonitor( ))
	{
		immersiveAppGap = _immersiveAppGap;
		immersiveAppOffset = _immersiveAppOffset;
		int	 barHeight = (int)(floor( .5 + dpi * APPBAR_THUMBHEIGHT_INCHES ) + 2 * floor( .5 + dpi * APPBAR_THUMB_V_SPACING_INCHES ));
		int forty = (int) (_DEFAULT_ICON_SIZE * McMonitorsMgr::getMainMonitor( )->getObjectScale( ));
		blank = (int) floor( _MC_BAR_GAP_PERCENT*barHeight );
		if (barHeight - blank < 10 + forty)
			blank = max( 0, barHeight - forty - 10 );
	}
	else
	{
		_immersiveAppGap = _immersiveAppOffset = 0;
	}

	McRect nR( dtItem->getNormalRect() );

	long fifty = (LONG) (_DEFAULT_ICON_PADDING * monitor->getObjectScale( ));

	if ( _immersiveAppOffset == 0 || !monitor->getIsMainMonitor())
	{

		fifty += (LONG) (10. * monitor->getObjectScale( ));
		vOffset = fifty;
		hOffset = (LONG) (vOffset * monitor->getAspect( ));

		if (hOffset > fifty )
		{
			hOffset = fifty;
			vOffset = (LONG) (hOffset / monitor->getAspect( ));
		}

		nR.top += vOffset;
		nR.bottom -= vOffset;
		nR.left += hOffset;
		nR.right -= hOffset;

	}
	else
	{
		long maxWidth = nR.getWidth( );
		long maxHeight = nR.getHeight( ) - _immersiveAppOffset + blank;

		long tHeight = maxHeight - 2 * (LONG) (dpi*DEFAULT_V_APPBAR_BG_PADDING_INCHES);
		long tWidth = (LONG) (tHeight * monitor->getAspect( ));

		long vPad = (maxHeight - tHeight) / 2;
		long hPad = (maxWidth - tWidth) / 2;

		BOOL tooTall = vPad < 2*fifty; 
		BOOL tooWide = hPad < 2*fifty;

		if ( tooTall && tooWide )
		{
			tHeight = maxHeight - fifty;
			tWidth = (LONG)(tHeight * monitor->getAspect( ));
		}

		vOffset = (maxHeight - tHeight) / 2;
		hOffset = (maxWidth - tWidth) / 2;

		nR.top += vOffset;
		nR.bottom -= (LONG) (vOffset + _immersiveAppOffset) - blank;
		nR.left += hOffset;
		nR.right -= hOffset;

	}

	nR.left++;
	nR.top++;

	dtItem->setThumbRect( &nR );
	int tPadding = (int)(monitor->getDpi( ) * 0.4);
	monitor->setMaxPilesDimension( nR.getWidth( )-tPadding, 
		(int)(nR.getHeight( )-tPadding*nR.getHeight()/(double)nR.getWidth() ));
}

void McWList::layoutDesktop( McState mode )
{
	int i;

	switch( mode )
	{
	case MCS_TransitionUp:
		for (i = 0; i < McMonitorsMgr::getEffectiveMonitorCount( ); i++)
		{
			desktopItem[i]->setEndRect( desktopItem[i]->getNormalRect( ) );
			desktopItem[i]->setStartRect( desktopItem[i]->getThumbRect( ) );
		}
		break;
	case MCS_TransitionDown:
		for (i = 0; i < McMonitorsMgr::getEffectiveMonitorCount( ); i++)
		{
			desktopItem[i]->setEndRect( desktopItem[i]->getThumbRect( ) );
			desktopItem[i]->setStartRect( desktopItem[i]->getNormalRect( ) );
		}
		break;
	case MCS_TransitionOver:
	case MCS_TransitionImmersiveApp:
	case MCS_TransitionDesk:
		for (i = 0; i < McMonitorsMgr::getEffectiveMonitorCount( ); i++)
			desktopItem[i]->setEndRect( desktopItem[i]->getThumbRect( ) );

		break;
	default:
		break;
	}
}

void McWList::createThumbWindows()
{
	for ( ITEM_IT it=itemList.begin();it!=itemList.end();it++ )
		(*it)->createThumbWindow();
}

void McWList::buildWindowList( )
{
	// Enumerate Windows Store Apps
	
	McModernAppMgr::EnumAllApps( myImmersiveAppWindowEnumCallback, (LPARAM)this );

	// Then Enumerate desktop windows

	EnumWindows( myWindowEnumCallback, (LPARAM)this );
}

#define WS_EX_MAGICAL (0x20000000|WS_EX_APPWINDOW)

void showRect( RECT *rect, char *text )
{
	McRect r( rect );
}

McWItem *McWList::verifyWindow( HWND hwnd, BOOL _isImmersive )
{
	char className[1000];
	char progName[1000];

	BOOL _isSnapped = FALSE;

	if (!IsWindowVisible( hwnd ))
		return NULL;

	// Get Process ID & Thread ID -- Thread ID not really needed but handy to know
	// for debugging purposes.

	DWORD threadId, processId;
	threadId = GetWindowThreadProcessId( hwnd, &processId );

	if (processId == MC::getProcessId( ))
		return NULL;

	// Get Style Info

	WINDOWINFO info = {};
	McWindowMgr::myGetWindowInfo( hwnd, &info );
	DWORD style = info.dwStyle;
	DWORD xstyle = info.dwExStyle;

	BOOL iconic = (style & WS_ICONIC) == WS_ICONIC;
	BOOL maximized = (style & WS_MAXIMIZE) == WS_MAXIMIZE;
	BOOL isW10App = FALSE;

	// First, determine if the window is cloaked ...
	DWORD cloaked = 0;

	DwmGetWindowAttribute( hwnd, DWMWA_CLOAKED, &cloaked, sizeof( DWORD ) );

#if MC_DESKTOPS
	BOOL same = McModernAppMgr::IsWindowOnCurrentDesktop( hwnd );

	// ... then, ignore cloaked windows on the current desktop, and
	// windows on other desktops if show all desktops parameter is unchecked

	if ((same && cloaked && !MC::inTabletMode( )) || (!same && !MC::getProperties( )->getShowAllDesktops( )))
		return NULL;
#endif

	HWND modernHwnd = 0;
	if (_isImmersive)
	{
		if (!McModernAppMgr::GetAppInfo( hwnd, processId, className, progName, &isW10App, 1000 ))
			return NULL;

		if (isW10App )
		{
			if (progName[0] == 0)
				return NULL;

			DWORD opid = processId;
			modernHwnd = McModernAppMgr::GetUnderlyingModernWindow( hwnd );

			if (!modernHwnd && !iconic)
			{
				if (!strcmp( progName, _MODERN_GET_STARTED_BYTE ))
				return NULL;

				modernHwnd = FindWindowExA( NULL, NULL, _MODERN_MODERN_APP_BYTE, progName );
				if (!modernHwnd)
					return NULL;
			
				DWORD mpid;
				GetWindowThreadProcessId( modernHwnd, &mpid );

				BOOL found = FALSE;
				HWND hx = NULL;

				do
				{
					hx = FindWindowEx( NULL, hx, _MODERN_WORKER, _MODERN_WORKER );
					if (hx)
					{
						DWORD xpid;
						GetWindowThreadProcessId( hx, &xpid );
						if (xpid == mpid)
							break;
					}
				} while (hx);

				if (!hx)
					return NULL;
			}

		}
	}
	else
	{
		// Get Class Name
		if ( !GetClassNameA( hwnd, className, 1000 ) )
			sprintf_s( className, sizeof( className ), "<CN UNKNOWN>" );

		if ( strstr( "NotifyIconOverflowWindow", className ) || strstr( "Xaml_WindowedPopupClass", className ) )
			return NULL;

		// Get ProcessName, which is what gets used to group windows into
		// piles.

		// The name is just the text between the final backslash in the path
		// to the executable and the first period encountered, usually the one
		// right before the extension.

		HANDLE handle = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId );

		char  *pNamePtr = progName;
		progName[0] = 0;
		if (0 < GetProcessImageFileNameA( handle, progName, 1000 ))
		{
			size_t i, j = strlen( progName );
			for (i = 0; i < j; i++)
			{
				if (progName[i] == '\\') pNamePtr = &progName[i + 1];
				if (progName[i] == '.') progName[i] = 0;
			}
			strcpy_s( progName, sizeof( progName ), pNamePtr );
		}
		else
			strcpy_s( progName, sizeof( progName ), className );

		CloseHandle( handle );

		if ( !strcmp( progName, "dllhost" ) )
			return NULL;

	
		if (!((style & WS_CAPTION) == WS_CAPTION || (xstyle & WS_EX_TOOLWINDOW) == 0))
			return NULL;

		if (!(xstyle & WS_EX_MAGICAL))
			return NULL;

		if (McModernAppMgr::MatchesModernClassname( className ))
			return NULL;
		
	}

	if (strstr( className, "SideBar_HTMLHostWindow" ))
		return NULL;

	if (MC::getProperties( )->getIsExcluded( progName ))
		return NULL;

	if (xstyle & WS_EX_DLGMODALFRAME)
	{
		HWND parent = GetAncestor( hwnd, GA_ROOTOWNER );
		BOOL reject = parent && (parent != hwnd);

		if (reject)
			if (!strstr( progName, "#32770" ))
				reject = FALSE;

		if (reject) return NULL;
	}

	if (strstr( progName, "explorer" ) )
	{
		char cb[1000];
		GetWindowTextA( hwnd, cb, 1000 ); 
		if (cb[0] == 0 )
			return FALSE;
	}

	// Build a rectangle describing the application on the screen and
	// another which wraps the content that we want displayed in the
	// thumbnail.   Not the same for maximized and some odd app windows.

	McRect paddedRect;
	McRect normalRect;
	McRect contentRect;

	// Ignore all completely off-desktop windows.   If we can't ever see
	// it no one will notice it's missing from the piles.

	WINDOWPLACEMENT wp = { 0 };
	McWindowMgr::myGetWindowPlacement( hwnd, &wp );

	McRect rcNormal( &wp.rcNormalPosition );
	BOOL restoreMax = wp.flags & WPF_RESTORETOMAXIMIZED;


	HMONITOR mon = MonitorFromWindow( hwnd, MONITOR_DEFAULTTONEAREST );

	MONITORINFO minfo;
	McMonitorsMgr::myGetMonitorInfo( mon, &minfo );;

	McRect rcWindow( &info.rcWindow );
	McRect rcClient( &info.rcClient );

	SIZE borders;

	borders.cx = min( (int) info.cxWindowBorders , (iconic&&restoreMax ? 100 : (rcWindow.getWidth( ) - rcClient.getWidth( )) / 2) );
	borders.cy = min( (int) info.cyWindowBorders, (iconic&&restoreMax ? 100 : rcWindow.getHeight( ) - rcClient.getHeight( )) );
	
	McRect mdmT( &minfo.rcMonitor );

	SIZE thumbSize = { 0 };

	if (!iconic)
	{
		paddedRect.set( &rcWindow );
		normalRect.set( &paddedRect );
		if ( maximized ) normalRect.top += borders.cy;
		normalRect.left += borders.cx;
		normalRect.right -= borders.cx;
		normalRect.bottom -= borders.cy;
	}
	else
	{
		HTHUMBNAIL thumbnail = NULL;
		HRESULT hr = DwmRegisterThumbnail( McWindowMgr::getMainW( )->getHwnd( ), hwnd, &thumbnail );
		DwmQueryThumbnailSourceSize( thumbnail, &thumbSize );
		if (thumbnail)
			DwmUnregisterThumbnail( thumbnail );

		maximized = restoreMax;
		paddedRect.set( -borders.cx, -borders.cy, thumbSize.cx-borders.cx, thumbSize.cy-borders.cy );
		normalRect.set( &paddedRect );
		if ( maximized ) normalRect.top += borders.cy;
		normalRect.left += borders.cx;
		normalRect.right -= borders.cx;
		normalRect.bottom -= borders.cy;

		if (restoreMax)
			normalRect.offset( minfo.rcWork.left, minfo.rcWork.top );
		else
			normalRect.offset( rcNormal.left, rcNormal.top );
		if ( !restoreMax ) normalRect.offset( borders.cx, borders.cy );
		paddedRect.offset( normalRect.left, normalRect.top );
	}

	contentRect.set( &normalRect );
	contentRect.offset( -normalRect.left + borders.cx, -normalRect.top + 
		( maximized ? borders.cy : 0) );

	if (normalRect.getWidth( ) < minWin || normalRect.getHeight( ) < minWin)
		return NULL;

	if (!McMonitorsMgr::getMainMonitor( )->getDesktopSize( )->overlaps( &normalRect ))
		return NULL;

	// OK, it's a keeper.

	if (!_isImmersive || isW10App)
	{
		string s( progName );
		desktopAppList.remove( s );
		desktopAppList.push_back( s );
	}

	if ((iconic || isW10App) && restoreMax)
		maximized = TRUE;

	if (isW10App)
	{
		if (!iconic && !maximized && !restoreMax)
		{
			McMonitorsMgr *m = McMonitorsMgr::getMonitor( mon );
			if (McMonitorsMgr::getMonitor( mon )->getMonitorSize( )->equals( &paddedRect ))
				maximized = TRUE;
		}
	}

	McRect *mmr = McMonitorsMgr::getMonitor( mon )->getMonitorWorkSize( );

	BOOL isEdgeSnapped = 
		(normalRect.getHeight( ) >= mmr->getHeight( ) - 5 &&
		 normalRect.getWidth( ) < mmr->getWidth( ) - 5 );

	McWItem *item = new McWItem(
		hwnd,
		mon,
		processId,
		threadId,
		FALSE,
		cloaked != 0,
		iconic,
		maximized,
		isW10App,
		isEdgeSnapped,
		className,
		progName,
		&borders,
		&paddedRect,
		&normalRect,
		&contentRect );

	if (isW10App)
		item->setModernHwnd( modernHwnd );

	return item;
}

void McWList::handleNextWindow( HWND hwnd, BOOL _isImmersive )
{
	McWItem *item = verifyWindow( hwnd, _isImmersive );

	
	if (item)
		itemList.push_front( item );
}

void McWList::handleNewWindow( HWND hwnd )
{
	char cn[1000];
	GetClassNameA( hwnd, cn, 1000 );

	McWItem *item = MC::getWindowList( )->findItem( hwnd );
	if (item) return;

	BOOL isI = FALSE;
	isI = 0 == strcmp( cn, _MODERN_MODERN10_APP_BYTE );
 	item = verifyWindow( hwnd, isI );
	
	if (item)
	{
		item->setIsNew( );
		item->createThumbWindow( );
		itemList.push_back( item );
		if (item->getIsOnAppBar( ))
			reinitLayout( MCS_TransitionOver );
		else
			reinitLayout( MCS_TransitionDesk );
	}
}

// Put the target at the back (z-order top) of the list.

void McWList::insertItemOnTop( McWItem *_item )
{
	itemList.remove( _item );
	itemList.push_back( _item );
}

McWItem **McWList::getDesktopItem()
{
	return desktopItem;
}

McWItem *McWList::getFirstVisibleItem()
{
	for ( list<McWItem *>::iterator it = itemList.begin(); it != itemList.end(); it++ )
		if ( (*it)->getVisibility() )
			return *it;
	return NULL;
}

McWItem *McWList::getLastVisibleItem( )
{
	for (list<McWItem *>::reverse_iterator rit = itemList.rbegin( ); rit != itemList.rend( ); rit++)
		if ((*rit)->getVisibility( ))
			return *rit;
	return NULL;
}

McWItem *McWList::getLastVisibleAppbarItem()
{
	for ( list<McWItem *>::reverse_iterator it = itemList.rbegin(); it != itemList.rend(); it++ )
		if ( (*it)->getIsOnAppBar() && (*it)->getVisibility() )
			return *it;
	return NULL;
}

McWItem *McWList::getFirstVisibleAppbarItem( )
{
	for (list<McWItem *>::iterator it = itemList.begin( ); it != itemList.end( ); it++)
		if ((*it)->getIsOnAppBar( ) && (*it)->getVisibility( ))
			return *it;
	return NULL;
}

// Find the handle of the window "above" a z-order value in the thumbnails.

McWItem* McWList::getItemAbove( McWItem *target )
{
	for ( list<McWItem *>::iterator it = itemList.begin(); it != itemList.end(); it++ )
		if ( (*it)->getZorder() < target->getZorder() )
			return (*it);
	return NULL;
}

McWItem* McWList::getPileItemAbove( McWItem *target )
{
	for (list<McWItem *>::iterator it = itemList.begin( ); it != itemList.end( ); it++)
	{
		McWItem *item = *it;
		if (item->getPileNumber( ) == target->getPileNumber( ) && item->getZorder( ) < target->getZorder( ))
			return item;
	}
	return NULL;
}

McWItem *McWList::getItemBelow( McWItem *target )
{
	for ( list<McWItem *>::reverse_iterator r_it = itemList.rbegin(); r_it != itemList.rend(); r_it++ )
		if ( (*r_it)->getZorder() > target->getZorder() )
			return ( *r_it );

	return NULL;
}

McWItem *McWList::getPileItemBelow( McWItem *target )
{
	for ( list<McWItem *>::reverse_iterator r_it = itemList.rbegin(); r_it != itemList.rend(); r_it++ )
		if ( (*r_it)->getPileNumber() == target->getPileNumber() && (*r_it)->getZorder() > target->getZorder() )
			return ( *r_it );
	return NULL;
}

// Find the target and insert the item before (under) it.

void McWList::insertItemBefore( McWItem *_item, McWItem *_target )
{
	itemList.remove( _item );
	for ( ITEM_IT it = itemList.begin(); it != itemList.end(); it++ )
		if ( (*it) == _target )
		{
			itemList.insert( it, _item );
			break;
		}
}

void McWList::changeFocus( McWItem *_item )
{
	if ( _item && !_item->getVisibility() )
		PostMessage( McWindowMgr::getMainW()->getHwnd(), WM_USER, WMMC_REINIT, NULL );
	else
	if ( _item != focusItem )
		{
			if ( focusItem )
				focusItem->loseFocus();
			if ( _item )
				_item->takeFocus();
		}
}


void McWList::keyPress( WPARAM key, LPARAM lp )
{
	if (!McWindowMgr::getZoomW( ))
		return;
	if ( McWindowMgr::getZoomW()->isZoomed() )
	{
		PostMessage( McWindowMgr::getZoomW()->getHwnd(), WM_KEYDOWN, key, NULL );
		return;
	}
	BOOL isOnAppBar = FALSE;
	if ( focusItem ) isOnAppBar = focusItem->getIsOnAppBar();

	if ( MC::getState() == MCS_Display )
		switch( key )
		{
		case VK_RETURN:
			if ( focusItem )
				focusItem->getThumbWindow( )->onSelectWindow( VK_IS_KEY_PRESSED( VK_CONTROL ) != 0 );
			break;
		case VK_S_KEY:	// S Key ( Settings )
			if (VK_IS_KEY_PRESSED( VK_CONTROL ))
				McWindowMgr::getMainW()->onSettings();
			break;
		case VK_M_KEY:	// M key (Minimize all windows if CTRL)
			if (VK_IS_KEY_PRESSED( VK_CONTROL ))
				McWindowMgr::getMainW()->onMinimizeAll( TRUE, NULL,
					focusItem ? focusItem->getAppMonitor() : NULL );
			break;
		case VK_D_KEY:   // D key (Minimize all Windows Store windows if CTRL)
			if ( (MC::getState( ) == MCS_Display)
				&&
				VK_IS_KEY_PRESSED( VK_CONTROL ) || lp)
			{
				McWindowMgr::getMainW( )->onMinimizeImmersiveApps(
					focusItem ? focusItem->getAppMonitor( ) : NULL );
			}
			break;
		case VK_N_KEY:	// N key (Normalize all windows if CTRL)
			if (VK_IS_KEY_PRESSED( VK_CONTROL ))
				McWindowMgr::getMainW()->onNormalizeAll(focusItem?focusItem->getAppMonitor():NULL);
			break;
		case VK_B_KEY:	// B Key (Move focus window to bottom of z order if CTRL)
			if (VK_IS_KEY_PRESSED( VK_CONTROL ))
				if ( focusItem )
					if (!focusItem->getIsOnAppBar( ))
						focusItem->getThumbWindow()->onMoveToBottom();
			break;
		case VK_ESCAPE:
			if (McWindowMgr::getZoomW( ))
			{
				if (McWindowMgr::getZoomW( )->isZoomed( ))
					PostMessage( McWindowMgr::getZoomW( )->getHwnd( ), WM_RBUTTONDOWN, NULL, NULL );
				else
					McWindowMgr::getMainW( )->onFinish( );
			}
			break;

		case VK_Z_KEY:	// Z Key (Zoom focus window if CTRL )
			
			if ( focusItem )
				if (VK_IS_KEY_PRESSED( VK_CONTROL ))
					focusItem->getThumbWindow( )->onZoom( VK_IS_KEY_PRESSED(VK_SHIFT) != 0 );
			break;
		case VK_R_KEY:	// R Key (Refresh entirely)
			if (VK_IS_KEY_PRESSED( VK_CONTROL ))
			{
				MC::getMC()->setRunAgain(TRUE);
				McWindowMgr::getMainW()->clearActiveApp();
				McWindowMgr::getMainW()->onFinish();
			}
			break;
		case VK_KILLKEY:  // K Key (Kill window if CTRL )
			if ((VK_IS_KEY_PRESSED( VK_CONTROL )) || lp)
				if ( focusItem )
					killApp( focusItem );
			break;
		case VK_TAB:
			if (VK_IS_KEY_PRESSED( VK_SHIFT ))
				nextPile( -1 );
			else
				nextPile( +1 );
			break;
		case VK_LEFT:
			if ( isOnAppBar )
			{
				if ( !nextWindow( +1 ) )
					onLeftArrowClick();
			}
			else
				nextPile( -1 );		// Move focus left one pile
			break;
		case VK_RIGHT:
			if (isOnAppBar)
			{
				if ( !nextWindow( -1 ) )
					onRightArrowClick();
			}
			else
				nextPile( +1 );		// Move focus right one pile
			break;
		case VK_UP:
			if (isOnAppBar)
				nextPile( -1 );
			else
				nextWindow( +1 );	// Move focus up within pile
			break;
		case VK_DOWN:
			if (isOnAppBar)
				nextPile( +1 );
			else
				nextWindow( -1 );	// Move focus down within pile
			break;
		default:
			break;
		}
	else if (McWindowMgr::getZoomW() )
		if ( McWindowMgr::getZoomW()->isZoomed() )
			SendMessage( McWindowMgr::getZoomW()->getHwnd(), WM_RBUTTONUP, NULL, NULL );
}


void McWList::nextPile( int _delta )
{
	int newPile;
	if ( !focusItem )
		return;

	MC *mc = MC::getMC();
	newPile = focusItem->getPileNumber()+_delta;
	if ( newPile < 0 )
		newPile = McPilesMgr::getNPiles( ) - 1;
	else
		if (newPile == McPilesMgr::getNPiles( ))
			newPile = 0;

	for ( list<McWItem*>::reverse_iterator it = itemList.rbegin(); it != itemList.rend(); it++ )
	{
		McWItem *wi = *it;
		if ( wi->getPileNumber() == newPile )
		{
			changeFocus( wi );
			return;
		}
	}
}


BOOL McWList::nextWindow( int _delta ) // Only sign of delta matters
{
	if ( !focusItem )
	{
		return TRUE;
	}
	McWItem *newItem = NULL;
	if ( _delta < 0 )
	{
		newItem = getPileItemBelow( focusItem );
		if (newItem && newItem->getIsOnAppBar( ) && !newItem->getVisibility( ))
			return FALSE;
	}
	else if ( _delta > 0 )
	{
		newItem = getPileItemAbove( focusItem );
		if (newItem && newItem->getIsOnAppBar( ) && !newItem->getVisibility( ))
			return FALSE;
	}
	if ( newItem != NULL )
	{
		changeFocus( newItem );
		return TRUE;
	}
	return FALSE;
}

void McWList::killApp( McWItem *_item )
{
	McWindowMgr::getMainW( )->clearActiveApp( _item );

	McWItem *newItem = NULL;

	for (list<McWItem *>::reverse_iterator rit = itemList.rbegin( ); rit != itemList.rend( ); rit++)
	{
		McWItem* rit_item = *rit;
		if (rit_item != _item)
		{
			newItem = rit_item;
			changeFocus( newItem );
			break;
		}
	}

	BOOL result = FALSE;

	_item->closeThumbWindow( );

	if (_item->getIsW10App( ))
	{
		if (McModernAppMgr::CloseApp( _item ))
		{
			if (McModernAppMgr::tabletModeDividerExists( ))
				McWindowMgr::getMainW( )->onFinish( );
		}
		return;
	}
	else
	{
		result = PostMessage( _item->getAppHwnd( ), WM_SYSCOMMAND, SC_CLOSE, NULL );
		if (strstr( _item->getProgName( ), "Stiky" ))
		{
			removeKilledApp( _item, "Stiky test" );
			return;
		}
	}

	if (!result)
	{
		char msg[1000];
		sprintf_s( msg, 1000,
			"\n\n\n\nEmcee was unable to close the underlying App, but will remove it from the thumnail view.  " );
		strcat_s( msg, 1000, "\n\n\n\n" );

		MC::reportError( TRUE, msg, NULL );

	}
	else
		removeKilledApp( _item, "killApp" );

}

McState McWList::removeKilledApp( McWItem *_item ,char *whence, BOOL reinit )
{

	McState transition = MCS_TransitionOver;

	if (_item->getIsOnAppBar( ))
	{
		int nAppBarApps = 0;
		list<McWItem *> *itemList = getItemList( );
		for (ITEM_IT it = itemList->begin( ); it != itemList->end( ); it++)
			if ((*it)->getIsOnAppBar( )) nAppBarApps++;
		if (nAppBarApps > 1)
			transition = MCS_TransitionImmersiveApp;
	}
	else
		transition = MCS_TransitionDesk;

	itemList.remove( _item );
	delete _item;

	if ( reinit ) reinitLayout( transition );

	return transition;
}

void McWList::reinitLayout( McState transition )
{
	MC *mc = MC::getMC( );

	if (McWindowMgr::getZoomW( ))
		if (McWindowMgr::getZoomW( )->isZoomed( ))
			McWindowMgr::getZoomW( )->doZoom( MC_ZOOM_PANIC );

	initializeRects( MCS_Display );

	// Group the windows into piles based on application name and
	// any grouping specified in the properties file.   Within each
	// pile arrange the thumbnails in a reasonable way.

	transition = cleanUp( transition );

	int _nPiles = 0;
	for (int i = 0; i < McMonitorsMgr::getEffectiveMonitorCount( ); i++)
	{

		markVisibleW10Windows( desktopItem[i]->getAppMonitor( ) );
		McPilesMgr *piles = new McPilesMgr( _nPiles, desktopItem[i]->getAppMonitor( ) );

		desktopItem[i]->setStartRect( desktopItem[i]->getThumbRect( ) );

		positionBackground( desktopItem[i], piles->getImmersiveAppOffset( ), piles->getImmersiveAppGap( ) );
		desktopItem[i]->createDesktopWindow( );

		piles->createPiles( );

		// Arrange the thumbnail piles _in the monitor space
		piles->layoutPiles( );
		_nPiles = piles->getNextPileNumber( );
		McPilesMgr::setNPiles( _nPiles );

		// Then map that inFormation onto the positions for the individual
		// thumbnails.

		if ( transition != MCS_TransitionImmersiveApp ) 
			piles->positionThumbs( );

		// Tell main and bg windows if navigation arrows are required

		if (desktopItem[i]->getAppMonitor( ) == McMonitorsMgr::getMainMonitor( )->getHMONITOR( ))
		{

			McRect leftArrowRect;
			if (piles->areArrowsNeeded( &leftArrowRect ))
				McWindowMgr::getBgW( )->setArrowsNeeded( &leftArrowRect );
			else
				McWindowMgr::getBgW( )->setArrowsNeeded( NULL );
		}

		delete piles;
	}

	McWindowMgr::getMainW()->setStartButtonLocation();
	McWindowMgr::getMainW( )->doTransitionOver( transition );
}

BOOL _itemCompare( McWItem *first, McWItem *second )
{
		return first->getZorder() > second->getZorder();
}

void McWList::sortWindows( )
{
	itemList.sort( _itemCompare );
}

void McWList::initializeRects( McState state )
{
	layoutDesktop( state );
	POINT *mdCenter;
	for ( ITEM_IT it=itemList.begin();it!=itemList.end();it++ )
	{
		McWItem *item = *it;

#if MC_DESKTOPS
		BOOL cloakedInfo = item->getIsCloaked() && MC::getProperties()->getShowAllDesktops();
#else
		BOOL cloakedInfo = FALSE;
#endif
		BOOL isObscured = ( item->getIsOnAppBar() && item->getIsWindowObscured( )) && !item->getIsMinimized( ) && !cloakedInfo;
		switch ( state )
		{
		case MCS_TransitionDown:
			mdCenter = McMonitorsMgr::getMonitor( item->getAppMonitor( ) )->getMonitorCenter( );
			if (item->getIsMinimized( ) || cloakedInfo || isObscured)
				MC::setVanishingPoint( item->getThumbRect( ), item->getStartRect( ), item->getIsOnAppBar( ), cloakedInfo,
					isObscured, mdCenter );
			else
				item->setStartRect( item->getNormalRect() );
			item->setEndRect( item->getThumbRect() );
			break;
		case MCS_TransitionUp:
			mdCenter = McMonitorsMgr::getMonitor( item->getAppMonitor( ) )->getMonitorCenter( );
			if (item->getIsMinimized( ) || cloakedInfo || isObscured )
				MC::setVanishingPoint( item->getThumbRect( ), item->getEndRect( ), item->getIsOnAppBar( ), cloakedInfo,
					isObscured, mdCenter );
			else
				item->setEndRect( item->getNormalRect( ) );
			item->setStartRect( item->getThumbRect() );
			break;
		case MCS_Display:
			item->setStartRect( item->getThumbRect() );
			break;
		case MCS_TransitionDesk:
		case MCS_TransitionOver:
			if (item->getIsNew( ))
				MC::setVanishingPoint( item->getThumbRect( ), item->getStartRect( ), item->getIsOnAppBar( ) );

		case MCS_TransitionImmersiveApp:
			item->setEndRect( item->getThumbRect() );
			break;
		default:
			return;
		}

		item->setCurRect( item->getStartRect() );
	}
}

void McWList::setFocusItem( McWItem *_focusitem )
{
	if ( focusItem )
		focusItem->setHaveFocus( FALSE );
	focusItem = _focusitem;
	if ( focusItem )
		focusItem->setHaveFocus( TRUE );
	}

void McWList::onLeftArrowClick( )
{
	if (!McWindowMgr::getBgW( )->getArrowsNeeded( ))
		return;

	McWItem *nitem = NULL;
	list<McWItem *>::iterator it;

	list<McWItem *>::reverse_iterator rit;

	for (rit = itemList.rbegin( ); rit != itemList.rend( ); rit++)
		if ((*rit)->getIsOnAppBar( ) && !(*rit)->getVisibility( ))
		{
			nitem = *rit;
			break;
		}
	if (!nitem) return;
	int z = nitem->getZorder( );
	for (rit = itemList.rbegin( ); rit != itemList.rend( ); rit++)
	{
		McWItem *item = *rit;
		if (item->getIsOnAppBar( ) && (item != nitem))
			item->setZorder( ++z );
	}
	itemList.remove( nitem );
	itemList.push_front( nitem );
	setLeftHanded( );
	reinitLayout( MCS_TransitionImmersiveApp );
	return;

}

void McWList::onRightArrowClick()
{
	if (!McWindowMgr::getBgW( )->getArrowsNeeded( ))
		return;

	McWItem *fitem = NULL, *nitem = NULL;
	for ( list<McWItem *>::reverse_iterator rit = itemList.rbegin(); rit != itemList.rend(); rit++ )
		if ( (*rit)->getIsOnAppBar() )
		{
			if ( (*rit)->getVisibility() )
			{
				if ( !fitem )
					fitem = *rit;
			}
			else
			{
				if ( !nitem )
					nitem = *rit;
			}
		}
	if ( fitem && nitem )
	{
		McWindowMgr::getMainW()->setOnDisplayFocusItem( (McWItem*)-1 );
		if ( fitem->getThumbWindow() )
			fitem->getThumbWindow()->immersiveAppMoveToBottom();
	}
}

McState McWList::cleanUp( McState transition )
{
	BOOL again = TRUE;
	while (again)
	{
		again = FALSE;
		for ( list<McWItem *>::iterator it = itemList.begin(); it != itemList.end(); it++ )
		{
			McWItem *item = *it;
			if ( (!item->getVisibility()) && !item->getIsOnAppBar() )
			{
				again = TRUE;
				itemList.remove( item );
				if ( transition == MCS_TransitionImmersiveApp )
					transition = MCS_TransitionOver;
				McWindowMgr::getMainW()->clearActiveApp( item );

				delete item;
				break;
			}
		}
	}
	return transition;
}

BOOL McWList::contains( McWItem *item )
{
	for ( list<McWItem *>::iterator it = itemList.begin(); it != itemList.end(); it++ )
		if ( item == *it ) return TRUE;
	return FALSE;
}

McWItem *McWList::findItem( HWND hwnd )
{
	for (list<McWItem *>::iterator it = itemList.begin( ); it != itemList.end( ); it++)
	{
		McWItem *wi = *it;
		if (wi->getAppHwnd( ) == hwnd)
			return wi;
	}
	return NULL;
}

BOOL CALLBACK McWList::myWindowEnumCallback( HWND hwnd, LPARAM obj )
{
	McWList *pThis = (McWList *) obj;
	if (pThis)
		pThis->handleNextWindow( hwnd );
	return TRUE;
}

BOOL CALLBACK McWList::myDtIconSearchCallback( HWND hwnd, LPARAM lparam )
{
	// SHELLDLL_DefView is a window class windows explorer uses to display a list of
	// files or icons or thumbnails.   One instance of this class is used to display icons
	// on the desktop.  GetWindowsText returns a zero-length string in that instance.

	wchar_t wbuff[1000], wbuff1[1000];
	GetClassName( hwnd, wbuff, 1000 );
	int tlength = GetWindowText( hwnd, wbuff1, 1000 ); 
	if (!wcscmp( wbuff, L"SHELLDLL_DefView" ) && tlength == 0)
	{
		*(HWND *) lparam = GetAncestor( hwnd, GA_ROOT );
		return FALSE;
	}
	return TRUE;
}


void McWList::markVisibleW10Windows( HMONITOR hmon )
{

	sortWindows( );

	McMonitorsMgr *monitor = McMonitorsMgr::getMonitor( hmon );

	BOOL haveObscuringWindow = FALSE;

	for (ITEM_RIT rit = itemList.rbegin( ); rit != itemList.rend( ); rit++)
	{
		McWItem *item = *rit;
		BOOL Obscurance = FALSE;
		BOOL full;
		if ( item->getIsOnAppBar() && item->getAppMonitor( ) == hmon && !item->getIsCloaked( ) && !item->getIsMinimized( ))
		{
			full = item->getNormalRect( )->getHeight( ) >= monitor->getMonitorWorkSize( )->getHeight( ) - 5 &&
				item->getNormalRect( )->getWidth( ) >= monitor->getMonitorWorkSize( )->getWidth( ) - 5;

			Obscurance = haveObscuringWindow;
			haveObscuringWindow |= full;
		}

		item->setIsWindowObscured( Obscurance );
	}
}








