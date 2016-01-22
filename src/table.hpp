#pragma once
#include <mysql.h>
#include <my_global.h>
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

		private:
		MYSQL * m_con;
	};
}
