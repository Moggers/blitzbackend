#include "matchhandler.hpp"
#include <stdlib.h>
#include "settings.hpp"
#include <unistd.h>
int main( int argc, char ** argv )
{
	char * confpath = (char*)calloc( 512, sizeof( char ) );
	sprintf( confpath, "%s/.config/blitzbackend/", getenv( "HOME" ) );
	Server::Settings::loadSettings( confpath );
	Server::MatchHandler * matchHandler = new Server::MatchHandler();
	signal( SIGINT, matchHandler->shutdown_callback );
	while( true ) {
		matchHandler->startNewServers();
		matchHandler->beginGames();
		matchHandler->shutdownServers();
		sleep( 1 );
	}
	return 0;
}
