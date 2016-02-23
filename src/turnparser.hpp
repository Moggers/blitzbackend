#pragma once
#include <json/json.h>
#include <string>
#include <iostream>
#include <fstream>
namespace Server
{
	typedef struct turn_battle_s
	{
		int provid;
		int nationa;
		int nationb;
	} turn_battle_t;

	class TurnParser
	{
		public:
		TurnParser( std::string turnfile, int turnN );

		int parseLine( std::string line );
		int changeTurn( int turn );
		void writeTurn( void );
		void newTurn( int turnN );
		
		private:
		Json::Value root;
		std::ifstream fileread;
		std::string jsondir;
		std::string filename;
		turn_battle_t cur_battle;

		void addProvinceOwnership( int nationid, int provid );
	};
}
