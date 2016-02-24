#pragma once
#include <mysql.h>
#include <stdint.h>
#include <my_global.h>
#undef min
#undef max
#include <vector>
#include "mod.hpp"
namespace Game
{
	class Match
	{
		public:
		Match( unsigned long id, int mapid, char * mapname, char * imgname, int age, char * name, int status, int port, int t1, int t2, int t3, int t4, int research, int renaming );
		Match( const MYSQL_ROW match, const MYSQL_ROW map, std::vector<Game::Mod*> * mods );
		Match( Game::Match * match );
		Match() {};
		~Match( void );
		char* createConfStr( void );
		void update( Game::Match * match );
		unsigned long id;
		int mapid;
		char * mapName;
		char * imgName;
		int status;
		char * name;
		int age;
		int port;
		int * t;
		int research;
		int renaming;
		int clientstart;
		int hostday;
		int hosthour;
		int hostint;
		int needsrestart;
		std::vector<Game::Mod*> * mods;
		private:
	};
}
