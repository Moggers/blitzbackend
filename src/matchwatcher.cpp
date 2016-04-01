#include <string>
#include <sys/stat.h>
#include <regex>
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
	MatchWatcher::MatchWatcher( popen2_t * proc, SQL::Table * table, Game::Match * match, EmailSender * emailSender ): 
		proc{proc}, 
		table{table}, 
		match{match},
		emailSender{emailSender}
	{
		mesg = 0;
		playerbitmap = 0;
		lastn = -1;
		kill = 0;
		pollproc = (popen2_t*)calloc( 1, sizeof( popen2_t) );
		this->currentturn = table->getTurnNumber( match );
		watchThread = std::thread( watchCallback, this );
		// Create turn parser
		std::stringstream stream;
		stream << Server::Settings::jsondir << "/" << match->id << "/";
		this->turnParser.newTurn( stream.str(), this->currentturn );
		// Check for bogus
		batcher.addCheck( R"(.*No 2h for.*)", [](const std::smatch &match){});
		batcher.addCheck( R"(.*packet.*)", [](const std::smatch &match){});
		// Check for death 
		batcher.addCheck( ".*nick fel.*", [this](const std::smatch &match){
			this->mesg = STATUS_FAILURE;
			this->kill = 1;
		;} );
		// Check for match start
		batcher.addCheck( R"(.* ([0-9]+) seconds.*)", [this](const std::smatch &match) {
			int countdown = atoi(match[1].str().c_str());
			if( countdown % 5 == 0 ) {
				this->mesg = 40 + countdown; 
			} if( countdown == 0 ) {
				this->sendAllNotifications(-1);
			}
		});
		// Check for received god
		batcher.addCheck( R"(.*Receiving god for ([0-9]+).*)", [this](const std::smatch &match){
			int nationid = atoi(match[1].str().c_str());
			Game::Nation * nation = this->table->getNation( this->match, nationid );
			if( this->table->checkPlayerPresent( this->match, nation ) == 0 ) {
				this->table->addNationToMatch( this->match, nation );
				fprintf( stdout, "Added nation %s to match %s(%lu)\n", nation->name, this->match->name, this->match->id );
			}
			delete( nation );
		});
		// Check for alert of pretender for nation ID
		batcher.addCheck( R"(.*Load newlord \(.*\) ([0-9]+).*)", [this](const std::smatch &match){
			Game::Nation * n = this->table->getNation( this->match, atoi(match[1].str().c_str()));
			this->lastn = n->id;
			delete( n );
		});
		// Once we have a pretender marked, check if we see what it's called
		batcher.addCheck( R"(.*\/(.*).2h.*)", [this](const std::smatch &match){
			if( this->lastn != -1 ){
				std::cout << match[1].str() << '\n';
				this->table->setTurnfileName( this->lastn, match[1].str().c_str());
				this->lastn = -1;
			}
		});
		// Check for turn number
		batcher.addCheck( R"(^fatherturn ([0-9]+).*)", [this](const std::smatch &match){
			int tn = atoi(match[1].str().c_str());
			if( this->currentturn != tn){
				this->table->addTurn( this->match, this->currentturn+1 );
				this->table->updateTimestamp( this->match );
				fprintf( stdout, "Turn rollover for %s:%d\n", this->match->name, this->currentturn );
				this->turnParser.writeTurn();
				this->turnParser.newTurn( this->currentturn+1 );
			}
			this->currentturn = tn;
		});
		batcher.addCheck(R"(.*tcp_get2hfile: gname:.* pl:([0-9]+).*)", [this](const std::smatch &match){
			fprintf( stdout, "Received turn on match %s for player %d\n", this->match->name, atoi(match[1].str().c_str()) );
			this->table->markTurnSubmitted( this->match, atoi(match[1].str().c_str()) );
		});
		batcher.addCheck( R"(.*Game Over.*)", [this](const std::smatch &match){
			this->mesg = 70;
		});
	}

	void* MatchWatcher::watchCallback( void* arg )
	{
		MatchWatcher * watcher = (MatchWatcher*)arg;
		char * buff = (char*)calloc( 65536, sizeof( char ) );
		char * line;
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
		free( matchdir );

		fcntl(watcher->proc->from_child, F_SETFL, flags | O_NONBLOCK);
		while( !watcher->kill ) { 
			if( ::kill( watcher->proc->child_pid, 0 ) == -1 ) {
				watcher->mesg = STATUS_FAILURE;
				free( buff );
				return NULL;
			}

			nbytes = read( watcher->proc->from_child, buff+strlen(buff), 65535-strlen(buff)); 
			if( nbytes == -1 ) {
				usleep( 500000 );
				continue;
			} else {
				stillreading = 1;
			}

			line = buff; 
			while( stillreading ) {
				std::lock_guard<std::mutex> guard(watcher->lock);
				newline = strchr( line, '\n' );
				if( newline == NULL ) {
					strcpy( buff, line );
					stillreading = 0; 
				} else {
					newline[0] = '\0';
					watcher->batcher.checkString( line );
					watcher->turnParser.parseLine( line );
					line = &newline[1];
				}
			}
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

	void MatchWatcher::sendAllNotifications( int type )
	{
		std::cout << "Sending notifications for game " << this->match->name << '\n';
		std::vector<emailrequest_t> * reqs = this->table->getEmailRequests( this->match->id );
		for( emailrequest_t &req: *reqs ) {
			if( req.hours == 0 ) {
				emailSender->sendNotification( type, req.address, this->match );
			}
		}
		delete( reqs );
	}
}
