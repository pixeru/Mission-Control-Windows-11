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
#include "McIconMgr.h"
#include "McModernAppMgr.h"
#include "McMonitorsMgr.h"

// Builds and maintains a set of icon images to use on windows' labels in
// thumbnail mode.  Works for both old-style desktop applications and
// "modern" windows-store apps (mostly).   

// For "modern" apps, there is probably an better way
// to do this, but I couldn't find any documentation of one so it was
// accomplished by extrating info from the registry.

#define _ROOTPATH L"Extensions\\ContractId\\Windows.Launch\\PackageId"
#define _ROOTPATH2 L"Local Settings\\Software\\Microsoft\\Windows\\CurrentVersion\\AppModel\\SystemAppData"

list< McAppxInfo*> McAppxInfo::appxList;

void McIconMgr::myThreadTimerProc( PTP_CALLBACK_INSTANCE, PVOID _object, PTP_TIMER )
{
	McIconMgr *object = (McIconMgr *) _object;
	if ( !object->mgrTerminating )
		object->signalThread( );
}

#pragma comment( lib, "gdiplus.lib" )
#pragma comment( lib, "Shlwapi.lib" )

using namespace Gdiplus;

void McAppxInfo::setPackage( WCHAR *p )
{
	if (Package) delete Package;
	Package = McAppData::wdup( p );
}
void McAppxInfo::setProgram( WCHAR *p ) 
{
	if (Program) delete Program;
	Program = McAppData::wdup( p );
}
void McAppxInfo::setIconPath( WCHAR *p ) 
{
	if (IconPath) delete IconPath;
	IconPath = McAppData::wdup( p );
}

McAppData::McAppData( )
{
	clear( FALSE );
}

McAppData::~McAppData( )
{
	clear( );
}

void McAppData::clear( BOOL doDelete )
{
	if (doDelete)
	{
		if (displayName) delete displayName;
		if (iconFileName) delete iconFileName;
		if (myImage) delete myImage;
	}
	displayName = iconFileName = NULL;
}

McAppData *McAppData::clone( )
{
	McAppData *r = new McAppData( );
	r->displayName = wdup( displayName );
	r->iconFileName = wdup( iconFileName );
	r->color = color;
	if (myImage)
	{
		r->myImage = myImage;
		myImage = NULL;
	}
	return r;
}

WCHAR *McAppData::wdup( WCHAR *wc )
{
	WCHAR *res = NULL;
	if (wc)
	{
		size_t len = wcslen( wc );
		res = new WCHAR[len + 1];
		wsprintf( res, L"%s", wc );
	}
	return res;
}

void McAppData::setIconFileName( WCHAR *lName )
{
	if (iconFileName) delete iconFileName;
	iconFileName = wdup( lName );
}

void McAppData::setColor( WCHAR *cstring )
{
	DWORD xcolor = std::wcstol( ++cstring, NULL, 16 );
	if (xcolor == 0x0078D7 || xcolor == 0xFFFFFF ) 
		xcolor = 0x000000;
	if (GetRValue( xcolor ) > 0xD0 && GetGValue( xcolor ) > 0xD0 && GetBValue( xcolor ) > 0xD0)
		color = 0xFF000000;
	else
		color.SetFromCOLORREF( 0xFF000000 | RGB( GetBValue( xcolor ), GetGValue( xcolor ), GetRValue( xcolor ) ) );
}

void McAppData::setDisplayName( WCHAR *nstring )
{
	if( displayName ) delete displayName;
	displayName = wdup( nstring );
}

BOOL McAppData::loadIcon( )
{

	if (myImage) return TRUE;
	
	myImage = Gdiplus::Bitmap::FromFile( getIconFileName(), FALSE ); 
	
	if (!myImage)
		return FALSE;

	if (myImage->GetWidth( ) < 12)
	{
		delete myImage;
		myImage = NULL;
		return FALSE;
	}

	return TRUE;

}

McIconMgr::McIconMgr( )
{
	mgrReady = FALSE;
	mgrTerminating = FALSE;

	myThreadEvent = CreateEvent( NULL, TRUE, FALSE, TEXT( "myThreadEvent" ));
	myThreadMutex = CreateMutex( NULL, FALSE, TEXT( "myThreadMutex" ));
	myThreadTimer = CreateThreadpoolTimer( myThreadTimerProc, this, NULL );

	myThread = CreateThread( NULL, 0, McIconMgr::myThreadProc, this, 0, NULL );

	Sleep( 1 );

	signalThread( );
}

