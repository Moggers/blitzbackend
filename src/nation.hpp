#pragma once
namespace Game
{
	class Nation
	{
		public:
		Nation( int id, char * name, char * title, char * turnname);
		int id;
		char * name;
		char * turnname;
		char * title;
	};
}

