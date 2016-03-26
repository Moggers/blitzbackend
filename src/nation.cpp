#include "nation.hpp"
#include <stdlib.h>
#include <string.h>

namespace Game
{
	Nation::Nation( int id, char * name, char * title, char * turnname ): id{id}, computer{0}
	{
		this->name = (char*)calloc( strlen( name ) + 1, sizeof( char ) );
		this->title = (char*)calloc( strlen( title ) + 1, sizeof( char ) );
		this->turnname = (char*)calloc( strlen( turnname ) + 1, sizeof( char ) );

		memcpy( this->name, name, strlen( name ) * sizeof(char) );
		memcpy( this->title, title, strlen( title ) * sizeof(char) );
		memcpy( this->turnname, turnname, strlen( turnname ) * sizeof(char) );
	}

	Nation::Nation( Nation * nat): id{nat->id}, computer{nat->computer}
	{
		this->name = strdup( nat->name );
		this->title = strdup( nat->title);
		this->turnname = strdup( nat->turnname );
	}

	Nation::~Nation()
	{
		free( name );
		free( title );
		free( turnname );
	}
}
