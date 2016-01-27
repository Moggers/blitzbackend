#include "util.hpp"
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
		static void* watchCallback( void* arg );
		int kill;

		public:
		int64_t playerbitmap;
		int port;
		MatchWatcher( popen2_t * proc );
		void destroyWatcher( void );
		int mesg;
	};
}
