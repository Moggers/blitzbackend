#include "nation.hpp"
#include <stdlib.h>
#include <string.h>

namespace Game
{
	Nation::Nation( int id, char * name, char * title, char * turnname ): id{id}, computer{1}
	{
		this->name = (char*)calloc( strlen( name ) + 1, sizeof( char ) );
		this->title = (char*)calloc( strlen( title ) + 1, sizeof( char ) );
		this->turnname = (char*)calloc( strlen( turnname ) + 1, sizeof( char ) );

		memcpy( this->name, name, strlen( name ) * sizeof(char) );
		memcpy( this->title, title, strlen( title ) * sizeof(char) );
		memcpy( this->turnname, turnname, strlen( turnname ) * sizeof(char) );
	}
}
