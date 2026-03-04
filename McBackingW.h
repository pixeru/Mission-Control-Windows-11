#pragma once
#include "McBaseW.h"

#pragma unmanaged

class McBackingW : public McBaseW<McBackingW>
{
public:

	McBackingW();
	~McBackingW();
	PCWSTR ClassName() const { return L"Emcee.BackingW"; }
	LRESULT HandleMessage( UINT uMsg, WPARAM wParam, LPARAM lParam );
	DWORD runThread() { return 0; }

	void Activate( );
	void Deactivate( );

};
