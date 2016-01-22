#include "matchhandler.hpp"
#include "settings.hpp"
int main( int argc, char ** argv )
{
	Server::Settings::loadSettings( "../server.cfg" );
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
