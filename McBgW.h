#pragma once
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
