#include "match.hpp"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "emailsender.hpp"
#include "settings.hpp"

namespace Game
{
	Match::Match( const MYSQL_ROW match, const MYSQL_ROW map, std::vector<Game::Mod*> * mods ) :
		id{strtoul(match[0], NULL, 10 )},
		mapid{atoi(match[1])},
		status{atoi(match[4])},
		age{atoi(match[2])},
		port{atoi(match[5])},
		research{atoi(match[10])},
		renaming{atoi(match[11])},
		clientstart{atoi(match[12])},
		hostday{atoi(match[14])},
		hosthour{atoi(match[15])},
		hostint{atoi(match[16])},
		needsrestart{atoi(match[18])},
		maxholdups{atoi(match[19])}
	{
		this->mapName = (char*)malloc( strlen( map[0] ) + 1 );
		strcpy( this->mapName, map[0] );
		this->imgName = (char*)malloc( strlen( map[1] ) + 1 );
		strcpy( this->imgName, map[1] );
		this->name = (char*)malloc( strlen( match[3] ) + 1 );
		strcpy( this->name, match[3] );
		if( strlen( match[20] ) != 0 ) {
			this->masterpass = (char*)malloc( strlen( match[20] ) + 1 );
			strcpy( this->masterpass, match[20] );
		} else { this->masterpass = 0; }
		this->t = (int*)calloc( 4, sizeof( int ) );
		this->t[0] = atoi(match[6]);
		this->t[1] = atoi(match[7]);
		this->t[2] = atoi(match[8]);
		this->t[3] = atoi(match[9]);
		this->mods = new std::vector<Game::Mod*>(*mods);
	}

	Match::Match( Game::Match * match ) :
	id{match->id},
	mapid{match->mapid},
	status{match->status},
	age{match->age},
	port{match->port},
	research{match->research},
	renaming{match->renaming},
	clientstart{match->clientstart},
	hostday{match->hostday},
	hosthour{match->hosthour},
	hostint{match->hostint},
	needsrestart{match->needsrestart},
	maxholdups{match->maxholdups}
	{
		this->mapName = (char*)calloc( 1024, sizeof( char ) );
		this->imgName = (char*)calloc( 1024, sizeof( char ) );
		this->name = (char*)calloc( 1024, sizeof( char ) );
		this->t = (int*)calloc( 4, sizeof( int ) );

		strcpy( this->mapName, match->mapName );
		strcpy( this->imgName, match->imgName );
		strcpy( this->name, match->name );
		this->t[0] = match->t[0];
		this->t[1] = match->t[1];
		this->t[2] = match->t[2];
		this->t[3] = match->t[3];

		this->mods = new std::vector<Mod*>();
		for( auto mod : *match->mods ) {
			this->mods->push_back( new Mod( mod ) );
		}
	}

	Match::~Match( void )
	{
		free( this->mapName );
		free( this->imgName );
		free( this->name );
		free( this->t );
		delete( this->mods );
	}

	char* Match::createConfStr( void )
	{
		char * str = (char*)calloc( 2048, sizeof( char ) );
		if( this->clientstart == 0 )
			sprintf( str, "--noclientstart " );
		if( !this->mods->empty() ) {
			for( std::vector<Game::Mod*>::iterator it = this->mods->begin(); it != this->mods->end(); it++ )
			{
				if( (*it) != NULL ) {
					sprintf( str + strlen( str ), "--enablemod %s ", (*it)->m_dmname );
				}
			}
		}
		if( this->hostint != 0 )
			sprintf( str + strlen( str ), "--minutes %d ", this->hostint );
		if( this->hosthour != 0 || this->hostday != 0 ) {
			sprintf( str + strlen( str ), "--hosttime %d %d ", this->hostday, this->hosthour );
		} if( this->maxholdups != 0 ) {
			sprintf( str + strlen( str ), "--maxholdups %d ", this->maxholdups );
		} if( this->masterpass != NULL ) {
			sprintf( str + strlen( str ), "--masterpass %s ", this->masterpass );
		} else {
			sprintf( str + strlen( str ), "--masterpass %s ", Server::Settings::masterpass );
		}
		sprintf( str + strlen( str ), "--maxholdups 1 --renaming %d -dd --research %d --era %d --thrones %d %d %d --requiredap %d --mapfile \"%s\" \"%s%lu\"", 
			this->renaming,
			this->research,
			this->age, 
			this->t[0], 
			this->t[1], 
			this->t[2], 
			this->t[3], 
			this->mapName, 
			this->name,
			this->id );
		return str;
	}

	void Match::update( Match * match )
	{
		this->mapid = match->mapid;
		free( this->mapName );
		this->mapName = (char*)calloc( strlen(match->mapName)+1, sizeof(char));
		strcpy( this->mapName, match->mapName );
		this->research = match->research;
		this->renaming = match->renaming;
		this->clientstart = match->clientstart;
		this->t[0] = match->t[0];
		this->t[1] = match->t[1];
		this->t[2] = match->t[2];
		this->t[3] = match->t[3];
		this->maxholdups = match->maxholdups;
		delete( match->mods );
		match->mods = new std::vector<Mod*>();
		for( auto m : *match->mods ) {
			match->mods->push_back( new Mod(m) );
		}
	}
}
