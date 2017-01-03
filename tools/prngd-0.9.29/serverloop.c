#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>

#include "prngd.h"

#ifdef NEED_SYS_SELECT_H
#include <sys/select.h>
#endif /* NEED_SYS_SELECT_H */

static volatile int caught_sigchild = 0;
static volatile int received_sighup = 0;

typedef enum
{
  wait_command, wait_second_byte, wait_third_byte, wait_fourth_byte,
  wait_read, wait_entropy1, wait_entropy2, wait_write, to_be_closed,
  to_be_error_closed
}
querystate_t;

typedef struct
{
  int fd;
  int state;
  int to_process;
  int yet_processed;
  int entropy_bits;
  unsigned char command, secbyte;
  unsigned char buffer[256];
} egd_query_t;


static void termination_handler(int received_signal)
{
  char msg_buf[128];

  if (received_signal != SIGTERM) {
    (void)snprintf(msg_buf, sizeof(msg_buf),
	"Termination handler called with wrong signal: %d != SIGTERM",
	received_signal);
    msg_buf[sizeof(msg_buf) - 1] = '\0';
    error_notice(msg_buf, 0);
  }
  if (debug)
    (void)fprintf(stderr, "Caught SIGTERM, shutting down\n");
  close_seedfile();
  if (pidfile) {
    if (remove(pidfile) == -1) {
      (void)snprintf(msg_buf, sizeof(msg_buf), "Could not remove pidfile %s",
		     pidfile);
      msg_buf[sizeof(msg_buf) - 1] = '\0';
      error_error(msg_buf, errno);
      exit(EXIT_FAILURE);
    }
  }
  exit(EXIT_SUCCESS);
}

static void sighup_handler(int received_signal)
{
  char msg_buf[128];

  if (received_signal != SIGHUP) {
    (void)snprintf(msg_buf, sizeof(msg_buf),
	"HUP handler called with wrong signal: %d != SIGHUP",
	received_signal);
    msg_buf[sizeof(msg_buf) - 1] = '\0';
    error_notice(msg_buf, 0);
  }
  received_sighup = 1;
}

static void child_handler(int received_signal)
{
  char msg_buf[128];

  if (received_signal != SIGCHLD) {
    (void)snprintf(msg_buf, sizeof(msg_buf),
	"Sigchild handler called with wrong signal: %d != SIGCHLD",
	received_signal);
    msg_buf[sizeof(msg_buf) - 1] = '\0';
    error_notice(msg_buf, 0);
  }
  /*
   * Wait for child process to avoid zombies. This signal comes in
   * asynchronously.
   *
   * PRNGD can only have one child at a time, so recording the information
   * could be trivial. However: if a gatherer call is stuck inside a system
   * call, the processes will only die much later, even though "kill -9"
   * has been called after PRNGD's gatherer timeout. Thus, several children
   * may die at the same time. On the other hand, due to process scheduling,
   * a gatherer may die before its PID is recorded. As we must not miss the
   * exit of the actual gatherer process (so that it can be killed after
   * timeout), we only catch SIGCHLD but the evaluation is done by a
   * seperate polling routine that does know about the active gatherer.
   */
  caught_sigchild = 1;
}

static void collect_children(pid_t *gatherer_pid)
{
  pid_t child_pid;
  int gatherer_status;

  while (((child_pid = waitpid(-1, &gatherer_status, WNOHANG)) > 0)
	 || ((child_pid < 0) && (errno == EINTR))) {
    if (child_pid > 0) {
      rand_add(&gatherer_status, sizeof(gatherer_status), 0.0);
      if (WIFEXITED(gatherer_status) || WIFSIGNALED(gatherer_status)) {
        if (debug)
	  (void)fprintf(stderr,
		      "Found child died (pid = %ld, gatherer_pid = %ld)\n",
		      (long)child_pid, (long)*gatherer_pid);
        if (child_pid == *gatherer_pid)
	*gatherer_pid = -1;
      }
    }
  }
}


