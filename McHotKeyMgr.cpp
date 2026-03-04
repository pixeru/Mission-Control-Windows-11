#include "stdafx.h"
#include "Emcee.h"
#include "McHotKeyMgr.h"
#include "McModernAppMgr.h"

// Code related to customizing and handling hot keys.

#pragma unmanaged

INPUT McHotKeyMgr::_in[_MC_KEY_COUNT];
list<HWND> McHotKeyMgr::hotKeyStack;
UINT McHotKeyMgr::keyValues[] = { 
	0x08, 0x09, 0x0D, 0x13, 0x21, 
	0x22, 0x23, 0x24, 0x25, 0x26,
	0x27, 0x28, 0x29, 0x2A, 0x2B, 
	0x2D, 0x2E, 0x2F, 0x30, 0x31, 
	0x32, 0x33, 0x34, 0x35, 0x36, 
	0x37, 0x38, 0x39, 0x41, 0x42, 
	0x43, 0x44, 0x45, 0x46, 0x47, 
	0x48, 0x49, 0x4A, 0x4B, 0x4C, 
	0x4D, 0x4E, 0x4F, 0x50, 0x51, 
	0x52, 0x53, 0x54, 0x55, 0x56, 
	0x57, 0x58, 0x59, 0x5A, 
	
	0xC0, 0xDB, 0xDD, 0xDE, 0xDC, 
	0xBF, 0xBA, 0xBB, 0xBD, 0xBE, 0xBC,
	
	0x60, 
	0x61, 0x62, 0x63, 0x64, 0x65, 
	0x66, 0x67, 0x68, 0x69, 
	0x6F, 0x6A, 0x6D, 0x6B, 0x6E,

	0x70, 0x71, 0x72, 0x73, 0x74, 
	0x75, 0x76, 0x77, 0x78, 0x79, 
	0x7A, 0x7B
	};

char *McHotKeyMgr::keyStrings[] = {
	"BKSPACE",	"TAB",	"ENTER",	"SPACE",	"PGUP",
	"PGDN",		"END",	"HOME",		"LEFT",		"UP",
	"RIGHT",	"DOWN",	"SELECT",	"PRINT",	"EXE",
	"INS",		"DEL",	"HELP",		"0",		"1",
	"2",		"3",	"4",		"5",		"6",
	"7",		"8",	"9",		"A",		"B",
	"C",		"D",	"E",		"F",		"G",
	"H",		"I",	"J",		"K",		"L",
	"M",		"N",	"O",		"P",		"Q",
	"R",		"S",	"T",		"U",		"V",
	"W",		"X",	"Y",		"Z",	
	"`~",		"[{",		"]}",		"'\"",	"\\|",
	"/?",		";:",		"+=",		"-_",	".>",	",<",
	"NUM 0",	"NUM 1",	"NUM 2",	"NUM 3",	"NUM 4",
	"NUM 5",	"NUM 6",	"NUM 7",	"NUM 8",	"NUM 9",
	"NUM /",	"NUM *",	"NUM -",	"NUM +",	"NUM .",
	"F1",	"F2",	"F3",	"F4",	"F5",
	"F6",	"F7",	"F8",	"F9",	"F10",
	"F11",	"F12"
	};

BOOL McHotKeyMgr::pushHotKey( HWND hwnd  )
{

	UINT value = MC::getProperties( )->getHotKeyCode( );
	if ( hotKeyStack.size() > 0 )
		doUnregister( hotKeyStack.front());
		
	if ( doRegister( hwnd, value ) )
	{
		hotKeyStack.push_front( hwnd );
		return TRUE;
	}
	else
	{
		if ( hotKeyStack.size() > 0 )
			doRegister( hotKeyStack.front(), value );
	}

	return FALSE;
}

size_t McHotKeyMgr::popAll( )
{
	size_t i = hotKeyStack.size( );
	while (hotKeyStack.size( ) > 0)
	{
		doUnregister( hotKeyStack.front( ) );
		hotKeyStack.pop_front( );
	}
	return i;
}

void McHotKeyMgr::popHotKey( HWND hwnd )
{
	if (hotKeyStack.size( ) == 0)
		return;

	if (hotKeyStack.front( ) != hwnd)
		return;

	doUnregister( hwnd );
	hotKeyStack.remove( hwnd );

	if ( hotKeyStack.size() == 0 ) return;
	hwnd = hotKeyStack.front();
	doRegister( hwnd, MC::getProperties()->getHotKeyCode() );

	return;
}

