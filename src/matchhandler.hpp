#pragma once
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
		// Default constructor
		MatchHandler( void );
		// Retrieve all servers in the database in the lobby phase and spin them up
		void startNewServers( void );
		// Retrieve all servers inthe database in the play phase and spin them up
		void beginGames( void );

		private:
		// Find an instance associated with a match
		std::vector<Server::MatchInstance*>::iterator getMatchInstance( Game::Match * c );
		// Retrieve a unique port from the stack
		static int getPort( void );
		// Notify the port stack that <port> is no longer being used
		static void addPort( int port );
		// Request a specific port from the stack, returns -1 if it's taken
		static int getSpecificPort( int port );
		// Database access for this handler
		SQL::Table * m_table;
		// List of servers
		std::vector<Server::MatchInstance*> m_matches;
		// Available ports
		static std::list<int> ports;
	};
}
