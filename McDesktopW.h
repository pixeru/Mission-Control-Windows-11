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
