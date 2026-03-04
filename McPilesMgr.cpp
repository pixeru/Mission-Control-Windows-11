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
#include "McMainW.h"
#include "McWList.h"
#include "McWItem.h"
#include "McPropsMgr.h"
#include "McMonitorsMgr.h"

#include "McPilesMgr.h"

// Creates and lays out piles of thumbs or individual thumbs.   The same logic is used for 
// piles and for zoomed piles.  If each window is given it's own McPile instead of it's own PileItem
// in a grouped McPile then you have expose instead of mission control like layout.

#pragma unmanaged
#define PI 3.141592654	/* Accurate for our purposes, working out displacement on an oval */

using namespace std;

// Static variables in the class

int McPilesMgr::nextPileNumber = 1;
int McPilesMgr::nPiles = 0;

/////////////////////////////////////////////////////////////////////
//                    SINGLE PILE ITEM CODE                        //
/////////////////////////////////////////////////////////////////////

PileItem::PileItem( McWItem *_wItem )
{
	wItem = _wItem;

	originalR.set( 0, 0,
		wItem->getNormalRect( )->getWidth( ), wItem->getNormalRect( )->getHeight( ) );
}

void PileItem::reScale( size_t n )
{
	if (!wItem->getIsOnAppBar( ))
	{
		double X = wItem->getNormalRect( )->getArea( ) / (double) (McMonitorsMgr::getMainMonitor( )->getMonitorSize( )->getArea( ));
		double sFactor = 0.61*pow( 1.64293, (1.0 - X) );
		double yFactor = max( 0.0, 1.0 - (n - 1)*.25 );
		sFactor = yFactor + (1 - yFactor)*sFactor;
		originalR.scale( sFactor );
	}
}


PileItem::~PileItem()
{
}

/////////////////////////////////////////////////////////////////////
//                      SINGLE PILE CODE                           //
/////////////////////////////////////////////////////////////////////

McPile::McPile( McMonitorsMgr *_displayMonitor, const char *_pName )
{
	if ( !_pName ) _pName = "zoom";
	pName = new char[ strlen( _pName ) + 1];
	strcpy_s( pName, strlen( _pName )+1 , _pName );
	filledRight = filledBelow = FALSE;
	row = 1;
	pileNumber = 0;  // Will set this once layed out
	needArrows = FALSE;
	displayMonitor = _displayMonitor;
}

McPile::~McPile()
{
	delete pName;
	for ( it=iList.begin(); it!=iList.end();it++ )
		delete *it;
}

void McPile::reScale( size_t nItems )
{
	for (it = iList.begin( ); it != iList.end( ); it++)
		(*it)->reScale( nItems );
}

BOOL McPile::matches( const char *_pName )
{
	return ( 0 == strcmp( pName, _pName ) );
}

void McPile::add( McWItem *wItem )
{
	PileItem *pItem = new PileItem( wItem );
	iList.push_back( pItem );
}

