#include "settings.hpp"
#include <sys/stat.h>
#include <unistd.h>
#include <libconfig.h>
#include <stdlib.h>
namespace Server
{
	const char * Settings::mappath_save;
	const char * Settings::mappath_load;
	const char * Settings::savepath;
	const char * Settings::exepath;
	const char * Settings::dbuser;
	const char * Settings::dbpass;
	const char * Settings::dbname;
	int Settings::loadSettings( const char * configpath )
	{
		char * configfile = (char*)calloc( 256, sizeof( char* ));
		sprintf( configfile,  "%sconfig.cfg", configpath );
		config_t cfg, * cf = NULL;
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

		if( canread && config_lookup_string( cf, "mappath_load", &mappath_load ) )
			fprintf( stdout, "Found mappath load directory: %s\n", mappath_load );
		else {
			fprintf( stdout, "Failed to find map load directory. Please tell me where it is.\n" );
			mappath_load = (const char *)calloc( 512, sizeof( char ) );
			fscanf( stdin, "%s", (char*)mappath_load );
			config_setting_set_string( config_lookup( cf, "mappath_load"), mappath_load );
		}
		
		if( canread && config_lookup_string( cf, "mappath_save", &mappath_save ) )
			fprintf( stdout, "Found mappath save directory: %s\n", mappath_save );
		else {
			fprintf( stdout, "Failed to find map save directory. Please tell me where it is.\n" );
			mappath_save = (const char *)calloc( 512, sizeof( char ) );
			fscanf( stdin, "%s", (char*)mappath_save );
			config_setting_set_string( config_lookup( cf, "mappath_save"), mappath_save );
		}

		if( canread && config_lookup_string( cf, "savepath", &savepath ) )
			fprintf( stdout, "Found mappath save directory: %s\n", savepath );
		else {
			fprintf( stdout, "Failed to find dom4 save directory. Please tell me where it is.\n" );
			savepath = (const char *)calloc( 512, sizeof( char ) );
			fscanf( stdin, "%s", (char*)savepath );
			config_setting_set_string( config_lookup( cf, "savepath"), savepath );
		}

		if( canread && config_lookup_string( cf, "exepath", &exepath ) )
			fprintf( stdout, "Found exepath: %s\n", exepath );
		else {
			fprintf( stdout, "Failed to find dom4 exe directory. Please tell me where it is.\n" );
			exepath = (const char *)calloc( 512, sizeof( char ) );
			fscanf( stdin, "%s", (char*)exepath );
			config_setting_set_string( config_lookup( cf, "exepath"), exepath );
		}

		if( canread && config_lookup_string( cf, "dbuser", &dbuser ) )
			fprintf( stdout, "Found dbuser: %s\n", dbuser );
		else {
			fprintf( stdout, "Failed to find database user. Please tell me who it is.\n" );
			dbuser = (const char *)calloc( 512, sizeof( char ) );
			fscanf( stdin, "%s", (char*)dbuser );
			config_setting_set_string( config_lookup( cf, "dbuser"), dbuser );
		}

		if( canread && config_lookup_string( cf, "dbpass", &dbpass ) )
			fprintf( stdout, "Found dbpass\n" );
		else {
			fprintf( stdout, "Failed to find database password. Please tell me what it is.\n" );
			dbpass = (const char *)calloc( 512, sizeof( char ) );
			fscanf( stdin, "%s", (char*)dbpass );
			config_setting_set_string( config_lookup( cf, "dbpass"), dbpass );
		}

		if( canread && config_lookup_string( cf, "dbname", &dbname ) )
			fprintf( stdout, "Found dbname: %s\n", dbname );
		else {
			fprintf( stdout, "Failed to find database name. Please tell me what it is.\n" );
			dbname = (const char *)calloc( 512, sizeof( char ) );
			fscanf( stdin, "%s", (char*)dbname );
			config_setting_set_string( config_lookup( cf, "dbname"), dbname );
		}

		if( config_write_file( cf, configfile ) == CONFIG_FALSE ) {
			fprintf( stdout, "Failed to write config\n" );
		}
		return 0;
	}
}
