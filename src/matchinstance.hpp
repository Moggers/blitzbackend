#pragma once
#include "match.hpp"
#include <stdio.h>
namespace Server
{
	class MatchInstance
	{
		public:
		Game::Match * match;
		FILE * pipe;
		int port;
		MatchInstance( FILE * pipe, Game::Match * match, int port );
		MatchInstance( void );
	};
}