void McPile::doImmersiveAppLayout()  // Layout the app bar
{
	BOOL leftHanded = MC::getWindowList()->getLeftHanded();
	
	double nItem = (double)iList.size();			// Number of immersiveApp items.
	float dpi = displayMonitor->getDpi( );	// Pixels per inch

	long HpadSize = (LONG)( .5+dpi * APPBAR_THUMB_H_SPACING_INCHES );
	long VpadSize = (LONG)floor( .5+dpi * APPBAR_THUMB_V_SPACING_INCHES );
	long thumbHeight = (LONG) floor( .5+dpi * APPBAR_THUMBHEIGHT_INCHES );
	int	 barHeight = thumbHeight + 2*VpadSize;
	
	deadSpace = (LONG) floor(.5+ dpi*DEFAULT_V_DEAD_SPACE );

	placementR.set( displayMonitor->getMonitorSize( ) );
	placementR.top = displayMonitor->getMonitorSize()->getHeight() - barHeight;

	int blank = (int) floor( _MC_BAR_GAP_PERCENT*barHeight ); // 
	int forty = (int) (_DEFAULT_ICON_SIZE * McMonitorsMgr::getMainMonitor( )->getObjectScale( ));

	if (barHeight - blank < 10 + forty )
		blank = max( 0, barHeight - forty - 10 );
	MC::getWindowList( )->setImmersiveappBlank( blank );
	long HoutsideSpace = (LONG) (_DEFAULT_ICON_PADDING * McMonitorsMgr::getMainMonitor( )->getObjectScale( ));

	list<PileItem *>::reverse_iterator rit;
	long currentLeft = 0;

	long maxWidthWide = (LONG) floor( .5 + displayMonitor->getMonitorSize( )->getWidth( ) - (2*HpadSize + HoutsideSpace ) );
	long maxWidthNarrow = maxWidthWide - 2 * (HoutsideSpace-VpadSize);

	// maxWidthWide is the amount of space for thumbs if arrows are not needed
	// maxWidthNarrow is the amount of space for thumbs if arows are needed
	
	// First, assign every window a position assuming 0 left origin

	for ( rit = iList.rbegin(); rit != iList.rend(); rit++ )
	{
		McWItem *item = (*rit)->wItem;
		McRect   *rect = item->getNormalRect();
		
		if ( rit != iList.rbegin() )
			currentLeft += HpadSize;

		// Get thumbnail width

		long thumbWidth = (LONG)floor( 1.0+thumbHeight * ((double)(rect->getWidth())) / rect->getHeight() );
		McRect *r = item->getThumbRect();

		r->left = placementR.left + currentLeft;
		r->top = placementR.top + VpadSize;
		r->right = r->left + thumbWidth; 
		r->bottom = r->top + thumbHeight; 

		currentLeft += thumbWidth;
	}

	int hiddenCount = 0;

	// Now go through and figure out which ones fit.
	// 
	// r->right <= maxWidthNarrow -> it fits always
	// r->right > maxWidthNarrow  && r->right <= maxWidthWide -> 
	//    it fits if it is the last one

	BOOL isFull = FALSE;
	for ( rit = iList.rbegin(); rit != iList.rend(); rit++ )
	{
		McWItem	   *item = (*rit)->wItem;
		McRect     *tr = item->getThumbRect();
		BOOL		firstTime = item->getThumbWindow() == NULL;
		BOOL		wasVisible = item->getVisibility();
		long		midPoint = (tr->top + tr->bottom )/2;

		BOOL itFits = FALSE;
		if (tr->right <= maxWidthNarrow) 
			itFits = TRUE;
		else
		{
			if (!isFull)
			{
				rit++;
				if (tr->right <= maxWidthWide && rit == iList.rend( ))
					itFits = TRUE;
				else
					isFull = TRUE;
				rit--;
			}
		}

		if ( !itFits )	// THIS THUMB DOESN'T FIT AND WON'T BE VISIBLE
		{
			hiddenCount++;
			tr->top = tr->bottom = midPoint;
			if ( firstTime || (!firstTime && !wasVisible) )
			{
				item->setVisibility( FALSE, FALSE );
				tr->left = tr->right = leftHanded? placementR.left : placementR.right;
			}
			else
			{
				item->setVisibility( TRUE, TRUE );
				tr->left = tr->right = leftHanded ? placementR.right : placementR.left;
			}
		}
		else	// THIS THUMBNAIL FITS AND WILL BE DISPLAYED
		{
			currentLeft = tr->right;
			item->setVisibility( TRUE, FALSE );
			if ( !wasVisible )
			{
				long edge = leftHanded ? placementR.left : placementR.right;
				item->getStartRect()->set( edge, midPoint, edge, midPoint );
			}
		}
	
	}

	appGap = 0; 

	if ( hiddenCount == 0 ) 
	{
		size_t totalWidth = 0;
		for (rit = iList.rbegin( ); rit != iList.rend( ); rit++)
			totalWidth += (*rit)->wItem->getThumbRect( )->getWidth( );
		totalWidth += (iList.size()-1)*HpadSize;

		long outsideP = (LONG)
			floor( .5 + (displayMonitor->getMonitorSize( )->getWidth( ) - totalWidth) / 2.0 );
		for (rit = iList.rbegin( ); rit != iList.rend( ); rit++)
		{
			McWItem *item = (*rit)->wItem;
			item->getThumbRect()->offset( outsideP, -appGap );
		}
	}
	else
	{
		long centerOffset = (displayMonitor->getMonitorSize( )->getWidth( ) - currentLeft) / 2;

		for ( rit = iList.rbegin(); rit != iList.rend(); rit++ )
		{
			McWItem *item = (*rit)->wItem;
			if ( item->getVisibility() )
				item->getThumbRect()->offset( centerOffset, -appGap );
		}

		needArrows = TRUE;

		leftArrowRect.left = placementR.left +  HpadSize/2;
		leftArrowRect.right = leftArrowRect.left + HoutsideSpace;

		leftArrowRect.top = placementR.top + (placementR.getHeight()-leftArrowRect.getWidth())/2;
		leftArrowRect.bottom = leftArrowRect.top + 2*leftArrowRect.getWidth();

		int q = 1;
	}

	appGap = placementR.bottom - (placementR.top + 2*(VpadSize-appGap) + thumbHeight);
}

