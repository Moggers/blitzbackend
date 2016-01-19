#include "settings.hpp"
#include <libconfig.h>
#include <stdlib.h>
namespace Server
{
	const char * Settings::mappath_save;
	const char * Settings::mappath_load;
	const char * Settings::exepath;
	const char * Settings::dbuser;
	const char * Settings::dbpass;
	const char * Settings::dbname;
	int Settings::loadSettings( const char * configpath )
	{
		config_t cfg, * cf = NULL;
		const config_setting_t *retries;

		cf = (config_t*)calloc( 1, sizeof( config_t ) );
		config_init( cf );
		
		if( !config_read_file( cf, configpath ) ) {
			fprintf( stdout, "%s:%d -%s\n",
				config_error_file( cf ),
				config_error_line( cf ),
				config_error_text( cf ) );
			config_destroy( cf );
			return 1;
		}

		if( config_lookup_string( cf, "mappath_load", &mappath_load ) )
			fprintf( stdout, "Found mappath load directory: %s\n", mappath_load );
		if( config_lookup_string( cf, "mappath_save", &mappath_save ) )
			fprintf( stdout, "Found mappath save directory: %s\n", mappath_save );
		if( config_lookup_string( cf, "exepath", &exepath ) )
			fprintf( stdout, "Found exepath: %s\n", exepath );
		if( config_lookup_string( cf, "dbuser", &dbuser ) )
			fprintf( stdout, "Found dbuser: %s\n", dbuser );
		if( config_lookup_string( cf, "dbpass", &dbpass ) )
			fprintf( stdout, "Found dbpass\n" );
		if( config_lookup_string( cf, "dbname", &dbname ) )
			fprintf( stdout, "Found dbname: %s\n", dbname );
		return 0;
	}
}
