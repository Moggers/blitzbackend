#include "matchinstance.hpp"
#include <sstream>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include "nation.hpp"
#include <signal.h>
#include <stdlib.h>
#include "table.hpp"
#include "util.hpp"
#include "settings.hpp"

namespace Server
{
	MatchInstance::MatchInstance( void )
	{
		process = (popen2_t*)calloc( 1, sizeof( popen2_t ) );
		match = NULL;
	}
	MatchInstance::MatchInstance( Game::Match * match, SQL::Table * table, int state, EmailSender * emailSender )
	{
		this->m_table = table;
		// Allocate command buffer 
		char * com = (char*)calloc( 512, sizeof( char ) );
		// Create match
		this->match = new Game::Match( *match );
		// Retrieve config string
		char * confstr = match->createConfStr();
		// Retrieve port
		if( this->match->port == 0 )
			this->match->port = get_port();
		else 
			this->match->port = try_get_port( match->port );
		// Copy the map files into dom4's map dir
		sprintf( com, "cp \"%s/%d/%s\" \"%s\"", Server::Settings::mappath_load, this->match->mapid, this->match->mapName, Server::Settings::mappath_save );
		system( com );
		sprintf( com, "cp \"%s/%d/%s\" \"%s\"", Server::Settings::mappath_load, this->match->mapid, this->match->imgName, Server::Settings::mappath_save );
		system( com );
		// Create location to send pretenders
		sprintf( com, "%s/%s%lu", Server::Settings::pretenderdir, match->name, match->id );
		// Create location to send turns
		mkdir( com, 0777 );
		sprintf( com, "%s/%s%lu", Server::Settings::jsondir, match->name, match->id );
		mkdir( com, 0777 );
		// and the mods
		for( auto mod : *(this->match->mods) ) {
			sprintf( com, "rsync -tr \"%s/%d/\" \"%s\"", Server::Settings::modpath_load, mod->m_id, Server::Settings::modpath_save );
			system( com );
		}
		// Create the dom4 exe string
		sprintf( com,  "%s --tcpserver -T --port %d %s", Server::Settings::exepath, this->match->port, confstr );
		// Create and open the process
		this->process = (popen2_t*)calloc( 1, sizeof( popen2_t ) );
		popen2( com, this->process );
		// Create the watcher
		this->watcher = new MatchWatcher( this->process, table, this->match, emailSender );
		this->watcher->port = this->match->port;
		// Free resources
		free( com );
		free( confstr );
	}
	MatchInstance::MatchInstance( popen2_t * process, Game::Match * match, SQL::Table * table, EmailSender * emailSender ) :process{process}
	{
		this->match = (Game::Match*)calloc( 1, sizeof( Game::Match ) );
		memcpy( this->match, match, sizeof( Game::Match ) );
		watcher = new MatchWatcher( process, table, match, emailSender );
		watcher->port = match->port;
	}
	int MatchInstance::checkTimer( Game::Match * match ) {
		if( match->hostint != this->match->hostint ) {
			this->match->hostint = match->hostint;
			this->restart();
			m_table->saveMatch( match );
			m_table->updateTimestamp( match );
			return 1;
		}
		if( match->hostday != this->match->hostday || match->hosthour != this->match->hosthour ) {
			this->match->hostday = match->hostday;
			this->match->hosthour = match->hosthour;
			this->restart();
			m_table->saveMatch( match );
			return 1;
		}
		return 0;
	}

	int MatchInstance::shutdown( void )
	{
		fprintf( stdout, "Destroying watcher\n" );
		this->watcher->destroyWatcher();
		fprintf( stdout, "Done\n" );
		int retcode = kill( process->child_pid, SIGTERM );
		waitpid( process->child_pid, NULL, 0 );
		return retcode;
	}
	void MatchInstance::restart( void )
	{
		fprintf( stdout, "Restarting sever %s\n", match->name );
		EmailSender * storesender = this->watcher->emailSender;
		this->watcher->destroyWatcher();
		int retcode = kill( process->child_pid, SIGTERM );
		waitpid( process->child_pid, NULL, 0 );
		char * com = (char*)calloc( 512, sizeof( char ) );
		match->port = try_get_port( match->port );
		sprintf( com,  "%s --tcpserver -T --port %d %s", Server::Settings::exepath, match->port, match->createConfStr() );
		popen2_t * proc = (popen2_t*)calloc( 1, sizeof( popen2_t ) );
		popen2( com, proc );
		this->process = proc;
		this->watcher = new MatchWatcher( this->process, m_table, this->match, storesender);
		this->watcher->port = this->match->port;
	}

	void MatchInstance::allowTurnChanges()
	{
		std::vector<Game::Nation*> * nat = this->m_table->getNations( this->match );
		for( Game::Nation* cn : *nat ) {
			std::ostringstream path;
			path << Server::Settings::savepath << "/" << this->match->name << this->match->id << "/" << cn->turnname << ".2h";
			chmod( path.str().c_str(), 0755 );
		}
	}

	void MatchInstance::moveInTurns()
	{
		fprintf( stdout, "Copying in pretenders\n" );
		std::vector<Game::Nation*> * nat = m_table->getNations( this->match );
		char * com = (char*)calloc( 1024, sizeof( char ) );
		sprintf( com, "rsync -trv \"%s/%s%lu/\" \"%s/%s%lu/\"", 
			Server::Settings::pretenderdir, this->match->name, this->match->id, Server::Settings::savepath, this->match->name, this->match->id );
		system( com );
	}
}