void McPile::doLayout()
{	
	PileItem *pItem = NULL;
	float rotation = 1.0;
	if (iList.size( ) > 0)
	{
		pItem = *(iList.begin( ));
		rotation = pItem->wItem->getRotation( );
	}

	// Do special case: only one item in the pile

	if ( 1 == iList.size() )
	{
		boundingR.set( 0, 0,
			pItem->originalR.getWidth( ),
			pItem->originalR.getHeight( ) );
		pItem->boundedR.set( &boundingR );
	}
	else
	{
		// Calculate Layout Width & Height

		double nItem = (double)iList.size();
		double nPad  = log( nItem ) / log( 2.0 );

		LONG maxW = 0;	// Largest dimension of item windows
		LONG maxH = 0;

		for ( it = iList.begin(); it != iList.end(); it++ )
		{
			maxW = max( maxW, (*it)->originalR.getWidth() );
			maxH = max( maxH, (*it)->originalR.getHeight() );
		}

		double paddingFactor = MC::getProperties()->getThumbnailSpacing()*0.75;
		double padX = (paddingFactor * maxW * nPad )/2;
		double padY = (paddingFactor * maxH * nPad )/2;

		double width = maxW + padX;
		double height = maxH + padY;

		// Calculate radians for placement of window centers on a streched circle

		double radS, radI;

		if ( ((LONG)nItem) % 2 == 0 ) 
			radS = 0.5 * PI * (1.0 + 1/nItem);		// Even n
		else
			radS = 0.5 * PI * (1.0 + 0.5/nItem);		// Odd n

		if (MC::getVariableRotation())
			rotation = (float) ((rand( ) / (double) RAND_MAX) > 0.75 / nItem ? 1.0 : -1.0);

		radI = 0.8*(2.0 * PI / nItem);

		// Initialize boundingR rectangle and indexer

		boundingR.set( (LONG)width, (LONG)height, 0, 0 );
		double dIdx = 0;

		// Calculate boundingR rectangle

		for ( it = iList.begin(); it != iList.end(); it++ )
		{
			pItem = *it;

			pItem->wItem->setRotation( rotation );

			double alpha = radS + dIdx * radI;

			pItem->boundedR.left = (LONG) floor( .5 + pItem->originalR.left + cos( alpha )*padX + width / 2 );
			pItem->boundedR.top = (LONG) floor( .5 + pItem->originalR.top - rotation * sin( alpha )*padY + height / 2 );
			pItem->boundedR.right = pItem->boundedR.left+pItem->originalR.getWidth(); 
			pItem->boundedR.bottom = pItem->boundedR.top+ pItem->originalR.getHeight(); 

			boundingR.left = min( boundingR.left, pItem->boundedR.left );
			boundingR.top  = min( boundingR.top,   pItem->boundedR.top );
			boundingR.right = max( boundingR.right, pItem->boundedR.right );
			boundingR.bottom = max (boundingR.bottom, pItem->boundedR.bottom );

			dIdx += 1.0;
		}

		for ( it = iList.begin(); it != iList.end(); it++ )
			(*it)->boundedR.offset( -boundingR.left, -boundingR.top );

		boundingR.offset( -boundingR.left, -boundingR.top );
	}

	// At this point, McPile::boundingR is a 0,0 origin rectangle bounding the
	// rectangles of the items in the pile, and each PileItem::boundedR
	// holds the coordinates of each item relative to boundingR.   Widths are
	// unscaled from the actual windows included in the pile.

}

void McPile::positionThumbs( double scaleRatio, int xoffset, int yoffset )
{
	for ( it = iList.begin(); it != iList.end(); it++ )
	{
		PileItem *pItem = *it;
		McWItem *wItem = pItem->wItem;

		McRect tRect( &pItem->boundedR );
		tRect.scale( scaleRatio/sRatio );
		tRect.offset( placementR.left+xoffset, placementR.top+yoffset );

		wItem->setThumbRect( &tRect );
	}

}

void McPile::positionZooms( double scaleRatio, int xoffset, int yoffset )
{
	for ( it = iList.begin(); it != iList.end(); it++ )
	{
		PileItem *pItem = *it;
		McWItem *wItem = pItem->wItem;

		McRect tRect( &pItem->boundedR );
		tRect.scale( scaleRatio/sRatio );
		tRect.offset( placementR.left+xoffset, placementR.top+yoffset );
		wItem->setZoomRect( &tRect );
	}

}

