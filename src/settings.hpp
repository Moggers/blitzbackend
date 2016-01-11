namespace Server
{
	class Settings
	{
		public:
		static const char * mappath;
		static const char * exepath;
		static int loadSettings( const char * configpath );
	};
}
