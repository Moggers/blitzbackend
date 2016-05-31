#include "table.hpp"
#include "util.hpp"
#include <iostream>
#include "settings.hpp"
#include <sys/stat.h>
#include <string.h>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>

namespace SQL
{
	Table::Table( void )
	{
		m_con = mysql_init( NULL );
		if( m_con == NULL ) {
			fprintf( stdout, "%s\n", mysql_error( m_con ) );
		}

		if( mysql_real_connect( m_con, "localhost", Server::Settings::dbuser, Server::Settings::dbpass, NULL, 0, NULL, 0) == NULL ) {
			fprintf( stdout, "%s\n", mysql_error( m_con ) );
			mysql_close( m_con );
		}

		char * query = (char*)calloc( 128, sizeof( char ) );
		sprintf( query, "use " );
		sprintf( query + strlen( query ), "%s",  Server::Settings::dbname );
		if( mysql_query( m_con, query) != 0 ) {
			fprintf( stdout, "%s\n", mysql_error( m_con ) );
			mysql_close( m_con );
		}
		free( query );

		fprintf( stdout, "Connected to database %s\n", Server::Settings::dbname );
	}
	Table::~Table( void )
	{
		std::lock_guard<std::recursive_mutex> scopelock(tablelock);
		mysql_close( m_con );
		mysql_library_end();
	}

	void Table::addTurn( Game::Match * match, int tn )
	{
		std::lock_guard<std::recursive_mutex> scopelock(tablelock);
		char * query = (char*)calloc( 128, sizeof( char ) );
		sprintf( query, "insert into turns (match_id,tn,time) values(%lu,%d,UTC_TIMESTAMP);", match->id, tn );
		if( mysql_query( m_con, query ) != 0 ) {
			fprintf( stdout, "Failed to insert turn\n" );
		}
		free( query );
	}

	void Table::deleteTurns( Game::Match * match )
	{
		char * query = (char*)calloc( 128, sizeof( char ) );
		sprintf( query, "delete from turns where match_id=%lu;", match->id );
		if( mysql_query( m_con, query ) != 0 ) {
			fprintf( stdout, "Failed to remove turns from db\n" );
		}
		free( query );
	}

	void Table::updateTimestamp( Game::Match * match )
	{
		std::lock_guard<std::recursive_mutex> scopelock(tablelock);
		char * query = (char*)calloc( 128, sizeof( char ) );
		sprintf( query, "update matches set lastturn=UTC_TIMESTAMP where id=%lu;", match->id );
		if( mysql_query( m_con, query ) != 0 ) {
			fprintf( stdout, "Failed to update match timestamp\n" );
		}
		free( query );
	}

	int Table::getTurnNumber( Game::Match * match )
	{
		std::lock_guard<std::recursive_mutex> scopelock(tablelock);
		char * query = (char*)calloc( 512, sizeof( char ) );
		sprintf( query, "select max(tn) from turns where match_id=%lu", match->id );
		if( mysql_query( m_con, query ) != 0 )
			fprintf( stdout, "Failed to retrieve turn count\n" );
		MYSQL_RES * res = mysql_store_result( m_con );
		if( res == NULL ) {
			free( query );
			return 0;
		}
		MYSQL_ROW crow = mysql_fetch_row( res );
		if( crow[0] != NULL ) {
			int ret = atoi(crow[0]);
			mysql_free_result( res );
			free( query );
			return ret;
		}
		mysql_free_result( res );
		free( query );
		return 0;
	}

	Game::Match ** Table::getAllMatches( void )
	{
		std::lock_guard<std::recursive_mutex> scopelock(tablelock);
		if( mysql_query( m_con, "select * from matches" ) != 0 ) {
			return NULL;
		}
		MYSQL_RES * res = mysql_store_result( m_con );
		if( res == NULL ) {
			return NULL;
		}

		Game::Match ** matches = (Game::Match**)calloc( 256, sizeof( Game::Match * ) );

		int ii = 0;
		while( MYSQL_ROW row = mysql_fetch_row( res ) ) {
			char * query = (char*)calloc( 128, sizeof( char ) );
			sprintf( query, "select mappath,imagepath from maps where id=%s", row[1] );
			if( mysql_query( m_con, query ) ) {
				free( query );
				int i=0;
				Game::Match * cm;
				while( (cm = matches[i++])!=NULL)
					delete( cm );
				free( matches );
				return NULL;
			}
			MYSQL_RES * mappath = mysql_store_result( m_con );
			if ( mappath == NULL ) {
				free( query );
				int i=0;
				Game::Match * cm;
				while( (cm = matches[i++])!=NULL)
					delete( cm );
				free( matches );
				return NULL;
			}

			MYSQL_ROW mappathrow = mysql_fetch_row( mappath );
			std::vector<Game::Mod*> * mods = getModsByMatch( atoi(row[0]) );
			matches[ii] = new Game::Match( row, mappathrow, mods );
			ii++;
			mysql_free_result( mappath );
			free( query );
		}
		return matches;
	};

