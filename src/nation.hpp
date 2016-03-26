#pragma once
namespace Game
{
	class Nation
	{
		public:
		Nation( int id, char * name, char * title, char * turnname);
		Nation( Nation * nat );
		~Nation();
		int id;
		char * name;
		char * turnname;
		char * title;
		int computer;
	};
}

