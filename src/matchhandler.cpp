#include "matchhandler.hpp"
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include "util.hpp"
#include <iterator>
#include "settings.hpp"
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <algorithm>
#include "matchhandler.hpp"

namespace Server
{
	std::list<int> MatchHandler::ports;

	MatchHandler::MatchHandler( void )
	{
		this->m_table = new SQL::Table();
		for( int ii = 2048; ii < 4096; ii++ ) {
			ports.push_back( ii );
		}
	}

	int MatchHandler::getPort( void )
	{
		int port = ports.back();
		ports.pop_back();
		return port;
	}
	void MatchHandler::addPort( int port )
	{
		ports.push_back( port );
	}
	int MatchHandler::getSpecificPort( int port )
	{
		auto res = std::find( ports.begin(), ports.end(), port );
		if( res == ports.end() ) return -1;
		ports.erase( res );
		return port;
	}

	std::vector<Server::MatchInstance*>::iterator MatchHandler::getMatchInstance( Game::Match * c )
	{
		return find_if( m_matches.begin(), m_matches.end(), [c](Server::MatchInstance * in)->int
			{ if( in->match->id == c->id ) return 1; return 0; } );
	}

	void MatchHandler::startNewServers( void )
	{
		Game::Match ** matches = m_table->getMatchesByStatus( 2, 0, 1 );
		if( matches == NULL ) return;
		int ii = 0;
		Game::Match * cmatch = NULL;
		while( ( cmatch = matches[ii++] ) != NULL ) {
			fprintf( stdout, "Found a match named %s (status %d). ", cmatch->name, cmatch->status );
			int port = 0;
			if( cmatch->status == 0 ) {
				fprintf( stdout, "It is new\n");
				port = getPort();
			} else if( getMatchInstance( cmatch ) == m_matches.end() ) {
				fprintf( stdout, "It is meant to be running on port %d, but I don't know about it\n", cmatch->port );
				port = getSpecificPort( cmatch->port );
				if( port == -1 ) {
					port = getPort();
					fprintf( stdout, "The port he wanted does not seem to be available, I'm instead assigning him %d\n", port );
				}
			} else {
				fprintf( stdout, "I already know about this.\n" );
				continue;
			}
			char * com = (char*)calloc( 128, sizeof( char ) );
			sprintf( com, "%s --era %d -T --tcpserver --port %d \"%s\"", 
				Server::Settings::exepath, 
				cmatch->age, 
				port,
				cmatch->name );
			popen2_t * proc = (popen2_t*)calloc( 1, sizeof( popen2_t ) );
			popen2( com, proc );
			Server::MatchInstance * inst = new Server::MatchInstance( proc, cmatch );
			m_matches.push_back( inst );
			fprintf( stdout, "Started server on port %d.\n", port );
			cmatch->status = 1;
			cmatch->port = port;
			m_table->saveMatch( cmatch );
		}
	}

	void MatchHandler::beginGames( void )
	{
		// Retrieve matches that need to be begun
		Game::Match ** matches = m_table->getMatchesByStatus( 2, 3, 4 );
		if( matches == NULL ) return;

		//Iterate through matches
		int ii = 0;
		Game::Match * cmatch = NULL;
		while( ( cmatch = matches[ii++] ) != NULL ) {
			std::vector<Server::MatchInstance*>::iterator cimatchi = getMatchInstance( cmatch );
			fprintf( stdout, "I see a request to begin a match named %s.\n", cmatch->name );
			Server::MatchInstance * cimatch = NULL;
			if( cimatchi == m_matches.end() ) { // If we reached the end of th matches then create a new one normally
				fprintf( stdout, "There didn't appear to be a lobby running so it was not necessary to kill it.\n" );
				cimatch = new Server::MatchInstance( (popen2_t*)calloc( 1, sizeof( popen2_t ) ), cmatch );
				m_matches.push_back( cimatch );
				// Try to grab the port the lobby (that apparently existed in some long ago era) was already on, or a new one if it's not available
				cmatch->port = getSpecificPort( cmatch->port );
				if( cmatch->port == -1 ) cmatch->port = getPort();
			} else { // Otherwise
				if( cmatch->status == 4 ) {  // We've already started it, exit gracefully
					fprintf( stdout, "It's already running so I wont worry about it.\n" );
					continue;
				}
				cimatch = *cimatchi;
				if( kill( cimatch->process->child_pid, SIGTERM ) != 0 ) { // Kill the lobby
					fprintf( stdout, "I somehow failed to kill the lobby so the game could be started\n" );
					continue;
				}
				fprintf( stdout, "I killed the lobby\n" );
				// There used to be some port grabbing code here but we can presume the lobby's port is now available
			}
			char * comstr = (char*)calloc( 256, sizeof( char ) );
			fprintf( stdout, "I'm initiating the game.\n" );
			// Copy the map files into dom4's search dir
			sprintf( comstr, "cp \"%s/%d/%s\" \"%s\"", Server::Settings::mappath_save, cmatch->mapid, cmatch->mapName, Server::Settings::mappath_load );
			system( comstr );
			sprintf( comstr, "cp \"%s/%d/%s\" \"%s\"", Server::Settings::mappath_save, cmatch->mapid, cmatch->imgName, Server::Settings::mappath_load );
			system( comstr );
			// Generate the first turn
			sprintf( comstr, "%s --era %d -T --mapfile \"%s\" --newgame \"%s\"", Settings::exepath, cmatch->age, cmatch->mapName, cmatch->name );
			popen2( comstr, cimatch->process );
			waitpid( cimatch->process->child_pid, NULL, 0 );
			// Create the server
			fprintf( stdout, "Done, hosting game now on port %d\n", cmatch->port );
			sprintf( comstr, "%s --era %d -T --tcpserver --port %d --mapfile \"%s\" \"%s\"", Settings::exepath, cmatch->age, cmatch->port, cmatch->mapName, cmatch->name );
			popen2( comstr, cimatch->process );
			// Mark the game as started and write it to the table
			cmatch->status = 4;
			m_table->saveMatch( cmatch );
		}
	}
}

