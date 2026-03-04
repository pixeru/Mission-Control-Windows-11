#pragma once
#include "stdafx.h"
#include <Threadpoolapiset.h>

#define _MPM_ROOT_DIR L"C:\\Program Files\\WindowsApps\\"
#define _MPM_ROOT_KEY L"Software\\Classes\\ActivatableClasses\\App"

#define _MPM_TIMER_MINUTES (5LL) /* 5 minutes */

using namespace std;
using namespace Gdiplus;

class McAppData
{
public:

	McAppData( );
	~McAppData( );
	BOOL loadIcon( );
	McAppData *clone( );
	void setColor( WCHAR * );
	void setDisplayName( WCHAR * );
	void setIconFileName( WCHAR * );
	WCHAR *getIconFileName( ) { return iconFileName; }
	WCHAR *getDisplayName( ){ return displayName; }
	Color *getColor( ){ return &color; }
	Bitmap *getImage( ){ return myImage; }
	void clear( BOOL = TRUE );
	BOOL isComplete( )
	{
		return displayName && iconFileName;
	}
	static WCHAR *wdup( WCHAR * );

private:

	Gdiplus::Color color;
	WCHAR *displayName;
	WCHAR *iconFileName;
	Gdiplus::Bitmap *myImage;
};

struct McAppxInfo
{

	WCHAR *Package;
	WCHAR *Program;
	WCHAR *IconPath;
	static list<McAppxInfo *>appxList;
	static void add( McAppxInfo *info )
	{
		appxList.push_back( info );
	}

	static McAppxInfo *find( WCHAR *package, WCHAR *program )
	{
		for (list<McAppxInfo *>::iterator it = appxList.begin( ); it != appxList.end( ); it++)
		{
			McAppxInfo *info = *it;
			if (!wcsstr( package, info->Package )) continue;
			if (!wcsstr( program, info->Program )) continue;
			return info;
		}
		return NULL;
	}

	static McAppxInfo *findLong( WCHAR *package, WCHAR *program )
	{
		WCHAR pack[1000];
		WCHAR newpack[1000];
		WCHAR *pp = NULL;
		size_t len = wcslen( package );
		wsprintf( pack, L"%s", package );
		size_t i;
		for (i = 0; i < len; i++)
			if (pack[i] == L'_')
			{
				pack[i] = 0;
				pp = &pack[i + 1];
				break;
			}	
		for (i = len - 1; i > 0; i--)
			if (pack[i] == L'_')
				break;

		if (i > 0)
			pp = &pack[i + 1];

		wsprintf( newpack, L"%s_%s", pack, pp );
		return find( newpack, program );
	}

	McAppxInfo( )
	{
		Package = Program = IconPath = NULL;
	}

	void set( WCHAR *package, WCHAR *program )
	{
		setPackage( package );
		setProgram( program );
		setIconPath( NULL );
	}

	~McAppxInfo( )
	{
		if (Package) delete Package;
		if (Program) delete Program;
		if (IconPath) delete IconPath;
	}

	McAppxInfo *clone( )
	{
		McAppxInfo *newInfo = new McAppxInfo( );
		newInfo->set( Package, Program );
		newInfo->setIconPath( IconPath );
		return newInfo;
	}

	void setPackage( WCHAR *p );
	void setProgram( WCHAR *p );
	void setIconPath( WCHAR *p );
};

class McIconMgr
{
public:
	McIconMgr( );
	~McIconMgr( );

	void addApp( McAppData *data );
	BOOL getReady( ) { return mgrReady; }
	list <McAppData *> *getAppList( );
	void Stop( );
	BOOL getAppLogo( WCHAR *appName, Bitmap **returnBitmap, Color **returnColor ); // Don't delete returned values
	static void DrawFromBitmap( Graphics *g, Color*c, Bitmap*b, UINT x, UINT y, UINT size );
	static void DrawFromIcon( Graphics *g, HICON h, UINT x, UINT y, UINT size );
	static HICON GetWindowIcon( HWND ); 

private:

	BOOL scanApps( );
	void scanAppX( );
	void analyzeAppX( WCHAR * );
	int  newApps;
	BOOL examinePackage( WCHAR * );
	BOOL examineProgram( WCHAR *, WCHAR *, WCHAR * );
	BOOL decodeResource( WCHAR *, WCHAR * );
	BOOL findColor( WCHAR *, WCHAR *, WCHAR * );
	BOOL getColor( WCHAR *, WCHAR * );
	McAppData *findAppByDisplayName( WCHAR *displayName, BOOL = FALSE );
	BOOL lccmp( WCHAR *, WCHAR * );
	McAppData workingData;
	McAppxInfo workingInfo;
	BOOL healthy;
	list <McAppData *> appList;
	BOOL mgrTerminating;
	BOOL mgrReady;
	HANDLE	myThread;		// Thread used to scan packages
	HANDLE	myThreadEvent;	// Event used to tell thread to scan packages
	HANDLE	myThreadMutex;	// Mutex used to protect package list
	TP_TIMER *myThreadTimer;// Timer used to reschedule package scanning
	void signalThread( );
	void obtainListLock( WCHAR *);
	void releaseListLock( WCHAR *);
	static void CALLBACK myThreadTimerProc( PTP_CALLBACK_INSTANCE, PVOID, PTP_TIMER );
	static DWORD CALLBACK myThreadProc( LPVOID object );
};
