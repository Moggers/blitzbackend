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
#include <signal.h>
#include "matchhandler.hpp"

namespace Server
{
	MatchHandler * MatchHandler::WhyDoIHaveToDoThis;

	MatchHandler::MatchHandler( void )
	{
		this->m_table = new SQL::Table();
		WhyDoIHaveToDoThis = this;
	}

	int MatchHandler::getPort( void )
	{
		int ii = 4096;
		for( ii = 4096; ii > 2048; ii-- ) {
			if( port_check( ii ) == 1 ) {
				return ii;
			}
		}
		
		return -1;
	}
	int MatchHandler::getSpecificPort( int port )
	{
		if( port_check( port ) == 1 ) 
			return port;
		else return -1;
	}

	std::vector<Server::MatchInstance*>::iterator MatchHandler::getMatchInstance( Game::Match * c )
	{
		return find_if( m_matches.begin(), m_matches.end(), [c](Server::MatchInstance * in)->int
			{ if( in->match->id == c->id ) return 1; return 0; } );
	}

	void MatchHandler::shutdown_callback( int signum )
	{
		fprintf( stdout, "Shutting down all servers\n" );
		for( std::vector<Server::MatchInstance*>::iterator iter = WhyDoIHaveToDoThis->m_matches.begin(); iter != WhyDoIHaveToDoThis->m_matches.end(); iter++ ) {
			(*iter)->shutdown();
			fprintf( stdout, "Shutting down %s\n", (*iter)->match->name );
		}
		exit( 1 );
	}

