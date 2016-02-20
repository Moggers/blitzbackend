#include <string>
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
		kill = 0;
		watchThread = (pthread_t*)calloc( 1, sizeof( pthread_t ) );
		pollproc = (popen2_t*)calloc( 1, sizeof( popen2_t) );
		lock = (pthread_mutex_t*)calloc( 1, sizeof( pthread_mutex_t ) );
		pthread_mutex_init( lock, NULL );
		pthread_create( watchThread, NULL, watchCallback, this );

	}

	void* MatchWatcher::watchCallback( void* arg )
	{
		MatchWatcher * watcher = (MatchWatcher*)arg;
		char * buff = (char*)calloc( 1024, sizeof( char ) );
		int nbytes;
		int flags = fcntl(watcher->proc->from_child, F_GETFL, 0);
		fcntl(watcher->proc->from_child, F_SETFL, flags | O_NONBLOCK);
		while( !watcher->kill ) {
			// Check if the child is even running, if it's not, send back a failure message and exit
			if( ::kill( watcher->proc->child_pid, 0 ) == -1 ) {
				watcher->mesg = STATUS_FAILURE;
				return NULL;
			}

			nbytes = read( watcher->proc->from_child, buff, 1024 ); // Pull any data from the pipe to the server
			pthread_mutex_lock( watcher->lock ); // Lock the mutex, we're doing Serious Shit now
			if( nbytes == -1 ) {
			} else {
				size_t pos = 0;
				// Search for failure string
				pos = std::string::npos;
				std::string recvMessage = std::string( buff );
				pos = recvMessage.find( "nick fel" );
				if( pos != std::string::npos ) {
					watcher->mesg = STATUS_FAILURE;
					return NULL;
				}
				// Search for start string
				pos = std::string::npos;
				pos = recvMessage.find( "second" );
				if( pos != std::string::npos ) {
					buff[pos - 1] = '\0';
					int countdown = atoi(&buff[pos-3]);
					if( countdown % 5 == 0 )
						watcher->mesg = 40 + countdown; 
				}
				// Search for player join string
				pos = std::string::npos;
				pos = recvMessage.find( "Receiving god for " );
				if( pos != std::string::npos ) {
					buff[pos+20] = '\0';
					int nationid = atoi(&buff[pos+17]);
					Game::Nation * nation = watcher->table->getNation( nationid );
					watcher->table->addNationToMatch( watcher->match, nation );
					fprintf( stdout, "Added nation %s to match %s(%d)\n", nation->name, watcher->match->name, watcher->match->id );
				}
				// Search for new turn
				pos = std::string::npos;
				pos = recvMessage.find( "putfatherland" );
				if( pos != std::string::npos ) {
					watcher->table->addTurn( watcher->match );
					watcher->table->updateTimestamp( watcher->match );
					fprintf( stdout, "Turn rollover for %s\n", watcher->match->name );
				}
			}

			pthread_mutex_unlock( watcher->lock );
		}
		return NULL;
	}

	void MatchWatcher::destroyWatcher( void )
	{ 
		pthread_mutex_lock( lock );
		this->kill = 1;
		pthread_mutex_unlock( lock );
	}
}
