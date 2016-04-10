#include "settings.hpp"
#include <sys/stat.h>
#include <unistd.h>
#include "matchhandler.hpp"
#include <stdlib.h>
namespace Server {
	const char * Settings::mappath_save;
	const char * Settings::mappath_load;
	const char * Settings::modpath_save;
	const char * Settings::modpath_load;
	const char * Settings::savepath;
	const char * Settings::exepath;
	const char * Settings::dbuser;
	const char * Settings::dbpass;
	const char * Settings::dbname;
	const char * Settings::jsondir;
	const char * Settings::pretenderdir;
	const char * Settings::emailserver_address;
	const char * Settings::emailuser;
	const char * Settings::emailpass;
	const char * Settings::domain;
	const char * Settings::masterpass;
	config_t * Settings::cf;

	void Settings::grabConfig( config_t * cf, const char * name, const char ** dest, int canread )
	{
		if( canread && config_lookup_string( cf, name, dest ) )
			fprintf( stdout, "Found %s load directory: %s\n", name, *dest );
		else {
			fprintf( stdout, "Failed to find %s. Please tell me where it is.\n", name );
			*dest = (const char *)calloc( 512, sizeof( char ) );
			fscanf( stdin, "%s", (char*)mappath_load );
			config_setting_set_string( config_lookup( cf, "mappath_load"), *dest );
		}
	}
	int Settings::loadSettings( const char * configpath )
	{
		char * configfile = (char*)calloc( 256, sizeof( char ));
		sprintf( configfile,  "%sconfig.cfg", configpath );
		cf = NULL;
		const config_setting_t *retries;

		cf = (config_t*)calloc( 1, sizeof( config_t ) );
		config_init( cf );

		int canread = 1;
		if( !config_read_file( cf, configfile ) ) {
			fprintf( stdout, "Failed to read config file at %s.\nI presume that it's just not there yet.\n(%s:%d -%s)\n",
				configfile,
				config_error_file( cf ),
				config_error_line( cf ),
				config_error_text( cf ) );
			mkdir( configfile, S_IREAD | S_IWRITE | S_IEXEC | S_IROTH | S_IXOTH );
			canread = 0;
		}

		grabConfig( cf, "mappath_load", &mappath_load, canread );
		grabConfig( cf, "mappath_save", &mappath_save, canread );
		grabConfig( cf, "modpath_load", &modpath_load, canread );
		grabConfig( cf, "modpath_save", &modpath_save, canread );
		grabConfig( cf, "savepath", &savepath, canread );
		grabConfig( cf, "exepath", &exepath, canread );
		grabConfig( cf, "dbuser", &dbuser, canread );
		grabConfig( cf, "dbpass", &dbpass, canread );
		grabConfig( cf, "dbname", &dbname, canread );
		grabConfig( cf, "jsondir", &jsondir, canread );
		grabConfig( cf, "pretenderdir", &pretenderdir, canread );
		grabConfig( cf, "emailserver_address", &emailserver_address, canread );
		grabConfig( cf, "emailuser", &emailuser, canread );
		grabConfig( cf, "emailpass", &emailpass, canread );
		grabConfig( cf, "domain", &domain, canread );
		grabConfig( cf, "masterpass", &masterpass, canread );
		if( config_write_file( cf, configfile ) == CONFIG_FALSE ) {
			fprintf( stdout, "Failed to write config\n" );
		}
		free( configfile );
		return 0;
	}

	void Settings::destroy( void )
	{
		config_destroy( Settings::cf );
	}
}