	void MatchHandler::startNewServers( void )
	{
		Game::Match ** matches = m_table->getMatchesByStatus( 6, 0, 1, 10, 11, 12, 13 );
		if( matches == NULL ) return;
		int ii = 0;
		Game::Match * cmatch = NULL;
		while( ( cmatch = matches[ii++] ) != NULL ) {
			int port = 0;
			if( cmatch->status == 0 ) {
				fprintf( stdout, "I found a new match\n");
				port = getPort();
			} else if( getMatchInstance( cmatch ) == m_matches.end() ) {
				fprintf( stdout, "I found a match, It is meant to be running on port %d, but I don't know about it\n", cmatch->port );
				port = getSpecificPort( cmatch->port );
				if( port == -1 ) {
					port = getPort();
					fprintf( stdout, "The port he wanted does not seem to be available, I'm instead assigning him %d\n", port );
				}
			} else {
				Server::MatchInstance * inst = *getMatchInstance(cmatch);
				if( inst->watcher->mesg > 0 ) {
					cmatch->status = inst->watcher->mesg / 5 + 10;
					if( cmatch->status == 10 ) cmatch->status = 3;
					m_table->saveMatch( cmatch );
					inst->watcher->mesg = 0;
				} else if( inst->watcher->mesg == -1 ) {
					fprintf( stdout, "Match %s has either died or failed to start\n", cmatch->name );
					cmatch->status = 99;
					m_table->saveMatch( cmatch );
				}
				continue;
			}
			// Copy the map files into dom4's search dir
			char * comstr = (char*)calloc( 256, sizeof( char ) );
			sprintf( comstr, "cp \"%s/%d/%s\" \"%s\"", Server::Settings::mappath_load, cmatch->mapid, cmatch->mapName, Server::Settings::mappath_save );
			system( comstr );
			sprintf( comstr, "cp \"%s/%d/%s\" \"%s\"", Server::Settings::mappath_load, cmatch->mapid, cmatch->imgName, Server::Settings::mappath_save );
			system( comstr );
			// Spawn
			char * com = (char*)calloc( 512, sizeof( char ) );
			sprintf( com,  "%s --tcpserver -T --port %d %s", Server::Settings::exepath, port, cmatch->createConfStr() );
			popen2_t * proc = (popen2_t*)calloc( 1, sizeof( popen2_t ) );
			popen2( com, proc );
			Server::MatchInstance * inst = new Server::MatchInstance( proc, cmatch );
			inst->watcher = new MatchWatcher( proc );
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
		Game::Match ** matches = m_table->getMatchesByStatus( 2, 2, 3 );
		if( matches == NULL ) return;

		//Iterate through matches
		int ii = 0;
		Game::Match * cmatch = NULL;
		while( ( cmatch = matches[ii++] ) != NULL ) {
			std::vector<Server::MatchInstance*>::iterator cimatchi = getMatchInstance( cmatch );
			Server::MatchInstance * cimatch = NULL;
			if( cimatchi == m_matches.end() ) { // If we reached the end of th matches then create a new one normally
				fprintf( stdout, "I found a match that needs to be begun.\nThere didn't appear to be a lobby running so it was not necessary to kill it.\n" );
				cimatch = new Server::MatchInstance( (popen2_t*)calloc( 1, sizeof( popen2_t ) ), cmatch );
				m_matches.push_back( cimatch );
				// Try to grab the port the lobby (that apparently existed in some long ago era) was already on, or a new one if it's not available
				cmatch->port = getSpecificPort( cmatch->port );
				if( cmatch->port == -1 ) cmatch->port = getPort();
			} else { // Otherwise
				if( cmatch->status == 3 ) {  // We've already started it, exit gracefully
					continue;
				}
				cimatch = *cimatchi;
				if( cimatch->shutdown() != 0 ) { // Kill the lobby
					fprintf( stdout, "I found a match that needed to begin with a lobby already running but I somehow failed to kill the lobby so the game could not be started\n" );
					continue;
				}
				waitpid( cimatch->process->child_pid, NULL, 0 );
				fprintf( stdout, "I found a game that needed to be started so I killed the lobby\n" );
				// There used to be some port grabbing code here but we can presume the lobby's port is now available
			}
			// Grab the map in case it for some reason was never grabbed when hosting
			char * comstr = (char*)calloc( 256, sizeof( char ) );
			sprintf( comstr, "cp \"%s/%d/%s\" \"%s\"", Server::Settings::mappath_load, cmatch->mapid, cmatch->mapName, Server::Settings::mappath_save );
			system( comstr );
			sprintf( comstr, "cp \"%s/%d/%s\" \"%s\"", Server::Settings::mappath_load, cmatch->mapid, cmatch->imgName, Server::Settings::mappath_save );
			system( comstr );
			// Generate the first turn if it hasn't already been done
			if( cmatch->status == 2 ) {
				fprintf( stdout, "I'm initiating the game.\n" );
				sprintf( comstr, "%s --newgame -T %s", Server::Settings::exepath, cmatch->createConfStr() );
				fprintf( stdout, "%s\n", comstr );
				popen2( comstr, cimatch->process );
				waitpid( cimatch->process->child_pid, NULL, 0 );
			}
			// Create the server
			fprintf( stdout, "Done, hosting game now on port %d\n", cmatch->port );
			sprintf( comstr, "%s --tcpserver -T --port %d %s", Server::Settings::exepath, cmatch->port, cmatch->createConfStr() );
			popen2( comstr, cimatch->process );
			// Mark the game as started and write it to the table
			cmatch->status = 3;
			m_table->saveMatch( cmatch );
		}
	}

	void MatchHandler::shutdownServers( void )
	{
		Game::Match ** matches = m_table->getMatchesByStatus( 1, -1 );
		if( matches == NULL ) return;

		// Iterate through matches
		int ii = 0;
		Game::Match * cmatch = NULL;
		while( ( cmatch = matches[ii++] ) != NULL ) {
			fprintf( stdout, "Shutting down match\n" );
			std::vector<Server::MatchInstance*>::iterator cimatchi = getMatchInstance( cmatch );
			Server::MatchInstance * cimatch = NULL;
			if( cimatchi  == m_matches.end() ) {
				// No need to shutd the match down
				fprintf( stdout, "Killing server process\n" );
			} else { // Shut the match down
				(*cimatchi)->shutdown();
				m_matches.erase( cimatchi );
			}
			m_table->deleteMatch( cmatch );
			fprintf( stdout, "Deleted match %s\n", cmatch->name );
		}
	}
}

