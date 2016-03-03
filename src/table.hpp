#pragma once
#include "nation.hpp"
#include <mysql.h>
#include <my_global.h>
#undef min
#undef max
#include "match.hpp"
#include <stdarg.h>
#include "mod.hpp"
#include <mutex>
#include <vector>
#include "emailsender.hpp"
namespace SQL
{
	class Table
	{
		public:
		Table( void );
		~Table( void );
		Game::Match ** getAllMatches( void );
		Game::Match ** getMatchesByStatus( int count, ... );
		std::vector<Game::Mod*> * getModsByMatch( int matchid );
		void saveMatch( Game::Match * match );
		void deleteMatch(Game::Match * match );
		void removeNationFromMatch( Game::Match * match, Game::Nation * nation );
		void addNationToMatch( Game::Match * match, Game::Nation * nation );
		void addTurn( Game::Match * match, int tn );
		void updateTimestamp( Game::Match * match );
		void updateMatchSettings( Game::Match * match );
		void markRestarted( Game::Match * match );
		int getTurnNumber( Game::Match * match );
		int checkPlayerPresent( Game::Match * match, Game::Nation * nation );
		Game::Nation * getNation( int id );
		Game::Nation ** getDeleteRequests( Game::Match * match );
		std::vector<Game::Nation*> * getNations( Game::Match * match );
		void setTurnfileName( int nationid, const char * name );
		std::vector<Server::emailrequest_t> * getEmailRequests( int match_id );
		std::vector<Server::emailrequest_t> * getUniqueEmailRequests( int match_id );

		private:
		MYSQL * m_con;
		std::recursive_mutex tablelock;
	};
}