	std::vector<Game::Match*> * Table::getMatchesByStatus( int count, ...  )
	{
		std::lock_guard<std::recursive_mutex> scopelock(tablelock);
		char * query = (char*)calloc( 256, sizeof( char ) );
		sprintf( query, "select * from matches where status in (" );

		// God I love va_arg
		va_list status;
		va_start( status, count );
		for( int ii = 0; ii < count; ii++ )
			sprintf( query + strlen( query ), "%d,", va_arg( status, int ) );
		va_end( status );
		//
		sprintf( query + strlen( query ) - 1, ");" );

		if( mysql_query( m_con, query ) != 0 ) {
			free( query );
			return NULL;
		}
		MYSQL_RES * res = mysql_store_result( m_con );
		if( res == NULL ) {
			free( query );
			return NULL;
		}
		free( query );
		
		std::vector<Game::Match*> * matches = new std::vector<Game::Match*>();

		int ii = 0;
		while( MYSQL_ROW row = mysql_fetch_row( res ) ) {
			char * query = (char*)calloc( 128, sizeof( char ) );
			sprintf( query, "select mappath,imagepath from maps where id=%s", row[1] );
			if( mysql_query( m_con, query ) ) {
				free( query );
				return NULL;
			}
			MYSQL_RES * mappath = mysql_store_result( m_con );
			if ( mappath == NULL ) {
				free( query );
				return NULL;
			}

			MYSQL_ROW mappathrow = mysql_fetch_row( mappath );
			std::vector<Game::Mod*> * mods = getModsByMatch( atoi(row[0]) );
			Game::Match * match = new Game::Match( row, mappathrow, mods );
			delete( mods );
			matches->push_back(  match);
			match->nations = getNations( match);
			ii++;
			free( query );
			mysql_free_result( mappath );
		}
		mysql_free_result( res );
		return matches;
	};

	std::vector<Game::Mod*> * Table::getModsByMatch( int matchid )
	{
		std::lock_guard<std::recursive_mutex> scopelock(tablelock);
		char * query = (char*)calloc( 128, sizeof( char ) );
		sprintf( query, "select mod_id from matchmods where match_id=%d;", matchid );
		if( mysql_query( m_con, query ) ) {
			free( query );
			return NULL;
		}
		MYSQL_RES * modids = mysql_store_result( m_con );
		if( modids == NULL ) {
			free( query );
			return NULL;
		}

		std::vector<Game::Mod*> * returnvec = new std::vector<Game::Mod*>();
		while( MYSQL_ROW modidrow = mysql_fetch_row(modids) ) {
			sprintf( query, "select dmname from mods where id=%s", modidrow[0] );
			if( mysql_query( m_con, query ) ) {
				free( query );
				delete( returnvec );
				return NULL;
			}
			MYSQL_RES * modname = mysql_store_result( m_con );
			MYSQL_ROW modnamerow = mysql_fetch_row( modname );
			Game::Mod * mod = new Game::Mod( atoi(modidrow[0]), modnamerow[0] );
			returnvec->push_back( mod );
			mysql_free_result( modname );
		}
		free( query );
		mysql_free_result( modids );
		return returnvec;
	}

	void Table::saveMatch( Game::Match * match )
	{
		std::lock_guard<std::recursive_mutex> scopelock(tablelock);
		char * query = (char*)calloc( 2048, sizeof( char ) );
		sprintf( query, "update matches set status=%d,port=%d where id=%lu;", match->status, match->port, match->id );
		int sqlerrno;
		if( (sqlerrno = mysql_query( m_con, query )) != 0 ) {
			fprintf( stdout, "Warning! Failed to save match: %d\nSQL was %s\n", sqlerrno, query );
		}
		free( query );
	}

