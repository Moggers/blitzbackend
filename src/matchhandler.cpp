#include <unistd.h>
#include <sys/wait.h>
#include <iostream>
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
		this->emailSender = new EmailSender();
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

	void MatchHandler::startNewServers( void )
	{
		std::vector<Game::Match*> * matches = m_table->getMatchesByStatus( 6, 0, 1, 10, 11, 12, 13 );
		for( Game::Match* cmatch : *matches ){
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
			Server::MatchInstance * inst = new MatchInstance( cmatch, m_table, 0, emailSender );
			m_matches.push_back( inst );
			fprintf( stdout, "Started server on port %d.\n", inst->match->port );
			inst->match->status = 1;
			m_table->saveMatch( inst->match );
		}
		for( Game::Match*  match : *matches ) {
			delete( match );
		}
		delete (matches );
	}

	void MatchHandler::beginGames( void )
	{
		std::vector<Game::Match*> * matches = m_table->getMatchesByStatus( 2, 2, 3);
		for( Game::Match* cmatch : *matches ){
			std::vector<Server::MatchInstance*>::iterator cimatchi = getMatchInstance( cmatch );
			Server::MatchInstance * cimatch = NULL;
			if( cimatchi == m_matches.end() ) { // If we reached the end of th matches then create a new one normally
				fprintf( stdout, "I found a match that needs to be begun.\nThere didn't appear to be a lobby running so it was not necessary to kill it.\n" );
				// Try to grab the port the lobby (that apparently existed in some long ago era) was already on, or a new one if it's not available
			} else { // Otherwise
				if( cmatch->status == 3 ) {  // We've already started it
					cimatch = *cimatchi;
					// Changed settings
					if( cmatch->needsrestart ) {
						cimatch->match->update( cmatch );
						cimatch->restart();
						m_table->updateMatchSettings( cimatch->match );
						m_table->markRestarted(cimatch->match);
						continue;
					}
					// Stale notifications
					std::vector<Server::emailrequest_t> * vec = m_table->getStaleNotifications( cmatch->id );
					int tn = m_table->getTurnNumber( cmatch );
					for( emailrequest_t req: *vec ) {
						if( tn > req.turn ) {
							if( m_table->hasSubmittedTurn( cmatch, req.matchnation, tn ) ) {
								fprintf( stdout, "Submitted turn so marked as don't notify\n" );
								m_table->setSNTN( req.id, tn );
							} else {
								if( req.hours > 0 ) {
									if( req.istime == 1 ) {
										std::cout << "Sending notification of game " << cmatch->name << " to " << req.address << " because it is rolling within " << req.hours << " hours\n";
										emailSender->sendNotification( req.hours, req.address, cmatch );
										m_table->setSNTN( req.id, tn );
									}
								} else {
									emailSender->sendNotification( req.hours, req.address, cmatch );
									m_table->setSNTN( req.id, tn );
								}
							}
						}
					}
					delete( vec );
					// Game end status
					if( cimatch->watcher->mesg == 70 ) {
						fprintf( stdout, "Match %s ended\n", cimatch->match->name );
						cimatch->match->status = 70;
						m_table->saveMatch(cimatch->match);
						cimatch->shutdown();
						m_matches.erase( getMatchInstance( cmatch ) );
						continue;
					}
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
			Server::MatchInstance * inst = new MatchInstance( cmatch, m_table, 0, emailSender );
			m_matches.push_back( inst );
			fprintf( stdout, "Started server on port %d.\n", inst->match->port );
			inst->match->status = 3;
			m_table->saveMatch( inst->match );
			m_table->updateTimestamp( inst->match );
		}
		for( Game::Match*  match : *matches ) {
			delete( match );
		}
		delete (matches );
	}

	void MatchHandler::restartServers( void )
	{
		std::vector<Game::Match*> * matches = m_table->getMatchesByStatus( 1, -2 );
		for( Game::Match * cmatch : *matches ) {
			m_table->deleteTurns( cmatch );
			cmatch->deleteFiles();
			if( getMatchInstance( cmatch ) == m_matches.end() ) {
				Server::MatchInstance * inst = new MatchInstance( cmatch, m_table, 0, emailSender );
				m_matches.push_back( inst );
				fprintf( stdout, "Started server on port %d.\n", inst->match->port );
			}
			Server::MatchInstance * inst = *getMatchInstance(cmatch);
			inst->moveInTurns();
			inst->restart();
			inst->match->status = 1;

			m_table->saveMatch( inst->match );
		}
		for( Game::Match*  match : *matches ) {
			delete( match );
		}
		delete (matches );
	}

	void MatchHandler::shutdownServers( void )
	{
		std::vector<Game::Match*> * matches = m_table->getMatchesByStatus( 2, -1, 71);
		for( Game::Match* cmatch : *matches ){
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
			if( cmatch->status == -1 ) {
				m_table->deleteMatch( cmatch );
				fprintf( stdout, "Deleted match %s\n", cmatch->name );
			} else if( cmatch->status == 71 ) {
				cmatch->status = 70;
				m_table->saveMatch( cmatch );
			}
		}
		for( Game::Match*  match : *matches ) {
			delete( match );
		}
		delete (matches );
	}
}

