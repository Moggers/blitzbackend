#include "matchhandler.hpp"
#include "util.hpp"
#include <iterator>
#include "settings.hpp"
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <algorithm>
#include "matchhandler.hpp"

namespace Server
{
	std::list<int> MatchHandler::ports;

	MatchHandler::MatchHandler( void )
	{
		this->m_table = new SQL::Table();
		for( int ii = 2048; ii < 4096; ii++ ) {
			ports.push_back( ii );
		}
	}

	int MatchHandler::getPort( void )
	{
		int port = ports.back();
		ports.pop_back();
		return port;
	}
	void MatchHandler::addPort( int port )
	{
		ports.push_back( port );
	}
	int MatchHandler::getSpecificPort( int port )
	{
		auto res = std::find( ports.begin(), ports.end(), port );
		if( res == ports.end() ) return -1;
		ports.erase( res );
		return port;
	}

	std::vector<Server::MatchInstance*>::iterator MatchHandler::getMatchInstance( Game::Match * c )
	{
		return find_if( m_matches.begin(), m_matches.end(), [c](Server::MatchInstance * in)->int
			{ if( in->match->id == c->id ) return 1; return 0; } );
	}

	void MatchHandler::startNewServers( void )
	{
		Game::Match ** matches = m_table->getMatchesByStatus( 2, 0, 1 );
		if( matches == NULL ) return;
		int ii = 0;
		Game::Match * cmatch = NULL;
		while( ( cmatch = matches[ii++] ) != NULL ) {
			fprintf( stdout, "Found a match named %s (status %d). ", cmatch->name, cmatch->status );
			int port = 0;
			if( cmatch->status == 0 ) {
				fprintf( stdout, "It is new\n");
				port = getPort();
			} else if( getMatchInstance( cmatch ) == m_matches.end() ) {
				fprintf( stdout, "It is meant to be running on port %d, but I don't know about it\n", cmatch->port );
				port = getSpecificPort( cmatch->port );
				if( port == -1 ) {
					port = getPort();
					fprintf( stdout, "The port he wanted does not seem to be available, I'm instead assigning him %d\n", port );
				}
			} else {
				fprintf( stdout, "I already know about this.\n" );
				continue;
			}
			char * com = (char*)calloc( 128, sizeof( char ) );
			sprintf( com, "%s --era %d -T --tcpserver --port %d \"%s\"", 
				Server::Settings::exepath, 
				cmatch->age, 
				port,
				cmatch->name );
			popen2_t * proc = (popen2_t*)calloc( 1, sizeof( popen2_t ) );
			popen2( com, proc );
			Server::MatchInstance * inst = new Server::MatchInstance( proc, cmatch, port );
			m_matches.push_back( inst );
			fprintf( stdout, "Started server on port %d.\n", port );
			cmatch->status = 1;
			cmatch->port = port;
			m_table->saveMatch( cmatch );
		}
	}

	void MatchHandler::beginGames( void )
	{
		Game::Match ** matches = m_table->getMatchesByStatus( 3, 4 );
		if( matches == NULL ) return;
		int ii = 0;
		Game::Match * cmatch = NULL;
		while( ( cmatch = matches[ii++] ) != NULL ) {
		}
	}
}

