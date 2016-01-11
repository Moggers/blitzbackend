#include "matchhandler.hpp"
#include "settings.hpp"
int main( int argc, char ** argv )
{
	Server::Settings::loadSettings( "../server.cfg" );
	Server::MatchHandler * matchHandler = new Server::MatchHandler();
	matchHandler->startNewServers();
	return 0;
}
