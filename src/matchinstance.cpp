#include "matchinstance.hpp"
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
	MatchInstance::MatchInstance( popen2_t * process, Game::Match * match ) :match{match}, process{process}
	{
	}

	int MatchInstance::shutdown( void )
	{
		int retcode = kill( process->child_pid, SIGTERM );
		waitpid( process->child_pid, NULL, 0 );
		return retcode;
	}

}
