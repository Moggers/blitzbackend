#include "matchinstance.hpp"
#include <stdlib.h>

namespace Server
{
	MatchInstance::MatchInstance( void )
	{
		process = (popen2_t*)calloc( 1, sizeof( popen2_t ) );
		port = 0;
		match = NULL;
	}
	MatchInstance::MatchInstance( popen2_t * process, Game::Match * match, int port ) :match{match}, port{port}, process{process}
	{
	}
}