	void Table::updateMatchSettings( Game::Match * match )
	{
		std::lock_guard<std::recursive_mutex> scopelock(tablelock);
		char * query = (char*)calloc( 2048, sizeof( char ) );
		sprintf( query, 
			"update matches set tone=%d,ttwo=%d,tthree=%d,\
			points=%d,map_id=%d,research_diff=%d,renaming=%d,\
			clientstart=%d where id=%lu;", 
			match->t[0], match->t[1], match->t[2], match->t[3], match->mapid, match->research, match->renaming, match->clientstart, match->id );
		int sqlerrno;
		if( (sqlerrno = mysql_query( m_con, query )) != 0 ) {
			fprintf( stdout, "Warning! Failed to save match: %d\nSQL was %s\n", sqlerrno, query );
		}
		free( query );
	}

	void Table::markRestarted( Game::Match * match )
	{
		std::lock_guard<std::recursive_mutex> scopelock(tablelock);
		char * query = (char*)calloc( 2048, sizeof( char ) );
		sprintf( query,  "update matches set needsrestart=0 where id=%lu", match->id );
		int sqlerrno;
		if( (sqlerrno = mysql_query( m_con, query )) != 0 ) {
			fprintf( stdout, "Failed to notify db of match restart %d\nSQL was %s\n", sqlerrno, query );
		}
		free( query );
	}


	void Table::deleteMatch( Game::Match * match )
	{
		std::lock_guard<std::recursive_mutex> scopelock(tablelock);
		char * query = (char*)calloc( 2048, sizeof( char ) );
		int sqlerrno;
		sprintf( query, "delete from matches where id=%lu", match->id );
		if( (sqlerrno = mysql_query( m_con, query )) != 0 )
			fprintf( stdout, "Warning! Failed to delete match %lu at sql %d\n", match->id, sqlerrno );
		free( query );
	}

	void Table::removeNationFromMatch( Game::Match * match, Game::Nation * nation )
	{
		std::lock_guard<std::recursive_mutex> scopelock(tablelock);
		char * query = (char*)calloc( 2048, sizeof( char ) );
		sprintf( query, "delete from matchnations where match_id=%lu AND nation_id=%d", match->id, nation->id );
		int sqlerrno;
		if( ( sqlerrno = mysql_query(m_con, query )) != 0 )
			fprintf( stdout, "Failed to delete nation from match in database (this is tried whenever a nation is added and is probably okay%d\n", sqlerrno );
		sprintf( query, "rm \"%s/%s%lu/%s.2h\"", Server::Settings::pretenderdir, match->name, match->id, nation->turnname );
		system( query );
		free( query );
	}

	int Table::checkPlayerPresent( Game::Match * match, Game::Nation * nation )
	{
		std::lock_guard<std::recursive_mutex> scopelock(tablelock);
		char * query = (char*)calloc( 2048, sizeof( char ) );
		sprintf( query, "select id from matchnations where match_id=%lu AND nation_id=%d", match->id, nation->id );
		int sqlerrno;
		if( ( sqlerrno = mysql_query( m_con, query ) ) != 0 )
			fprintf( stdout, "Failed to poll database for nation's existance in game %s\n", match->name );
		MYSQL_RES * res = mysql_store_result( m_con );
		MYSQL_ROW row;
		if( ( row = mysql_fetch_row( res ) ) != 0 ) {
			mysql_free_result( res );
			free( query );
			return 1;
		}
		mysql_free_result( res );
		free( query );
		return 0;
	}

