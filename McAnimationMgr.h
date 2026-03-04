#pragma once
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Mission Control for Windows 11 by pixeru
// Source code originally by Emcee (https://sourceforge.net/projects/mcsoft/)
// Extensive development by pixeru due to discontinued development of original project.
// Copyright (c) pixeru. All rights reserved.#include "stdafx.h"

#include "UIAnimationHelper.h"
#include "Emcee.h"
#include "McMainW.h"
#include "McZoomW.h"
#include "McBackingW.h"
#include "McPropsMgr.h"
#include "McWindowMgr.h"

#pragma unmanaged

template <class T> void SafeRelease( T **ppT )
{
	if (*ppT)
	{
		(*ppT)->Release();
		*ppT = NULL;
	}
}

class McAnimationMgr
{
public:

	McAnimationMgr();
	HRESULT Initialize();
	HRESULT doAnimate( HWND, double, double = 1.0, double = -1.0 );
	~McAnimationMgr();

private:

	IUIAnimationManager2 *animationManager;
	IUIAnimationTimer *animationTimer;
	IUIAnimationTransitionLibrary2 *transitionLibrary;

	// Animated Variables

	IUIAnimationVariable2 *animationVariable;

public:

	static HWND getAnimatedHwnd()
	{
		return animatedHwnd;
	}

	static IUIAnimationVariable2 *getVariable()
	{
		return variable;
	}

private:

	static void setAnimatedInfo( HWND _animatedHwnd, IUIAnimationVariable2 *_variable )
	{
		animatedHwnd = _animatedHwnd;
		variable = _variable;
	}
	static HWND animatedHwnd;
	static IUIAnimationVariable2 *variable;
};

class CTimerEventHandler :
	public CUIAnimationTimerEventHandlerBase<CTimerEventHandler>
{
public:

	static HRESULT CreateInstance (
		IUIAnimationTimerEventHandler **ppTimerEventHandler ) throw()
	{
		CTimerEventHandler *timerEventHandler;
		HRESULT hr = CUIAnimationCallbackBase::CreateInstance(
			ppTimerEventHandler, &timerEventHandler );
		return hr;
	}

	// IUIAnimationTimerEventHandler

	IFACEMETHODIMP OnPreUpdate()
	{
		return 0;
	}

	IFACEMETHODIMP OnPostUpdate()
	{
		HWND hwnd = McAnimationMgr::getAnimatedHwnd();

		double value;

		McAnimationMgr::getVariable()->GetValue(&value);

		if (McWindowMgr::getMainW()->getHwnd() == hwnd)
		{
			McWindowMgr::getMainW()->calcFrame(value);
			RedrawWindow(hwnd, NULL, NULL, RDW_UPDATENOW | RDW_INVALIDATE);
		}
		else if (McWindowMgr::getZoomW( ) )
			if (McWindowMgr::getZoomW( )->getHwnd( ) == hwnd)
			{
				McWindowMgr::getZoomW( )->calcFrame( value );
				RedrawWindow(McWindowMgr::getBackingW()->getHwnd(), NULL, NULL, RDW_UPDATENOW | RDW_INVALIDATE );
				RedrawWindow(hwnd, NULL, NULL, RDW_UPDATENOW | RDW_INVALIDATE);
			}

		return S_OK;
	}

	IFACEMETHODIMP OnRenderingTooSlow( UINT32 fps )
	{
		return S_OK;
	}

protected:

	CTimerEventHandler()
	{
	}

};
