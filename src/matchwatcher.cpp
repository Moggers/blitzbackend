#include <string>
#include <signal.h>
#include <fcntl.h>
#include "matchwatcher.hpp"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

namespace Server
{
	MatchWatcher::MatchWatcher( popen2_t * proc ): proc{proc}
	{
		watchThread = (pthread_t*)calloc( 1, sizeof( pthread_t ) );
		lock = (pthread_mutex_t*)calloc( 1, sizeof( pthread_mutex_t ) );
		pthread_mutex_init( lock, NULL );
		pthread_create( watchThread, NULL, watchCallback, this );
		mesg = 0;
	}

	void* MatchWatcher::watchCallback( void* arg )
	{
		MatchWatcher * watcher = (MatchWatcher*)arg;
		char * buff = (char*)calloc( 1024, sizeof( char ) );
		int nbytes;
		int flags = fcntl(watcher->proc->from_child, F_GETFL, 0);
		fcntl(watcher->proc->from_child, F_SETFL, flags | O_NONBLOCK);
		while( 1 ) {
			// Empty namespace, we have a member called kill but we want to call the kill from signal.h
			if( ::kill( watcher->proc->child_pid, 0 ) == -1 ) {
				watcher->mesg = -1;
				return NULL;
			}
			pthread_mutex_lock( watcher->lock );
			nbytes = read( watcher->proc->from_child, buff, 1024 );
			if( nbytes == -1 ) {
				pthread_mutex_unlock( watcher->lock );
				continue;
			}
			std::string * recvMessage = new std::string( buff );
			size_t pos = recvMessage->find( "second" );
			if( pos != std::string::npos ) {
				buff[pos - 1] = '\0';
				int countdown = atoi(&buff[pos-3]);
				if( countdown % 5 == 0 ){
					fprintf( stdout, "Notifying countdown\n" );
					watcher->mesg = countdown;
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
