#pragma once
#include "stdafx.h"
#pragma unmanaged

using namespace std;

#define MOD_NONE 0x0000

#define	_HK_ID							0x564
#define _HK_COMPRESS( modifier, vkey )	(10*(vkey)+(modifier))
#define _HK_MODIFIER( hk)				( (hk)%10 )
#define _HK_VKEY( hk )					( (hk)/10 )
#define _HK_DEFAULT						_HK_COMPRESS( MOD_CONTROL, VK_TAB )
#define _MC_KEY_COUNT 6

class McHotKeyMgr
{
private:

	static list<HWND>hotKeyStack;
	static BOOL doRegister( HWND, UINT );
	static void doUnregister( HWND );
	static INPUT _in[_MC_KEY_COUNT];
	static void updateHotKeyData();
	
public:

	static UINT keyValues[];
	static char *keyStrings[];
	static BOOL pushHotKey( HWND  );
	static void popHotKey( HWND );
	static void reRegister();
	static BOOL testHotKey( UINT );
	static int	getIndexFromCode( UINT );
	static UINT getCodeByIndex( int );
	static char *getStringFromCode( UINT );
	static void createHotKeyData();
	static int injectHotKey();
	static int injectPositionedHotKey( POINT *p );
	static size_t getStackSize( );
	static size_t popAll( );

};
