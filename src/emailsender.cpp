#include "emailsender.hpp"
#include <string>
#include <iostream>
#include <sstream>
#include "settings.hpp"

namespace Server
{
	EmailSender::EmailSender( void ): session{Poco::Net::SecureSMTPClientSession("blitzserver.net")}
	{
		try {
			session.open();
			Poco::Net::initializeSSL();
			ptrHandler = new Poco::Net::AcceptCertificateHandler(false);
			ptrContext = new Poco::Net::Context(Poco::Net::Context::CLIENT_USE, "", "", "", Poco::Net::Context::VERIFY_RELAXED, 9, true, "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH");
			Poco::Net::SSLManager::instance().initializeClient(0, ptrHandler, ptrContext);
			session.login();
		} catch( Poco::Net::NetException &e ) {
			std::cout << "Threw exception trying to contact mail server: " <<e.message() << '\n';
			return;
		}
		try {
			if( session.startTLS(ptrContext) ) {
				session.login( Poco::Net::SecureSMTPClientSession::AUTH_LOGIN, Server::Settings::emailuser, Server::Settings::emailpass );
			} else {
				fprintf( stdout, "Failed to open TLS connection\n" );
			}
			fprintf( stdout, "Connected to email server\n" );
			return;
		} catch( Poco::Net::NetException &e ) {
			std::cout << " Threw exception trying to open TLS connection: " << e.message() << '\n';
			return;
		} catch( Poco::IOException &e ) {
			std::cout << " Threw IO exception trying to open TLS connection: " << e.message() << '\n';
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
			stream << "New turn for match." << cmatch->name;
		} else if( hours == -1 ) {
			stream << "Match " << cmatch->name << " is starting.";
		} else {
			stream << "The next turn for match " << cmatch->name << " is rolling over in " << hours << " hours, but you have not submitted yet. Please do so!";
		}
		stream << " You can connect to it using " << Server::Settings::domain << ":" << cmatch->port;
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