void McPile::scale( float sc )
{
	for ( it = iList.begin(); it != iList.end(); it++ )
	{
		PileItem *item = *it;
		item->boundedR.scale( sc );
		item->originalR.scale( sc );
	}
}

void McPile::setPileNumbers()
{
	for ( it = iList.begin(); it != iList.end(); it++ )
	{
		McWItem* wItem = (*it)->wItem;
		wItem->setPileNumber( pileNumber );
	}
}


/////////////////////////////////////////////////////////////////////
//                 COLLECTION OF PILES CODE                        //
/////////////////////////////////////////////////////////////////////

McPilesMgr::McPilesMgr( int _nextPileNumber, HMONITOR _monitor )
{
	nextPileNumber = _nextPileNumber;
	immersiveAppPile = NULL;
	monitor = McMonitorsMgr::getMonitor( _monitor );
	mainMonitor = McMonitorsMgr::getMainMonitor( );
	isMainMonitor = (monitor == mainMonitor);
	
	doAppBarCalculations( );
	wSpacing = (LONG) (.75  *
		MC::getProperties( )->getThumbnailSpacing( ) * monitor->getDpi( ) / 2 );
}

McPilesMgr::~McPilesMgr()
{
	for ( it = pList.begin(); it != pList.end(); it++ )
		delete *it;
	if ( immersiveAppPile )
		delete immersiveAppPile;
}

void McPilesMgr::createZoomPiles( list<McWItem *> *zooms )
{
	for ( list<McWItem *>::iterator it = zooms->begin(); it != zooms->end(); it++ )
	{
		McPile *pile = new McPile( NULL ); 
		pile->add( *it );
		pList.push_back( pile );
		pile->doLayout();
	}
}

void McPilesMgr::doAppBarCalculations( )
{
	BOOL haveAppBar = FALSE;

	immersiveAppGap = 0;
	immersiveAppOffset = 0;

	McWList *wlist = MC::getWindowList( );

	if (isMainMonitor)
	{
		if (MC::getProperties( )->getUseAppBar( ))
		{
			list<McWItem*> *itemList = MC::getWindowList( )->getItemList( );
			for (list<McWItem*>::iterator it = itemList->begin( ); it != itemList->end( ); it++)
				if ((*it)->getIsOnAppBar( ))
				{
					haveAppBar = TRUE;
					break;
				}
			if (haveAppBar)
			{
				float dpi = monitor->getDpi( );
				long VpadSize = (LONG) floor( .5 + dpi * APPBAR_THUMB_V_SPACING_INCHES );
				immersiveAppGap = 0; //APPBAR
				long thumbHeight = (LONG) floor( .5 + dpi * APPBAR_THUMBHEIGHT_INCHES );
				immersiveAppOffset = thumbHeight + 2 * VpadSize;
			}
		}
	}
}

void McPilesMgr::createPiles( )
{
	McWList *wList = MC::getWindowList( );
	list<McWItem *> *itemList = wList->getItemList( );
	
	for (ITEM_IT itt = itemList->begin( ); itt != itemList->end( ); itt++)
	{
		McWItem *wItem = *itt;
		if (wItem->getIsOnAppBar( ))
		{
			if ((MC::getMM( ) && isMainMonitor) || !MC::getMM( ))
			{
				if (!immersiveAppPile)
					immersiveAppPile = new McPile( mainMonitor, "AppBarApps" );
				immersiveAppPile->add( wItem );
			}
		}
		else if (
			(MC::getMM( ) && (monitor->getHMONITOR( ) == wItem->getAppMonitor( ))) ||
			!MC::getMM( ))
		{
			McPile *pile = NULL;
			
			// The logic below groups windows into piles.  

			const char *group;

			group = MC::getProperties( )->getGroupName( wItem->getProgName( ), wItem->getIsW10App() );
			if (!group)
				group = _W10_APP_PILE_NAME;

			BOOL stackWindows = MC::getProperties()->getStackWindows();

			if (stackWindows)
			{
				for (it = pList.begin( ); it != pList.end( ); it++)
				{
					pile = *it;
					if (pile->matches( group ))
						break;
				}
			}
			else
			{
				it = pList.end();
			}

			if (it == pList.end( ))
			{
				pile = new McPile( MC::getMM( ) ? monitor : mainMonitor, group );
				pList.push_back( pile );
			}
			if (pile)
				pile->add( wItem );
		}
	}

	// Have the immersiveApp pile contruct a layout

	if (immersiveAppPile)
	{
		immersiveAppPile->doImmersiveAppLayout( );
		immersiveAppPile->setPileNumber( getNextPileNumber( ) );
	}

	for (it = pList.begin( ); it != pList.end( ); it++)
	{
		(*it)->reScale( pList.size( ) );
		(*it)->doLayout( );
	}
}

