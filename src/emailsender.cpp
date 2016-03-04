#include "emailsender.hpp"
#include <string>
#include <iostream>
#include <sstream>
#include "settings.hpp"

namespace Server
{
	EmailSender::EmailSender( void ): session{Poco::Net::SecureSMTPClientSession("blitzserver.net")}
	{
		session.open();
		Poco::Net::initializeSSL();
		ptrHandler = new Poco::Net::AcceptCertificateHandler(false);
		ptrContext = new Poco::Net::Context(Poco::Net::Context::CLIENT_USE, "", "", "", Poco::Net::Context::VERIFY_RELAXED, 9, true, "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH");
		Poco::Net::SSLManager::instance().initializeClient(0, ptrHandler, ptrContext);
		try {
			session.login();
			if( session.startTLS() ) {
				session.login( Poco::Net::SecureSMTPClientSession::AUTH_LOGIN, Server::Settings::emailuser, Server::Settings::emailpass );
			} else {
				fprintf( stdout, "Failed to open TLS connection\n" );
			}
			return;
		} catch( Poco::Net::NetException &e ) {
			std::cout << e.message() << '\n';
			return;
		} catch( Poco::IllegalStateException &e ) {
			std::cout << e.message() << '\n';
			return;
		}
	}

	EmailSender::~EmailSender( void )
	{
		session.close();
	}
	void EmailSender::sendNotification( int hours, const char * address, Game::Match * cmatch )
	{
		std::ostringstream stream;
		if( hours == 0 ) {
			stream << "New turn for match " << cmatch->name;
		} else if( hours == -1 ) {
			stream << "Match " << cmatch->name << " is starting";
		} else {
			stream << "Next turn for match " << cmatch->name << " in " << hours << " hours";
		}
		stream << ", you can connect to it using " << Server::Settings::domain << ":" << cmatch->port;
		std::string subject = Poco::Net::MailMessage::encodeWord(stream.str(), "UTF-8");
		Poco::Net::MailMessage message;
		std::ostringstream sendstream;
		sendstream << Server::Settings::emailuser;
		message.setSender( sendstream.str() );
		message.addRecipient(Poco::Net::MailRecipient(Poco::Net::MailRecipient::PRIMARY_RECIPIENT, std::string(address)));
		message.setSubject( subject );
		message.setContentType( "text/plain; charset=UTF-8");
		message.setContent( stream.str(), Poco::Net::MailMessage::ENCODING_8BIT );

		try {
			session.login();
			session.sendMessage(message);
			return;
		} catch( Poco::Net::NetException &e ) {
			std::cout << "Failed to send turn update to " << address << ": " << e.message() << '\n';
			return;
		} catch( Poco::IllegalStateException &e ) {
			std::cout << e.message() << '\n';
			return;
		}
	}
}