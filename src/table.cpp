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
			matches[ii] = new Game::Match( row, mappathrow );
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
			matches[ii] = new Game::Match( row, mappathrow );
			ii++;
		}

		return matches;
	};

	void Table::saveMatch( Game::Match * match )
	{
		char * query = (char*)calloc( 2048, sizeof( char ) );
		sprintf( query, "update matches set status=%d,port=%d where id=%d;", match->status, match->port, match->id );
		int sqlerrno;
		if( (sqlerrno = mysql_query( m_con, query )) != 0 ) {
			fprintf( stdout, "Warning! Failed to save data back to sql server: %d\n", sqlerrno );
		}
	}

	void Table::deleteMatch( Game::Match * match )
	{
		char * query = (char*)calloc( 2048, sizeof( char ) );
		sprintf( query, "delete from matches where id=%d", match->id );
		int sqlerrno;
		if( (sqlerrno = mysql_query( m_con, query )) != 0 ) {
			fprintf( stdout, "Warning! Failed to save data back to sql server: %d\n", sqlerrno );
		}
	}
}
