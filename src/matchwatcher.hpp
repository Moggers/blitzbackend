#include "util.hpp"
#include "table.hpp"
#include <regex>
#include <stdint.h>
#include <thread>
#include <mutex>
#include "turnparser.hpp"
#include "regexbatcher.hpp"
namespace Server
{
	class MatchWatcher
	{
		private:
		popen2_t * proc;
		popen2_t * pollproc;
		std::thread watchThread;
		std::mutex lock;
		SQL::Table * table;
		Game::Match * match;
		static void* watchCallback( void* arg );
		int kill;
		int lastn;
		int currentturn;
		std::vector<std::regex> regex_set;
		RegexBatcher batcher;
		TurnParser turnParser;

		public:
		int64_t playerbitmap;
		int port;
		MatchWatcher( popen2_t * proc, SQL::Table * table, Game::Match * match, EmailSender * emailSender );
		void destroyWatcher( void );
		int mesg;
		EmailSender * emailSender;

		void sendAllNotifications( int type );
	};
}
