#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "prngd.h"

/*
 * Handle read and write operations so that they are protected against
 * EINTR and EAGAIN "non-errors".
 */
ssize_t socket_read(int fildes, void *buf, size_t nbyte)
{
  ssize_t retval;

  for (;;) {
    retval = read(fildes, buf, nbyte);
    if (retval >= 0)
      break;
    else if (errno == EINTR)
      continue;
    else if (errno == EAGAIN)
      continue;
#ifdef EWOULDBLOCK
    else if (errno == EWOULDBLOCK)
      continue;
#endif
    else if (retval == -1) {
      error_error("read() in socket_read() failed", errno);
      break;
    }
    else
      break;
  }

  return retval;
}

ssize_t socket_write(int fildes, void *buf, size_t nbyte)
{
  ssize_t retval;

  for (;;) {
    retval = write(fildes, buf, nbyte);
    if (retval >= 0)
      break;
    else if (errno == EINTR)
      continue;
    else if (errno == EAGAIN)
      continue;
#ifdef EWOULDBLOCK
    else if (errno == EWOULDBLOCK)
      continue;
#endif
    else if (errno == EPIPE) {
      error_notice("write() in socket_write() failed", errno);
      break;
    }
    else if (retval == -1) {
      error_error("write() in socket_write() failed", errno);
      break;
    }
    else
      break;
  }

  return retval;
}
