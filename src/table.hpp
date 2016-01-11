#pragma once
#include <mysql.h>
#include <my_global.h>
#include "match.hpp"
namespace SQL
{
	class Table
	{
		public:
		Table( void );
		~Table( void );
		Game::Match ** getAllMatches( void );
		Game::Match ** getNewMatches( void );
		void saveMatch( Game::Match * match );

		private:
		MYSQL * m_con;
	};
}
