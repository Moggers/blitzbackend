#include "turnparser.hpp"
#include <string>
#include <regex>
#include <stdlib.h>
#include <sys/stat.h>

namespace Server
{
	TurnParser::TurnParser( void )
	{
		cur_battle.provid = -1;
		batcher.addCheck(R"(.*packet.*)", [](const std::smatch &match){});
		batcher.addCheck(R"(.*No 2h for.*)", [](const std::smatch &match){});
		batcher.addCheck(R"(^nation ([0-9]+)  start land ([0-9]+).*)", [this](const std::smatch &match){
			std::cout << "Nation " << match[1].str() << " starts in " << match[2].str() << "\n";
			addProvinceOwnership( atoi(match[1].str().c_str()),atoi(match[2].str().c_str()) );
		});
		batcher.addCheck(R"(^ *([0-9]+) vs ([0-9]+) in lnr ([0-9]+).*)", [this](const std::smatch &match){
			std::cout << "Battle starting in " << match[3].str() << " between " << match[1].str() << " and " <<match[2].str() << "\n";
			cur_battle.nationa = atoi(match[1].str().c_str());
			cur_battle.nationb = atoi(match[2].str().c_str());
			cur_battle.provid = atoi(match[3].str().c_str());
		});
		batcher.addCheck(R"(^_{5}The winner is ([0-9]+)_{7}.*)", [this](const std::smatch &match){
			if( cur_battle.provid != -1 ) {
				addProvinceOwnership( atoi(match[1].str().c_str()), cur_battle.provid );
				std::cout << "The winner was " << match[1].str() << "\n";
				cur_battle.provid = -1;
			}
		});
		batcher.addCheck(R"(.*([0-9]+) was conquered by ([0-9]+).*)", [this](const std::smatch &match){
			addProvinceOwnership( atoi(match[2].str().c_str()), atoi(match[1].str().c_str()));
		});
	}

	void TurnParser::newTurn( const std::string &jsondir, int turnN )
	{
		this->jsondir = std::string(jsondir);
		this->newTurn( turnN );
	}

	void TurnParser::newTurn( int turnN )
	{
		std::ostringstream fname;
		mkdir( this->jsondir.c_str(), 0775 );
		fname << this->jsondir << "/" << turnN << ".json";
		this->filename = fname.str();
		fileread = std::ifstream( fname.str() ); 
		if( fileread.is_open() ) {
			fileread >> root;
		}
		cur_battle.provid = -1;
		std::cout << "New turn " << turnN << " writing to " << filename << "\n";
		fname << ".log";
		this->logwriter.open( fname.str(), std::ofstream::out );
	}

	void TurnParser::addProvinceOwnership( int nationid, int provid )
	{ 
		root["provinces"][provid] = nationid;
	}

	int TurnParser::parseLine( std::string line )
	{
		this->logwriter << line << '\n';
		batcher.checkString( line );
		return 1;
	}

	void TurnParser::writeTurn( void )
	{
		std::cout << "Writing turn\n";
		logwriter.close();
		std::ofstream writer(filename, std::ofstream::out);
		writer << root << std::endl;
		writer.close();
	}
}
