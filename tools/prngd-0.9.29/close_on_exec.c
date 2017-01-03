#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include "prngd.h"

/*
 * Set or remove "close on exec" flag for file descriptor.'
 * Inspired by close_on_exec() function in Wietse Venema's Postfix.
 * On failure, there is nothing left what we can do.
 */
#define PATTERN FD_CLOEXEC

void close_on_exec(int fd, int on)
{
  int flags;

  if ((flags = fcntl(fd, F_GETFD, 0)) == -1) {
    error_fatal("fcntl() failed to get file descriptor flags", errno);
    exit(EXIT_FAILURE);
  }
  if (fcntl(fd, F_SETFD, on ? flags | PATTERN : flags & ~PATTERN) == -1) {
    error_fatal("fcntl() failed to set file descriptor flags", errno);
    exit(EXIT_FAILURE);
  }
}