	void Table::addNationToMatch( Game::Match * match, Game::Nation * nation )
	{
		std::lock_guard<std::recursive_mutex> scopelock(tablelock);
		removeNationFromMatch( match, nation );
		char * query = (char*)calloc( 2048, sizeof( char ) );
		sprintf( query, "insert into matchnations values (0, %d, %lu, 0, 0)", 
			nation->id, match->id );
		std::stringstream stream;
		stream << Server::Settings::pretenderdir << "/" << match->name << match->id << "/" << nation->turnname << ".2h";
		char * com = (char*)calloc( 2048, sizeof( char ) );
		sprintf( com, "%s/%s%lu/", Server::Settings::pretenderdir, match->name, match->id );
		mkdir( com, 0755 );
		sprintf( com, "rsync -trv \"%s/%s%lu/%s.2h\" \"%s\"", Server::Settings::savepath, match->name, match->id, nation->turnname, stream.str().c_str() );
		system( com );
		int sqlerrno;
		if( ( sqlerrno = mysql_query(m_con, query )) != 0 )
			fprintf( stdout, "Failed to add nation to match in database (%lu,%d) %d\n", match->id, nation->id, sqlerrno );
		free( com );
		free( query );
	}

	Game::Nation * Table::getNation( Game::Match * match, int dom_id )
	{
		std::lock_guard<std::recursive_mutex> scopelock(tablelock);
		char * query = (char*)calloc(2048, sizeof( char ) );
		sprintf( query, "select * from nations where dom_id=%d AND mod_id in (select mod_id from matchmods where match_id=%lu)", dom_id, match->id );
		int sqlerrno;
		if( (sqlerrno = mysql_query( m_con, query )) != 0 )
			fprintf( stdout, "Failed to retrieve nation: %d\n", sqlerrno );
		MYSQL_RES * nationstuff = mysql_store_result( m_con );
		if( nationstuff == NULL ) {
			free( query );
			return NULL;
		}
		MYSQL_ROW nationrow = mysql_fetch_row( nationstuff );
		Game::Nation * newNation = new Game::Nation( atoi(nationrow[0]), nationrow[1], nationrow[2], nationrow[3] );
		free( query );
		mysql_free_result( nationstuff );
		return newNation;
	}

	Game::Nation * Table::getNationById( int nation_id )
	{
		std::lock_guard<std::recursive_mutex> scopelock(tablelock);
		char * query = (char*)calloc(2048, sizeof( char ) );
		sprintf( query, "select * from nations where id=%d", nation_id);
		int sqlerrno;
		if( (sqlerrno = mysql_query( m_con, query )) != 0 )
			fprintf( stdout, "Failed to retrieve nation: %d\n", sqlerrno );
		MYSQL_RES * nationstuff = mysql_store_result( m_con );
		if( nationstuff == NULL ) {
			free( query );
			return NULL;
		}
		MYSQL_ROW nationrow = mysql_fetch_row( nationstuff );
		Game::Nation * newNation = new Game::Nation( atoi(nationrow[0]), nationrow[1], nationrow[2], nationrow[3] );
		free( query );
		mysql_free_result( nationstuff );
		return newNation;
	}

	std::vector<Game::Nation*> * Table::getNations( Game::Match * match )
	{
		std::lock_guard<std::recursive_mutex> scopelock(tablelock);
		char * query = (char*)calloc(128, sizeof( char ) );
		sprintf( query, "select dom_id from nations where id in (select nation_id from matchnations where match_id=%lu)", match->id );
		int sqlerrno;
		if( (sqlerrno = mysql_query( m_con, query )) != 0 ) {
			fprintf( stdout, "Failed to retrieve list of nations for match %lu: %d\n", match->id, sqlerrno );
		}
		MYSQL_RES * nations = mysql_store_result( m_con );
		std::vector<Game::Nation*> * retnat = new std::vector<Game::Nation*>();
		if( nations != NULL ) {
			MYSQL_ROW row;
			while( ( row = mysql_fetch_row( nations ) ) != NULL ) {
				retnat->push_back( this->getNation( match, atoi(row[0]) ) );
			}
		}
		free( query );
		mysql_free_result( nations );
		return retnat;
	}

