#include "stdafx.h"
#include "McBgW.h"
#include "McMainW.h"
#include "McWList.h"
#include "McWItem.h"
#include "Emcee.h"
#include "McModernAppMgr.h"
#include "McPropsMgr.h"
#include "McColorTools.h"	
#include "McButtonW.h"
#include "McWindowMgr.h"

#pragma unmanaged

// McBgW draws the underlying background, including lsettings icon, app bar, arrows,
// start menu.   That's all.   Well, almost all; the button windows are controlled by
// this guy and generally hover nearby.

McBgW::McBgW( ) : McBaseW( )
{
	isActivated = FALSE;
	arrowsNeeded = FALSE;
	currentArrowSize = 0;
	firstTime = TRUE;

	taskViewB = startB = leftArrowB = rightArrowB = NULL;

	desktopBgBrush = immersiveBgBrush = NULL;
	immersiveBgPen = NULL;    

	Bitmap *bm1, *bm2;

	forty = (int) (_DEFAULT_ICON_SIZE * McMonitorsMgr::getMainMonitor( )->getObjectScale( ));

	MC::loadBitmap( IDI_MODERN, &bm1 );
	MC::loadBitmap( IDI_MODERN, &bm2 );
	startB = McWindowMgr::allocateButtonW( L"Start", this, forty + 10, forty + 10, bm1, bm2, WMMC_DOSTART );

	MC::loadBitmap( IDI_VD1, &bm1 );
	MC::loadBitmap( IDI_VD2, &bm2 );
	taskViewB = McWindowMgr::allocateButtonW( L"TaskView", this, forty + 10, forty + 10, bm1, bm2, WMMC_DOTASKVIEW );

	MC::loadBitmap( IDI_LEFTA, &bm1 );
	MC::loadBitmap( IDI_LEFTA_S, &bm2 );
	leftArrowB = McWindowMgr::allocateButtonW( L"ShiftLeft", this, forty / 2, forty, bm1, bm2, WMMC_SHIFTLEFT );

	MC::loadBitmap( IDI_RIGHTA, &bm1 );
	MC::loadBitmap( IDI_RIGHTA_S, &bm2 );
	rightArrowB = McWindowMgr::allocateButtonW( L"ShiftRight", this, forty / 2, forty, bm1, bm2, WMMC_SHIFTRIGHT );

}

McBgW::~McBgW( )
{
	Deactivate( );
	McWindowMgr::freeButtonW( taskViewB );
	McWindowMgr::freeButtonW( startB );
	McWindowMgr::freeButtonW( leftArrowB );
	McWindowMgr::freeButtonW( rightArrowB );
	DestroyWindow( getHwnd( ) );
}

void McBgW::Activate( )
{
	if (!isActivated)
	{
		BOOL hwAccel = MC::getProperties()->getHardwareAcceleration();
		Create( L"BgW", NULL, hwAccel );
		DWORD xsettings = GetWindowLong( getHwnd( ), GWL_EXSTYLE );
		SetWindowLong( getHwnd( ), GWL_EXSTYLE, xsettings | WS_EX_LAYERED );
		SetLayeredWindowAttributes( getHwnd( ), McModernAppMgr::GetBgColor( ), 0, LWA_COLORKEY );
		createDrawingTools( );
		COLORREF bgColor = McModernAppMgr::GetBgColor( );
		startB->Activate( bgColor, McModernAppMgr::GetAppBarColor( ) );
		taskViewB->Activate( bgColor, McModernAppMgr::GetAppBarColor( ) );
#if MC_DESKTOPS
		McModernAppMgr::RefreshVirtualDesktopInfo( );
#endif
		arrowsNeeded = FALSE;
		currentArrowSize = 0;
		SetWindowText( getHwnd( ), L"BgW" );
		isActivated = TRUE;
	}

}

void McBgW::Deactivate( )
{
	if (isActivated)
	{
		isActivated = FALSE;
		ShowWindow( getHwnd( ), SW_HIDE );
		startB->Deactivate( );
		taskViewB->Deactivate( );
		leftArrowB->Deactivate( );
		rightArrowB->Deactivate( );
		deleteDrawingTools( );
		DestroyWindow( getHwnd( ) );
		m_hwnd = NULL;
	}
}

void McBgW::Show( HWND belowHwnd )
{
	if (isActivated)
	{
		McRect *r = McMonitorsMgr::getMainMonitor( )->getDesktopSize( );

		SetLayeredWindowAttributes( getHwnd( ), McModernAppMgr::GetBgColor( ), 0, LWA_COLORKEY );
		RedrawWindow( getHwnd( ), NULL, NULL, RDW_ERASE );
		SetWindowPos( getHwnd( ),
			belowHwnd, 
			r->left, r->top, r->getWidth( ), r->getHeight( ),
			SWP_NOACTIVATE );

	}
}

void McBgW::Hide( )
{
	ShowWindow( getHwnd(), SW_HIDE );
}

