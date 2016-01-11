#include "matchhandler.hpp"
#include "settings.hpp"
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>

namespace Server
{
	int MatchHandler::ports[4096];
	int MatchHandler::porttop;

	MatchHandler::MatchHandler( void )
	{
		this->m_table = new SQL::Table();
		for( int ii = 0; ii < 4096; ii++ ) {
			ports[ii] = ii;
			porttop = 4095;
		}
	}

	int MatchHandler::getPort( void )
	{
		return ports[porttop--];
	}
	int MatchHandler::addPort( int port )
	{
		return ports[++porttop] = port;
	}

	void MatchHandler::startNewServers( void )
	{
		Game::Match ** matches = m_table->getNewMatches();
		int ii = 0;
		Game::Match * cmatch = NULL;
		while( ( cmatch = matches[ii++] ) != NULL )
		{
			fprintf( stdout, "Found a new requested match with map %s\n", cmatch->mapName );
			int port = getPort();
			char * com = (char*)calloc( 128, sizeof( char ) );
			sprintf( com, "%s --era %d --mapfile %s%s -T --tcpserver --port %d %s", 
				Server::Settings::exepath, 
				cmatch->age, 
				Settings::mappath, 
				cmatch->mapName, 
				port,
				cmatch->name );
			FILE * pipe = popen( com, "r" );
			Server::MatchInstance * inst = new Server::MatchInstance( pipe, cmatch, port );
			fprintf( stdout, "Started server on port %d\nThe pointer to the pipe is %p\n", port, pipe );
			cmatch->status = 1;
			m_table->saveMatch( cmatch );
		}
	}
}

