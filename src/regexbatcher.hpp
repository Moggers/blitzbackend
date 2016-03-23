#pragma once
#include <functional>
#include <vector>
#include <mutex>
#include <regex>
namespace Server
{
	class RegexJob
	{
		private:
		const std::function<void(const std::smatch&)> callback;
		const std::regex pattern;
		public:
		RegexJob( const std::string &pattern, const std::function<void(const std::smatch&)> &callback );
		int check( const std::string &line );
	};

	class RegexBatcher
	{
		public:
		void addCheck( const std::string &pattern, const std::function<void(const std::smatch&)> &callback );
		int checkString( const std::string &line );
		private:
		std::vector<RegexJob> jobs;
		static std::recursive_mutex mut;
	};
}

