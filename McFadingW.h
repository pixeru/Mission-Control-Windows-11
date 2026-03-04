#pragma once
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