void main_loop(int *service_socket, int numsock, time_t seed_stat_interval,
	       time_t seed_ext_interval, entropy_source_t *entropy_source,
	       int max_gatherer_bytes)
{
  int i, i1;
  int accept_fd;
  struct sockaddr addr;
  SOCKLEN_T addrlen;
  int num, numbytes, error;
  int pipedes[2];
  time_t last_seed_stat = 0, last_seed_ext = 0;
  int max_query = 0;
  int alloc_query = 0;
  int prev_location = 0;
  egd_query_t *egd_query = NULL;
  egd_query_t *egd_query_old;
  int max_fd;
  int max_query_old;
  pid_t gatherer_pid;
  int gather_fd;
  int gatherer_entry;
  int gatherer_bytes;
  int first_gather = 1;
  int entropy_required;
  double est = 0.0;
  time_t gather_start;
  unsigned char gatherbuf[10240];
  char msg_buf[1024];
  struct sigaction new_action;
  fd_set rfds, wfds, efds;
  struct timeval timeout;
  int saved_errno;

  gatherer_pid = -1;
  gather_fd = -1;
  gatherer_entry = 0;

  gatherer_bytes = 0;		/* Initialize to make gcc happy */
  gather_start = time(NULL);	/* Initialize to make gcc happy */

  /*
   * Set up the signal handling. TERM signals shall lead to clean
   * shutdown of prngd (save the seed and exit).
   * HUP signals shall lead to a re-read of the config file.
   * SIGPIPE might occur when a peer shuts down the connection. Of course,
   * we don't want prngd to be affected in any way (it should simply close
   * the connection), so SIGPIPE shall be ignored.
   */
  (void)memset(&new_action, 0, sizeof(struct sigaction));
  new_action.sa_handler = termination_handler;
  (void)sigemptyset(&new_action.sa_mask);
  new_action.sa_flags |= SA_RESTART;
  (void)sigaction(SIGTERM, &new_action, NULL);

  (void)memset(&new_action, 0, sizeof(struct sigaction));
  new_action.sa_handler = sighup_handler;
  (void)sigemptyset(&new_action.sa_mask);
  new_action.sa_flags |= SA_RESTART;
  (void)sigaction(SIGHUP, &new_action, NULL);

  (void)memset(&new_action, 0, sizeof(struct sigaction));
  new_action.sa_handler = child_handler;
  (void)sigemptyset(&new_action.sa_mask);
  new_action.sa_flags |= SA_RESTART;
  (void)sigaction(SIGCHLD, &new_action, NULL);

  (void)memset(&new_action, 0, sizeof(struct sigaction));
  new_action.sa_handler = SIG_IGN;
  (void)sigemptyset(&new_action.sa_mask);
  new_action.sa_flags |= SA_RESTART;
  (void)sigaction(SIGPIPE, &new_action, NULL);

  for (;;)
  {
    FD_ZERO(&rfds);
    FD_ZERO(&wfds);
    FD_ZERO(&efds);
    max_fd = 0;
    /*
     * Do not arm select for the listener socket if we are not ready
     * to accept another connection.
     */
    if (count_openfd < limit_openfd - OPEN_FD_RESERVE) {
      for (i = 0; i < numsock; i++)
      {
        FD_SET(service_socket[i], &rfds);
        if (service_socket[i] > max_fd)
	max_fd = service_socket[i];
      }
    }
    if (gather_fd != -1)
    {
      FD_SET(gather_fd, &rfds);
#ifndef SCO3
      FD_SET(gather_fd, &efds);
#endif
      if (gather_fd > max_fd)
	max_fd = gather_fd;
    }
    entropy_required = 0;
    for (i = 0; i < max_query; i++)
    {
      if (egd_query[i].fd > max_fd)
	max_fd = egd_query[i].fd;
      FD_SET(egd_query[i].fd, &efds);
      if ((egd_query[i].state == wait_command) ||
	  (egd_query[i].state == wait_second_byte) ||
	  (egd_query[i].state == wait_third_byte) ||
	  (egd_query[i].state == wait_fourth_byte) ||
	  (egd_query[i].state == wait_read))
	FD_SET(egd_query[i].fd, &rfds);
      else if (egd_query[i].state == wait_write)
	FD_SET(egd_query[i].fd, &wfds);
      else if ((egd_query[i].state == wait_entropy1) ||
	       (egd_query[i].state == wait_entropy2))
        entropy_required = 1;
    }

    /*
     * If we just entered the loop and gather for the first time, we
     * don't want to wait. We also want to refill our pool when drained.
     * If there are connections waiting for random numbers to be generated,
     * go ahead.
     */
    if (first_gather || (rand_pool() < (ENTROPY_THRESHOLD)) ||
        (entropy_required && (rand_pool() > 0)))
    {
      timeout.tv_sec = 0;
      timeout.tv_usec = 1;
    }
    else {
      timeout.tv_sec = (seed_stat_interval < seed_ext_interval ?
			 seed_stat_interval : seed_ext_interval);
      timeout.tv_usec = 0;
      if ((gatherer_pid > 0) && (gather_fd < 0))
	timeout.tv_sec = 1;	/* if we are still waiting for a child, don't
				   sit long in select() */
    }
    num = select(max_fd + 1, &rfds, &wfds, &efds, &timeout);
    saved_errno = errno;

    collect_children(&gatherer_pid);

    if (received_sighup) {
      received_sighup = 0;
      if (debug)
	(void)fprintf(stderr, "Received SIGHUP\n");
    }

    if (time(0) >= last_seed_stat + seed_stat_interval) {
      seed_stat();
      last_seed_stat = time(0);
    }

    seed_internal(NULL);

    if (num < 0) {
      if (saved_errno == EINTR)
        continue;		/* Ignore rfds and friends, since invalid */
      else {
	error_fatal("select() failed", saved_errno);
	exit(EXIT_FAILURE);
      }
    }

    /*
     * First thing to do: check, whether new connections are waiting.
     * The queue on the listen() socket is of limited length, so
     * querying processes might run into "connection refused" errors,
     * when the queuelength is exceeded.
     */
    max_query_old = max_query;
    for (i = 0; i < numsock; i++)
    {
      if (count_openfd >= limit_openfd - OPEN_FD_RESERVE) {
	(void)snprintf(msg_buf, sizeof(msg_buf),
		"could not accept connection: no more slots (%d of %d)\n",
		count_openfd, limit_openfd - OPEN_FD_RESERVE);
	msg_buf[sizeof(msg_buf) - 1] = '\0';
	error_notice(msg_buf, 0);
	continue;	/* do not accept new connection */
      }
      if (FD_ISSET(service_socket[i], &rfds))
      {
        addrlen = (SOCKLEN_T) sizeof(addr);
        accept_fd = accept(service_socket[i], &addr, &addrlen);
        if (accept_fd < 0)
	  continue;		/*
				 * Ignore and stay alive 
				 */

	count_openfd++;
	non_blocking(accept_fd, 1);
	close_on_exec(accept_fd, 1);

        /*
         * Create a new structure for the incoming request 
         */
        if (max_query + 1 > alloc_query)
        {
	  egd_query_old = egd_query;
	  if (alloc_query == 0)
	    egd_query = malloc((max_query + 1) * sizeof(egd_query_t));
	  else
	    egd_query =
	        realloc(egd_query_old,
		        (max_query + 1) * sizeof(egd_query_t));
	  if (!egd_query)
	  {
	    error_error("realloc() failed", errno);
	    (void)close(accept_fd);	/*
				 * Better this than complete shut down 
				 */
	    count_openfd--;
	    egd_query = egd_query_old;
	    continue;
	  }
	  alloc_query = max_query + 1;
        }
        /*
         * This routine is safe here, as in case of error we do "continue;"
         * in case of problems. So we have succesfully reached this point.
         */
        max_query++;
        egd_query[max_query - 1].fd = accept_fd;
        egd_query[max_query - 1].state = wait_command;
	if (debug) {
          (void)fprintf(stderr, "Opening service fd %d\n",
	  	egd_query[max_query-1].fd);
	}
	/*
	 * Finally, mix information about the connection into the pool.
	 * Maybe its coming from TCP and has some foreign port number
	 * in it. Only count it as '0', because maybe there is no
	 * entropy in it at all.
	 */
	rand_add(&addr, addrlen, 0.0);
      }
    }


    if (gather_fd != -1) {
      if (FD_ISSET(gather_fd, &rfds)) {
	/*
	 * Bytes are coming back from the gatherer process. Read chunks of
	 * up to 10240 bytes and feed them into the PRNG
	 */
	numbytes = socket_read(gather_fd, gatherbuf, 10240);
	rand_add(gatherbuf, numbytes, est * numbytes);
	if (debug)
	  (void)fprintf(stderr, "Read %d bytes from gatherer\n", numbytes);
	gatherer_bytes += numbytes;
	if ((numbytes <= 0) || (gatherer_bytes >= max_gatherer_bytes)) {
	  if (debug)
	    (void)fprintf(stderr, "Closing connection to gatherer\n");
	  (void)close(gather_fd);
	  gather_fd = -1;
	  count_openfd--;
	}
      }
      else if (FD_ISSET(gather_fd, &efds)) {
	if (debug)
	  (void)fprintf(stderr, "Closing connection to gatherer (efds)\n");
	(void)close(gather_fd);
	gather_fd = -1;
	count_openfd--;
      }
      if (gather_fd == -1) {
	/*
	 * The formerly running gatherer has just been shut down for normal
	 * or error conditions. Choose the next one.
	 */
	gatherer_entry++;
        if (entropy_source[gatherer_entry].path == NULL) {
	  first_gather = 0;	/* done */
	  gatherer_entry = 0;
	}
      }
    }

    collect_children(&gatherer_pid);

    if ((gatherer_pid > 0) &&
	(difftime(time(NULL), gather_start) > seed_ext_interval / 2 + 1)) {
      if (debug)
	(void)fprintf(stderr, "Gathering time exceeded, kill process %ld\n",
		      (long)gatherer_pid);
      if (kill(gatherer_pid, SIGKILL) == -1) {
      	error_error("kill() failed", errno);
      }
      gatherer_pid = -1;
      if (gather_fd != -1) {
	(void)close(gather_fd);
	gather_fd = -1;
	count_openfd--;
	gatherer_entry++;
	if (entropy_source[gatherer_entry].path == NULL) {
	  first_gather = 0;	/* done */
	  gatherer_entry = 0;
	}
      }
    }

    collect_children(&gatherer_pid);

    /*
     * Call a gatherer, if it is time to do so. When starting up, call
     * all gatherers once to obtain initial seeding.
     */
    if (((time(0) >= last_seed_ext + seed_ext_interval) || first_gather ||
	(rand_pool() < (ENTROPY_THRESHOLD))) &&
	(gatherer_pid == -1) && (gather_fd == -1))
    {
      last_seed_ext = time(0);
      /*
       * time to call another gatherer 
       */

      /*
       * Select the next gatherer to be used
       */
      if ((gatherer_entry == 0) &&
		(entropy_source[gatherer_entry].path == NULL)) {
        first_gather = 0;	/* no gatherer available */
        continue;
      }

      if (my_pipe(pipedes) < 0)
	continue;		/*
				 * We cannot do anything about it 
				 */

      est = entropy_source[gatherer_entry].rate; /* for later use */
      gatherer_bytes = 0;

      gatherer_pid = fork();
      if (gatherer_pid == -1) {
	error_error("Could not fork", errno);
	(void)close(pipedes[0]);
	(void)close(pipedes[1]);
	continue;
      }
      if (gatherer_pid == 0) {
	/*
         * Child
	 */
	my_dup2(devnull, STDIN_FILENO);
        my_dup2(pipedes[1], STDOUT_FILENO);
        my_dup2(pipedes[1], STDERR_FILENO);
        (void)close(pipedes[0]);
        (void)close(pipedes[1]);
        (void)close(devnull); 
	if (execl(entropy_source[gatherer_entry].path,
	      entropy_source[gatherer_entry].args[0],
	      entropy_source[gatherer_entry].args[1],
	      entropy_source[gatherer_entry].args[2],
	      entropy_source[gatherer_entry].args[3],
	      entropy_source[gatherer_entry].args[4],
	      NULL) == -1)
	{
		(void)snprintf(msg_buf, sizeof(msg_buf),
			"Failed to execl(%s, %s, %s, %s, %s, %s)",
			entropy_source[gatherer_entry].path,
			entropy_source[gatherer_entry].args[0] ?
			entropy_source[gatherer_entry].args[0] : "",
			entropy_source[gatherer_entry].args[1] ?
			entropy_source[gatherer_entry].args[1] : "",
			entropy_source[gatherer_entry].args[2] ?
			entropy_source[gatherer_entry].args[2] : "",
			entropy_source[gatherer_entry].args[3] ?
			entropy_source[gatherer_entry].args[3] : "",
			entropy_source[gatherer_entry].args[4] ?
			entropy_source[gatherer_entry].args[4] : "");
		msg_buf[sizeof(msg_buf) - 1] = '\0';
		error_fatal(msg_buf, errno);
		_exit(EXIT_FAILURE);
	}
      }
      else {
	/*
	 * Parent
	 */
	(void)close(pipedes[1]);
	gather_fd = pipedes[0];
	count_openfd++;
	non_blocking(gather_fd, 1);
	gather_start = time(NULL);
	if (debug)
	{
	  (void)fprintf(stderr, "Spawned gatherer (pid = %ld): %s (%s",
		(long)gatherer_pid,
		entropy_source[gatherer_entry].path,
		entropy_source[gatherer_entry].args[0]);
	  i = 1;
	  while ((i < 5) && (entropy_source[gatherer_entry].args[i])) {
	    (void)fprintf(stderr, " %s",
	    	entropy_source[gatherer_entry].args[i++]);
	  }
	  (void)fprintf(stderr, ")\n");
	}
	collect_children(&gatherer_pid);
      }
      continue;

    }
    /*
     * Finally, serve the active sockets. First step: check errors reported
     * and mark the corresponding descriptor "to_be_error_closed". Perform
     * the loop only over the sockets already open, as the data returned
     * from select() obviously cannot make sense for the connections
     * freshly opened.
     * Service is in the sequence the fds were opened. The generation of
     * random numbers is performed in a seperate loop later on, as this
     * takes some time. Service all available I/O operations first.
     */
    for (i = 0; i < max_query_old; i++)
    {
      if (FD_ISSET(egd_query[i].fd, &efds))
      {
	if (debug)
	  (void)fprintf(stderr, "Error flagged for service fd %d\n",
	  	egd_query[i].fd);
	egd_query[i].state = to_be_error_closed;
      }
      else if (FD_ISSET(egd_query[i].fd, &rfds) ||
	       FD_ISSET(egd_query[i].fd, &wfds)) {
      switch (egd_query[i].state)
      {
      case wait_command:
	error = socket_read(egd_query[i].fd, &(egd_query[i].command), 1);
	if (error == -1)
	  egd_query[i].state = to_be_error_closed;
	else if (error != 1)
	  egd_query[i].state = to_be_closed;
	else if (egd_query[i].command > 4)
	  egd_query[i].state = to_be_error_closed;
	else if (egd_query[i].command == 0)
	{
	  numbytes = rand_pool();
	  egd_query[i].buffer[0] = numbytes / 16777216;
	  egd_query[i].buffer[1] = (numbytes % 16777216) / 65536;
	  egd_query[i].buffer[2] = (numbytes % 65536) / 256;
	  egd_query[i].buffer[3] = numbytes % 256;
	  egd_query[i].to_process = 4;
	  egd_query[i].yet_processed = 0;
	  egd_query[i].state = wait_write;
	}
	else if ((egd_query[i].command == 1) ||
		 (egd_query[i].command == 2) ||
		 (egd_query[i].command == 3))
	  egd_query[i].state = wait_second_byte;
	else if (egd_query[i].command == 4)
	{
	  egd_query[i].buffer[0] = 12;
	  (void)snprintf((char *) (egd_query[i].buffer + 1), 
			 sizeof(egd_query[i].buffer) - 1, "%12ld",
			 (long)getpid());
	  egd_query[i].buffer[sizeof(egd_query[i].buffer) - 1] = '\0';
	  egd_query[i].to_process = 13;
	  egd_query[i].yet_processed = 0;
	  egd_query[i].state = wait_write;
	}
	break;
      case wait_second_byte:
	error = socket_read(egd_query[i].fd, &(egd_query[i].secbyte), 1);
	if (error != 1)
	  egd_query[i].state = to_be_error_closed;
	else if (egd_query[i].command == 1)
	{
	  seed_internal(NULL);	/* Mix in just before query */
	  seed_stat();
	  if (rand_pool() > 0) {
	    egd_query[i].buffer[0] = (char)egd_query[i].secbyte;
	    egd_query[i].to_process = egd_query[i].secbyte + 1;
            egd_query[i].yet_processed = 0;
            egd_query[i].state = wait_entropy1;
	  }
	  else {
	    numbytes = 0;
	    egd_query[i].buffer[0] = (char)0;
	    egd_query[i].to_process =  1;
	    egd_query[i].yet_processed = 0;
	    egd_query[i].state = wait_write;
	  }
	}
	else if (egd_query[i].command == 2)
	{
	  seed_internal(NULL);	/* Mix in just before query */
	  seed_stat();
	  egd_query[i].to_process = egd_query[i].secbyte;
	  egd_query[i].yet_processed = 0;
	  egd_query[i].state = wait_entropy2;
	}
	else if (egd_query[i].command == 3)
	{
	  egd_query[i].entropy_bits = egd_query[i].secbyte * 256;
	  egd_query[i].state = wait_third_byte;
	}
	break;
      case wait_third_byte:
	error = socket_read(egd_query[i].fd, &(egd_query[i].secbyte), 1);
	if (error != 1)
	  egd_query[i].state = to_be_error_closed;
	else
	{
	  egd_query[i].entropy_bits += egd_query[i].secbyte;
	  egd_query[i].state = wait_fourth_byte;
	}
	break;
      case wait_fourth_byte:
	error = socket_read(egd_query[i].fd, &(egd_query[i].secbyte), 1);
	if (error != 1)
	  egd_query[i].state = to_be_error_closed;
	else
	{
	  egd_query[i].to_process = egd_query[i].secbyte;
	  egd_query[i].yet_processed = 0;
	  egd_query[i].state = wait_read;
	}
	break;
      case wait_read:
	num = egd_query[i].to_process - egd_query[i].yet_processed;
	error = socket_read(egd_query[i].fd, egd_query[i].buffer, num);
	if (error <= 0)
	  egd_query[i].state = to_be_error_closed;
	else
	{
	  rand_add(egd_query[i].buffer, error,
		   (double) error * egd_query[i].entropy_bits / BITS_PER_BYTE /
		   egd_query[i].to_process);
	  egd_query[i].yet_processed += error;
	  if (egd_query[i].yet_processed >= egd_query[i].to_process)
	    egd_query[i].state = wait_command;
	}
	break;
      case wait_write:
	error = socket_write(egd_query[i].fd,
		      &(egd_query[i].buffer[egd_query[i].yet_processed]),
		      egd_query[i].to_process -
		      egd_query[i].yet_processed);
	if (error <= 0)
	  egd_query[i].state = to_be_error_closed;
	else
	{
	  egd_query[i].yet_processed += error;
	  if (egd_query[i].yet_processed == egd_query[i].to_process) {
	    egd_query[i].state = wait_command;
	    (void)fprintf(stderr, "Sent %d bytes to service %d\n",
			  egd_query[i].to_process, egd_query[i].fd);
	  }
	}
	break;
      default:
	break;
      }
      }
    }

    /*
     * Now retrieve entropy for those processes waiting for it.
     * Only retrieve one chunk of entropy, such that we don't hang on
     * here too long. For fairness, we record the location of the last
     * query-connection, so that we can continue with the next one.
     *
     * Calls to rand_bytes() need not be checked for failure, as the
     * rand_pool() check in the loop condition is present.
     */
    for (i = 0; (i < max_query_old) && (rand_pool() > 0); i++) {
      i1 = (prev_location + i) % max_query_old;
      if (egd_query[i1].state == wait_entropy1) {
	seed_internal(NULL);	/* Mix in just before query */
	seed_stat();
	rand_bytes(egd_query[i1].buffer + 1, egd_query[i1].secbyte);
	egd_query[i1].state = wait_write;
	prev_location = (i1 + 1) % max_query_old;
	break;
      }
      if (egd_query[i1].state == wait_entropy2) {
	seed_internal(NULL);	/* Mix in just before query */
	seed_stat();
	rand_bytes(egd_query[i1].buffer, egd_query[i1].secbyte);
	egd_query[i1].state = wait_write;
	prev_location = (i1 + 1) % max_query_old;
	break;
      }
    }

    /*
     * Perform the final cleanup
     */
    for (i = 0; i < max_query; i++) {
      if ((egd_query[i].state == to_be_closed) ||
	  (egd_query[i].state == to_be_error_closed)) {
	if (debug)
	  (void)fprintf(stderr, "Closing service fd %d, error=%d\n",
			  egd_query[i].fd,
			  (egd_query[i].state == to_be_error_closed));
	else if (egd_query[i].state == to_be_error_closed) {
	  (void)snprintf(msg_buf, sizeof(msg_buf),
	  	"closing service fd %d for error.", egd_query[i].fd);
	  msg_buf[sizeof(msg_buf) - 1] = '\0';
	  error_notice(msg_buf, 0);
	}
	(void)close(egd_query[i].fd);
	count_openfd--;
	/* Remove delivered entropy */
	(void)memset(egd_query[i].buffer, 0, 256);
	if (i < prev_location)
	  prev_location--;	/* No need to wrap around, we only have to care
				 * about fairness.
				 */
	if (i < max_query - 1)
	{
	  (void)memmove(egd_query + i, egd_query + max_query - 1,
		  sizeof(egd_query_t));
	  i--;
	}
	max_query--;
      }
    }


    collect_children(&gatherer_pid);

  }

}