McIconMgr::~McIconMgr( )
{
	mgrTerminating = TRUE;
	signalThread( );
	Sleep( 10 );
	obtainListLock(L"~McIconMgr" );

	for (list<McAppData *>::iterator it = appList.begin( ); it != appList.end( ); it++)
	{
		delete (*it);
		*it = NULL;
	}

	releaseListLock(L"!McIconMgr" );

	Sleep( 1 );
	
	CloseHandle( myThreadMutex );
	CloseHandle( myThreadEvent );

}

DWORD McIconMgr::myThreadProc( LPVOID _object )
{
	McIconMgr *object = (McIconMgr *) _object;

	while (!object->mgrTerminating)
	{
		DWORD hr = WaitForSingleObject( object->myThreadEvent, INFINITE );
		switch (hr)
		{
		case WAIT_OBJECT_0:
			break;
		default:
			return 0;
		}

		ResetEvent( object->myThreadEvent );

		if ( !object->mgrTerminating )
			object->scanApps( );
	}

	return 0;
}

void McIconMgr::signalThread( )
{
	if (myThreadEvent) SetEvent( myThreadEvent );
}

BOOL McIconMgr::decodeResource( WCHAR *what, WCHAR *resource )
{
	if (resource[0] != L'@')
		return TRUE;
	WCHAR buff[1000] = { 0 };
	HRESULT res = SHLoadIndirectString( resource, buff, 1000, NULL );
	if (res == ERROR_SUCCESS)
		wsprintf( resource, L"%s", buff );

	return (res == ERROR_SUCCESS);
}

BOOL McIconMgr::getColor( WCHAR *path, WCHAR *appx )
{
	HKEY rootKey = NULL;
	LONG res;
	WCHAR buff[1000];
	wsprintf( buff, L"%s\\%s", path, appx );
	res = RegOpenKeyEx( HKEY_CLASSES_ROOT,
		buff, 0,
		KEY_QUERY_VALUE, &rootKey );

	if (res == ERROR_SUCCESS)
	{
		WCHAR color[1000] = { 0 };
		DWORD size = 1000;
		DWORD type = REG_SZ;
		res = RegQueryValueEx( rootKey, L"BackgroundColor", NULL, &type, (BYTE *) color, &size );
		if (res == ERROR_SUCCESS)
			workingData.setColor( color );
		RegCloseKey( rootKey );
		rootKey = NULL;
	}

	return res == ERROR_SUCCESS;
}

BOOL McIconMgr::findColor( WCHAR *pstub, WCHAR *program, WCHAR *subkey )
{
	HKEY rootKey = NULL;
	LONG res;

	WCHAR path[1000];
	wsprintf( path, L"%s\\%s\\SplashScreen", _ROOTPATH2, subkey );
	WCHAR stub[1000];
	wsprintf( stub, L"!%s", program );

	res = RegOpenKeyEx( HKEY_CLASSES_ROOT,
		path, 0,
		KEY_ENUMERATE_SUB_KEYS, &rootKey );

	if (res == ERROR_SUCCESS)
	{
		int i;
		for (i = 0; i < 1000 && res != ERROR_NO_MORE_ITEMS; i++)
		{
			WCHAR subkey[1000] = { 0 };
			DWORD subkeysize = 1000;
			res = RegEnumKeyEx( rootKey, i, subkey, &subkeysize, NULL, NULL, NULL, NULL );
			if (res == ERROR_SUCCESS)
			{
				if (wcsstr( subkey, stub ))
				{
					if (!getColor( path, subkey ))
						res = ERROR_APP_DATA_NOT_FOUND;
					break;
				}
			}
		}
		if (i == 1000 )
			res = ERROR_NO_MORE_ITEMS;
		RegCloseKey( rootKey );
		rootKey = NULL;
	}

	return res == ERROR_SUCCESS;

}