BOOL McBgW::createDrawingTools()
{
	COLORREF oldAppBarColor = appBarColor;
	COLORREF oldDtBgColor = dtBgColor;
	appBarColor = McModernAppMgr::GetAppBarColor();
	dtBgColor = McModernAppMgr::GetBgColor( );
	if (isActivated && (dtBgColor == 0 || (oldAppBarColor == appBarColor && oldDtBgColor == dtBgColor)))
		return FALSE;

	if ( immersiveBgBrush ) DeleteObject( immersiveBgBrush );
	immersiveBgBrush = 		CreateSolidBrush( appBarColor );

	if (immersiveBgPen) DeleteObject( immersiveBgPen );
	immersiveBgPen = CreatePen(PS_SOLID, 1, McModernAppMgr::GetPaletteColor(0) );

	if (desktopBgBrush) DeleteObject( desktopBgBrush );
	desktopBgBrush =	CreateSolidBrush( dtBgColor );

	return TRUE;
}

void McBgW::deleteDrawingTools( )
{
	if (immersiveBgBrush)
		DeleteObject( immersiveBgBrush );
	immersiveBgBrush = NULL;
	if (immersiveBgPen)
		DeleteObject( immersiveBgPen );
	immersiveBgPen = NULL;
	if (desktopBgBrush)
		DeleteObject( desktopBgBrush );
	desktopBgBrush = NULL;
}

LRESULT McBgW::HandleMessage( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if (uMsg == WM_SYSCOMMAND)
		return 0;
	
	switch (uMsg)
	{
	case WM_ERASEBKGND:
		break;

	case WM_DESTROY:
		return 0;

	case WM_PAINT:
		if (isActivated)
		{
			McMonitorsMgr *mainMonitor = McMonitorsMgr::getMainMonitor( );
			POINT *offsets = mainMonitor->getOffsets( );
			McRect mR( mainMonitor->getMonitorSize( ) );
			McRect dR( mainMonitor->getDesktopSize( ) );
			mR.offset( offsets->x, offsets->y );
			dR.offset( offsets->x, offsets->y );

			PAINTSTRUCT ps;
			HDC hdc = BeginPaint( m_hwnd, &ps );

			Graphics g( hdc );

			g.SetCompositingMode( CompositingModeSourceOver );

			McState state = MC::getState( );
			if (MC::getState( ) == MCS_Display || MC::getState( ) == MCS_TransitionDown)
			{
				if (MC::getMD( ) && !MC::getMM( ))
					PaintDesktop( hdc );
				McRect r (MC::getMM( ) ? &dR : &mR);
				r.right++;
				r.bottom++;
				FillRect( hdc, &r, desktopBgBrush );
			}

#if MC_DESKTOPS
			if (MC::getProperties( )->getShowDesktopList( ))
			{
				McModernAppMgr::McVirtualDesktopInfo *vdi = McModernAppMgr::GetVirtualDesktopInfo( );
				if (vdi->count > 1)
				{
					McRect bgR( MC::getMM( ) ? &dR : &mR );
					float objScale = mainMonitor->getObjectScale( );
					int rectWidth = (int)(130 * objScale);
					int rectHeight = (int)(32 * objScale);
					int spacing = (int)(10 * objScale);
					int totalWidth = vdi->count * (rectWidth + spacing) - spacing;
					int startX = bgR.left + (bgR.getWidth( ) - totalWidth) / 2;
					int startY = bgR.top + (int)(10 * objScale);

					SolidBrush normalBrush( Color( 140, 50, 50, 50 ) );
					SolidBrush currentBrush( Color( 200, 76, 194, 255 ) );
					SolidBrush textBrush( Color( 255, 255, 255, 255 ) );
					Pen currentPen( Color( 255, 76, 194, 255 ), 2.0f );
					Pen borderPen( Color( 120, 180, 180, 180 ), 1.0f );
					Gdiplus::Font font( L"Segoe UI", 9.5f * objScale, Gdiplus::FontStyleRegular, Gdiplus::UnitPoint );
					StringFormat sf;
					sf.SetAlignment( StringAlignmentCenter );
					sf.SetLineAlignment( StringAlignmentCenter );

					g.SetSmoothingMode( SmoothingModeAntiAlias );
					g.SetTextRenderingHint( TextRenderingHintAntiAlias );

					for (int i = 0; i < vdi->count; i++)
					{
						int x = startX + i * (rectWidth + spacing);
						RectF rect( (REAL)x, (REAL)startY, (REAL)rectWidth, (REAL)rectHeight );
						REAL radius = 4.0f * objScale;
						GraphicsPath path;
						path.AddArc( rect.X, rect.Y, radius * 2, radius * 2, 180, 90 );
						path.AddArc( rect.X + rect.Width - radius * 2, rect.Y, radius * 2, radius * 2, 270, 90 );
						path.AddArc( rect.X + rect.Width - radius * 2, rect.Y + rect.Height - radius * 2, radius * 2, radius * 2, 0, 90 );
						path.AddArc( rect.X, rect.Y + rect.Height - radius * 2, radius * 2, radius * 2, 90, 90 );
						path.CloseFigure( );

						if (i == vdi->currentIndex)
						{
							g.FillPath( &currentBrush, &path );
							g.DrawPath( &currentPen, &path );
						}
						else
						{
							g.FillPath( &normalBrush, &path );
							g.DrawPath( &borderPen, &path );
						}

						g.DrawString( vdi->names[i], -1, &font, rect, &sf, &textBrush );
					}
				}
			}
#endif

			if (appBarColor == 0)
			{
				EndPaint( m_hwnd, &ps );
				return 0;
			}

			int stripe = MC::getWindowList( )->getImmersiveappOffset( );

			if (stripe > 0 && MC::getState( ))
			{
				McRect sR( &mR );
				sR.top = MC::getWindowList( )->getImmersiveAppBlank( ) + sR.bottom - stripe;
				sR.bottom -= MC::getWindowList( )->getImmersiveappGap( ) -1;
				sR.right++;
				FillRect( hdc, &sR, immersiveBgBrush );
				SelectObject( hdc, immersiveBgPen );
				MoveToEx( hdc, sR.left, sR.top-1, NULL );
				LineTo( hdc, sR.right, sR.top-1 );
			}
			

			EndPaint( m_hwnd, &ps );

			return 0;
		}
		break;

	case WM_USER:

		if (isActivated)
		{
			if (wParam == WMMC_BGCHANGED)
			{
				onBgColorChanged( );
				raiseButtons( );
			}
			return 0;
		}
		break;

	default:
		break;
	}
	return DefWindowProc( m_hwnd, uMsg, wParam, lParam );
}

