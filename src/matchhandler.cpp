#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include "mod.hpp"
#include <signal.h>
#include <iterator>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <signal.h>
#include <algorithm>
#include "util.hpp"
#include "settings.hpp"
#include "matchhandler.hpp"

namespace Server
{
	MatchHandler * MatchHandler::WhyDoIHaveToDoThis;

	MatchHandler::MatchHandler( void )
	{
		this->m_table = new SQL::Table();
		WhyDoIHaveToDoThis = this;
	}

	int MatchHandler::blockUntilPortFree( int port )
	{
		while( port_check( port != 1 ) ) {
		}
		return port;
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
			fprintf( stdout, "Shutting down %s\n", (*iter)->match->name );
			(*iter)->shutdown();
		}
		Server::Settings::destroy();
		exit( 1 );
	}

	// I would call this black magic. But black magic typically inspires both fear, and respect.
	// There is no respect here. Only abject terror.
	void MatchHandler::startNewServers( void )
	{
		Game::Match ** matches = m_table->getMatchesByStatus( 6, 0, 1, 10, 11, 12, 13 );
		if( matches == NULL ) return;
		int ii = 0;
		Game::Match * cmatch = NULL;
		while( ( cmatch = matches[ii++] ) != NULL ) {
			int port = 0;
			if( cmatch->status == 0 ) { // New
				fprintf( stdout, "I found a new match\n");
				port = get_port();
			} else if( getMatchInstance( cmatch ) == m_matches.end() ) { // Already started
				fprintf( stdout, "I found a match, It is meant to be running on port %d, but I don't know about it\n", cmatch->port );
			} else { // Running
				Server::MatchInstance * inst = *getMatchInstance(cmatch);
				if( cmatch->needsrestart ) {
					inst->match->update( cmatch );
					inst->restart();
					m_table->updateMatchSettings( inst->match );
					m_table->markRestarted(inst->match);
					continue;
				}
				if( inst->watcher->mesg > 0 ) {
					inst->match->status = (inst->watcher->mesg-40)/ 5 + 10;
					// Just before the game starts, copy the pretender files for the correct players into the match
					if( inst->watcher->mesg == 45 ) {
						inst->moveInTurns();
					}
					if( inst->match->status == 10 ) {
						 inst->match->status = 3;
						 m_table->updateTimestamp( inst->match );
						 // THIS IS MEANT TO UNDO THE READONLY CHANGES THAT ARE PART OF THE FAILED SYSTEM TO PROTECT PRETENDERS FROM REUPLOAD
						 //inst->allowTurnChanges();
					}
					inst->watcher->mesg = 0;
				} else if( inst->watcher->mesg == -1 ) {
					fprintf( stdout, "Match %s has either died or failed to start\n", cmatch->name );
					inst->match->status = 99;
					m_table->saveMatch( inst->match );
					inst->shutdown();
					continue;
				}

				// Delete unwanted nations
				Game::Nation ** nations = m_table->getDeleteRequests( cmatch );
				if( nations[0] != NULL ) {
					for( int ii = 0; nations[ii] != NULL; ii++ ) {
						char * str= (char*)calloc( 512, sizeof( char ) );
						sprintf( str, "%s/%s%lu/%s.2h", Settings::savepath, cmatch->name, cmatch->id, nations[ii]->turnname );
						remove( str );
						m_table->removeNationFromMatch( cmatch, nations[ii] );
					}
					fprintf( stdout, "Shutting down for player remove\n" );
					inst->restart();
				}
				free( nations );
				// Update host timers
				inst->checkTimer( cmatch );

				m_table->saveMatch( inst->match );
				continue;
			}
			// Spawn
			Server::MatchInstance * inst = new MatchInstance( cmatch, m_table, 0 );
			m_matches.push_back( inst );
			fprintf( stdout, "Started server on port %d.\n", inst->match->port );
			inst->match->status = 1;
			m_table->saveMatch( inst->match );
		}
		ii = 0;
		Game::Match * ptr;
		while(( ptr = matches[ii++])  != NULL )
			delete( ptr );
		free( matches );
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
				// Try to grab the port the lobby (that apparently existed in some long ago era) was already on, or a new one if it's not available
			} else { // Otherwise
				if( cmatch->status == 3 ) {  // We've already started it
					cimatch = *cimatchi;
					cimatch->checkTimer( cmatch );
					m_table->saveMatch( cimatch->match );
					continue;
				} else {
					cimatch = *cimatchi;
					if( cimatch->shutdown() != 0 ) { // Kill the lobby
						fprintf( stdout, "I found a match that needed to begin with a lobby already running but I somehow failed to kill the lobby so the game could not be started\n" );
						continue;
					}
				}
				fprintf( stdout, "I found a game that needed to be started so I killed the lobby\n" );
			}
			Server::MatchInstance * inst = new MatchInstance( cmatch, m_table, 0 );
			m_matches.push_back( inst );
			fprintf( stdout, "Started server on port %d.\n", inst->match->port );
			inst->match->status = 3;
			m_table->saveMatch( inst->match );
			m_table->updateTimestamp( inst->match );
		}
		free( matches );
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
				// No need to shut the match down
			} else { // Shut the match down
				fprintf( stdout, "Killing server process\n" );
				(*cimatchi)->shutdown();
				m_matches.erase( cimatchi );
			}
			m_table->deleteMatch( cmatch );
			fprintf( stdout, "Deleted match %s\n", cmatch->name );
		}
		free( matches );
	}
}

