#include "matchinstance.hpp"
#include <string.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdlib.h>

namespace Server
{
	MatchInstance::MatchInstance( void )
	{
		process = (popen2_t*)calloc( 1, sizeof( popen2_t ) );
		match = NULL;
	}
	MatchInstance::MatchInstance( popen2_t * process, Game::Match * match ) :process{process}
	{
		this->match = (Game::Match*)calloc( 1, sizeof( Game::Match ) );
		memcpy( this->match, match, sizeof( Game::Match ) );
	}

	int MatchInstance::shutdown( void )
	{
		this->watcher->destroyWatcher();
		int retcode = kill( process->child_pid, SIGTERM );
		waitpid( process->child_pid, NULL, 0 );
		return retcode;
	}

}
