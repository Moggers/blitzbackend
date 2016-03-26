#include "emailsender.hpp"
#include <string>
#include <iostream>
#include <sstream>
#include "settings.hpp"

namespace Server
{
	EmailSender::EmailSender( void )
	{
		try {
			Poco::Net::initializeSSL();
			// Create memes
			ptrCert = new Poco::Net::AcceptCertificateHandler(false);
			ptrContext = new Poco::Net::Context(Poco::Net::Context::CLIENT_USE, "", "", "", Poco::Net::Context::VERIFY_RELAXED, 9, true, "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH");
			Poco::Net::SSLManager::instance().initializeClient(0, ptrCert, ptrContext);
			// Connect
			ptrSSLSocket = new Poco::Net::SecureStreamSocket( ptrContext );
			ptrSSLSocket->connect(Poco::Net::SocketAddress( Server::Settings::emailserver_address, 465 ));
			session = new Poco::Net::SecureSMTPClientSession( *ptrSSLSocket );
			session->login(Poco::Net::SMTPClientSession::AUTH_LOGIN, Server::Settings::emailuser, Server::Settings::emailpass);
		} catch( Poco::Exception &e ) {
			std::cout << " Threw IO exception trying to open TLS connection: " << e.message() << '\n';
			return;
		}
	}

	EmailSender::~EmailSender( void )
	{
		session->close();
		Poco::Net::uninitializeSSL();
	}
	void EmailSender::sendNotification( int hours, const char * address, Game::Match * cmatch )
	{
		std::cout << "Sending email to " << address << "\n";
		std::ostringstream stream;
		if( hours == 0 ) {
			stream << "New turn for match " << cmatch->name;
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
			session->sendMessage(message);
			return;
		} catch( Poco::Net::NetException &e ) {
			std::cout << "Failed to send turn update to " << address << ": " << e.message() << '\n';
			return;
		} catch( Poco::IllegalStateException &e ) {
			std::cout << e.message() << '\n';
			return;
		} catch( Poco::TimeoutException &e ) {
			ptrSSLSocket->connect(Poco::Net::SocketAddress( Server::Settings::emailserver_address, 465 ));
			session = new Poco::Net::SecureSMTPClientSession( *ptrSSLSocket );
			session->login(Poco::Net::SMTPClientSession::AUTH_LOGIN, Server::Settings::emailuser, Server::Settings::emailpass);
			session->sendMessage(message);
		}
	}
}
