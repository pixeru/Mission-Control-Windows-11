#pragma once
#include "stdafx.h"
#include "Emcee.h"
#include "McBaseW.h"
#include "McWItem.h"

// Window underneath a thumbnail in thumbnail view

#pragma unmanaged

class McDesktopW : public McBaseW<McDesktopW>
{
public:

	McDesktopW();
	~McDesktopW();
	void Activate( McWItem *owner, McRect *rect );
	void Deactivate( );
	DWORD runThread( ) { return 0; }
	void setPosition( BOOL show, McRect * = NULL );
	PCWSTR ClassName() const { return L"Emcee.DesktopW"; }
	LRESULT HandleMessage( UINT uMsg, WPARAM wParam, LPARAM lParam );
	McWItem *getOwner() { return owner; }

private:

	void onDestroyWindow();
	McRect ourRect;
	McWItem *owner;
};
