#pragma once
#include "match.hpp"
#include "util.hpp"
#include "matchwatcher.hpp"
#include "table.hpp"
#include <stdio.h>
#include "emailsender.hpp"
namespace Server
{
	static int STATE_LOBBY = 0;
	static int STATE_RUNNING = 1;
	static int STATE_MAPGEN = 2;
	class MatchInstance
	{
		public:
		Game::Match * match;
		popen2_t * process;
		SQL::Table * m_table;
		MatchInstance( Game::Match * match, SQL::Table * table, int state, EmailSender * emailSender );
		MatchInstance( void );
		int shutdown( void );
		void restart( void );
		int checkTimer( Game::Match * match );
		void allowTurnChanges();
		void moveInTurns();
		Server::MatchWatcher * watcher;

		private:
		MatchInstance( popen2_t * process, Game::Match * match, SQL::Table * table, EmailSender * emailSender );
	};
}
