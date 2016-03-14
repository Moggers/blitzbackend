//////
// This popen2 implementation was written by Jeff Epler
// I found it at https://emergent.unpythonic.net/01108826729
// That's the closest thing I have to a copyright notice I'm just a cowboy programmer please don't sue me ._.
/////
#include <string.h>
#include "util.hpp"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>


int popen2(const char *cmdline,  popen2_t *childinfo) {
    pid_t p;
    int pipe_stdin[2], pipe_stdout[2];

    if(pipe(pipe_stdin)) return -1;
    if(pipe(pipe_stdout)) return -1;

    //printf("pipe_stdin[0] = %d, pipe_stdin[1] = %d\n", pipe_stdin[0], pipe_stdin[1]);
    //printf("pipe_stdout[0] = %d, pipe_stdout[1] = %d\n", pipe_stdout[0], pipe_stdout[1]);

    p = fork();
    if(p < 0) return p; /* Fork failed */
    if(p == 0) { /* child */
        close(pipe_stdin[1]);
        dup2(pipe_stdin[0], 0);
        close(pipe_stdout[0]);
        dup2(pipe_stdout[1], 1);
		setpgid( 0, 0 );
        execl("/bin/sh", "sh", "-c", cmdline, 0);
        perror("execl"); exit(99);
    }
    childinfo->child_pid = p;
    childinfo->to_child = pipe_stdin[1];
    childinfo->from_child = pipe_stdout[0];
    return 0; 
}

int port_check( int portno )
{
	int sockfd;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	const char * hostname = "localhost";

	sockfd = socket( AF_INET, SOCK_STREAM, 0 );
	if( sockfd < 0 ) {
		fprintf( stdout, "Failed checking socket\n" );
		return -1;
	}

	server = gethostbyname( hostname );

	if( server == NULL ) {
		fprintf( stdout, "Failed to load hostname\n"  );
		return -1;
	}

	bzero((char *) &serv_addr, sizeof( serv_addr ) );
	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr,
		(char *)&serv_addr.sin_addr.s_addr,
		server->h_length);

	serv_addr.sin_port = htons(portno);
	usleep( 500000 );
	if( connect( sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
		close( sockfd );
		return 1;
	} else {
		close( sockfd );
		return 0;
	}
}

int get_port( void )
{
	struct addrinfo hints, *res;
	int sockfd;

	// first, load up address structs with getaddrinfo():

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;  // use IPv4 or IPv6, whichever
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;     // fill in my IP for me

	getaddrinfo(NULL, "0", &hints, &res);
	sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	int port = -1;

	bind( sockfd, res->ai_addr, res->ai_addrlen );
	sockaddr_in sa;
	socklen_t sa_len = sizeof( sa );
	if( getsockname( sockfd, (sockaddr*)&sa, &sa_len ) == -1 ) {
		return -1;
	}
	port = sa.sin_port;
	close( sockfd );
	free( res );
	return port;
}
int get_specific_port( int port )
{
	if( port_check( port ) == 1 ) 
		return port;
	else return -1;
}

int try_get_port( int port )
{
	struct addrinfo hints, *res;
	int sockfd;

	// first, load up address structs with getaddrinfo():

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;  // use IPv4 or IPv6, whichever
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;     // fill in my IP for me

	char * w = (char*)calloc( 16, sizeof(char) );
	sprintf( w, "%d", port );
	getaddrinfo(NULL, w, &hints, &res);

	sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

	if( bind( sockfd, res->ai_addr, res->ai_addrlen ) != 0 ) {
		freeaddrinfo( res );
		getaddrinfo(NULL, "0", &hints, &res);
		bind( sockfd, res->ai_addr, res->ai_addrlen );
		sockaddr_in sa;
		socklen_t sa_len = sizeof( sa );
		if( getsockname( sockfd, (sockaddr*)&sa, &sa_len ) == -1 ) {
			freeaddrinfo( res );
			free( w );
			return -1;
		}
		port = sa.sin_port;
	}
	close( sockfd );
	free( w );
	freeaddrinfo( res );
	return port;
}

int satoi( const char * val )
{
	if( val == 0 ) {
		return -1;
	} else {
		return atoi( val );
	}
}
