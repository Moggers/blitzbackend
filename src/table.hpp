#pragma once
#include "nation.hpp"
#include <mysql.h>
#include <my_global.h>
#undef min
#undef max
#include "match.hpp"
#include <stdarg.h>
namespace SQL
{
	class Table
	{
		public:
		Table( void );
		~Table( void );
		Game::Match ** getAllMatches( void );
		Game::Match ** getMatchesByStatus( int count, ... );
		void saveMatch( Game::Match * match );
		void deleteMatch(Game::Match * match );
		void removeNationFromMatch( Game::Match * match, Game::Nation * nation );
		void addNationToMatch( Game::Match * match, Game::Nation * nation );
		Game::Nation * getNation( int id );
		Game::Nation ** getDeleteRequests( Game::Match * match );

		private:
		MYSQL * m_con;
	};
}
