#pragma once
#include <libconfig.h>
namespace Server
{
	class Settings
	{
		public:
		static const char * mappath_save;
		static const char * mappath_load;
		static const char * modpath_save;
		static const char * modpath_load;
		static const char * savepath;
		static const char * exepath;
		static const char * dbuser;
		static const char * dbpass;
		static const char * dbname;
		static const char * jsondir;
		static const char * pretenderdir;
		static const char * emailserver_address;
		static const char * emailuser;
		static const char * emailpass;
		static const char * domain;
		static const char * masterpass;
		static int loadSettings( const char * configpath );
		static void destroy( void );
		private:
		static config_t * cf;
		static void grabConfig( config_t* cf, const char * name, const char ** dest, int canread );
	};
}
