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


#pragma unmanaged

using namespace std;

class McMonitorsMgr
{
public:

	McMonitorsMgr(HMONITOR);
	~McMonitorsMgr();
	POINT*		getMonitorCenter( )		{ return &this->mdCenter; }
	McRect*		getMonitorSize()		{ return &this->monitorSize; }
	McRect*		getMonitorWorkSize( )	{ return &this->monitorWorkSize;	}
	McRect*		getDesktopSize( )		{ return &this->dtR; }
	POINT*		getOffsets( )			{ return &this->mdOffset;	}
	float		getDpi()				{ return monitorDpi; }
	float		getObjectScale( )		{ return monitorObjectScale; }
	float		getScale()				{ return scaleFactor; }
	float		getAspect( )			{ return aspect;	}
	int			getOrdinal( )			{ return ordinal;	}
	int			getMaxPileWidth( )		{ return maxPileWidth; }
	int			getMaxPileHeight( )		{ return maxPileHeight;	}
	HMONITOR	getHMONITOR()			{ return hMonitor;  }
	BOOL		getIsMainMonitor()		{ return isMainMonitor;  }

	void setMaxPilesDimension( int width, int height )
	{
		maxPileWidth = width;
		maxPileHeight = height;
	}

	// Static

	static void				DeleteMonitorInformation	( ); 
	static void				CreateMonitorInformation( BOOL = FALSE );
	static McMonitorsMgr*	getMainMonitor()			{ return mainMonitor; }
	static McMonitorsMgr*	getMonitor( HMONITOR );
	static BOOL				getIsTouchscreen( )			{ return isTouchScreen; }
	static size_t			getMonitorCount( )			{return monitorCount; }
	static size_t			getEffectiveMonitorCount( ) { return effectiveMonitorCount; }
	static void				setEffectiveMonitorCount( );
	static list<McMonitorsMgr*> monitorList;
	static BOOL				myGetMonitorInfo(HMONITOR, MONITORINFO*);

private:

	HMONITOR			hMonitor;		// Monitor Handle
	int					ordinal;		// ordinal: 0, 1, 2, ... in sort order
	McRect				monitorSize;	// Coordinates of Monitor corners
	McRect				monitorWorkSize;// Monitor Work Area
	BOOL				isMainMonitor;	// TRUE if it is the primary monitor
	float				monitorDpi;		// dots per inch on this monitor
	float				monitorObjectScale; // Scale factor for painted objects on background
	float				scaleFactor;	// scale factor on this monitor
	float				aspect;			// aspect for this monitor
	POINT				mdOffset;		// Defined only for main monitor
	POINT				mdCenter;		// Centerpoint
	McRect				dtR;			// Defined only for main monitor
	int					maxPileWidth;
	int					maxPileHeight;
	// Static
	static bool _mcomp( McMonitorsMgr *, McMonitorsMgr * );
	static McMonitorsMgr* mainMonitor;
	static BOOL CALLBACK monitorCallback(HMONITOR, HDC, LPRECT, LPARAM);
	static BOOL monitorsCreated;
	static BOOL isTouchScreen;
	static size_t  monitorCount;
	static size_t  effectiveMonitorCount;
};