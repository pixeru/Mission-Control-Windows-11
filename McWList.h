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

#pragma unmanaged

using namespace std;

class McWItem;
enum McState;

class McWList
{
public:

	McWList();
	~McWList();

	void createThumbWindows( );
	void positionBackground( McWItem *,int, int );
	void sortWindows( );
	void initializeRects( McState );
	McWItem* findItem( HWND );
	list<McWItem*> *getItemList( ) {
		return &itemList;
	}

	McWItem *getFirstVisibleItem();
	McWItem *getLastVisibleItem( );
	McWItem *getLastVisibleAppbarItem();
	McWItem *getFirstVisibleAppbarItem( );
	McWItem **getDesktopItem();
	McWItem *getItemAbove( McWItem* );
	McWItem *getItemBelow( McWItem* );
	McWItem *getPileItemAbove( McWItem* );
	McWItem *getPileItemBelow( McWItem* );
	BOOL contains( McWItem* );
	void	onRightArrowClick();
	void	onLeftArrowClick();
	void insertItemBefore( McWItem *_insert, McWItem *_before );
	void insertItemOnTop( McWItem *_intert );
	void changeFocus( McWItem *_focusitem = NULL );
	McWItem *getFocusItem() { return focusItem; }
	void setFocusItem( McWItem *);
	void takeFocus();
	void loseFocus();
	void keyPress( WPARAM key, LPARAM lp = NULL );
	void nextPile( int delta );
	BOOL nextWindow( int delta );
	void killApp( McWItem * );
	McState removeKilledApp( McWItem *, char *, BOOL = TRUE );
	void reinitLayout( McState );
	void setLeftHanded( ) { leftHanded = TRUE; }
	BOOL getLeftHanded() { BOOL was=leftHanded; leftHanded = FALSE; return was; }
	int getImmersiveappOffset() { return immersiveAppOffset; }
	int getImmersiveAppBlank( ) { return immersiveAppBlank;	}
	void setImmersiveappBlank(  int blank ) {immersiveAppBlank = blank;	}
	int getImmersiveappGap() { return immersiveAppGap; }
	list<string> *getDesktopAppList() { return &desktopAppList; }
	McState cleanUp( McState );
	void handleNewWindow( HWND hwnd );
	void markVisibleW10Windows( HMONITOR monitor );
private:
	McWItem *verifyWindow( HWND, BOOL = FALSE );
	void handleNextWindow( HWND hwnd, BOOL isImmersive = FALSE );
	void handleNextDesktop( LPTSTR lpszDesktop );
	void layoutDesktop( McState );
	list<McWItem*> itemList;
	list<McWItem*>::iterator forwardIter;
	list<McWItem*>::reverse_iterator backwardIter;
	void buildWindowList();					// Build List
	McWItem** desktopItem;
	McWItem*	focusItem;					// Thumbnail with the focus
	BOOL isProcessed;
	POINT borderSize;
	BOOL leftHanded;
	int minWin;
	int immersiveAppBlank;
	int immersiveAppOffset;
	int immersiveAppGap;
	list<string> desktopAppList;

private:

	// Callback from the Request to Enumerate Desktop Windows
	
	static BOOL CALLBACK myWindowEnumCallback( HWND hwnd, LPARAM obj );
	static BOOL CALLBACK myDtIconSearchCallback( HWND, LPARAM );

	// Callback from the Request to Enumerate Desktop Windows
	
	static BOOL CALLBACK myImmersiveAppWindowEnumCallback( HWND hwnd, LPARAM obj )
	{
		McWList *windowList = (McWList *)obj;
		if ( windowList )
			windowList->handleNextWindow( hwnd, TRUE );
		return TRUE;
	}
};
