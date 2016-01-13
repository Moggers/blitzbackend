//////
// This popen2 implementation was written by Jeff Epler
// I found it at https://emergent.unpythonic.net/01108826729
// That's the closest thing I have to a copyright notice I'm just a cowboy programmer please don't sue me ._.
/////
#include "util.hpp"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>


int popen2(const char *cmdline,  popen2_t *childinfo) {
    pid_t p;
    int pipe_stdin[2], pipe_stdout[2];

    if(pipe(pipe_stdin)) return -1;
    if(pipe(pipe_stdout)) return -1;

    printf("pipe_stdin[0] = %d, pipe_stdin[1] = %d\n", pipe_stdin[0], pipe_stdin[1]);
    printf("pipe_stdout[0] = %d, pipe_stdout[1] = %d\n", pipe_stdout[0], pipe_stdout[1]);

    p = fork();
    if(p < 0) return p; /* Fork failed */
    if(p == 0) { /* child */
        close(pipe_stdin[1]);
        dup2(pipe_stdin[0], 0);
        close(pipe_stdout[0]);
        dup2(pipe_stdout[1], 1);
        execl("/bin/sh", "sh", "-c", cmdline, 0);
        perror("execl"); exit(99);
    }
    childinfo->child_pid = p;
    childinfo->to_child = pipe_stdin[1];
    childinfo->from_child = pipe_stdout[0];
    return 0; 
}
