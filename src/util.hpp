#pragma once
#include <sys/types.h>
#define PORT_MIN 2048 
#define PORT_MAX 4096
typedef struct popen2_s {
    pid_t child_pid;
    int   from_child, to_child;
} popen2_t;
int popen2(const char *cmdline, popen2_t *childinfo);

int port_check( int portno );
