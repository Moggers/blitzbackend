#include "match.hpp"
#include <stdlib.h>
#include <string.h>

namespace Game
{
	Match::Match( int id, int mapid, char * mapname, char * imgname, int age, char * name, int status, int port ) : id{id}, mapid{mapid}, status{status}, age{age}, port{port}
	{
		this->mapName = (char*)malloc( strlen( mapname ) );
		strcpy( this->mapName, mapname );
		this->imgName = (char*)malloc( strlen( imgname ) );
		strcpy( this->imgName, imgname );
		this->name = (char*)malloc( strlen( name ) );
		strcpy( this->name, name );
	}
}
