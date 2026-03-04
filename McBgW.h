#pragma once
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Mission Control for Windows 11 by pixeru
// Source code originally by Emcee (https://sourceforge.net/projects/mcsoft/)
// Extensive development by pixeru due to discontinued development of original project.
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
	McButtonW *leftArrowB;
	McButtonW *rightArrowB;
};