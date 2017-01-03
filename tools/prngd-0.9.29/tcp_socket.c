/*
 * Handle TCP sockets.
 */
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "prngd.h"

/*
 * Open a connection to the specified peer. Syntax for the connection is:
 * tcp/address:port. Currently exactly tcp/localhost:port is supported.
 * Most errors are considered fatal, that a connection cannot be opened
 * (e.g. since no peer is listening) is not considered fatal.
 */
int tcp_connect(const char *connection, int non_block, int close_exec)
{
  struct sockaddr_in sockin;
  int sockfd, port;
  char msg_buf[1024];

  if (strncmp(connection, "tcp/localhost:", strlen("tcp/localhost:"))) {
    (void)snprintf(msg_buf, sizeof(msg_buf),
		   "Invalid TCP port specification: %s\n", connection);
    msg_buf[sizeof(msg_buf) - 1] = '\0';
    error_fatal(msg_buf, 0);
    exit(EXIT_FAILURE);
  }

  port = atoi(connection + strlen("tcp/localhost:"));
  if (port <= 0) {
    (void)snprintf(msg_buf, sizeof(msg_buf),
		   "Invalid port number %d from %s\n", port, connection);
    msg_buf[sizeof(msg_buf) - 1] = '\0';
    error_fatal(msg_buf, 0);
    exit(EXIT_FAILURE);
  }

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    error_fatal("Could not create socket", errno);
    exit(EXIT_FAILURE);
  }

  /*
   * Bind the socket to the TCP port.
   */
  (void)memset(&sockin, 0, sizeof(sockin));
  sockin.sin_family = AF_INET;
  sockin.sin_port = htons(port);
  sockin.sin_addr.s_addr = inet_addr("127.0.0.1");

  if (connect(sockfd, (struct sockaddr *) &sockin, sizeof(sockin)) < 0) {
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
 * Initiate a listener socket on the specified connection. Syntax for
 * the connection is: tcp/address:port. Currently exactly tcp/localhost:port
 * is supported. All errors are considered fatal.
 */
int tcp_listen(const char *connection, int non_block, int close_exec)
{
  struct sockaddr_in sockin;
  int sockfd, port, on = 1;
  char msg_buf[1024];

  if (strncmp(connection, "tcp/localhost:", strlen("tcp/localhost:"))) {
    (void)snprintf(msg_buf, sizeof(msg_buf),
		   "Invalid TCP port specification: %s\n", connection);
    msg_buf[sizeof(msg_buf) - 1] = '\0';
    error_fatal(msg_buf, 0);
    exit(EXIT_FAILURE);
  }

  port = atoi(connection + strlen("tcp/localhost:"));
  if (port <= 0) {
    (void)snprintf(msg_buf, sizeof(msg_buf),
		   "Invalid port number %d from %s\n", port, connection);
    msg_buf[sizeof(msg_buf) - 1] = '\0';
    error_fatal(msg_buf, 0);
    exit(EXIT_FAILURE);
  }

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0)
  {
    error_fatal("Could not create socket", errno);
    exit(EXIT_FAILURE);
  }
  non_blocking(sockfd, non_block);
  close_on_exec(sockfd, close_exec);

  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on,
		 (SOCKLEN_T) sizeof(on)) < 0) {
    error_fatal("Could not set SO_REUSEADDR", errno);
    exit(EXIT_FAILURE);
  }
  if (setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, &on,
		 (SOCKLEN_T) sizeof(on)) < 0) {
    error_fatal("Could not set SO_KEEPALIVE", errno);
    exit(EXIT_FAILURE);
  }
  /*
   * Bind the socket to the TCP port.
   */
  (void)memset(&sockin, 0, sizeof(sockin));
  sockin.sin_family = AF_INET;
  sockin.sin_port = htons(port);
  sockin.sin_addr.s_addr = inet_addr("127.0.0.1");
  if (bind(sockfd, (struct sockaddr *) &sockin, sizeof(sockin)) < 0)
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