BOOL McIconMgr::lccmp( WCHAR *str, WCHAR *substr )
{
	size_t len = wcslen( substr );
	if (len > (size_t)wcslen(str))
		return FALSE;
	for (int i = 0; i < len; i++)
		if (tolower( str[i] ) != tolower( substr[i] ))
			return FALSE;
	return TRUE;
}


BOOL McIconMgr::examineProgram( WCHAR *pname, WCHAR *root, WCHAR *node )
{
	HKEY rootKey = NULL;
	LONG res;

	WCHAR searchKey[1000];
	wsprintf( searchKey, L"%s\\%s", root, node );

	res = RegOpenKeyEx( HKEY_CLASSES_ROOT,
		searchKey, 0,
		KEY_QUERY_VALUE, &rootKey );

	if (res == ERROR_SUCCESS)
	{
		WCHAR resource[1000] = { 0 };
		DWORD size = 1000;
		DWORD type = REG_SZ;
		res = RegQueryValueEx( rootKey, L"DisplayName", NULL, &type, (BYTE *) resource, &size );
		if (res == ERROR_SUCCESS)
		{
			if (!decodeResource( L"NAME", resource ))
			{
				RegCloseKey( rootKey );
				return FALSE;
			}
			if (!findAppByDisplayName( resource ))
				workingData.setDisplayName( resource );
			else
			{
				RegCloseKey( rootKey );
				return FALSE;
			}
		}
		else
			return FALSE;

		McAppxInfo *info = McAppxInfo::findLong( pname, node );
		if (info)
			workingData.setIconFileName( info->IconPath );
		else
		{
			size = 1000;
			WCHAR newResource[1000];
			wsprintf( newResource, L"%s", resource );
			res = RegQueryValueEx( rootKey, L"Icon", NULL, &type, (BYTE *) resource, &size );
			if (res == ERROR_SUCCESS)
			{
				if (!decodeResource( L"ICON", resource ))
				{
					RegCloseKey( rootKey );
					return FALSE;
				}
				workingData.setIconFileName( resource );
			}
			else
			{
				RegCloseKey( rootKey );
				return FALSE;
			}
		}
		RegCloseKey( rootKey );
		rootKey = FALSE;
	}
	else
		return FALSE;

	WCHAR program[1000];
	wsprintf( program, node );
	WCHAR *p = wcsstr( program, L".wwa" );
	if (p) *p = 0;

	// program now has <pgm>

	WCHAR pstub[1000];
	wsprintf( pstub, L"%s", pname );
	p = wcsstr( pstub, L"_" );
	if (p) *p = 0;

	rootKey = 0;
	res = RegOpenKeyEx( HKEY_CLASSES_ROOT,
		_ROOTPATH2, 0,
		KEY_ENUMERATE_SUB_KEYS, &rootKey );

	if (res == ERROR_SUCCESS)
	{
		int i;
		WCHAR subkey[1000] = { 0 };
		for (i = 0; i < 1000 && res != ERROR_NO_MORE_ITEMS; i++)
		{
			DWORD subkeysize = 1000;
			res = RegEnumKeyEx( rootKey, i, subkey, &subkeysize, NULL, NULL, NULL, NULL );
			if (res == ERROR_SUCCESS)
			{
				if (lccmp( subkey, pstub ))
				{
					if (!findColor( pstub, program, subkey ))
						res = ERROR_APP_DATA_NOT_FOUND;
					break;
				}
			}
		}
		RegCloseKey( rootKey );
		rootKey = NULL;
	}
	return res == ERROR_SUCCESS;
}

BOOL McIconMgr::examinePackage( WCHAR *pName )
{
	HKEY rootKey = NULL;
	LONG res;
	WCHAR buff[1000];
	wsprintf( buff, L"%s\\%s\\ActivatableClassId", _ROOTPATH, pName );

	res = RegOpenKeyEx( HKEY_CLASSES_ROOT,
		buff, 0,
		KEY_ENUMERATE_SUB_KEYS, &rootKey );

	if (res == ERROR_SUCCESS)
	{
		int i;
		for (i = 0; i < 1000 && res != ERROR_NO_MORE_ITEMS; i++)
		{
			WCHAR subkey[1000] = { 0 };
			DWORD subkeysize = 1000;
			res = RegEnumKeyEx( rootKey, i, subkey, &subkeysize, NULL, NULL, NULL, NULL );
			if (res == ERROR_SUCCESS)
			{
				if (examineProgram(pName, buff, subkey))
					if (workingData.isComplete())
						if (workingData.loadIcon())
							addApp(workingData.clone());
						else
							workingData.clear();
			}
			else
				break;
		}

		RegCloseKey( rootKey );
		rootKey = NULL;
		return ( res == ERROR_SUCCESS || res == ERROR_NO_MORE_ITEMS );
	}
	else
		return FALSE;
}

