#pragma once
namespace Game
{
	class Match
	{
		public:
		Match( int id, char * mapname, int age, char * name, int status, int port );
		int id;
		char * mapName;
		int status;
		char * name;
		int age;
		int port;
		private:
	};
}
