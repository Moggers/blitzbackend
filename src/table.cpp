#include "table.hpp"
#include "settings.hpp"
#include <string.h>
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
		mysql_close( m_con );
	}

	Game::Match ** Table::getAllMatches( void )
	{
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
			if( mysql_query( m_con, query ) )
				return NULL;
			MYSQL_RES * mappath = mysql_store_result( m_con );
			if ( mappath == NULL )
				return NULL;

			MYSQL_ROW mappathrow = mysql_fetch_row( mappath );
			std::vector<Game::Mod*> * mods = getModsByMatch( atoi(row[0]) );
			matches[ii] = new Game::Match( row, mappathrow, mods );
			ii++;
		}

		return matches;
	};

	Game::Match ** Table::getMatchesByStatus( int count, ...  )
	{
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
			return NULL;
		}
		MYSQL_RES * res = mysql_store_result( m_con );
		if( res == NULL ) {
			return NULL;
		}
		free( query );

		Game::Match ** matches = (Game::Match**)calloc( 256, sizeof( Game::Match * ) );

		int ii = 0;
		while( MYSQL_ROW row = mysql_fetch_row( res ) ) {
			char * query = (char*)calloc( 128, sizeof( char ) );
			sprintf( query, "select mappath,imagepath from maps where id=%s", row[1] );
			if( mysql_query( m_con, query ) )
				return NULL;
			MYSQL_RES * mappath = mysql_store_result( m_con );
			if ( mappath == NULL )
				return NULL;

			MYSQL_ROW mappathrow = mysql_fetch_row( mappath );
			std::vector<Game::Mod*> * mods = getModsByMatch( atoi(row[0]) );
			matches[ii] = new Game::Match( row, mappathrow, mods );
			ii++;
			free( query );
			mysql_free_result( mappath );
		}
		mysql_free_result( res );
		return matches;
	};

	std::vector<Game::Mod*> * Table::getModsByMatch( int matchid )
	{
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
		std::vector<Game::Mod*>::iterator it = returnvec->begin();
		while( MYSQL_ROW modidrow = mysql_fetch_row(modids) ) {
			sprintf( query, "select dmname from mods where id=%s", modidrow[0] );
			if( mysql_query( m_con, query ) ) {
				free( query );
				return NULL;
			}
			MYSQL_RES * modname = mysql_store_result( m_con );
			MYSQL_ROW modnamerow = mysql_fetch_row( modname );
			Game::Mod * mod = new Game::Mod( atoi(modidrow[0]), modnamerow[0] );
			returnvec->insert( it++, mod );
			mysql_free_result( modname );
		}
		free( query );
		mysql_free_result( modids );
		return returnvec;
	}

	void Table::saveMatch( Game::Match * match )
	{
		char * query = (char*)calloc( 2048, sizeof( char ) );
		sprintf( query, "update matches set status=%d,port=%d where id=%d;", match->status, match->port, match->id );
		int sqlerrno;
		if( (sqlerrno = mysql_query( m_con, query )) != 0 ) {
			fprintf( stdout, "Warning! Failed to save data back to sql server: %d\n", sqlerrno );
		}
		free( query );
	}

	void Table::deleteMatch( Game::Match * match )
	{
		char * query = (char*)calloc( 2048, sizeof( char ) );
		sprintf( query, "delete from matchnations where match_id=%d", match->id );
		int sqlerrno;
		if( (sqlerrno = mysql_query( m_con, query )) != 0 )
			fprintf( stdout, "Warning! Failed to remove players from match %d\n", sqlerrno );
		sprintf( query, "delete from matchmods where match_id=%d", match->id );
		if( (sqlerrno = mysql_query( m_con, query )) != 0 )
			fprintf( stdout, "Warning! Failed to remove mods from match %d\n", sqlerrno );
		sprintf( query, "delete from matches where id=%d", match->id );
		if( (sqlerrno = mysql_query( m_con, query )) != 0 )
			fprintf( stdout, "Warning! Failed to delete match %d at sql %d\n", match->id, sqlerrno );
	}

	void Table::removeNationFromMatch( Game::Match * match, Game::Nation * nation )
	{
		char * query = (char*)calloc( 2048, sizeof( char ) );
		sprintf( query, "delete from matchnations where match_id=%d AND nation_id=%d", match->id, nation->id );
		int sqlerrno;
		if( ( sqlerrno = mysql_query(m_con, query )) != 0 )
			fprintf( stdout, "Failed to delete nation from match in database (this is tried whenever a nation is added and is probably okay%d\n", sqlerrno );
	}

	void Table::addNationToMatch( Game::Match * match, Game::Nation * nation )
	{
		removeNationFromMatch( match, nation );
		char * query = (char*)calloc( 2048, sizeof( char ) );
		sprintf( query, "insert into matchnations values (0, %d, %d, 0)", nation->id, match->id );
		int sqlerrno;
		if( ( sqlerrno = mysql_query(m_con, query )) != 0 )
			fprintf( stdout, "Failed to add nation to match in database (%d,%d) %d\n", match->id, nation->id, sqlerrno );
	}

	Game::Nation * Table::getNation( int id )
	{
		char * query = (char*)calloc(2048, sizeof( char ) );
		sprintf( query, "select * from nations where id=%d", id );
		int sqlerrno;
		if( (sqlerrno = mysql_query( m_con, query )) != 0 )
			fprintf( stdout, "Failed to retrieve nation: %d\n", sqlerrno );
		MYSQL_RES * nationstuff = mysql_store_result( m_con );
		if( nationstuff == NULL ) 
			return NULL;
		MYSQL_ROW nationrow = mysql_fetch_row( nationstuff );
		Game::Nation * newNation = new Game::Nation( id, nationrow[1], nationrow[2], nationrow[3] );
		return newNation;
	}

	Game::Nation ** Table::getDeleteRequests( Game::Match * match )
	{
		char * query = (char*)calloc( 2048, sizeof( char ) );
		sprintf( query, "select nation_id from matchnations where match_id=%d AND markdelete=1", match->id );
		int sqlerrno;
		if( (sqlerrno = mysql_query( m_con, query )) != 0 )
			fprintf( stdout, "Failed to retrieve nation delete requests: %d\n", sqlerrno );
		MYSQL_RES * nationstuff = mysql_store_result( m_con );
		if( nationstuff == NULL ) 
			return NULL;
		MYSQL_ROW nationrow;
		int i = 0;
		Game::Nation ** nations = (Game::Nation**)calloc( 64, sizeof( Game::Nation* ) );
		while( ( nationrow = mysql_fetch_row( nationstuff ) ) != NULL ) {
			nations[i] = getNation( atoi(nationrow[0]) );
			i++;
		}
		mysql_free_result( nationstuff );
		return nations;
	}

}
