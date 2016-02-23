#include "turnparser.hpp"
#include <string>
#include <regex>
#include <stdlib.h>

namespace Server
{
	TurnParser::TurnParser( std::string jsondir, int turnN )
	{
		std::ostringstream fname;
		this->jsondir = jsondir;
		fname << this->jsondir << turnN;
		this->filename = fname.str();
		fileread = std::ifstream( fname.str() ); 
		if( fileread.is_open() ) {
			fileread >> root;
		}
		cur_battle.provid = -1;
		std::cout << "First (seen) turn " << turnN << " writing to " << filename << "\n";
	}

	void TurnParser::newTurn( int turnN )
	{
		std::ostringstream fname;
		fname << this->jsondir << turnN;
		this->filename = fname.str();
		fileread = std::ifstream( fname.str() ); 
		if( fileread.is_open() ) {
			fileread >> root;
		}
		cur_battle.provid = -1;
		std::cout << "New turn " << turnN << " writing to " << filename << "\n";
	}

	void TurnParser::addProvinceOwnership( int nationid, int provid )
	{ 
		root["provinces"][provid] = nationid;
	}

	int TurnParser::parseLine( std::string line )
	{
		std::string sl(line);
		std::smatch smatch;
		if( std::regex_match( sl, smatch, std::regex(R"(^nation ([0-9]+)  start land ([0-9]+).*)"))) {
			std::cout << "Nation " << smatch[1].str() << " starts in " << smatch[2].str() << "\n";
			addProvinceOwnership( atoi(smatch[1].str().c_str()),atoi(smatch[2].str().c_str()) );
			return 0;
		}
		if( std::regex_match( sl, smatch, std::regex(R"(^ *([0-9]+) vs ([0-9]+) in lnr ([0-9]+).*)"))) {
			std::cout << "Battle starting in " << smatch[3].str() << " between " << smatch[1].str() << " and " <<smatch[2].str() << "\n";
			cur_battle.nationa = atoi(smatch[1].str().c_str());
			cur_battle.nationb = atoi(smatch[2].str().c_str());
			cur_battle.provid = atoi(smatch[3].str().c_str());
			return 0;
		}
		if( cur_battle.provid != -1 ) {
			if( std::regex_match( sl, smatch, std::regex(R"(^_{5}The winner is ([0-9]+)_{7}.*)"))) {
				addProvinceOwnership( cur_battle.provid, atoi(smatch[1].str().c_str()) );
				std::cout << "The winner was " << smatch[1].str() << "\n";
				cur_battle.provid = -1;
			}
		}
		return 1;
	}

	int TurnParser::changeTurn( int turn )
	{
		return 0;
	}

	void TurnParser::writeTurn( void )
	{
		std::ofstream writer(filename);
		writer << root;
		writer.close();
	}
}
