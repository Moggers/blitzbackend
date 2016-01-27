#include <string>
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
	MatchWatcher::MatchWatcher( popen2_t * proc ): proc{proc}
	{
		watchThread = (pthread_t*)calloc( 1, sizeof( pthread_t ) );
		pollproc = (popen2_t*)calloc( 1, sizeof( popen2_t) );
		lock = (pthread_mutex_t*)calloc( 1, sizeof( pthread_mutex_t ) );
		pthread_mutex_init( lock, NULL );
		pthread_create( watchThread, NULL, watchCallback, this );
		mesg = 0;
		playerbitmap = 0;
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

			pthread_mutex_lock( watcher->lock ); // Lock the mutex, we're doing Serious Shit now
			nbytes = read( watcher->proc->from_child, buff, 1024 ); // Pull any data from the pipe to the server
			if( nbytes == -1 ) {
			} else {
				size_t pos = 0;
				std::string * recvMessage = new std::string( buff );
				// Search for start string
				pos = recvMessage->find( "second" );
				if( pos != std::string::npos ) {
					buff[pos - 1] = '\0';
					int countdown = atoi(&buff[pos-3]);
					if( countdown % 5 == 0 ){
						watcher->mesg = countdown; 
					}
				}
				// Search for player join string
				pos = recvMessage->find( "Receiving god for " );
				if( pos != std::string::npos ) {
					buff[pos+20] = '\0';
					int pretenderid = atoi(&buff[pos+17]);
					watcher->playerbitmap |= (uint64_t)pow( 2, pretenderid );
					fprintf( stdout, "Found player: %ld\n", watcher->playerbitmap );
					fflush( stdout );
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