void McIconMgr::analyzeAppX( WCHAR *appx )
{
	HKEY rootKey = NULL;
	LONG res;
	WCHAR buff[1000], buff1[1000];
	wsprintf( buff, L"%s\\Application", appx );
	res = RegOpenKeyEx( HKEY_CLASSES_ROOT,
		buff, 0,
		KEY_QUERY_VALUE, &rootKey );
	if (res == ERROR_SUCCESS)
	{
		DWORD size = 1000;
		DWORD type = REG_SZ;
		res = RegQueryValueEx( rootKey, L"AppUserModelID", NULL, &type, (BYTE *) buff, &size );
		if (res == ERROR_SUCCESS)
		{
			WCHAR bb[1000];
			wsprintf( bb, L"%s", buff );
			WCHAR *p = wcschr( bb, L'!' );
			if (p)
			{
				*p = 0;
				if (!McAppxInfo::find( bb, ++p ))
				{
					workingInfo.set( bb, p );
					size = 1000;
					res = RegQueryValueEx( rootKey, L"ApplicationIcon", NULL, &type, (BYTE *) buff1, &size );
					if (res == ERROR_SUCCESS)
					{
						if (McIconMgr::decodeResource( NULL, buff1 ))
						{
							workingInfo.setIconPath( buff1 );
							McAppxInfo::add( workingInfo.clone() );
						}
					}
				}
			}
			RegCloseKey( rootKey );
			rootKey = NULL;
		}
	}
}

void McIconMgr::scanAppX( )
{
	size_t oldCount = McAppxInfo::appxList.size( );
	HKEY rootKey = NULL;
	LONG res;
	res = RegOpenKeyEx( HKEY_CLASSES_ROOT,
		NULL, 0,
		KEY_QUERY_VALUE, &rootKey );

	if (res == ERROR_SUCCESS)
	{
		BOOLEAN found = FALSE;
		WCHAR subkey[1000] = { 0 };
		DWORD subkeysize;
		int i = 0;
		while (TRUE)
		{
			i++;
			subkeysize = 1000;
			res = RegEnumKeyEx( rootKey, i, subkey, &subkeysize, NULL, NULL, NULL, NULL );
			if (res == ERROR_SUCCESS)
			{
				if (wcslen( subkey ) > 5)
					if (0 == wcsncmp( L"AppX", subkey, 4 ))
					{
					analyzeAppX( subkey );
					found = TRUE;
					}
					else
						if (found) break;
			}
			if (res == ERROR_NO_MORE_ITEMS) break;
		}
		size_t newCount = McAppxInfo::appxList.size( ) - oldCount;
		getchar( );
		RegCloseKey( rootKey );
		rootKey = NULL;
	}

}

BOOL McIconMgr::scanApps( )
{

	scanAppX( );

	HKEY rootKey = NULL;
	LONG res;

	res = RegOpenKeyEx( HKEY_CLASSES_ROOT,
		_ROOTPATH, 0,
		KEY_ENUMERATE_SUB_KEYS, &rootKey );

	if (res == ERROR_SUCCESS)
	{
		int i;
		for (i = 0; i < 1000 && res != ERROR_NO_MORE_ITEMS; i++)
		{
			WCHAR subkey[1000] = { 0 };
			DWORD subkeysize = 1000;
			res = RegEnumKeyEx( rootKey, i, subkey, &subkeysize, NULL, NULL, NULL, NULL );
			if (res == ERROR_SUCCESS)
				examinePackage( subkey );
		}
		RegCloseKey( rootKey );
		rootKey = NULL;
	}

	if (!mgrTerminating)
	{
		FILETIME FileDueTime = { 0 };
		ULARGE_INTEGER ulDueTime = { 0 };
		ulDueTime.QuadPart = (ULONGLONG)-(_MPM_TIMER_MINUTES * 10 * 60 * 1000 * 1000);
		FileDueTime.dwHighDateTime = ulDueTime.HighPart;
		FileDueTime.dwLowDateTime = ulDueTime.LowPart;
		SetThreadpoolTimer( myThreadTimer, &FileDueTime, 0, 0 );
	}

	mgrReady = TRUE;
	return res == ERROR_SUCCESS;
}

