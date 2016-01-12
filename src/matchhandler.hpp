#include <vector>
#include "table.hpp"
#include "matchinstance.hpp"
#include "match.hpp"
#include <list>
namespace Server
{
	class MatchHandler
	{
		public:
		static int getPort( void );
		static void addPort( int port );
		static int getSpecificPort( int port );
		MatchHandler( void );
		void startNewServers( void );

		private:
		SQL::Table * m_table;
		std::vector<Server::MatchInstance*> m_matches;
		static std::list<int> ports;
	};
}
