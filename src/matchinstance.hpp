#pragma once
#include "match.hpp"
#include "util.hpp"
#include "matchwatcher.hpp"
#include <stdio.h>
namespace Server
{
	class MatchInstance
	{
		public:
		Game::Match * match;
		popen2_t * process;
		MatchInstance( popen2_t * process, Game::Match * match );
		MatchInstance( void );
		int shutdown( void );
		Server::MatchWatcher * watcher;
	};
}
