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
		static int loadSettings( const char * configpath );
	};
}
