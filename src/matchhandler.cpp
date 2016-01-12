#include "matchhandler.hpp"
#include <iterator>
#include "settings.hpp"
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <algorithm>

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

	void MatchHandler::startNewServers( void )
	{
		Game::Match ** matches = m_table->getMatchesByStatus( 2, 0, 1 );
		if( matches == NULL ) return;
		int ii = 0;
		Game::Match * cmatch = NULL;
		while( ( cmatch = matches[ii++] ) != NULL )
		{
			fprintf( stdout, "Found a match named %s (status %d). ", cmatch->name, cmatch->status );
			int port = 0;
			if( cmatch->status == 0 ) {
				fprintf( stdout, "It is new\n");
				port = getPort();
			} else if( find_if( m_matches.begin(), m_matches.end(),[cmatch](Server::MatchInstance * in)->int
				{ if( in->match->id == cmatch->id ) return 1; return 0; } ) == m_matches.end() ) {
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
			sprintf( com, "%s --era %d --mapfile \"%s%s\" -T --tcpserver --port %d \"%s\"", 
				Server::Settings::exepath, 
				cmatch->age, 
				Settings::mappath, 
				cmatch->mapName, 
				port,
				cmatch->name );
			FILE * pipe = popen( com, "r" );
			Server::MatchInstance * inst = new Server::MatchInstance( pipe, cmatch, port );
			m_matches.push_back( inst );
			fprintf( stdout, "Started server on port %d.\n", port );
			cmatch->status = 1;
			cmatch->port = port;
			m_table->saveMatch( cmatch );
		}
	}
}

