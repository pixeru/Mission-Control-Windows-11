#pragma once
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Mission Control for Windows 11 by pixeru
// Source code originally by Emcee (https://sourceforge.net/projects/mcsoft/)
// Extensive development by pixeru due to discontinued development of original project.
// Copyright (c) pixeru. All rights reserved.
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
