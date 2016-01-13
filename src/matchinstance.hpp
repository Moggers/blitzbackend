#pragma once
#include "match.hpp"
#include "util.hpp"
#include <stdio.h>
namespace Server
{
	class MatchInstance
	{
		public:
		Game::Match * match;
		popen2_t * process;
		int port;
		MatchInstance( popen2_t * process, Game::Match * match, int port );
		MatchInstance( void );
	};
}
