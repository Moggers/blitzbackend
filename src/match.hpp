#pragma once
namespace Game
{
	class Match
	{
		public:
		Match( int id, int mapid, char * mapname, char * imgname, int age, char * name, int status, int port );
		int id;
		int mapid;
		char * mapName;
		char * imgName;
		int status;
		char * name;
		int age;
		int port;
		private:
	};
}
