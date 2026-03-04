#pragma once
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Mission Control for Windows 11 by pixeru
// Source code originally by Emcee (https://sourceforge.net/projects/mcsoft/)
// Extensive development by pixeru due to discontinued development of original project.
#include "stdafx.h"
#include "McBaseW.h"

#pragma unmanaged
class McFadingW : public McBaseW<McFadingW>
{
public:

	McFadingW( );
	~McFadingW( );
	PCWSTR ClassName( ) const { return L"Emcee.FadingW"; }
	LRESULT HandleMessage( UINT uMsg, WPARAM wParam, LPARAM lParam );
	DWORD runThread( ) { return 0; }
	void Activate( );
	void Deactivate( );

};