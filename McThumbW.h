#pragma once
#include "stdafx.h"
#include "Emcee.h"
#include "McBaseW.h"
#include "McWItem.h"

// Window underneath a thumbnail in thumbnail view

#pragma unmanaged

class McWItem;
class McMainW;

class McThumbW : public McBaseW<McThumbW>
{
public:

	McThumbW( );
	~McThumbW( );

	void Activate( McWItem * );
	void Deactivate( );

	PCWSTR ClassName( ) const { return L"Emcee.ThumbW"; }

	LRESULT HandleMessage( UINT uMsg, WPARAM wParam, LPARAM lParam );
	DWORD	runThread( ) 
			{ return 0; }
	void	onSelectWindow( BOOL isCtrl, BOOL doFinish = TRUE );
	void	onSelectWindowTM( BOOL doFinish = TRUE );
	void	onMoveToBottom( McState transition = MCS_TransitionOver );
	void	immersiveAppMoveToBottom( );
	void	immersiveAppMoveToTop( );
	void	onZoom( BOOL isSuper );
	void	onFastZoom( );
	McWItem *getOwner( )
			{ return owner; }
	void createLabel( );
	WCHAR *getLabels( BOOL original )
	{
		WCHAR *lab = original ? rawLabels : labels;
		WCHAR *newLabels = new WCHAR[wcslen( lab ) + 1];
		wcscpy_s( newLabels, 1+wcslen( lab ), lab );
		return newLabels;
	}
	void setLabels( WCHAR *_labels )
	{
		if (labels)
			delete labels;
		labels = new WCHAR[wcslen( _labels ) + 1];
		wcscpy_s( labels, 1+wcslen( _labels ), _labels );
	}

private:

	BOOL haveMouse;
	void onDestroyWindow( );
	HWND topWindow;
	void onTouch( TC_Message * );
	void onMouseWheel( WPARAM, LPARAM );
	WCHAR *labels;
	WCHAR *rawLabels;
	McWItem *owner;
};