	Game::Nation ** Table::getDeleteRequests( Game::Match * match )
	{
		std::lock_guard<std::recursive_mutex> scopelock(tablelock);
		char * query = (char*)calloc( 2048, sizeof( char ) );
		sprintf( query, "select (select dom_id from nations where id=mn.nation_id),computer from matchnations as mn where match_id=%lu AND markdelete=1", match->id );
		int sqlerrno;
		if( (sqlerrno = mysql_query( m_con, query )) != 0 ) {
			fprintf( stdout, "Failed to retrieve nation delete requests: %d\n", sqlerrno );
			free( query );
			return NULL;
		}
		MYSQL_RES * nationstuff = mysql_store_result( m_con );
		if( nationstuff == NULL ) {
			free( query );
			return NULL;
		}
		MYSQL_ROW nationrow;
		int i = 0;
		Game::Nation ** nations = (Game::Nation**)calloc( 64, sizeof( Game::Nation* ) );
		while( ( nationrow = mysql_fetch_row( nationstuff ) ) != NULL ) {
			nations[i] = getNation( match, atoi(nationrow[0]) );
			if( nations[i] == NULL ) {
				free( query );
				int i=0;
				Game::Nation * cn;
				while( (cn = nations[i++])!=NULL)
					delete( cn );
				free( nations );
				return NULL;
			}
			nations[i]->computer = atoi(nationrow[1]);
			i++;
		}
		mysql_free_result( nationstuff );
		free( query );
		return nations;
	}

	void Table::setTurnfileName( int nationid, const char * name )
	{
		std::lock_guard<std::recursive_mutex> scopelock(tablelock);
		char * query = (char*)calloc( 2048, sizeof( char ) );
		sprintf( query, "update nations set turn_name='%s' where id=%d;", name, nationid );
		int sqlerrno;
		if( (sqlerrno = mysql_query( m_con, query )) != 0 )
			fprintf( stdout, "Failed to update nation turn name %d\n", sqlerrno );
		else
			fprintf( stdout, "Updated nation (%d) turn name is now %s\n", nationid, name );
		free( query );
	}

	std::vector<Server::emailrequest_t> * Table::getEmailRequests( int match_id )
	{
		std::lock_guard<std::recursive_mutex> scopelock(tablelock);
		char * query = (char*)calloc( 2048, sizeof( char ) );
		sprintf( query, "select email from emailrequests where match_id=%d AND hours=0", match_id );
		int sqlerrno;
		if( (sqlerrno = mysql_query( m_con, query )) != 0 ) {
			fprintf( stdout, "Failed to retrieve notification requests %d\n", sqlerrno );
			free( query );
			return NULL;
		} else {
			std::vector<Server::emailrequest_t> * vec = new std::vector<Server::emailrequest_t>();
			MYSQL_RES * emailreqs = mysql_store_result( m_con );
			if( emailreqs == NULL ) {
			} else {
				MYSQL_ROW emailrow;
				while( (emailrow = mysql_fetch_row( emailreqs ) ) != NULL ) {
					vec->push_back(Server::emailrequest_t{0, match_id, strdup(emailrow[0]), 0,0,0,0});
				}
				mysql_free_result( emailreqs );
			}
			free( query );
			return vec;
		}
	}

	std::vector<Server::emailrequest_t> * Table::getStaleNotifications( int match_id )
	{
		std::lock_guard<std::recursive_mutex> scopelock(tablelock);
		char * query = (char*)calloc( 2048, sizeof( char ) );
		sprintf( query, "select e.id,e.match_id,email,hours,turn,matchnation_id,if(utc_timestamp>date_add((select lastturn from matches where id=e.match_id), interval (select hostinterval from matches where id=e.match_id)/60-e.hours hour),1,0) from emailrequests e where match_id=%lu;", match_id );
		int sqlerrno;
		if( (sqlerrno = mysql_query( m_con, query )) != 0 ) {
			fprintf( stdout, "Failed to request stale notifications\n" );
			free( query );
			return NULL;
		} else {
			std::vector<Server::emailrequest_t> * vec = new std::vector<Server::emailrequest_t>();
			MYSQL_RES * emailreqs = mysql_store_result( m_con );
			if( emailreqs == NULL ) {
			} else {
				MYSQL_ROW emailrow;
				while( (emailrow = mysql_fetch_row( emailreqs ) ) != NULL ) {
					char * address = (char*)calloc( 1024, sizeof( char ) );
					strcpy( address, emailrow[2] );
					vec->push_back(Server::emailrequest_t{
						atoi(emailrow[0]),	// ID
						atoi(emailrow[1]),	// Match ID
						address,			// Address
						atoi(emailrow[3]),	// Hours until host
						atoi(emailrow[4]),	// Turn number
						satoi(emailrow[5]),	// Matchnation ID
						atoi(emailrow[6])});// Is it time to send
				}
				mysql_free_result( emailreqs );
			}
			free( query );
			return vec;
		}
	}

