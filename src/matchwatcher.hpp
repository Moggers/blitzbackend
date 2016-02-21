#include "util.hpp"
#include "table.hpp"
#include <stdint.h>
#include <pthread.h>
namespace Server
{
	class MatchWatcher
	{
		private:
		popen2_t * proc;
		popen2_t * pollproc;
		pthread_t * watchThread;
		pthread_mutex_t * lock;
		SQL::Table * table;
		Game::Match * match;
		static void* watchCallback( void* arg );
		int kill;
		int lastn;

		public:
		int64_t playerbitmap;
		int port;
		MatchWatcher( popen2_t * proc, SQL::Table * table, Game::Match * match );
		void destroyWatcher( void );
		int mesg;
	};
}