BOOL tileCompare( McPile *first, McPile *second )
{
	return first->boundingR.getArea( ) >
			second->boundingR.getArea( );
}

BOOL tileCompare2( McPile *first, McPile *second )
{
	if (second->row > first->row) return TRUE;
	if (second->row < first->row) return FALSE;
	return first->placementR.left < second->placementR.left;
}

void McPilesMgr::layoutPiles( BOOL doExtra )
{
	if ( pList.size() == 0 ) return;
	McMonitorsMgr *mon = MC::getMM( ) ? monitor : mainMonitor;

	// First Sort the Piles, so the Largest (Area) ones come first

	pList.sort( tileCompare );

	// Second, create the initial placementR to a little bigger than screen.

	McRect workingRect( mon->getMonitorSize() );

	if (immersiveAppPile)
		workingRect.bottom -= (immersiveAppPile->placementR.getHeight( ) + immersiveAppPile->deadSpace);

	McRect truePlacement( &workingRect );
	placementR.set( 0, 0, truePlacement.getWidth( ), truePlacement.getHeight( ) );

	placementR.scale( 0.25 );

	// Place first tile in upper left

	double monH = workingRect.getHeight();
	double monW = workingRect.getWidth();

	mdAspect = monW/monH; // May be different from monitor aspect

	boundingR.clear();

	// Now place the remaining tiles.

	for (list<McPile *>::iterator l_it = pList.begin( ); l_it != pList.end( ); l_it++)
		placeWindowPile( *l_it );

	// All Tiles Have Been Placed.
	// Squeeze out extra space, scale to monitor,
	// and center.

	McRect bounds(placementR.right, placementR.bottom, placementR.left, placementR.top );

	for( it = pList.begin(); it != pList.end(); it++ )
	{
		bounds.left = min( bounds.left, (*it)->placementR.left );
		bounds.top = min( bounds.top, (*it)->placementR.top );
	}

	for( it = pList.begin(); it != pList.end(); it++ )
		(*it)->placementR.offset( -bounds.left, -bounds.top );

	sRatio = min( 1.0, min( monW/placementR.getWidth(), monH/placementR.getHeight()) ) ;
	placementR.scale( sRatio );

	placementR.right = max( placementR.right, (LONG)monW );
	placementR.bottom = max( placementR.top, (LONG)monH );

	placementR.offset( truePlacement.left, truePlacement.top );

	for (it = pList.begin( ); it != pList.end( ); it++)
		(*it)->placementR.scale( sRatio );

	// Make pretty.

	tuneLayout( doExtra );
	
	if (doExtra)
	{
		int maxWidth = mon->getMaxPileWidth( );
		int maxHeight = mon->getMaxPileHeight( );
		for (list<McPile *>::iterator l_it = pList.begin( ); l_it != pList.end( ); l_it++)
		{
			McPile *pile = *l_it;
			McRect *r = &pile->placementR;
			if (pile->iList.size( ) == 1 && r->getWidth( ) > maxWidth && r->getHeight( ) > maxHeight)
			{
				int rWidth = r->getWidth( );
				int rHeight = r->getHeight( );
				int wDiff, hDiff;
				float sc;

				if ((float) maxWidth / rWidth > (float) maxHeight / rHeight)
				{
					wDiff = (int) floor( .5 + (rWidth - maxWidth) / 2.0 );
					hDiff = (int) floor( .5 + wDiff / mon->getAspect( ) );
					sc = (float) maxWidth / rWidth;
				}
				else
				{
					hDiff = (int) floor( .5 + (rHeight - maxHeight) / 2.0 );
					wDiff = (int) floor( .5 + hDiff * mon->getAspect( ) );
					sc = (float) maxHeight / rHeight;
				}

				r->left += wDiff;
				r->right -= wDiff;
				r->top += hDiff;
				r->bottom -= hDiff;

				pile->scale( sc );
			}
		}
	}
	
	if ( immersiveAppPile && pList.size() > 0 )
		if ( pList.size() > 0 )
			immersiveAppPile->row = (*pList.rbegin())->row+1;
		else
			immersiveAppPile->row = 0;

}

// Make sure target rectangle doesn't overlap any other tiles.

