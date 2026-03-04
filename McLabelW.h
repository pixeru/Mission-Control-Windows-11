#pragma once
#include "stdafx.h"
#include "Emcee.h"
#include "McBaseW.h"

#pragma unmanaged

class McWItem;
class McThumbW;
class McMonitorsMgr;

#define _MCLABEL_DISPLAYX	 0x01
#define _MCLABEL_DISPLAYTEXT 0x02
#define _MCLABEL_FADEIN		 0x04
#define _MCLABEL_DISPLAYICON 0x08
#define _MCLABEL_DISPLAYALL  0x0F

class McLabelW : public McBaseW<McLabelW>
{
public:

	McLabelW();
	~McLabelW();
	void Activate( );
	void Deactivate( );
	void Show( McThumbW * );
	void Hide();
	void Pause( ) { paused = TRUE; }
	void Resume(  ) { paused = FALSE; }
	BOOL getPaused( ) {	return paused; }
	LRESULT HandleMessage( UINT, WPARAM, LPARAM );
	static void NewLabelFontSize();

private:

	PCWSTR	ClassName() const { return L"Emcee.LabelW"; };
	DWORD runThread() { return 0; }
	void doMeasurements();
	HFONT labelFont;
	McRect labelDimensions;
	int lineHeight;
	int labelPadding;
	COLORREF labelColor;
	Gdiplus::Color *bgColor;
	Gdiplus::SolidBrush *solidBrush;
	Gdiplus::SolidBrush *textBrush;
	float textWidth;
	int vCenterOffset;
	Gdiplus::Image *blackImage;
	Gdiplus::Image *redImage;
	UINT displayMask;
	McRect xRect;
	HICON hicon;
	int xsize;
	BOOL haveMouse;
	BOOL isVisible;
	BOOL mouseWasOverX;
	BOOL mouseButtonDown;
	int isize;
	McThumbW *thumbWindow;
	McWItem  *windowItem;
	McMonitorsMgr *monitor;
	BOOL paused;
	BOOL disabled;
	static BOOL		labelFontStale;
};
