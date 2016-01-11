#include <array>
#include "table.hpp"
#include "matchinstance.hpp"
#include "match.hpp"
namespace Server
{
	class MatchHandler
	{
		public:
		static int getPort( void );
		static int addPort( int port );
		MatchHandler( void );
		void startNewServers( void );

		private:
		SQL::Table * m_table;
		std::array<Server::MatchInstance, 128> m_matches;
		static int ports[4096];
		static int porttop;
	};
}