BOOL McPilesMgr::checkOverlap( McRect *targetR )
{
	for ( list<McPile *>::iterator cit = placedPiles.begin(); cit!=placedPiles.end(); cit++ )
		if ( targetR->intersection( &(*cit)->placementR ) > 0.0 )
			return TRUE;
	return FALSE;
}


#define MCP_Right 0
#define MCP_Below 1
#define _MINC 5

void McPilesMgr::moveItUp( McPile *pile )
{
	McRect proposed( &pile->placementR );
	BOOL intersects = FALSE;
	while (proposed.top > _MINC*2 && !intersects)
	{
		proposed.top -= _MINC;
		proposed.bottom -= _MINC;
		intersects = checkOverlap( &proposed );
	}
	if (intersects)
	{
		proposed.top += _MINC;
		proposed.bottom += _MINC;
	}
	pile->placementR.set( &proposed );
}

void McPilesMgr::moveItLeft( McPile *pile )
{
	McRect proposed( &pile->placementR );
	BOOL intersects = FALSE;
	while (proposed.left > _MINC*2 && !intersects)
	{
		proposed.left -= _MINC;
		proposed.right -= _MINC;
		intersects = checkOverlap( &proposed );
	}
	if (intersects)
	{
		proposed.left += _MINC;
		proposed.right += _MINC;
	}
	pile->placementR.set( &proposed );
}

void McPilesMgr::placeWindowPile( McPile *pile )
{

	// First place piles (or thumbs) sequentially to minimize the metric, balancing desire to reduce total
	// area required to show the piles with desire to retain the aspect ratio of the monitor.

	double aspectWeight = .5;

	if ( placedPiles.size() == 0 )
		pile->placementR.set( 0, 0, pile->boundingR.getWidth(), pile->boundingR.getHeight() );
	else
	{
		McPile*			bestCandidate = NULL;
		double			bestMetric = 0.01;
		unsigned		bestLocation;
		McRect			bestR;

		McRect			targetR;

		// Check every candidate, find one that minimizes the metric (1.0 is optimal)

		McRect currentRect( 100000, 100000, -1000000, -100000 );
		for (list<McPile *>::iterator xit = placedPiles.begin( ); xit != placedPiles.end( ); xit++)
		{
			McPile *xp = *xit;
			currentRect.left = min( currentRect.left, xp->placementR.left );
			currentRect.right = max( currentRect.right, xp->placementR.right );
			currentRect.top = min( currentRect.top, xp->placementR.top );
			currentRect.bottom = max( currentRect.bottom, xp->placementR.bottom );
		}

		for (list<McPile *>::iterator c_it = placedPiles.begin( ); c_it != placedPiles.end( ); c_it++)
		{
			McPile *candidate = *c_it;

			for (int pl = MCP_Below; pl >= MCP_Right; pl--)
			{
				// First, calculate the target placement we're consider

				switch (pl)
				{
				case MCP_Right:

					if (candidate->filledRight) continue;

					targetR.left = candidate->placementR.right + 1;
					targetR.top = candidate->placementR.top +
						(candidate->placementR.getHeight( ) - pile->boundingR.getHeight( )) / 2;
					break;

				case MCP_Below:

					if (candidate->filledBelow) continue;

					targetR.top = candidate->placementR.bottom + 1;
					targetR.left = candidate->placementR.left +
						(candidate->placementR.getWidth( ) - pile->boundingR.getWidth( )) / 2;
					break;
				}

				targetR.right = targetR.left + pile->boundingR.getWidth( );
				targetR.bottom = targetR.top + pile->boundingR.getHeight( );

				// Next calculate the metric for the location

				double metric1 =  // Area component
					((double) (currentRect.getArea( ))) /
					(
					(-currentRect.top + max( targetR.bottom, currentRect.bottom ))*
					(-currentRect.left + max( targetR.right, currentRect.right ))
					);

				double aspect =
					((double) max( boundingR.right, targetR.right )) /
					((double) max( boundingR.bottom, targetR.bottom ));

				// aspect ratio component

				double metric2 = pow( (aspect < mdAspect) ? aspect / mdAspect : mdAspect / aspect, 2 );

				double metric = aspectWeight * metric2 + (1.0 - aspectWeight) * metric1;

				if (metric >= bestMetric)
					if (!checkOverlap( &targetR ))
					{
						bestMetric = metric;
						bestCandidate = candidate;
						bestLocation = pl;
						bestR.set( &targetR );
					}
			}
		}

		pile->placementR.set( &bestR );

		switch (bestLocation)
		{
		case MCP_Right:
			if (bestCandidate) bestCandidate->filledRight = TRUE;
			pile->row = bestCandidate->row;
			break;
		case MCP_Below:
			if (bestCandidate) bestCandidate->filledBelow = TRUE;
			pile->row = bestCandidate->row + 1;
			break;

		}

	}

	boundingR.right = max( boundingR.right, pile->placementR.right );
	boundingR.bottom = max( boundingR.bottom, pile->placementR.bottom );

	// Make placement area bigger (but in proportion) if necessary

	if ( boundingR.right > placementR.right )
	{
		LONG deltaW = ( boundingR.right - placementR.right );
		placementR.right += deltaW;
		placementR.bottom += (LONG)( deltaW / mdAspect );
	}

	if ( boundingR.bottom > placementR.bottom )
	{
		LONG deltaH = ( boundingR.bottom - placementR.bottom );
		placementR.bottom += deltaH;
		placementR.right += (LONG)( mdAspect * deltaH );
	}

	placedPiles.push_back( pile );

}