McAppData *McIconMgr::findAppByDisplayName( WCHAR *_displayName, BOOL deepSearch )
{
	McAppData *result = NULL;
	obtainListLock( L"findAppByDisplayName");
	for (list<McAppData *>::iterator it = appList.begin( ); it != appList.end( ); it++)
		if (!wcscmp( _displayName, (*it)->getDisplayName( ) ))
		{
		result = *it;
		break;
		}
	if (deepSearch && !result)
	{
		for (list<McAppData *>::reverse_iterator it = appList.rbegin( ); it != appList.rend( ); it++)
			if (!wcsncmp( _displayName, (*it)->getDisplayName( ), wcslen( _displayName ) ))
			{
				result = *it;
				break;
			}
	}
	releaseListLock( L"findAppByDisplayName");
	return result;
}

void McIconMgr::addApp( McAppData *data )
{
	if (data)
	{
		obtainListLock( L"addApp" );
		appList.push_back( data );
		newApps++;
		releaseListLock( L"addApp");
	}
}

void McIconMgr::obtainListLock( WCHAR *where )
{
	DWORD res = WaitForSingleObject( myThreadMutex, INFINITE );
}

void McIconMgr::releaseListLock( WCHAR *where )
{
	if (myThreadMutex)
		ReleaseMutex( myThreadMutex );
}

list <McAppData *> *McIconMgr::getAppList( )
{
	return &appList;
}

void McIconMgr::Stop( )
{
	mgrTerminating = TRUE;
}

void McIconMgr::DrawFromBitmap( Graphics *g, Color *c, Bitmap*b, UINT x, UINT y, UINT size )
{
	g->DrawImage( (Image *) (b), (INT)x, (INT)y, size, size );
}

void McIconMgr::DrawFromIcon( Graphics *g, HICON h, UINT x, UINT y, UINT size )
{
	DrawIconEx( g->GetHDC( ), x, y, h, size, size, 0,
		NULL, DI_COMPAT | DI_NORMAL );
}

HICON McIconMgr::GetWindowIcon( HWND hwnd )
{
	HICON hicon = (HICON) (SendMessage( hwnd, WM_GETICON, ICON_BIG, NULL ));

	if (!hicon)
		hicon = (HICON) GetClassLongPtr( hwnd, GCLP_HICON );

	if (!hicon)
	{
		DWORD processId;
		GetWindowThreadProcessId( hwnd, &processId );
		HANDLE handle = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId );	
		if (handle)
		{
			WCHAR fileName[1000] = { 0 };
			DWORD sz = 1000;
			if (0 < QueryFullProcessImageName( handle, 0, fileName, &sz ))
			{
				HICON iconLarge;
				HICON iconSmall;
				ExtractIconEx( fileName, 0, &iconLarge, &iconSmall, 1 );
				if (iconLarge)
				{
					hicon = iconLarge;
					DestroyIcon( iconSmall );
				}
				else
					if (iconSmall)
						hicon = iconSmall;
			}
			CloseHandle( handle );
		}
	}

	if ( !hicon )
		hicon = LoadIcon( GetModuleHandle(NULL), IDI_APPLICATION );

	return hicon;
}

BOOL McIconMgr::getAppLogo( WCHAR *appName, Bitmap **returnBitmap, Color **returnColor )
{
	if (!returnBitmap || !returnColor)
		return FALSE;

	*returnBitmap = NULL;
	*returnColor = NULL;

	McAppData *data = NULL;
	
	data = findAppByDisplayName( appName, TRUE );
	if (!data)
		return FALSE;

	(*returnBitmap) = data->getImage( );
	(*returnColor) = data->getColor( );

	if (*returnBitmap == NULL) return FALSE;

	return TRUE;

}