void McBgW::setArrowsNeeded( McRect *leftRect ) 
{
	if ( leftRect ) // ARROWS NEEDED
	{
		int width = (INT)(leftRect->getWidth() );
		int arrowSize = 2*width;

		if ( arrowsNeeded )	// No Change
			return;

		arrowsNeeded = TRUE;

		// Now calculate the actual positions

		McRect *mR = (McMonitorsMgr::getMainMonitor( )->getMonitorSize( ));

		int hadjustment = 20 +(width - arrowSize/2)/2;  
		int vadjustment = 10+(MC::getWindowList( )->getImmersiveAppBlank( )) / 2;
		leftArrowPos.x = leftRect->left + hadjustment;	
		leftArrowPos.y = leftRect->top + vadjustment;

		rightArrowPos.x = mR->getWidth( ) - forty / 2 - leftRect->left - hadjustment;
		rightArrowPos.y = leftArrowPos.y;

		leftArrowB->Activate( McModernAppMgr::GetAppBarColor( ) );
		leftArrowB->Show( leftArrowPos.x, leftArrowPos.y );

		rightArrowB->Activate( McModernAppMgr::GetAppBarColor( ) );
		rightArrowB->Show( rightArrowPos.x, rightArrowPos.y );

	}
	else
	{	// MESSAGE: no arrows needed
		arrowsNeeded = FALSE;
		currentArrowSize = 0;
		leftArrowB->Deactivate( );
		rightArrowB->Deactivate( );
	}

}

void McBgW::onBgColorChanged()
{
	if (createDrawingTools( ))
		doRepaint( FALSE );
}

void McBgW::doRepaint( BOOL positionButtons )
{
	InvalidateRect( getHwnd(), NULL, FALSE);
	UpdateWindow( getHwnd() );

	if (!isActivated) return;

	McMonitorsMgr *mainMonitor = McMonitorsMgr::getMainMonitor( );
	POINT *offsets = mainMonitor->getOffsets( );
	McRect mR( mainMonitor->getMonitorSize( ) );

	int w10offset = MC::getWindowList( )->getImmersiveAppBlank( );
	int stripe = MC::getWindowList( )->getImmersiveappOffset( );

	long bottom = mR.bottom;
	if (stripe > 0)
		bottom -= stripe;

	if ( startB ) 
		startB->Show( 
			mR.left, (stripe > 0 ? w10offset-11 : -9 ) + bottom - forty );
	if (taskViewB && !MC::inTabletMode( ) )
		taskViewB->Show( mR.left+mR.getWidth() - forty - 10, (stripe > 0 ? w10offset-11 : -9) + bottom - forty );
	
}

void McBgW::lowerButtons( )
{
	if (taskViewB && !MC::inTabletMode( )) 	taskViewB->Lower( );
	if (startB) startB->Lower( );
	if (leftArrowB && arrowsNeeded ) leftArrowB->Lower( );
	if (rightArrowB && arrowsNeeded ) rightArrowB->Lower( );
}
void McBgW::raiseButtons( )
{
	if (taskViewB && !MC::inTabletMode( )) taskViewB->Raise( );
	if (startB) startB->Raise( );
	if (leftArrowB && arrowsNeeded ) leftArrowB->Raise( );
	if (rightArrowB && arrowsNeeded ) rightArrowB->Raise( );

}
void McBgW::hideButtons( )
{
	if (taskViewB && !MC::inTabletMode( )) taskViewB->Hide( );
	if (startB) startB->Hide( );
	if (leftArrowB) leftArrowB->Hide( );
	if (rightArrowB) rightArrowB->Hide( );
}
