// Thank You Microsoft

#include "stdafx.h"
#include "Emcee.h"
#include "McAnimationMgr.h"

// Animation specific code

#pragma unmanaged
HWND McAnimationMgr::animatedHwnd = NULL;
IUIAnimationVariable2 *McAnimationMgr::variable = NULL;

McAnimationMgr::McAnimationMgr()
{
	animationManager = NULL;
	animationTimer = NULL;
	transitionLibrary = NULL;
	animationVariable = NULL;
}

McAnimationMgr::~McAnimationMgr()
{
	SafeRelease( &animationVariable );
	SafeRelease(&animationManager);
	SafeRelease(&animationTimer);
	SafeRelease(&transitionLibrary);
	animatedHwnd = NULL;
	variable = NULL;
}

HRESULT McAnimationMgr::Initialize()
{
	IUIAnimationTimerUpdateHandler *timerUpdateHandler = NULL;
	IUIAnimationTimerEventHandler  *timerEventHandler = NULL;

	 HRESULT hr = CoCreateInstance(
		CLSID_UIAnimationManager2,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_PPV_ARGS(&animationManager)
		);

	// Create Animation Timer
	if (SUCCEEDED(hr))
		hr = CoCreateInstance(
			CLSID_UIAnimationTimer,
			NULL,
			CLSCTX_INPROC_SERVER,
			IID_PPV_ARGS(&animationTimer)
		);

	// Create Animation Transition Library
	if ( SUCCEEDED( hr ) )
		hr = CoCreateInstance(
			CLSID_UIAnimationTransitionLibrary2,
			NULL,
			CLSCTX_INPROC_SERVER,
			IID_PPV_ARGS(&transitionLibrary)
			);

	// Connect the animation manager to the timer.
	// UI_ANIMATION_IDLE_BEHAVIOR_DISABLE tells the timer to shut itself
	// off when there is nothing to animate.
	if ( SUCCEEDED( hr ) )
		hr = animationManager->QueryInterface(
			IID_PPV_ARGS(&timerUpdateHandler) );

	if (SUCCEEDED(hr))
	{
		hr = animationTimer->SetTimerUpdateHandler(
			timerUpdateHandler, UI_ANIMATION_IDLE_BEHAVIOR_DISABLE);
		SafeRelease( &timerUpdateHandler );
	}

	// Create and set the Timer Event Handler
	if (SUCCEEDED(hr))
		hr = CTimerEventHandler::CreateInstance(
			&timerEventHandler );


	if (SUCCEEDED(hr))
	{
		hr = animationTimer->SetTimerEventHandler(
			timerEventHandler );
		SafeRelease( &timerEventHandler );
	}
	
	return hr;
}

HRESULT McAnimationMgr::doAnimate( HWND hwnd, double initialValue, double timeDivisor, double endingValue )
{

	McWindowMgr::getMainW( )->initLastValue( );
	McWindowMgr::getZoomW( )->initLastValue( );
	McWindowMgr::setHoverTime( 500 );
	SafeRelease( &animationVariable );

	HRESULT hr = animationManager->CreateAnimationVariable( initialValue, &animationVariable );

	if (endingValue < 0.0)
		endingValue = ( initialValue < 0.5 ? 1.0 : 0.0 );

	setAnimatedInfo( hwnd, animationVariable );

	IUIAnimationStoryboard2 *pStoryboard = NULL;
	IUIAnimationTransition2 *pTransition = NULL;
	UI_ANIMATION_SECONDS secondsNow;

	if (SUCCEEDED(hr))
		hr = animationManager->CreateStoryboard( &pStoryboard );

	if (SUCCEEDED( hr ))
	{
		hr = transitionLibrary->CreateCubicBezierLinearTransition
		( MC::getProperties( )->getAnimationSpeed( ) / timeDivisor, 
		 endingValue,
		 .25, .1, .25, 1.0,
		&pTransition);
		
	}
	
	if (SUCCEEDED(hr))
		hr = pStoryboard->AddTransition(
			animationVariable,pTransition );

	if (SUCCEEDED(hr))
		hr = animationTimer->GetTime( &secondsNow );

	if (SUCCEEDED(hr))
		hr = pStoryboard->Schedule(	secondsNow );

	SafeRelease( &pTransition );
	SafeRelease( &pStoryboard );

	return hr;
}