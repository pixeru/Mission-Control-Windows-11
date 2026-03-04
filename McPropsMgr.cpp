#include "stdafx.h"

#include "Emcee.h"
#include "McPropsMgr.h"

// Read / Write the properties file.

#pragma unmanaged
McGroup::McGroup( char *_groupName )
{
	groupName.assign( _groupName );
}

McGroup::McGroup( McGroup *g )
{
	groupName = g->groupName;
	stringList = g->stringList;
}

McGroup::~McGroup()
{
}

void McGroup::add( char *_progName )
{
	string progName( _progName );
	stringList.push_front( progName );
}

McPropsMgr::McPropsMgr()
{
	data = new McPropData();
	read();
}

McPropsMgr::~McPropsMgr()
{
	delete data;
}

BOOL McPropsMgr::getIsExcluded( const char *_progName )
{
	for ( list<string>::iterator s_it = data->excludeList.begin(); s_it != data->excludeList.end(); s_it++ )
		if ( 0 == _stricmp( _progName, (*s_it).c_str() ) )
			return TRUE;
	return FALSE;
}

const char*  McPropsMgr::getGroupName( const char *_progName, BOOL isW10App )
{
	for ( list<McGroup*>::iterator g_it = data->masterList.begin(); g_it != data->masterList.end(); g_it++ )
	{
		list<string> sList = (*g_it)->stringList;
		for ( list<string>::iterator s_it = sList.begin(); s_it != sList.end(); s_it++ )
			if ( 0 == _stricmp( _progName, (*s_it).c_str() ) )
				return (*g_it)->groupName.c_str();
	}		
	if (isW10App) 
		return 	NULL;
	else
		return _progName;
}

McGroup* McPropsMgr::getGroup( char *_grpName )
{
	for ( list<McGroup*>::iterator g_it = data->masterList.begin(); g_it != data->masterList.end(); g_it ++ )
		if ( 0 == _stricmp( (*g_it)->groupName.c_str(), _grpName ) )
			return (*g_it);
	McGroup *g = new McGroup( _grpName );
	data->masterList.push_back( g );
	return g;
}

void McPropsMgr::reread()
{
	delete data;
	data = new McPropData();
	read();
}

void McPropsMgr::read()
{
	FILE *f = NULL;
	BOOL writeAlso = MC::openAppFile( "Emcee.cfg", "r", &f );
	if ( !f )
		return;

	while ( !feof( f ) )
	{
		char lineb[4096];
		char errb[4096];
		char *line = fgets( lineb, 4096, f ); 
		if ( line == NULL )
			continue;

		strcpy_s( errb, 4096, lineb );

		char *pch = strchr( line, '#' );
		if ( pch != NULL )
			*pch = 0;

		pch = strtok_s( line, " ,=\t", &token_holder );
		if ( pch == NULL )
			continue;

		char *result = NULL;
		if ( 0 == _stricmp( pch, "<value" ) )
			result = scanValue();
		else if ( 0 == _stricmp( pch, "<Group" ) )
			result = scanGroup();
		else if ( 0 == _stricmp( pch, "<exclude>" ) )
			result = scanExclude();
	}

	fclose( f );

	if ( writeAlso )
		write();
}

char *McPropsMgr::unspace( char *str )
{
	if ( !str ) return NULL;
	char *context = NULL;
	char *c = strtok_s( str, "\n\t ", &context );
	return c;
}

