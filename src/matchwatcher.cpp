#include <string>
#include <regex>
#include "turnparser.hpp"
#include <string.h>
#include "nation.hpp"
#include "settings.hpp"
#include <math.h>
#include <signal.h>
#include <fcntl.h>
#include "matchwatcher.hpp"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

#define STATUS_FAILURE -1

namespace Server
{
	MatchWatcher::MatchWatcher( popen2_t * proc, SQL::Table * table, Game::Match * match ): proc{proc}, table{table}, match{match}
	{
		mesg = 0;
		playerbitmap = 0;
		lastn = -1;
		kill = 0;
		pollproc = (popen2_t*)calloc( 1, sizeof( popen2_t) );
		watchThread = std::thread( watchCallback, this );
	}

	void* MatchWatcher::watchCallback( void* arg )
	{
		MatchWatcher * watcher = (MatchWatcher*)arg;
		char * buff = (char*)calloc( 2048, sizeof( char ) );
		char * line = NULL;
		char * newline = 0;
		int nbytes;
		int flags = fcntl(watcher->proc->from_child, F_GETFL, 0);
		int stillreading = 0;
		int turn;
		std::smatch match;

		char * matchdir = (char*)calloc( 1024, sizeof(char) );
		sprintf( matchdir, "%s/%d/", Server::Settings::jsondir, watcher->match->id );
		std::ostringstream stream;
		stream << Server::Settings::jsondir << watcher->match->id << "/";
		Server::TurnParser turnParser(matchdir, watcher->table->getTurnNumber( watcher->match ));
		free( matchdir );

		fcntl(watcher->proc->from_child, F_SETFL, flags | O_NONBLOCK);
		while( !watcher->kill ) { // If we're not meant to close
			fprintf( stdout, "Locking mutex in watcher\n" );
			std::lock_guard<std::mutex> guard(watcher->lock);
			fprintf( stdout, "Done\n" );
			if( ::kill( watcher->proc->child_pid, 0 ) == -1 ) { // Die if the child died
				watcher->mesg = STATUS_FAILURE;
				free( buff );
				return NULL;
			}
			if( stillreading  == 0 ) { // Are we done reading? (Was the last attempt to grab a line incomplete?)
				nbytes = read( watcher->proc->from_child, buff+strlen(buff), 2040 ); // Grab the new data (insert it into strlen(buff) incase there's already half a line left)
				if( nbytes == -1 ) {
					continue;
				}
				line = buff; // Reset the line
			}
			newline = strchr( line, '\n' ); // Find the end of line
			if( newline == NULL ) { // If we couldn't find a full line copy the half finished line back to the buffer and mark the data as done
				strcpy( buff, line-1 );
				stillreading = 0; 
			} else {
				stillreading = 1;
				newline[0] = '\0'; // Mark the end of the line

				std::string recvMessage( line );
				turnParser.parseLine( recvMessage );
				// Search for failure string
				if( std::regex_match( recvMessage, match, std::regex( ".*nick fel.*" ) ) ) {
					watcher->mesg = STATUS_FAILURE;
					free( buff );
					return NULL;
				}
				// Search for start string
				if( std::regex_match( recvMessage, match, std::regex( ".*([0-9]+) seconds.*" ) ) ) {
					fprintf( stdout, "Updating countdown\n" );
					int countdown = atoi(match[1].str().c_str());
					if( countdown % 5 == 0 )
						watcher->mesg = 40 + countdown; 
					continue;
				}
				// Search for player join string
				if( std::regex_match( recvMessage, match, std::regex( ".*Receiving god for ([0-9]+).*" ) ) ) {
					int nationid = atoi(match[0].str().c_str());
					Game::Nation * nation = watcher->table->getNation( nationid );
					watcher->table->addNationToMatch( watcher->match, nation );
					watcher->lastn = nationid;
					fprintf( stdout, "Added nation %s to match %s(%d)\n", nation->name, watcher->match->name, watcher->match->id );
					free( nation );
					continue;
				}
				// Search for nation 2h name (but only if we know a nation just joined)
				if( watcher->lastn != -1 ) {
					if( std::regex_match( recvMessage, match, std::regex( R"(.*saving as \/(.*\/)(.*).2h.*)"))) {
						const char * name = match[2].str().c_str();
						fprintf( stdout, "%s\n", name );
						watcher->table->setTurnfileName( watcher->lastn, name );
						watcher->lastn = -1;
						continue;
					}

				}
				// Check for a turn number
				if( std::regex_match( recvMessage, match, std::regex(R"(^fatherturn ([0-9]+).*)"))) {
					watcher->currentturn = atoi(match[1].str().c_str());
					continue;
				}
				// Search for new turn
				if( std::regex_match( recvMessage, match, std::regex(R"(^_+ month +([0-9]+) _+.*)"))) {
					watcher->table->addTurn( watcher->match, watcher->currentturn+1 );
					watcher->table->updateTimestamp( watcher->match );
					fprintf( stdout, "Turn rollover for %s:%d\n", watcher->match->name, watcher->currentturn );
					turnParser.writeTurn();
					turnParser.newTurn( watcher->currentturn+1 );
					continue;
				}

				// Find the next line
				line = newline+1;
				
			}
			// Unlock the mutex and prepare for another run
		}
		free( buff );
		return NULL;
	}

	void MatchWatcher::destroyWatcher( void )
	{ 
		{
			fprintf( stdout, "Locking mutex in destroy\n" );
			std::lock_guard<std::mutex> guard(this->lock);
			this->kill = 1;
			fprintf( stdout, "Done\n" );
		}
		fprintf( stdout, "Joining\n" );
		this->watchThread.join();
		fprintf( stdout, "Done\n" );
	}
}
