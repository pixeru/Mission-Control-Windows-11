#pragma once
#include "Emcee.h"

// Simple utility classes, add methods to Windows RECT structure

class McRect : public RECT
{
public:

	McRect( )
	{
		clear( );
	}

	void McRect::clear( )
	{
		left = right = bottom = top = 0;
	}

	McRect( RECT *r )
	{
		set( r );
	}

	void McRect::set( RECT *r )
	{
		left = r->left;
		top = r->top;
		right = r->right;
		bottom = r->bottom;
	}

	void interpolate( McRect *start, McRect *finish, double value, BOOL funny = FALSE );

	BOOL McRect::contains( long x, long y )
	{
		return (left <= x && x <= right && top <= y && y <= bottom);
	}

	BOOL McRect::encloses( McRect *other )
	{
		return left <= other->left && top <= other->top &&
			right >= other->right && bottom >= other->bottom;
	}

	BOOL McRect::overlaps( McRect *win )
	{
		return !(win->right < left || win->bottom < top || win->left > right || win->top > bottom);
	}


	McRect( LONG l, LONG t, LONG r, LONG b )
	{
		set( l, t, r, b );
	}

	void McRect::set( LONG l, LONG t, LONG r, LONG b )
	{
		left = l;
		top = t;
		right = r;
		bottom = b;
	}

	void McRect::scale( double _scale )
	{
		left = (LONG) floor( .5 + _scale*left );
		top = (LONG) floor( .5 + _scale*top );
		right = (LONG) floor( .5 + _scale*right );
		bottom = (LONG) floor( .5 + _scale*bottom );
	}

	LONG McRect::getArea( )
	{
		return getWidth( )*getHeight( );
	}

	void McRect::offset( long _xOffset, long _yOffset )
	{
		left += _xOffset;
		right += _xOffset;
		top += _yOffset;
		bottom += _yOffset;
	}

	void McRect::offset( POINT *o )
	{
		offset( o->x, o->y );
	}

	LONG McRect::getWidth( )
	{
		return right - left + 1;
	}

	LONG McRect::getHeight( )
	{
		return bottom - top + 1;
	}

	BOOL McRect::equals( McRect *two )
	{
		return left == two->left &&
			top == two->top &&
			bottom == two->bottom &&
			right == two->right;
	}

	LONG McRect::intersection( McRect *that )
	{
		return	max( 0, min( right, that->right ) - max( left, that->left ) )
			*	max( 0, min( bottom, that->bottom ) - max( top, that->top ) );
	}
};
