#pragma once
#include <Poco/Net/SMTPClientSession.h>
#include <Poco/Net/SecureSMTPClientSession.h>
#include <Poco/Net/MailMessage.h>
#include <Poco/Net/NetException.h>
#include <Poco/Net/MailRecipient.h>
#include <Poco/Net/AcceptCertificateHandler.h>
#include <Poco/Net/Context.h>
#include <Poco/Net/SSLManager.h>
#include "match.hpp"
namespace Server
{
	class EmailSender
	{
		public:
		EmailSender(void);
		~EmailSender(void);
		void sendNotification( int hours, const char * address, Game::Match * cmatch );
		
		private:
		Poco::Net::SecureSMTPClientSession session;
		Poco::Net::Context::Ptr ptrContext;
		Poco::SharedPtr<Poco::Net::InvalidCertificateHandler> ptrHandler;
	};

	typedef struct emailrequest_s {
		int match_id;
		char * address;
		int hours;
	} emailrequest_t;
}
