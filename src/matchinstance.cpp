#include "matchinstance.hpp"

namespace Server
{
	MatchInstance::MatchInstance( void )
	{
		pipe = NULL;
		port = 0;
		match = NULL;
	}
	MatchInstance::MatchInstance( FILE * pipe, Game::Match * match, int port ) :match{match}, pipe{pipe}, port{port}
	{
	}
}
