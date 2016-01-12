#include "matchhandler.hpp"
#include "settings.hpp"
int main( int argc, char ** argv )
{
	Server::Settings::loadSettings( "../server.cfg" );
	Server::MatchHandler * matchHandler = new Server::MatchHandler();
	while( true ) {
		fprintf( stdout, "Polling\n" );
		matchHandler->startNewServers();
		sleep( 1 );
		fprintf( stdout, "Done\n" );
	}
	return 0;
}
