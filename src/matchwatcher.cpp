#include <string>
#include <sys/stat.h>
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
		regex_set.push_back( std::regex(".*nick fel.*" ) );
		regex_set.push_back( std::regex( ".* ([0-9]+) seconds.*" ) );
		regex_set.push_back( std::regex( ".*Receiving god for ([0-9]+).*" ) );
		regex_set.push_back( std::regex(R"(.*Load newlord \(.*\) ([0-9]+).*)" ) );
		regex_set.push_back( std::regex( R"(.*\/(.*).2h.*)" ) );
		regex_set.push_back( std::regex(R"(^fatherturn ([0-9]+).*)" ) );
		regex_set.push_back( std::regex(R"(^_+ month +([0-9]+) _+.*)" ) );
		regex_set.push_back( std::regex(R"(.*packet.*)" ) );
		regex_set.push_back( std::regex(R"(.*No 2h for.*)" ) );
	}

	void* MatchWatcher::watchCallback( void* arg )
	{
		MatchWatcher * watcher = (MatchWatcher*)arg;
		char * buff = (char*)calloc( 4096, sizeof( char ) );
		char * line = NULL;
		char * newline = 0;
		int nbytes;
		int flags = fcntl(watcher->proc->from_child, F_GETFL, 0);
		int stillreading = 0;
		int turn;
		std::smatch match;

		char * matchdir = (char*)calloc( 1024, sizeof(char) );
		sprintf( matchdir, "%s/%lu/", Server::Settings::jsondir, watcher->match->id );
		std::ostringstream stream;
		stream << Server::Settings::jsondir << watcher->match->id << "/";
		Server::TurnParser turnParser(matchdir, watcher->table->getTurnNumber( watcher->match ));
		free( matchdir );

		fcntl(watcher->proc->from_child, F_SETFL, flags | O_NONBLOCK);
		while( !watcher->kill ) { // If we're not meant to close
			if( ::kill( watcher->proc->child_pid, 0 ) == -1 ) { // Die if the child died
				watcher->mesg = STATUS_FAILURE;
				free( buff );
				return NULL;
			}
			if( stillreading  == 0 ) { // Are we done reading? (Was the last attempt to grab a line incomplete?)
				nbytes = read( watcher->proc->from_child, buff+strlen(buff), 4090 ); // Grab the new data (insert it into strlen(buff) incase there's already half a line left)
				if( nbytes == -1 ) {
					usleep( 500000 );
					continue;
				}
				line = buff; // Reset the line
			}
			std::lock_guard<std::mutex> guard(watcher->lock);
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
				if( std::regex_match( recvMessage, match, watcher->regex_set[0] ) ) {
					watcher->mesg = STATUS_FAILURE;
					free( buff );
					return NULL;
				}
				// Search for bullshit
				if( std::regex_match( recvMessage, match, watcher->regex_set[7] ) ) {
					goto end;
				}
				if( std::regex_match( recvMessage, match, watcher->regex_set[8] ) ) {
					goto end;
				}
				// Search for start string
				if( std::regex_match( recvMessage, match, watcher->regex_set[1] ) ) {
					int countdown = atoi(match[1].str().c_str());
					if( countdown % 5 == 0 ) {
						watcher->mesg = 40 + countdown; 
					}
					goto end;
				}
				// Search for player join string
				if( std::regex_match( recvMessage, match, watcher->regex_set[2] ) ) {
					int nationid = atoi(match[1].str().c_str());
					Game::Nation * nation = watcher->table->getNation( nationid );
					if( watcher->table->checkPlayerPresent( watcher->match, nation ) == 0 ) {
						watcher->table->addNationToMatch( watcher->match, nation );
						fprintf( stdout, "Added nation %s to match %s(%lu)\n", nation->name, watcher->match->name, watcher->match->id );
						// THE FOLLOWING IS MEANT TO MARK A PRETENDER AS READONLY TO PREVENT OVERWRITING OTHERS' WITH YOUR OWN
						// IT SIMPLY CAUSES DOM4 TO CRASH DUE TO BEING UNABLE TO WRITE TO THE FILE
						/*std::stringstream path;
						path << Server::Settings::savepath << "/" << watcher->match->name << watcher->match->id << "/" << watcher->table->getNation(nationid)->turnname << ".2h";
						fprintf( stdout, "Found the pretender name, marking him as readonly now at %s\n", path.str().c_str() );
						if( chmod( path.str().c_str(), 0000 ) != 0 ) {
							fprintf( stdout, "Failed to mark .2h as read only %d\n", errno );
						} else {
							fprintf( stdout, "Done\n" );
						} */
					}
					free( nation );
					goto end;
				}
				if( std::regex_match( recvMessage, match, watcher->regex_set[3] ) ) {
				//if( std::regex_match( recvMessage, match, std::regex(R"(.*Load newlord.*)" ) ) ) {
					fprintf( stdout, "Found mention of 2h: %s\n", recvMessage.c_str() );
					watcher->lastn = atoi(match[1].str().c_str());
				}
				// Search for nation 2h name (but only if we know a nation just joined)
				if( watcher->lastn != -1 ) {
					if( std::regex_match( recvMessage, match, watcher->regex_set[4] ) ) {
						const char * name = match[1].str().c_str();
						fprintf( stdout, "Found name: %s\n", name );
						watcher->table->setTurnfileName( watcher->lastn, name );
						watcher->lastn = -1;
						goto end;
					}

				}
				// Check for a turn number
				if( std::regex_match( recvMessage, match, watcher->regex_set[5] ) ) {
					watcher->currentturn = atoi(match[1].str().c_str());
					goto end;
				}
				// Search for new turn
				if( std::regex_match( recvMessage, match, watcher->regex_set[6] ) ) {
					watcher->table->addTurn( watcher->match, watcher->currentturn+1 );
					watcher->table->updateTimestamp( watcher->match );
					fprintf( stdout, "Turn rollover for %s:%d\n", watcher->match->name, watcher->currentturn );
					turnParser.writeTurn();
					turnParser.newTurn( watcher->currentturn+1 );
					goto end;
				}

				// Find the next line
				end:
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
			std::lock_guard<std::mutex> guard(this->lock);
			this->kill = 1;
		}
		fprintf( stdout, "Joining\n" );
		this->watchThread.join();
		fprintf( stdout, "Done\n" );
	}
}
