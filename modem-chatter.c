#define _GNU_SOURCE 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <error.h>
#include <sys/select.h>

#include <readline/readline.h>
#include <readline/history.h>

// We begin with \001 and end with \002 to avoid confusing readline.
// If we don't use the \001 and \002, line wrapping won't work:
// http://stackoverflow.com/questions/9468435
#define KRED  "\001\x1B[31m\002"
#define KGRN  "\001\x1B[32m\002"
#define KNRM  "\001\x1B[0m\002"

#define INPUTPROMPT  KRED "->|"
#define OUTPUTPROMPT KGRN "<-|"

void process_input(int serialfd, int timeout);

int
main(int argc, char *argv[])
{
    struct termios term;
    int i;
    int serialfd;
    char c;

    char *device = argv[1];

    serialfd = open (device, O_RDWR | O_NOCTTY);
    if (serialfd < 0){
        error (1, errno, "open");
    }

    memset (&term, 0, sizeof (term));

    term.c_cflag = CREAD | CS8;
    term.c_cc[VMIN] = 1;

    cfsetispeed (&term, B115200);
    cfsetospeed (&term, B115200);

    tcsetattr (serialfd, TCSANOW, &term);

    /* Flush input buffer.  */
    int oldfl = fcntl (serialfd, F_GETFL);
    fcntl (serialfd, F_SETFL, oldfl | O_NONBLOCK);
    while (read (serialfd, &c, 1) != -1);
    fcntl (serialfd, F_SETFL, oldfl);

    while(1){
        char *line = readline(OUTPUTPROMPT);
        if(!line){
            puts(KNRM);
            break;
        }
        else {
            dprintf(serialfd,"%s", line);
            add_history(line);
            free(line);
        }
        process_input(serialfd,1);
    }
    close (serialfd);
    return EXIT_SUCCESS;
}

void
process_input(int serialfd, int timeout)
{

    char c;
    int newline = 1;
    //// Initialize file descriptor sets
    while(1) {
        fd_set read_fds, write_fds, except_fds;
        FD_ZERO(&read_fds);
        FD_ZERO(&write_fds);
        FD_ZERO(&except_fds);
        FD_SET(serialfd, &read_fds);

        // Set timeout to 1.0 seconds
        struct timeval tv;
        tv.tv_sec = timeout; tv.tv_usec = 0;

        // Wait for input to become ready or until the time out; the first parameter is
        // 1 more than the largest file descriptor in any of the sets
        if (select(serialfd + 1, &read_fds, &write_fds, &except_fds, &tv) == 1){
            read (serialfd, &c, 1);
            if (newline){
                newline = 0;
                printf(KRED "->|");
            }
            putchar (c);
            if(c == '\n'){
                newline = 1;
            }
        }
        else { //we ran into timeout
            if (!newline){
                putchar('\n');
            }
            break;
        }

    }
}
