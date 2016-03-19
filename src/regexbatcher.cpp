#include "regexbatcher.hpp"

namespace Server
{
	RegexJob::RegexJob( const std::string &pattern, const std::function<void(const std::smatch&)> &callback ) :
		pattern{ std::regex( pattern ) },
		callback{ callback }
	{
	}
	int RegexJob::check( const std::string &line )
	{
		std::smatch match;
		if( std::regex_match( line, match, this->pattern ) ) {
			this->callback( match );
			return 1;
		} return 0;
	}

	RegexBatcher::RegexBatcher()
	{
	}

	void RegexBatcher::addCheck( const std::string &pattern, const std::function<void(const std::smatch&)> &callback )
	{
		jobs.push_back( RegexJob( pattern, callback ) );
	}
	int RegexBatcher::checkString( const std::string &line )
	{
		for( auto& j : jobs ) {
			if( j.check( line ) == 1 )
				return 1;
		}
		return 0;
	}
}
