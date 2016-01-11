#include "settings.hpp"
#include <libconfig.h>
#include <stdlib.h>
namespace Server
{
	const char * Settings::mappath;
	const char * Settings::exepath;
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

		if( config_lookup_string( cf, "mappath", &mappath ) )
			fprintf( stdout, "Found mappath: %s\n", mappath );
		if( config_lookup_string( cf, "exepath", &exepath ) )
			fprintf( stdout, "Found exepath: %s\n", exepath );
		return 0;
	}
}