char *McPropsMgr::scanValue( )
{
	char nbuff[4096];
	char *name = strtok_s( NULL, ">,\"\t", &token_holder );
	if ( *token_holder == '>' ) token_holder++;
	name = unspace(name);

	if ( name == NULL )
		return "Expected setting name, didn't find";

	strcpy_s( nbuff, sizeof( nbuff ), name );

	char *value = strtok_s( NULL, " ,=\t\n", &token_holder );
	if ( value == NULL )
		return "Expected setting value, didn't find.";

	if ( !_stricmp( name, "disablelog" ) ||
#if MC_DESKTOPS
		 !_stricmp( name, "showAllDesktops" ) ||
#endif
		 !_stricmp( name, "useAppBar" ) ||
		 !_stricmp( name, "multiMonitor" ) ||
		 !_stricmp( name, "disableLabels" ) ||
		 !_stricmp( name, "hardwareAcceleration" ) ||
		 !_stricmp( name, "animationEffects" ) ||
		 !_stricmp( name, "stackWindows" ) ||
		 !_stricmp( name, "dtAutoColor" ))
	{
		BOOL val;
		if ( !_stricmp( value, "true" ) )
			val = TRUE;
		else if ( !_stricmp( value, "false" ) )
			val = FALSE;
		else
			return "Expected true or false, didn't find";
		if ( !_stricmp( name, "disableLabels" ) ) 	data->disableLabels = val;
		else if (!_stricmp( name, "multiMonitor" )) data->multiMonitor = val;
#if MC_DESKTOPS
		else if ( !_stricmp( name, "showAllDesktops" )) data->showAllDesktops = val;
#endif
		else if ( !_stricmp( name, "useAppBar" )) data->useAppBar = val;
		else if (!_stricmp( name, "dtAutoColor" )) data->dtAutoColor = val;
		else if (!_stricmp( name, "hardwareAcceleration" )) data->hardwareAcceleration = val;
		else if (!_stricmp( name, "stackWindows" )) data->stackWindows = val;
		else if (!_stricmp( name, "animationEffects" )) data->animationEffects = val;
		return NULL;
	}

	if ( !_stricmp( name, "immersiveTileHeight" ) || 
		 !_stricmp( name, "thumbnailspacing" ) )
	{
		double val = atof( value );
		if ( !_stricmp( name, "immersiveTileHeight" ) )
		{
			if ( val < 0.25 || val > 3.0 ) return "immersiveTileHeight value out of range.\n";
			data->immersiveTileHeight = val;
		}
		else if ( !_stricmp( name, "thumbnailspacing" ) )
		{
			if ( val < 0.0625 || val > 1.0 ) return "thumbnailSpacing value out of range 1/16 - 1.0";
			data->thumbnailSpacing = val;
		}
		return NULL;
	}

	if ( !_stricmp( name, "dtbgcolor" ) || 
		 !_stricmp( name, "labelfontsize" ) || 
		 !_stricmp( name, "animationspeed" )   ||
		 !_stricmp( name, "mouseCornerActivate" ) ||
		 !_stricmp( name, "hotKeyCode" ) )
	{
		long val = atol( value );
		if (!_stricmp( name, "dtbgcolor" ))
		{
			data->dtBgColor = val;
		}
		else
		{
			if ( _stricmp( name, "hotKeyCode" )  ) 
			{
				if ( !_stricmp( name, "labelfontsize" ) && ( val < 6 || val > 42 ) )
					return "Expected value 6 to 42: outside range.";
				else if ( _stricmp( name, "labelfontsize" ) )
					if ( val < (_stricmp( name, "mouseCornerActivate" )?1:0) || val > 9 ) 
						return "Expected value 1 to 9: outside range.";
			}
			if ( !_stricmp( name, "labelfontsize" ) )
				data->labelFontSize = val;
			else if ( !_stricmp( name, "animationspeed" ) )
				data->animationSpeed = val;
			else if ( !_stricmp( name, "hotkeycode" ) ) 
				data->hotKeyCode = val;
			else if ( !_stricmp( name, "mouseCornerActivate" ) ) data->mouseCornerActivate = (int)val;
		}
		return NULL;
	}

	return "Found an unrecognized data name.";
}

char *McPropsMgr::scanExclude()
{

	char *ptr = token_holder;

	while ( NULL != *ptr )
	{
		char *pattern;

		if ( *ptr == '"' )
		{
			token_holder++;
			pattern = "\"";
		}
		else if ( isalpha( *ptr ) )
			pattern = " ,\t\n";
		else if ( NULL == *ptr )
			break;
		else
		{
			ptr++;
			continue;
		}

		ptr = strtok_s( NULL, pattern, &token_holder );

		string ev( ptr );
		data->excludeList.push_back( ev );

		ptr = token_holder;
	}
	return NULL;
}

