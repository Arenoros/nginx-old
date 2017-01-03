#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include "prngd.h"

/*
 * Set or remove "non_blocking" flag for file descriptor.'
 * Inspired by non_blocking() function in Wietse Venema's Postfix.
 * On failure, there is nothing left what we can do.
 */
#ifdef FNDELAY
#define PATTERN FNDELAY
#else
#define PATTERN O_NONBLOCK
#endif

void non_blocking(int fd, int on)
{
  int flags;

  if ((flags = fcntl(fd, F_GETFL, 0)) == -1) {
    error_fatal("fcntl() failed to get file descriptor flags", errno);
    exit(EXIT_FAILURE);
  }
  if (fcntl(fd, F_SETFL, on ? flags | PATTERN : flags & ~PATTERN) == -1) {
    error_fatal("fnctl() failed to set file descriptor flags", errno);
    exit(EXIT_FAILURE);
  }
}
