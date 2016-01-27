#include "util.hpp"
#include <pthread.h>
namespace Server
{
	class MatchWatcher
	{
		private:
		popen2_t * proc;
		pthread_t * watchThread;
		pthread_mutex_t * lock;
		static void* watchCallback( void* arg );
		int kill;

		public:
		MatchWatcher( popen2_t * proc );
		void destroyWatcher( void );
		int mesg;
	};
}