char *McPropsMgr::scanGroup()
{
	char *name = strtok_s( NULL, ">,\"\t", &token_holder );
	if ( *token_holder == '>' ) token_holder++;

	if ( name == NULL )
		return "Expected Group name, didn't find.";

	McGroup *g = this->getGroup( name );

	char *ptr = token_holder;

	while ( NULL != *ptr )
	{
		char *pattern;

		if ( *ptr == '"' )
		{
			token_holder++;
			pattern = "\"";
		}
		else if ( isalpha( *ptr ) )
			pattern = " ,\t\n";
		else if ( NULL == *ptr )
			break;
		else
		{
			ptr++;
			continue;
		}

		ptr = strtok_s( NULL, pattern, &token_holder );

		g->add( ptr );
		ptr = token_holder;
	}
	return NULL;
}

void McPropsMgr::write()
{
	FILE *f = NULL;
	MC::openAppFile( "Emcee.cfg", "w", &f );
	if ( !f )
		return;
	SYSTEMTIME time;
	GetLocalTime( &time );
	fprintf( f, "# AUTOMATICALLY GENERATED FILE: DO NOT EDIT\n" );
	fprintf( f, "# Created %d:%02d:%02d on %d/%d/%d\n", time.wHour, time.wMinute, time.wSecond,
		time.wMonth, time.wDay, time.wYear );

	fprintf( f, "<value mouseCornerActivate> %d\n", data->mouseCornerActivate );
	fprintf( f, "<value disableLabels> %s\n", data->disableLabels?"TRUE":"FALSE" );
	fprintf( f, "<value multiMonitor> %s\n", data->multiMonitor ? "TRUE" : "FALSE" );
#if MC_DESKTOPS
	fprintf( f, "<value showAllDesktops> %s\n", data->showAllDesktops ? "TRUE" : "FALSE" );
#endif
	fprintf( f, "<value useAppBar> %s\n", data->useAppBar ? "TRUE" : "FALSE" );
	fprintf( f, "<value hotKeyCode> %u\n", data->hotKeyCode );

	fprintf( f, "<value immersiveTileHeight> %lf\n", data->immersiveTileHeight );
	fprintf( f, "<value thumbnailSpacing> %lf\n", data->thumbnailSpacing );

	fprintf( f, "<value labelFontSize> %d\n", data->labelFontSize );
	fprintf( f, "<value animationSpeed> %d\n", data->animationSpeed );

	fprintf( f, "<value dtAutoColor> %s\n", data->dtAutoColor ? "TRUE" : "FALSE" );
	fprintf( f, "<value dtBgColor> %ld #0x%x\n", data->dtBgColor, data->dtBgColor );
	fprintf( f, "<value hardwareAcceleration> %s\n", data->hardwareAcceleration ? "TRUE" : "FALSE" );
	fprintf( f, "<value stackWindows> %s\n", data->stackWindows ? "TRUE" : "FALSE" );
	fprintf( f, "<value animationEffects> %s\n", data->animationEffects ? "TRUE" : "FALSE" );

	for ( list<McGroup *>::iterator gi = data->masterList.begin(); gi != data->masterList.end(); gi++ )
	{
		McGroup *g = *gi;
		fprintf( f, "<Group %s> ", g->groupName.c_str() );
		for ( list<string>::iterator si = g->stringList.begin(); si != g->stringList.end(); si++ )
		{
			const char *c = (*si).c_str();
			if ( strchr( c, ' ' ) )
				fprintf( f, "\"%s\" ", c );
			else
				fprintf( f, "%s ", c );
		}
		fprintf( f, "\n" );
	}

	if ( data->excludeList.size() > 0 )
	{
		fprintf( f, "<Exclude> " );
		for ( list<string>::iterator si =data->excludeList.begin(); si != data->excludeList.end(); si++ )
		{
			const char *c = (*si).c_str();
			if ( strchr( c, ' ' ) )
				fprintf( f, "\"%s\" ", c );
			else
				fprintf( f, "%s ", c );
		}
		fprintf( f, "\n" );
	}

	fclose( f );
}

double McPropsMgr::getThumbnailSpacing( )	
{ 
	return data->thumbnailSpacing;
}

BOOL McPropsMgr::getMultiMonitor()			
{
	return data->multiMonitor;
}
