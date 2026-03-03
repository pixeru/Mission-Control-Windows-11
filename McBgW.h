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
#include "Emcee.h"
#include "McBaseW.h"

#pragma unmanaged

using namespace Gdiplus;

// McBgW is bottommost window -- paints logos, app bar, etc.

class McButtonW;

class McBgW : public McBaseW<McBgW>
{
public:

	McBgW();
	~McBgW();
	DWORD runThread(){ return 0;  };
	void Activate( );
	void Deactivate( );
	void Show( HWND belowThisHwnd );
	void Hide( );
	void setArrowsNeeded( McRect * );
	BOOL getArrowsNeeded( ) { return arrowsNeeded;	}
	PCWSTR  ClassName( ) const { return L"Emcee.BgW"; }
	void calcFrame( double value ) {}
	void onBgColorChanged();
	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	void doRepaint( BOOL positionButtons );
	void lowerButtons( );
	void raiseButtons( );
	void hideButtons( );
	

private:
	
	BOOL firstTime;
	int forty;
	BOOL arrowsNeeded;
	int  currentArrowSize;
	POINT leftArrowPos, rightArrowPos;
	HBRUSH desktopBgBrush;
	HBRUSH immersiveBgBrush;
	HPEN immersiveBgPen;
	COLORREF iconBgColor;
	COLORREF appBarColor;
	COLORREF dtBgColor;
	BOOL createDrawingTools( );
	void deleteDrawingTools( );
	McButtonW *taskViewB;
	McButtonW *startB;
	McButtonW *settingsB;
	McButtonW *leftArrowB;
	McButtonW *rightArrowB;
};