	void Table::setSNTN( int id, int n )
	{
		std::lock_guard<std::recursive_mutex> scopelock(tablelock);
		char * query = (char*)calloc( 128, sizeof( char ) );
		sprintf( query, "update emailrequests set turn=%d where id=%d", n, id );
		int sqlerrno;
		if( (sqlerrno = mysql_query( m_con, query )) != 0 ) {
			fprintf( stdout, "Failed to mark turn number in notification: %s\n", query );
		}
		free( query );
		return;
	}

	void Table::markTurnSubmitted( Game::Match * match, int pl )
	{
		std::lock_guard<std::recursive_mutex> scopelock(tablelock);
		char * query = (char*)calloc( 2048, sizeof( char ) );
		sprintf( query, "select id from matchnations where nation_id in (select id from nations where dom_id=%d AND mod_id in (select mod_id from matchmods where match_id=%lu)) AND match_id=%lu", 
			pl, match->id, match->id );
		int matchnation_id;
		int turn_id;
		int sqlerrno;
		if( (sqlerrno = mysql_query( m_con, query )) != 0 ) {
			fprintf( stdout, "Failed to query matchnation for match %lu nation %d\n", match->id, pl );
			fprintf( stdout, query );
		} else {
			MYSQL_RES * res = mysql_store_result( m_con );
			if( res == NULL ) {
				fprintf( stdout, "Failed to fetch results matchnation for match %lu nation %d\n", match->id, pl );
				fprintf( stdout, query );
				free( query );
				return;
			}
			MYSQL_ROW row = mysql_fetch_row( res );
			if( row == NULL ) {
				fprintf( stdout, "Failed to retrieve row matchnation for match %lu nation %d\n", match->id, pl );
				fprintf( stdout, query );
				free( query );
				return;
			}
			matchnation_id = atoi(row[0]);
			mysql_free_result( res );

			sprintf( query, "select id from turns where match_id=%lu order by tn desc limit 1", match->id );
			if( (sqlerrno = mysql_query( m_con, query )) != 0 ) {
				fprintf( stdout, "Failed to retrieve last turn for match %lu\n", match->id);
			} else {
				MYSQL_RES * res = mysql_store_result( m_con );
				if( res == NULL ) {
					fprintf( stdout, "Failed to retrieve last turn for match %lu\n", match->id);
					free( query );
					return;
				}
				MYSQL_ROW row = mysql_fetch_row( res );
				if( row == NULL ) {
					fprintf( stdout, "Failed to retrieve last turn for match %lu\n", match->id);
					free( query );
					return;
				}
				turn_id = atoi(row[0]);
				mysql_free_result( res );

				sprintf( query, "insert into matchnationturns (matchnation_id,turn_id) values(%d,%d)", matchnation_id, turn_id );
				if( (sqlerrno = mysql_query( m_con, query )) != 0 ) {
					fprintf( stdout, "Failed to insert turn %d into match %s for nation %d\n", turn_id, match->name, matchnation_id);
				}
				fprintf( stdout, "Inserted new turn %d into match %s for player %d\n", turn_id, match->name, matchnation_id );
			}
		}
		free( query );
	}
	int Table::hasSubmittedTurn( Game::Match * match, int pl, int tn )
	{
		int ret = 0, sqlerrno;
		std::lock_guard<std::recursive_mutex> scopelock(tablelock);
		char * query = (char*)calloc( 2048, sizeof( char ) );
		sprintf( query, "select id from matchnationturns where matchnation_id=%d AND turn_id=(select id from turns where match_id=%lu AND tn=%d)", pl, match->id, tn );
		if( (sqlerrno = mysql_query( m_con, query )) != 0 ) {
			fprintf( stdout, "Failed to check if there is a turn for %d in match %lu", pl, match->id );
		} else {
			MYSQL_RES * res = mysql_store_result( m_con );
			if( res == NULL ) {
				fprintf( stdout, "Failed to get res for turn check for %d in match %lu", pl, match->id );
			} else {
				MYSQL_ROW row = mysql_fetch_row( res );
				if(  row == NULL ) {
					ret = 0;
				} else {
					ret = 1;
				}
			}
			mysql_free_result( res );
		}
		free( query );
		return ret;
	}
}
