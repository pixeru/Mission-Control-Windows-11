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
// Copyright (c) Emcee App Software. All rights reserved#include "stdafx.h"

#include "Emcee.h"
#include "McMonitorsMgr.h"
#include <gdiplus.h>

#pragma unmanaged

using namespace std;

enum wiThumbLocation
{
	WIL_NO_WINDOW,
	WIL_MAIN_WINDOW,
	WIL_THUMB_WINDOW,
	WIL_ZOOM_WINDOW,
	WIL_BG_WINDOW,
	WIL_DT_WINDOW
};

class McWItem
{
public:

	McWItem(
		HWND	_appHwnd,
		HMONITOR _appMonitor,
		DWORD	_appPid,
		DWORD	_appTid,
		BOOL	_isDesktop,
		BOOL	_isCloaked,
		BOOL	_isMinimized,
		BOOL	_isMaximized,
		BOOL	_isW10App,
		BOOL	 w10snapped,
		char*	_className,
		char*	_processName,
		SIZE *	_borders,
		RECT*	_paddedRect,
		RECT*	_normalRect,
		RECT*	_contentRect,
		HWND	_iconShell = NULL
		);
	
	~McWItem();

	// Action Functions

	HRESULT registerThumbnail( wiThumbLocation );		// useAlpha=FALSE, unreg=FALSE
	HRESULT registerThumbnail( wiThumbLocation, BOOL );  // specify useAlpha
	HRESULT registerThumbnail( BOOL, wiThumbLocation );  // useAlpha=FALSE, specify unregister
	HRESULT registerThumbnail( BOOL, wiThumbLocation, BOOL );
	void	unregisterThumbnail( HTHUMBNAIL=NULL );	// Unregister for DWM Thumbnail
	HRESULT	showThumbnail();						// Activate Thumbnail at currentRect
	void	createThumbWindow();					// Create the thumb window: call prior to registerThumbnail.
	void	disposeThumbWindow( );
	void	createDesktopWindow( );	
	void	disposeDesktopWindow( );// Create the desktop window: call prior to registerThumbnail.
	void	showDesktopWindow( );
	void	closeDesktopWindow( );
	void	showThumbWindow();						// Display and position thumb window
	void	showThumbWindow( HWND );
	void	closeThumbWindow();						// Hide thumb window
	void	placeInPile(BOOL=FALSE);
	void	pushToBottom();							// Moves both thumb and application window to bottom
	void	raiseToTop(BOOL=FALSE, BOOL=TRUE);							// Moves both thumb and appliation window to top
	void	raiseApp();
	// Convenience Functions -- coded in header
	HWND	getAppHwnd() { return appHwnd; }
	HMONITOR getAppMonitor( ) {	return appMonitor;	}
	void	setAppMonitor( HMONITOR m ) { appMonitor = m; }
	DWORD	getAppPid() { return appPid; }
	DWORD	getAppTid() { return appTid; }
	BOOL	getIsDesktop() { return isDesktop; }
	BOOL	getIsMinimized() { return isMinimized; }
	void    setIsMinimized( BOOL im ) {	isMinimized = im; }
	BOOL	getIsCloaked( ) { return isCloaked;	}
	void	setIsCloaked( BOOL ic ) { isCloaked = ic; }
	BOOL	getIsMaximized() { return isMaximized; }
	void	setIsMaximized( BOOL im ) { isMaximized = im; }
	const char*	getClassName() { return (className->c_str()); }
	const char*	getProgName() { return (processName->c_str()); }
	McRect*	getCurRect() { return &currentRect; }
	void	setCurRect(McRect *r) { currentRect.set(r); }
	SIZE*	getBorders( ) {	return &borders; }
	BOOL	getIsEdgeSnapped( ) { return isEdgeSnapped; }
	void	setBorders( SIZE *_borders )
	{
		if (_borders)
		{
			borders.cx = _borders->cx;
			borders.cy = _borders->cy;
		}
		else
			borders.cx = borders.cy = NULL;
	}
	McRect *getPaddedRect( ) { return &paddedRect; }
	void	setPaddedRect(McRect *r ) {	paddedRect.set( r ); }
	McRect*	getNormalRect() { return &normalRect; }
	void	setNormalRect(McRect *r) { normalRect.set(r); }
	McRect*	getContentRect() { return &contentRect; }
	McRect*	getEndRect() { return &endRect; }
	void	setEndRect(McRect *r) { endRect.set(r); }
	McRect*	getThumbRect() { return &thumbRect; }
	void	setThumbRect( McRect *r) { thumbRect.set( r ); }
	McRect*	getStartRect() { return &startRect; }
	void	setStartRect( McRect *r) { startRect.set( r ); }
	McRect* getZoomRect() { return &zoomRect; }
	void	setZoomRect( McRect *r ) { zoomRect.set( r ); }
	LONG	getLeft() { return currentRect.left; }
	LONG	getTop() { return currentRect.top; }
	LONG	getRight() { return currentRect.right; }
	LONG	getBottom() { return currentRect.bottom; }
	LONG	getWidth() { return currentRect.getWidth(); }
	LONG	getHeight() { return currentRect.getHeight(); }
	int		getZorder() { return zOrder; }
	void	setZorder( int _zOrder ) { zOrder = _zOrder; }
	HWND	getThumbHwnd() { return thumbHwnd; }
	void	setPileNumber( int _pileNumber ) { pileNumber = _pileNumber; }
	int		getPileNumber() { return pileNumber; }
	HWND	getModernHwnd( ) { return modernHwnd; }
	void	setModernHwnd( HWND hwnd ) { modernHwnd = hwnd;	}
	BOOL	needsTransparency( ) { return isCloaked || (isMinimized /*&& !getIsOnAppBar( )*/); }
	BOOL	getIsW10App( ) { return isW10App; }
	BOOL	getIsAPackage( ) { return isW10App; }
	BOOL	getIsOnAppBar( ) 
	{ 
		return onAppBar;
	}
	BOOL	getOnMainMonitor( ) { return onMainMonitor;	}

