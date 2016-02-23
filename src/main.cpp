#include "matchhandler.hpp"
#include <stdlib.h>
#include "settings.hpp"
#include <sstream>
#include <fstream>
#include <string>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <sstream>
#include "turnparser.hpp"
int main( int argc, char ** argv )
{
	char * confpath = (char*)calloc( 512, sizeof( char ) );
	sprintf( confpath, "%s/.config/blitzbackend/", getenv( "HOME" ) );
	Server::Settings::loadSettings( confpath );
	Server::MatchHandler * matchHandler = new Server::MatchHandler();
	signal( SIGINT, matchHandler->shutdown_callback );
	while( !sleep(1) ) {
		matchHandler->startNewServers();
		matchHandler->beginGames();
		matchHandler->shutdownServers();
	}
	return 0;
}
