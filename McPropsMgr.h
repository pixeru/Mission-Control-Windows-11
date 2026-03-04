#pragma once
#include "stdafx.h"
#include "Emcee.h"
#include "McHotKeyMgr.h"

#pragma managed(push,off)

// Declaration of classes for handling properties

using namespace std;

// Defines for hot corner location: 0 means hot corner is off

#define MCA_UPPERLEFT	0x0001
#define MCA_UPPERRIGHT	0x0002
#define MCA_LOWERLEFT	0x0004
#define MCA_LOWERRIGHT	0x0008

// Group for combining different apps into a pile

class McGroup
{
public:

	McGroup( char *_groupName );
	McGroup( McGroup *_group );
	~McGroup();
	void add( char *_progName );
	string groupName;
	list<string> stringList;
};

// Properties data structure

struct McPropData
{
	McPropData()
	{
		multiMonitor		= TRUE;
		mouseCornerActivate	= 0;
		disableLabels		= TRUE;
#if MC_DESKTOPS
		showAllDesktops		= FALSE;
#endif
		useAppBar			= TRUE;
		immersiveTileHeight	= 1.5;
		labelFontSize		= 23;
		thumbnailSpacing	= .3125;
		dtAutoColor			= TRUE;
		dtBgColor			= RGB( 141, 141, 141 );
		hotKeyCode			= _HK_DEFAULT;
		animationSpeed		= 5;
		hardwareAcceleration= TRUE;
		stackWindows		= TRUE;

		HKEY rootKey;
		animationEffects = TRUE;
		if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Control Panel\\Desktop\\WindowMetrics", 0, KEY_QUERY_VALUE, &rootKey) == ERROR_SUCCESS)
		{
			wchar_t value[256] = { 0 };
			DWORD size = sizeof(256*sizeof(wchar_t));
			if (RegQueryValueExW(rootKey, L"MinAnimate", NULL, NULL, (LPBYTE)&value, &size) == ERROR_SUCCESS)
			{
				animationEffects = (value[0] != L'0');
			}
			RegCloseKey(rootKey);
		}
	}

	~McPropData()
	{
		while (masterList.size()>0)
		{
			list<McGroup *>::iterator it = masterList.begin();
			McGroup *group = *it;
			masterList.remove( group );
			delete group;
		}
	}

	McPropData *clone()
	{
		McPropData *nd = new McPropData();
		nd->set( this );
		return nd;
	}

	void set( McPropData *d )
	{
		multiMonitor = d->multiMonitor;
		disableLabels = d->disableLabels;
		mouseCornerActivate = d->mouseCornerActivate;
		hotKeyCode = d->hotKeyCode;
		immersiveTileHeight = d->immersiveTileHeight;
		labelFontSize = d->labelFontSize;
		thumbnailSpacing = d->thumbnailSpacing;
		dtBgColor = d->dtBgColor;
		dtAutoColor = d->dtAutoColor;
		animationSpeed = d->animationSpeed;
		animationEffects = d->animationEffects;
		hardwareAcceleration = d->hardwareAcceleration;
		stackWindows = d->stackWindows;
#if MC_DESKTOPS
		showAllDesktops = d->showAllDesktops;
#endif
		useAppBar = d->useAppBar;
		masterList.clear();
		for ( list<McGroup *>::iterator it = d->masterList.begin(); it!= d->masterList.end(); it++ )
		{
			McGroup *ng = new McGroup( *it );
			masterList.push_back( ng );
		}
		excludeList = d->excludeList;
	}

	BOOL			multiMonitor;
	BOOL			disableLabels;
	int				mouseCornerActivate;
	long			hotKeyCode;
#if MC_DESKTOPS
	BOOL			showAllDesktops;
#endif
	BOOL			useAppBar;
	double			immersiveTileHeight;
	int				labelFontSize;
	double			thumbnailSpacing;
	BOOL			dtAutoColor;
	COLORREF		dtBgColor;
	int				animationSpeed;
	BOOL			animationEffects;
	BOOL			hardwareAcceleration;
	BOOL			stackWindows;
	list<McGroup*>	masterList;
	list<string>	excludeList;
};

// Properties file class

class McPropsMgr
{
public:

	McPropsMgr();
	~McPropsMgr();

	void reread();
	void write();

	void setData( McPropData *_data )
	{
		data->set( _data );
	}

	McPropData* getData() 
	{
		return data;
	}

	BOOL getMultiMonitor();
	BOOL getDisableLabels()			{ return data->disableLabels; }
#if MC_DESKTOPS
	BOOL getShowAllDesktops( )		{ return data->showAllDesktops; }
#endif
	BOOL getUseAppBar( )			{ return data->useAppBar;	}
	BOOL getHardwareAcceleration( )	{ return data->hardwareAcceleration; }
	BOOL getStackWindows( )			{ return data->stackWindows; }
	int  getMouseCornerActivate()	{ return data->mouseCornerActivate; }
	long getHotKeyCode()			{ return data->hotKeyCode; }

	double		getAppBarTileHeight()	{ return data->immersiveTileHeight; }
	int			getLabelFontSize()		{ return data->labelFontSize; }
	COLORREF	getDtBgColor()			{ return data->dtBgColor; }
	BOOL		getDtAutoColor( )		{ return data->dtAutoColor;	}
	double		getAnimationSpeed()		
	{ 
		if (!data->animationEffects) return 0.0001;
		return
			.65*( .8 - (data->animationSpeed - 1)*0.1);
			//.92*pow( .8 - (data->animationSpeed - 1)*0.1, 1.25 );
	}

	double		getThumbnailSpacing( );
	BOOL		getIsExcluded( const char* );
	McGroup*	getGroup( char * );
	const char* getGroupName( const char*, BOOL = FALSE );

private:

	McPropData	*data;

	void read();
	char *unspace(char*);
	char *scanValue();
	char *scanGroup();
	char *scanExclude();
	char *token_holder;

};

#pragma managed(pop)
