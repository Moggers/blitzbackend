#pragma once
#include <json/json.h>
#include "regexbatcher.hpp"
#include <string>
#include <iostream>
#include <fstream>
#include <regex>
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
		TurnParser( void );

		int parseLine( std::string line );
		int changeTurn( int turn );
		void writeTurn( void );
		void newTurn( int turnN );
		void newTurn( const std::string &jsondir, int turnN );
		
		private:
		Json::Value root;
		std::ifstream fileread;
		std::string jsondir;
		std::string filename;
		std::string log;
		std::ofstream logwriter;
		turn_battle_t cur_battle;
		std::vector<std::regex> regex_set;
		RegexBatcher batcher;

		void addProvinceOwnership( int nationid, int provid );
	};
}
