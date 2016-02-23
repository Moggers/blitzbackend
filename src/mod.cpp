#include "mod.hpp"
#include <string.h>
#include <stdlib.h>

namespace Game
{
	Mod::Mod( int id, char *dmname ): m_id(id)
	{
		m_dmname = (char*)calloc( strlen(dmname)+1,sizeof(char));
		strcpy( m_dmname, dmname );
	}

	Mod::Mod( Game::Mod * mod )
	{
		this->m_dmname = (char*)calloc( strlen( mod->m_dmname)+1, sizeof( char ) );
		strcpy( this->m_dmname, mod->m_dmname );
	}
}
