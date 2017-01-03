/*
 * Handle UNIX sockets.
 */
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "prngd.h"

#ifdef NO_UNIX_SOCKET
int unix_connect(const char *connection, int non_block, int close_exec)
{
  error_error("No unix domain sockets on this system", EINVAL);
  return -1;
}

int unix_listen(const char *connection, int non_block, int close_exec)
{
  error_fatal("No unix domain sockets on this system", EINVAL);
  exit(EXIT_FAILURE);
}
#else

#include <sys/un.h>

/*
 * Open a connection to the specified peer. Most errors are considered fatal,
 * that a connection cannot be opened (e.g. since no peer is listening) is not
 * considered fatal.
 */
int unix_connect(const char *connection, int non_block, int close_exec)
{
  struct sockaddr_un sockun;
  int sockfd;
  char msg_buf[1024];


  sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (sockfd < 0)
  {
    error_fatal("Could not create socket", errno);
    exit(EXIT_FAILURE);
  }

  (void)memset(&sockun, 0, sizeof(sockun));
  sockun.sun_family = AF_UNIX;
  if (sizeof(sockun.sun_path) < strlen(connection) + 1) {
    (void)snprintf(msg_buf, sizeof(msg_buf),
		   "Sockfilename too long: %s, maxlen=%ld\n", connection,
		   (long int)sizeof(sockun.sun_path));
    msg_buf[sizeof(msg_buf) - 1] = '\0';
    error_error(msg_buf, 0);
    exit(EXIT_FAILURE);
  }
  (void)strcpy(sockun.sun_path, connection);

  if (connect(sockfd, (struct sockaddr *) &sockun, sizeof(sockun)) < 0) {
    (void)close(sockfd);
    sockfd = -1;
  }
  else {
    non_blocking(sockfd, non_block);
    close_on_exec(sockfd, close_exec);
  }

  return sockfd;
}

/*
 * Initiate a listener socket on the specified path. All errors are
 * considered fatal.
 */
int unix_listen(const char *connection, int non_block, int close_exec)
{
  struct sockaddr_un sockun;
  int sockfd;
  char msg_buf[1024];

  sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (sockfd < 0)
  {
    error_fatal("Could not create socket", errno);
    exit(EXIT_FAILURE);
  }
  non_blocking(sockfd, non_block);
  close_on_exec(sockfd, close_exec);

  (void)memset(&sockun, 0, sizeof(sockun));
  sockun.sun_family = AF_UNIX;
  if (sizeof(sockun.sun_path) < strlen(connection) + 1) {
    (void)snprintf(msg_buf, sizeof(msg_buf),
		   "Sockfilename too long: %s, maxlen=%ld\n", connection,
		   (long int)sizeof(sockun.sun_path));
    msg_buf[sizeof(msg_buf) - 1] = '\0';
    error_error(msg_buf, 0);
    exit(EXIT_FAILURE);
  }
  (void)strcpy(sockun.sun_path, connection);

  /*
   * Bind the socket to the path.
   */
  if (bind(sockfd, (struct sockaddr *) &sockun, sizeof(sockun)) < 0)
  {
    (void)snprintf(msg_buf, sizeof(msg_buf),
		   "Could not bind socket to %s", connection);
    msg_buf[sizeof(msg_buf) - 1] = '\0';
    error_fatal(msg_buf, errno);
    exit(EXIT_FAILURE);
  }
  if (listen(sockfd, SOMAXCONN) < 0)
  {
    error_fatal("Could not listen on socket", errno);
    exit(EXIT_FAILURE);
  }

  return sockfd;
}

#endif
