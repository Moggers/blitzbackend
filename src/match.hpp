#pragma once
#include <mysql.h>
#include <my_global.h>
namespace Game
{
	class Match
	{
		public:
		Match( int id, int mapid, char * mapname, char * imgname, int age, char * name, int status, int port, int t1, int t2, int t3, int t4, int research, int renaming );
		Match( MYSQL_ROW match, MYSQL_ROW map );
		char* createConfStr( void );
		int id;
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
		private:
	};
}
