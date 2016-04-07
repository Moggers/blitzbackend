#include "match.hpp"
#include <stdlib.h>
#include <string.h>
#include <sstream>
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
		maxholdups{atoi(match[19])},
		siterarity{atoi(match[23])}

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
		this->nations = NULL;
	}

	Match::Match( const Match& match ) :
	id{match.id},
	mapid{match.mapid},
	status{match.status},
	age{match.age},
	port{match.port},
	research{match.research},
	renaming{match.renaming},
	clientstart{match.clientstart},
	hostday{match.hostday},
	hosthour{match.hosthour},
	hostint{match.hostint},
	needsrestart{match.needsrestart},
	maxholdups{match.maxholdups},
	siterarity{match.siterarity}
	{
		this->mapName = (char*)calloc( 1024, sizeof( char ) );
		this->imgName = (char*)calloc( 1024, sizeof( char ) );
		this->name = (char*)calloc( 1024, sizeof( char ) );
		this->t = (int*)calloc( 4, sizeof( int ) );

		strcpy( this->mapName, match.mapName );
		strcpy( this->imgName, match.imgName );
		strcpy( this->name, match.name );
		this->t[0] = match.t[0];
		this->t[1] = match.t[1];
		this->t[2] = match.t[2];
		this->t[3] = match.t[3];

		if( match.masterpass == NULL ) {
			this->masterpass = NULL;
		} else {
			this->masterpass = strdup( match.masterpass );
		}

		this->mods = new std::vector<Mod*>();
		for( auto mod : *match.mods ) {
			this->mods->push_back( new Mod( mod ) );
		}
		this->nations = new std::vector<Nation*>();
		for( auto nation : *match.nations ) {
			this->nations->push_back( new Nation( nation ) );
		}

	}

	Match::~Match( void )
	{
		free( this->mapName );
		free( this->imgName );
		free( this->name );
		free( this->t );
		free( this->masterpass );
		for( Game::Mod * mod : *this->mods )
		{
			delete( mod );
		}
		for( Game::Nation * nation : *this->nations )
		{
			delete( nation );
		}
		delete( this->mods );
		delete( this->nations );
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
		if( this->nations != NULL ) {
			for( std::vector<Game::Nation*>::iterator it = this->nations->begin(); it != this->nations->end(); it++ )
			{
				if( (*it) != NULL ) {
					if( (*it)->computer == 1 ) { 
						sprintf( str + strlen(str), "--masterai %d ", (*it)->id );
					}
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
		sprintf( str + strlen( str ), "--magicsites %d --maxholdups 1 --renaming %d -dd --research %d --era %d --thrones %d %d %d --requiredap %d --mapfile \"%s\" \"%s%lu\"", 
			this->siterarity,
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

	char** Match::createEnvVars( void )
	{
		char ** envs = (char**)calloc( 6, sizeof( char* ) );
		envs[0] = (char*)calloc( 512, sizeof( char ) ); // MAP PATH
		envs[1] = (char*)calloc( 512, sizeof( char ) ); // LOCALMAP PATH
		envs[2] = (char*)calloc( 512, sizeof( char ) ); // MOD PATH
		envs[3] = (char*)calloc( 512, sizeof( char ) ); // SAVE PATH
		envs[4] = (char*)calloc( 512, sizeof( char ) ); // SAVE PATH
		envs[5] = (char*)calloc( 512, sizeof( char ) ); // SAVE PATH
		std::ostringstream stream;
		stream << "DOM4_MAPS=" << Server::Settings::savepath << this->name << this->id << "/maps/";
		envs[0] = strdup(stream.str().c_str());
		stream.str("");
		stream << "DOM4_LOCALMAPS=" << Server::Settings::savepath << this->name << this->id << "/maps/";
		envs[1] = strdup(stream.str().c_str());
		stream.str("");
		stream << "DOM4_MODS=" << Server::Settings::savepath << this->name << this->id << "/mods/";
		envs[2] = strdup(stream.str().c_str());
		stream.str("");
		stream << "DOM4_SAVE=" << Server::Settings::savepath;
		envs[3] = strdup(stream.str().c_str());
		stream.str("");
		stream << "DOM4_DATA=" << Server::Settings::exepath << "/data/";
		envs[4] = strdup(stream.str().c_str());
		stream.str("");
		stream << "DOM4_CONF=" << Server::Settings::savepath << "/../";
		envs[5] = strdup(stream.str().c_str());
		return envs;
	}

	void Match::destroyEnvVars( char ** envs )
	{
		for( int i = 0; i < 6; i++)
			free( envs[i] );
		free( envs );
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
		this->siterarity = match->siterarity;
		delete( match->mods );
		match->mods = new std::vector<Mod*>();
		for( auto m : *match->mods ) {
			match->mods->push_back( new Mod(m) );
		}
	}
}