BOOL McHotKeyMgr::doRegister( HWND hwnd, UINT value )
{
	UINT modifier = MOD_NOREPEAT | _HK_MODIFIER( value );
	UINT code = _HK_VKEY( value );
	BOOL outcome = RegisterHotKey( hwnd, _HK_ID, modifier, code );
	return outcome;
}

void McHotKeyMgr::doUnregister( HWND hwnd )
{
	UnregisterHotKey( hwnd, _HK_ID );
}

void McHotKeyMgr::reRegister()
{
	if ( !hotKeyStack.size() ) return;

	HWND hwnd = hotKeyStack.front();
	doUnregister( hwnd );
	doRegister( hwnd, MC::getProperties()->getHotKeyCode() );
}

size_t McHotKeyMgr::getStackSize( )
{
	return hotKeyStack.size( );
}

BOOL McHotKeyMgr::testHotKey( UINT value )
{
	HWND oldHwnd = NULL;
	if ( hotKeyStack.size() > 0 )
	{
		oldHwnd = hotKeyStack.front();
		doUnregister( oldHwnd );
	}

	BOOL outcome = FALSE;

	HWND newHwnd = oldHwnd == 0 ? MC::_eventHwnd : oldHwnd;

	if ( doRegister( newHwnd, value ) )
	{
		doUnregister( newHwnd );
		outcome = TRUE;
	}
	
	if ( oldHwnd )
		doRegister( oldHwnd, MC::getProperties()->getHotKeyCode() );

	return outcome;
}

int McHotKeyMgr::getIndexFromCode( UINT keyCode )
{
	for ( int i=0;i<99;i++ )
		if ( keyValues[i] == keyCode ) return i;
	return 1;
}

char *McHotKeyMgr::getStringFromCode( UINT keyCode )
{
	for ( int i=0;i<99;i++ )
		if ( keyValues[i] == keyCode ) return keyStrings[i];
	return NULL;
}

UINT McHotKeyMgr::getCodeByIndex( int index )
{
	return keyValues[index];
}

void McHotKeyMgr::updateHotKeyData()
{
	long hotKeyCode = MC::getProperties()->getHotKeyCode();
	_in[1].ki.wVk = _in[2].ki.wVk = (WORD)_HK_VKEY( hotKeyCode );
	UINT hotKeyModifier = _HK_MODIFIER( hotKeyCode );
	if ( hotKeyModifier == MOD_NONE )
		_in[0].ki.wVk = _in[3].ki.wVk = 0;
	else if ( hotKeyModifier & MOD_CONTROL )
		_in[0].ki.wVk = _in[3].ki.wVk = VK_CONTROL;
	else if ( hotKeyModifier & MOD_ALT )
		_in[0].ki.wVk = _in[3].ki.wVk = VK_MENU;
	else if ( hotKeyModifier & MOD_SHIFT )
		_in[0].ki.wVk = _in[3].ki.wVk = VK_SHIFT;
	
}

void McHotKeyMgr::createHotKeyData()
{
	memset( _in, 0, _MC_KEY_COUNT*sizeof( INPUT ) );

	_in[0].type = _in[1].type = _in[2].type = _in[3].type = INPUT_KEYBOARD;
	_in[0].ki.wVk = _in[3].ki.wVk = VK_CONTROL;
	_in[1].ki.wVk = _in[2].ki.wVk = VK_TAB;
	_in[2].ki.dwFlags = _in[3].ki.dwFlags = KEYEVENTF_KEYUP;
	_in[4].type = _in[5].type = INPUT_MOUSE;
	_in[4].mi.dwFlags = MOUSEEVENTF_MIDDLEDOWN;
	_in[5].mi.dwFlags = MOUSEEVENTF_MIDDLEUP;
}

int McHotKeyMgr::injectHotKey()
{
	updateHotKeyData();
	SendInput( _MC_KEY_COUNT-2, _in, sizeof( INPUT ) );
	return 0;
}

int McHotKeyMgr::injectPositionedHotKey( POINT *p )
{
	p->y += 10;
	_in[4].mi.dx = _in[5].mi.dx = p->x;
	_in[4].mi.dy = _in[5].mi.dy = p->y;
	SendInput( 2, &_in[_MC_KEY_COUNT-2], sizeof(INPUT) );

	injectHotKey();
	return 0;
}
