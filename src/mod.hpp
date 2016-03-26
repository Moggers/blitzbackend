#pragma once

namespace Game
{
	class Mod
	{
		public:
		int m_id;
		char * m_dmname;
		Mod( int id, char * dmname );
		Mod( Game::Mod * mod );
		~Mod();
	};
}
