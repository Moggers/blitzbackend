#include "match.hpp"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

namespace Game
{
	Match::Match( int id, int mapid, char * mapname, char * imgname, int age, char * name, int status, int port, int t1, int t2, int t3, int t4, int research, int renaming ) : id{id}, mapid{mapid}, status{status}, age{age}, port{port}, research{research}, renaming{renaming}
	{
		this->mapName = (char*)malloc( strlen( mapname ) );
		strcpy( this->mapName, mapname );
		this->imgName = (char*)malloc( strlen( imgname ) );
		strcpy( this->imgName, imgname );
		this->name = (char*)malloc( strlen( name ) );
		strcpy( this->name, name );
		this->t = (int*)calloc( 4, sizeof( int ) );
		this->t[0] = t1;
		this->t[1] = t2;
		this->t[2] = t3;
		this->t[3] = t4;
	}

	Match::Match( MYSQL_ROW match, MYSQL_ROW map ) :
		id{atoi(match[0])},
		mapid{atoi(match[1])},
		status{atoi(match[4])},
		age{atoi(match[2])},
		port{atoi(match[5])},
		research{atoi(match[10])},
		renaming{atoi(match[11])},
		clientstart{atoi(match[12])}
	{
		this->mapName = (char*)malloc( strlen( map[0] ) + 1 );
		strcpy( this->mapName, map[0] );
		this->imgName = (char*)malloc( strlen( map[1] ) + 1 );
		strcpy( this->imgName, map[1] );
		this->name = (char*)malloc( strlen( match[3] ) + 1 );
		strcpy( this->name, match[3] );
		this->t = (int*)calloc( 4, sizeof( int ) );
		this->t[0] = atoi(match[6]);
		this->t[1] = atoi(match[7]);
		this->t[2] = atoi(match[8]);
		this->t[3] = atoi(match[9]);
	}

	char* Match::createConfStr( void )
	{
		char * str = (char*)calloc( 1024, sizeof( char ) );
		if( this->clientstart == 0 )
			sprintf( str, "--noclientstart " );
		sprintf( str + strlen( str ), "--renaming %d -d --research %d --era %d --thrones %d %d %d --requiredap %d --mapfile \"%s\" \"%s\"", 
			this->renaming,
			this->research,
			this->age, 
			this->t[0], 
			this->t[1], 
			this->t[2], 
			this->t[3], 
			this->mapName, 
			this->name );
		return str;
	}
}
