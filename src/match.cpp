#include "match.hpp"
#include <stdlib.h>
#include <string.h>

namespace Game
{
	Match::Match( int id, char * mapname, int age, char * name, int status, int port ) : id{id}, status{status}, age{age}, port{port}
	{
		this->mapName = (char*)malloc( strlen( mapname ) );
		strcpy( this->mapName, mapname );
		this->name = (char*)malloc( strlen( name ) );
		strcpy( this->name, name );
	}
}