	BOOL	getIsMoveable( )
	{
		if (getIsOnAppBar( ) ) return FALSE;
		return TRUE;
	}
	McThumbW*	getThumbWindow() { return thumbWindow; }
	McDesktopW *getDesktopWindow( ) { return desktopWindow;	}
	HTHUMBNAIL	getThumbnail() { return thumbnail; }
	HTHUMBNAIL	getIconThumb( ) { return iconThumb;	}
	void		clearTempThumbnail( ) { tempThumbnail = NULL; }
	void		setTempThumbnail() { tempThumbnail = thumbnail; }
	HTHUMBNAIL  getTempThumbnail( void ) { return tempThumbnail; }
	BOOL		haveIcon( ) 
	{ 
		return hicon!=NULL || iconBitmap !=0;	
	}
	void		DrawIcon( HDC hdc, int x, int y, int width, int height );
	void setAlpha( double );
	void setImmersiveAppAlpha( double );
	void setHaveFocus( BOOL _havefocus ) { haveFocus = _havefocus; }
	void takeFocus();
	void loseFocus();
	BOOL getVisibility() { return isVisible && !becomesInvisible; }
	void adjustVisibility();
	void setVisibility( BOOL, BOOL );
	void setRotation( float rot = 1.0 )
	{
		rotation = rot;
	}
	float getRotation( )
	{
		return rotation;
	}
	void setIsWindowObscured( BOOL _vis )
	{
		isWindowObscured = _vis;
	}
	BOOL getIsWindowObscured( )
	{
		return isWindowObscured;
	}

	void setIsNew(  ) 
	{
		isNew = TRUE;
		isVisible = FALSE;
		setVisibility( TRUE, FALSE );
	}

	BOOL getIsNew( ) 
	{
		BOOL wasNew = isNew;
		isNew = FALSE;
		return wasNew;
	}

	void calculateZoomedPosition( );

private:						

	HWND appHwnd;			// Handle of desktop window
	HMONITOR appMonitor;	// Monitor of Window
	DWORD appPid;			// Process ID of desktop window
	DWORD appTid;
	BOOL isDesktop;			// Is it a desktop view
	BOOL isCloaked;
	BOOL isMinimized;		// Is desktop window minimized?
	BOOL isMaximized;		// Is desktop window maximized?
	BOOL isW10App;			// Is it a W10 modern app?
	HWND modernHwnd;		//     If so, this is the hwnd of the Windows.UI window
	BOOL onAppBar;
	BOOL onMainMonitor;		// Is it on the main monitor?
	BOOL isEdgeSnapped;
	BOOL isNew;				// TRUE if newly added to the windowlist
	BOOL isWindowObscured;		// non-zero if this is obscured
	HWND iconShell;
	string *className;
	string *processName;
	HTHUMBNAIL tempThumbnail;
	HTHUMBNAIL thumbnail;
	HTHUMBNAIL iconThumb;
	wiThumbLocation thumbLocation;
	McThumbW *thumbWindow;
	HWND	thumbHwnd; 
	McDesktopW *desktopWindow;
	McRect			animateRect;
	wiThumbLocation animateLocation;
	Gdiplus::Bitmap *iconBitmap;
	Gdiplus::Color  *iconColor;
	HICON hicon;
	BOOL		isVisible;
	BOOL		wasVisible;
	BOOL		becomesInvisible;
	int			pileNumber;
	BOOL		haveFocus;
	McRect startRect;		// starting position & size
	McRect endRect;			// ending position & size
	McRect currentRect;		// position & size at the moment
	McRect zoomRect;		// zoomed thumbnail;
	SIZE   borders;
	McRect paddedRect;
	McRect normalRect;		// Desktop view position & size
	McRect thumbRect;		// Thumbnail view position & size
	McRect contentRect;
	int zOrder;
	BOOL useAlpha;
	BYTE alpha;
	float rotation;
};

#define ITEM_IT list<McWItem*>::iterator
#define ITEM_RIT list<McWItem *>::reverse_iterator