void McPilesMgr::tuneLayout( BOOL doExtra )
{

	// After placement, piles are nudged around to make a more pleasing display.

	list<McPile*> pLists[10]; //Allow for 10 rows
	list<McPile*>::reverse_iterator r_it;

	// FIRST, CREATE THE LISTS OF Piles BY ROW

	int r,
		rows = 0;

	//	Count the rows

	for (it = pList.begin( ); it != pList.end( ); it++)
		rows = max( rows, (*it)->row );

	rows++;

	// Build the array of lists of Piles, by row

	for (r = 0; r < rows; r++)
		for (it = pList.begin( ); it != pList.end( ); it++)
			if ((*it)->row == 1 + r)
				pLists[r].push_back( *it );

	// SECOND, SEPARATE THE ROWS SO HORIZONTAL SHIFTS CAN BE DONE

	LONG maxB = 0;

	for (r = 0; r < rows; r++)
	{

		if (r > 0)
		{
			// position each window in this row below the lowest in prev row
			for (it = pLists[r].begin( ); it != pLists[r].end( ); it++)
				if ((*it)->placementR.top <= maxB)
					(*it)->placementR.offset( 0, 1 + maxB - (*it)->placementR.top );
		}

		// Calculate the lowest point of this row

		for (it = pLists[r].begin( ); it != pLists[r].end( ); it++)
			maxB = max( maxB, (*it)->placementR.bottom );
	}

	// NOW DISTRIBUTE THE PILES EVENLY ACROSS EACH ROW
	// AND CREATE PILE NUMBERS

	LONG oldCenter = 0, oldWidth = 0;
	for (r = 0; r < rows; r++)
	{
		size_t padding = placementR.getWidth( );
		size_t count = pLists[r].size( );

		// Calculate empty spaces

		for (it = pLists[r].begin( ); it != pLists[r].end( ); it++)
			padding -= (*it)->placementR.getWidth( );

		padding = max( 0, padding ) / (count + 1);

		// padding is the width of each spacing

		LONG cumX = 0;
		LONG w;
		if (r % 2)
			for (r_it = pLists[r].rbegin( ); r_it != pLists[r].rend( ); r_it++)
			{
				(*r_it)->setPileNumber( McPilesMgr::getNextPileNumber( ) );
				cumX += (long)padding;
				w = (*r_it)->placementR.getWidth( );
				(*r_it)->placementR.left = cumX;
				cumX += w;
				(*r_it)->placementR.right = cumX;
			}
		else
			for (it = pLists[r].begin( ); it != pLists[r].end( ); it++)
			{
				(*it)->setPileNumber( McPilesMgr::getNextPileNumber( ) );
				cumX += (long)padding;
				w = (*it)->placementR.getWidth( );
				(*it)->placementR.left = cumX;
				cumX += w;
				(*it)->placementR.right = cumX;
			}

		LONG oldBottom = placementR.bottom + 100;
		for (it = pLists[r].begin( ); it != pLists[r].end( ); it++)
		{
			McPile *pile = *it;
			if (pile->placementR.bottom < oldBottom)
			{
				oldBottom = pile->placementR.bottom;
				oldCenter = (pile->placementR.left + pile->placementR.right) / 2;
				oldWidth = pile->placementR.getWidth( );
			}
		}
	}


	// NOW MOVE EACH PILE UP AS FAR AS POSSIBLE WITHOUT OVERLAPPING WITH ANY IN THE PREV ROW

	list<McPile *>::iterator jt;

	for (r = 1; r < rows; r++)
	{
		long highestBottom = 10000;
		for (jt = pLists[r - 1].begin( ); jt != pLists[r - 1].end( ); jt++)
			if ((*jt)->placementR.bottom < highestBottom)
				highestBottom = (*jt)->placementR.bottom;

		for (it = pLists[r].begin( ); it != pLists[r].end( ); it++)
		{
			BOOL finished = FALSE;
			while (!finished)
			{
				(*it)->placementR.offset( 0, -1 );
				for (jt = pLists[r - 1].begin( ); jt != pLists[r - 1].end( ); jt++)
					if ((*it)->placementR.intersection( &(*jt)->placementR ) > 0 || ((*it)->placementR.top < highestBottom))
					{
						(*it)->placementR.offset( 0, 1 );
						finished = TRUE;
						break;
					}
			}
		}
	}


	// ASSIGN PILE NUMBERS TO THE McWItem objects

	if (doExtra)
	{
		if (immersiveAppPile)
			immersiveAppPile->setPileNumbers( );
		for (it = pList.begin( ); it != pList.end( ); it++)
			(*it)->setPileNumbers( );
	}

	// FIND OUT IF WE STILL RUN OVER THE WINDOW
	// one more time do a bounding rectangle

	McRect bR( 0, 0, placementR.getWidth( ), placementR.getHeight( ) );
	for (it = pList.begin( ); it != pList.end( ); it++)
	{
		bR.bottom = max( bR.bottom, (*it)->placementR.bottom );
		bR.right = max( bR.right, (*it)->placementR.right );
		bR.left = min( bR.left, (*it)->placementR.left );
		bR.top = min( bR.top, (*it)->placementR.top );
	}

	double finalScale = max( (double) (bR.getWidth( )) / (double) (placementR.getWidth( )),
		(double) (bR.getHeight( )) / (double) (placementR.getHeight( )) );

	// make BR the right size
	if (finalScale > 1.0)
	{
		sRatio /= finalScale;
		for (it = pList.begin( ); it != pList.end( ); it++)
			(*it)->placementR.scale( 1.0 / finalScale );
	}

	// Do Rescale and center

	McRect bounds( placementR.getWidth( ), placementR.getHeight( ), 0, 0 );

	for (it = pList.begin( ); it != pList.end( ); it++)
	{
		McRect ir( (*it)->placementR );
		bounds.top = min( bounds.top, (*it)->placementR.top );
		bounds.left = min( bounds.left, (*it)->placementR.left );
		bounds.right = max( bounds.right, (*it)->placementR.right );
		bounds.bottom = max( bounds.bottom, (*it)->placementR.bottom );
	}

	for (it = pList.begin( ); it != pList.end( ); it++)
	{

		McRect *pr = &(*it)->placementR;
		pr->offset(
			(placementR.getWidth( ) - bounds.getWidth( )) / 2 - bounds.left,
			(placementR.getHeight( ) - bounds.getHeight( )) / 2 - bounds.top );
		double yxF = ((double) pr->getHeight( )) / ((double) pr->getWidth( ));

		if (doExtra)
		{
			pr->left += wSpacing / 2;
			pr->right -= wSpacing / 2;
			pr->top += (LONG) (0.5*yxF*wSpacing);
			pr->bottom -= (LONG) (0.5*yxF*wSpacing);
			(*it)->sRatio = ((double) (pr->getWidth( ) + wSpacing)) / ((double) pr->getWidth( ));
		}
		else
		{
			int ws = (int) floor( .5 + .15 *
				MC::getProperties( )->getThumbnailSpacing( ) * pr->getHeight( ) );
			pr->top += ws / 2;
			pr->bottom -= ws / 2;
			pr->left += (LONG) (0.5*ws / yxF);
			pr->right -= (LONG) (0.5*ws / yxF);
			(*it)->sRatio = ((double) (pr->getHeight( ) + ws)) / ((double) pr->getHeight( ));
		}

	}
}


void McPilesMgr::positionThumbs( )
{
	for ( it = pList.begin(); it != pList.end(); it++ )
		(*it)->positionThumbs( sRatio, placementR.left, placementR.top );
}

void McPilesMgr::positionZooms()
{
	for ( it = pList.begin(); it != pList.end(); it++ )
		(*it)->positionZooms( sRatio, placementR.left, placementR.top );
}

BOOL McPilesMgr::areArrowsNeeded( McRect *target )
{
	if ( !immersiveAppPile ) return FALSE;
	if ( !immersiveAppPile->needArrows ) return FALSE;
	if ( target ) target->set( &immersiveAppPile->leftArrowRect );
	return TRUE;
}


