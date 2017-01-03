#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include "prngd.h"

void*
my_malloc(size_t size)
{
	void* buf = NULL;
	buf = malloc(size);

	if (buf == NULL) {
		error_fatal("malloc() failed", errno);
		exit(EXIT_FAILURE);
	}
	return buf;
}


int
my_pipe(int filedes[2])
{
	int rv;
	rv = pipe(filedes);

	if (rv == -1) {
		error_error("pipe() failed", errno);
	}
	return rv;
}


void
my_dup2(int fd1, int fd2)
{
	int rv;
	rv = dup2(fd1, fd2);

	if (rv == -1) {
		error_error("dup2() failed", errno);
	}
}


void
my_gettimeofday(struct timeval* tp, void* null)
{
	int rv;
	rv = gettimeofday(tp, null);

	if (rv == -1) {
		error_error("gettimeofday() failed", errno);
	}
}


void
my_shutdown(int s, int how)
{
	int rv;
	rv = shutdown(s, how);

	if (rv == -1) {
#ifdef	ENOTCONN
	    if (errno != ENOTCONN)
#endif
		error_error("shutdown() failed", errno);
	}
}
