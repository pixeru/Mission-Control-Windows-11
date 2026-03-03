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

// These classes are used to layout thumbnails on the display.  The same code is used
// for laying out piles in the main display, or zoomed thumbnails in zoomed mode.    
// Once the piles are layed out, position & pile information is transfered
// to the window objects and these layout objects are destroyed.

// Default stuff

#define APPBAR_THUMBHEIGHT_INCHES			(MC::getProperties()->getAppBarTileHeight())
#define APPBAR_THUMB_H_SPACING_INCHES		(APPBAR_THUMBHEIGHT_INCHES*0.06)
#define APPBAR_THUMB_V_SPACING_INCHES		(APPBAR_THUMBHEIGHT_INCHES*0.06)
#define DEFAULT_H_DKTP_BG_PADDING_INCHES	(0)
#define	DEFAULT_V_APPBAR_BG_PADDING_INCHES	(APPBAR_THUMB_H_SPACING_INCHES)
#define DEFAULT_V_DEAD_SPACE                (DEFAULT_V_APPBAR_BG_PADDING_INCHES/2)
#define _MC_BAR_GAP_PERCENT .66

// Special Pile Names

#define _MODERN_APP_PILE_NAME	"_MODERN_APP_PILE" /* Pile name used for app bar */
#define _W10_APP_PILE_NAME	"_W10_APP_PILE"

/////////////////////////////////////////////////////////////////////
//                    SINGLE PILE ITEM CODE                        //
/////////////////////////////////////////////////////////////////////

// Window included in a pile (may be the only one)

class PileItem
{
public:

	PileItem( McWItem *_wItem );
	~PileItem();
	void reScale( size_t );
	McRect		originalR;
	McRect		boundedR;	
	McWItem*	wItem;
};

class McWList;

/////////////////////////////////////////////////////////////////////
//                      SINGLE PILE CODE                           //
/////////////////////////////////////////////////////////////////////

// Collection of one or more windows included in a pile

class McPile
{
public:

	McPile( McMonitorsMgr *_displayMonitor, const char * = NULL );
	~McPile( );
	void add( McWItem *wItem );
	void doLayout( );				// Lay out the desktop items in the pile
	void doImmersiveAppLayout( );	// Lay out the immersiveApp items in the pile
	void assignZorder( );			// Give zorders to Immersive apps in this pile
	void positionThumbs( double _sRatio, int, int );	// Position thumbnails
	void positionZooms( double _sRatio, int, int );		// Position zooms
	BOOL matches( const char * );
	void reScale( size_t );
	void scale( float );
	McRect	boundingR;		// (0,0) origin bounding rectangle for the pile.
	McRect	placementR;		// Rectangle occupied by placed tile;
	BOOL	filledRight;	// TRUE when the space to the right has been filled.
	BOOL	filledBelow;	// TRUE when the space to the bottom has been filled.
	int		getPileNumber() { return pileNumber; }
	void	setPileNumber( int _pileNumber ) { pileNumber = _pileNumber; }
	void	setPileNumbers();
	char*	pName;
	int		row;
	long	deadSpace;
	double	sRatio;
	list<PileItem *>iList;
	list<PileItem *>::iterator it;
	McWList *wList;
	int			pileNumber;
	BOOL		needArrows;
	McRect		leftArrowRect;
	int			appGap;
	McMonitorsMgr *displayMonitor;
};

/////////////////////////////////////////////////////////////////////
//                 COLLECTION OF PILES CODE                        //
/////////////////////////////////////////////////////////////////////

// Collection of thumbnail piles for a single monitor

class McPilesMgr
{
	
public:

	McPilesMgr( int, HMONITOR );
	~McPilesMgr();
	void createPiles();
	void layoutPiles( BOOL = TRUE );
	void positionThumbs( );
	void createZoomPiles( list<McWItem *>* );
	void positionZooms();
	int getImmersiveAppOffset( )
	{
		return immersiveAppOffset;
	}
	int getImmersiveAppGap( )
	{
		return immersiveAppGap;
	}
	BOOL McPilesMgr::areArrowsNeeded( McRect * );

private:

	list<McPile*>			pList;
	list<McPile*>::iterator	it;
	McPile *immersiveAppPile;
	long immersiveAppOffset;
	long immersiveAppGap;
	McMonitorsMgr *monitor;
	McMonitorsMgr *mainMonitor;
	BOOL		   isMainMonitor;
	McRect placementR;
	McRect boundingR;
	BOOL checkOverlap( McRect * );
	void tuneLayout( BOOL );
	void doAppBarCalculations( );
	void placeWindowPile( McPile * );
	void moveItUp( McPile * );
	void moveItLeft( McPile * );
	LONG wSpacing;
	double mdAspect;
	double sRatio;
	list<McPile*> placedPiles;

public:

	static int				getNextPileNumber() { return nextPileNumber++; }
	static int				getNPiles( ) { return nPiles; }
	static void				setNPiles( int _npiles ) { nPiles = _npiles; }

private:
	
	static int nextPileNumber;
	static int nPiles;

};

