#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <assert.h>
#include <syslog.h>
#include <pwd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/wait.h>

#include "prngd.h"

static int randsavefd = -1;
static int pidfilefd = -1;
char *pidfile = NULL;
int debug = 0;
int fg = 0;

int devnull = -1;

int limit_openfd = 0;
int count_openfd = 0;

static void write_seedfile(void);
static void open_pidfile(const char *pidfilename)
{
  char msg_buf[1024];

  remove(pidfilename);
  pidfilefd = open(pidfilename, O_WRONLY | O_CREAT, 400);
  if (pidfilefd < 0)
  {
    (void)snprintf(msg_buf, sizeof(msg_buf),
	"could not open %s: %s\n", pidfilename, strerror(errno));
    msg_buf[sizeof(msg_buf) - 1] = '\0';
    error_fatal(msg_buf, 0);
    exit(EXIT_FAILURE);
  }
  close_on_exec(pidfilefd, 1);
}


static void open_seedfile(const char *seedfile)
{
  int bytes, bytes_read_total;
  unsigned char buffer[PRNGD_STATE_SIZE];
  char msg_buf[1024];

  randsavefd = open(seedfile, O_RDWR | O_CREAT | O_SYNC | O_RSYNC, 400);
  if (randsavefd < 0)
  {
    (void)snprintf(msg_buf, sizeof(msg_buf),
	"could not open %s: %s\n", seedfile, strerror(errno));
    msg_buf[sizeof(msg_buf) - 1] = '\0';
    error_fatal(msg_buf, 0);
    exit(EXIT_FAILURE);
  }
  close_on_exec(randsavefd, 1);

  /*
   * The contents of the seedfile may be sensitive, since we start seeding
   * from the file.
   */
  if (chmod(seedfile, S_IRUSR | S_IWUSR) < 0)
  {
    (void)snprintf(msg_buf, sizeof(msg_buf),
	"Could not chmod %s: %s\n", seedfile, strerror(errno));
    msg_buf[sizeof(msg_buf) - 1] = '\0';
    error_fatal(msg_buf, 0);
    exit(EXIT_FAILURE);
  }

  /*
   * Do some initial seeding stuff to pre-stir the PRNG.
   */
  seed_internal(NULL);

  /*
   * Now read as much from the file as we can get. 
   */
  bytes_read_total = 0;
  while ((bytes = read(randsavefd, buffer, PRNGD_STATE_SIZE)) > 0)
  {
    rand_add(buffer, bytes, 0.0);
    bytes_read_total += bytes;
  }
  if (bytes < 0)
  {
    (void)snprintf(msg_buf, sizeof(msg_buf),
	"Error reading %s: %s\n", seedfile, strerror(errno));
    msg_buf[sizeof(msg_buf) - 1] = '\0';
    error_fatal(msg_buf, 0);
    exit(EXIT_FAILURE);
  }

  /*
   * Done, let's stir around some more
   */
  seed_internal(NULL);

  if (debug)
    (void)fprintf(stderr, "Read %d bytes\n", bytes_read_total);
  /*
   * Finally, account for the bytes read. We don't know how much entropy
   * was in the bytes we've read, so we only account for the minimum needed
   * and have PRNGD do the rest. If not enough bytes are read, only account
   * the bytes. Don't care for the contents of the buffer, it doesn't hurt
   * to stir the pool.
   */
  if (bytes_read_total > ENTROPY_NEEDED)
    rand_add(buffer, PRNGD_STATE_SIZE, ENTROPY_NEEDED);
  else
    rand_add(buffer, PRNGD_STATE_SIZE, bytes_read_total);

  /*
   * Ok, we don't want to reuse the same seed again, so we will immediately
   * replace the contents of the seed-file.
   */
  write_seedfile();
}

static void write_seedfile(void)
{
  int bytes;
  unsigned char buffer[PRNGD_STATE_SIZE];

  bytes = lseek(randsavefd, 0, SEEK_SET);
  if (bytes < 0)
  {
    error_fatal("Error writing seedfile", errno);
    exit(EXIT_FAILURE);
  }
  bytes = rand_bytes(buffer, PRNGD_STATE_SIZE);
  if (bytes == 0)
  {
    error_error("Info: Random pool not (yet) seeded", 0);
  }
  else
  {
    bytes = write(randsavefd, buffer, PRNGD_STATE_SIZE);
    if (bytes < 0)
    {
      error_fatal("Error writing seedfile", errno);
      exit(EXIT_FAILURE);
    }
  }
  if (debug)
    (void)fprintf(stderr, "Wrote %d bytes back to seedfile\n", bytes);

  bytes = ftruncate(randsavefd, bytes);
}

void close_seedfile(void)
{
  if (randsavefd >= 0) {
    write_seedfile();
    (void)close(randsavefd);
  }
}

static void close_pidfile(void)
{
  if (pidfilefd >= 0) {
    (void)close(pidfilefd);
  }
}


static void killpeer(int service_socket)
{
  unsigned char buffer[256];
  char msg_buf[1024];
  int num;
  pid_t peer_id;

  buffer[0] = 0x04;
  if (write(service_socket, buffer, 1) < 1)
  {
    error_fatal("Cannot contact peer", errno);
    exit(EXIT_FAILURE);
  }
  if (read(service_socket, buffer, 1) < 1)
  {
    error_fatal("Cannot read from peer", errno);
    exit(EXIT_FAILURE);
  }
  num = buffer[0];
  if (read(service_socket, buffer, num) < num)
  {
    error_fatal("Cannot read from peer", errno);
    exit(EXIT_FAILURE);
  }
  buffer[num] = '\0';
  peer_id = (pid_t)atol((char *) buffer);
  (void)fprintf(stderr, "Killing: %ld\n", (long)peer_id);

  if (kill(peer_id, SIGTERM) < 0)
  {
    (void)snprintf(msg_buf, sizeof(msg_buf), "Cannot kill %ld", (long)peer_id);
    msg_buf[sizeof(msg_buf) - 1] = '\0';
    error_fatal(msg_buf, errno);
    exit(EXIT_FAILURE);
  }
  exit(EXIT_SUCCESS);
}

static void usage(const char *progname)
{
  (void)fprintf(stderr, "prngd %s (%s)\n", PRNGD_VERSION, PRNGD_DATE);
  (void)fprintf(stderr,
	  "Usage: %s [options] (/path/to/socket1 | tcp/localhost:port) "
		"[(/path/to/socket2 | tcp/localhost:port)] ... \n"
	  "Options:\n"
	  "\t-f/--fg: do not fork\n"
	  "\t-d/--debug: debugging on\n"
	  "\t-c/--cmdfile cmdpath: use cmdpath for entropy commands [%s]\n"
	  "\t-s/--seedfile seedpath: use seedpath as seedfile [%s]\n"
	  "\t-p/--pidfile pidfilepath: use pidfilepath as pidfile\n"
	  "\t-n/--no-seedfile: no seedfile, keep pool in memory only\n"
	  "\t-m/--mode mode: use mode for sockets [%04o]\n"
	  "\t-k/--kill: kill daemon on other side\n"
	  "\t-v/--version: print version and exit\n",
	  progname, CONFIGFILE, RANDSAVENAME, SOCKETMODE);
}

int main(int argc, char *argv[])
{
  int argp;
  int ret;
  int statval;
  int killmode = 0, killfail = 0;
  time_t seed_stat_interval = SEED_STAT_INTERVAL;
  time_t seed_ext_interval = SEED_EXT_INTERVAL;
  int i, numsock_file = 0, numsock_tcp = 0;
  char **randsock_tcp = NULL;
  char **randsock_file = NULL;
  char *cmdpath = CONFIGFILE;
  char *seedpath = RANDSAVENAME;
  char msg_buf[1024];
  unsigned int socketmode = SOCKETMODE;
  int *service_socket, service_count = 0;
  int kill_socket;
  struct stat randsock_file_stat;
  struct passwd *pwent;
  char *username = "unknown";
  char pidfilebuffer[32];
  entropy_source_t *entropy_source;

  /*
   * Process arguments 
   */
  argp = 1;
  while (argp < argc)
  {
    if (!strcmp(argv[argp], "-v") || !strcmp(argv[argp], "--version")) {
      (void)fprintf(stdout, "prngd %s (%s)\n", PRNGD_VERSION, PRNGD_DATE);
      exit(EXIT_SUCCESS);
    }
    else if (!strcmp(argv[argp], "-d") || !strcmp(argv[argp], "--debug"))
      debug = 1;
    else if (!strcmp(argv[argp], "-f") || !strcmp(argv[argp], "--fg"))
      fg = 1;
    else if ((!strcmp(argv[argp], "-c") || !strcmp(argv[argp], "--cmdfile"))
	     && (argp + 1 < argc))
      cmdpath = argv[++argp];
    else if ((!strcmp(argv[argp], "-s") || !strcmp(argv[argp], "--seedfile"))
	     && (argp + 1 < argc))
      seedpath = argv[++argp];
    else if ((!strcmp(argv[argp], "-p") || !strcmp(argv[argp], "--pidfile"))
	     && (argp + 1 < argc))
      pidfile = argv[++argp];
    else if ((!strcmp(argv[argp], "-m") || !strcmp(argv[argp], "--mode"))
	     && (argp + 1 < argc))
    {
      if (sscanf(argv[++argp], "%o", &socketmode) == 0)
      {
	(void)snprintf(msg_buf, sizeof(msg_buf),
		"Cannot interpret socket mode '%s'\n", argv[argp]);
	msg_buf[sizeof(msg_buf) - 1] = '\0';
	error_fatal(msg_buf, 0);
	exit(EXIT_FAILURE);
      }
    }
    else if (!strcmp(argv[argp], "-n") || !strcmp(argv[argp], "--no-seedfile"))
      seedpath = NULL;
    else if (!strcmp(argv[argp], "-k") || !strcmp(argv[argp], "--kill"))
      killmode = 1;
#ifndef NO_UNIX_SOCKET
    else if (argv[argp][0] == '/') {
      if (numsock_file == 0)
        randsock_file = malloc(sizeof(char *));
      else
	randsock_file = realloc(randsock_file,
				(numsock_file + 1) * sizeof(char *));
      if (randsock_file == NULL) {
	error_fatal("Could not allocate memory", errno);
	exit(EXIT_FAILURE);
      }
      randsock_file[numsock_file++] = argv[argp];
    }
#endif /* NO_UNIX_SOCKET */
    else if (!strncmp(argv[argp], "tcp/localhost:", strlen("tcp/localhost:"))) {
      if (numsock_tcp == 0)
        randsock_tcp = malloc(sizeof(char *));
      else
	randsock_tcp = realloc(randsock_tcp,
				(numsock_tcp + 1) * sizeof(char *));
      if (randsock_tcp == NULL) {
	error_fatal("Could not allocate memory", errno);
	exit(EXIT_FAILURE);
      }
      randsock_tcp[numsock_tcp++] = argv[argp];
    } 
    else
    {
      usage(argv[0]);
      exit(EXIT_FAILURE);
    }
    argp++;
  }
  if ((numsock_file == 0) && (numsock_tcp == 0)) {
    usage(argv[0]);
    error_fatal("No listen socket given, exiting!", 0);
    exit(EXIT_FAILURE);
  }

  if (!killmode)
    parse_configfile(cmdpath, &entropy_source);

  if (debug)
    (void)fprintf(stdout, "Debugging enabled\n");

  /*
   * Obtain the maximum number of open filedescriptors and adjust the
   * counter of actual open files. As always we have open stdin, stdout,
   * and stderr.
   */
  limit_openfd = obtain_limit();
  count_openfd = 3;

  /*
   * Open /dev/null
   */
  devnull = open("/dev/null", O_RDWR);
  if (devnull == -1) {
    error_fatal("Couldn't open /dev/null", errno);
    exit(EXIT_FAILURE);
  }
  count_openfd++;

  /*
   * Now, initialize the PRNG (allocate the memory).
   */
  rand_init(PRNGD_STATE_SIZE, ENTROPY_NEEDED);


  /*
   * Start by reading back the seed saved from former runs, if usage of a
   * seedfile is wanted. 
   */
  if (!killmode && seedpath) {
    open_seedfile(seedpath);
    count_openfd++;
  }
  if (!killmode && pidfile) {
    open_pidfile(pidfile);
    count_openfd++;
  }

  if (!(service_socket = malloc((numsock_file + numsock_tcp) * sizeof(int)))) {
    error_fatal("Could not allocate memory", errno);
    exit(EXIT_FAILURE);
  }

  if (killmode) {
    for (i = 0; i < numsock_file; i++) {

    /*
     * Try to kill daemons on the other side of a unix socket. If we cannot
     * connect to a daemon, we set a flag but continue, since there may be
     * other daemons waiting for kill.
     */
      statval = stat(randsock_file[i], &randsock_file_stat);
      if ((statval < 0) && (errno != ENOENT)) {
        (void)snprintf(msg_buf, sizeof(msg_buf),
		       "Cannot stat socket position %s", randsock_file[i]);
        msg_buf[sizeof(msg_buf) - 1] = '\0';
        error_error(msg_buf, errno);
        killfail = 1;
        continue;
      }
      if ((statval < 0) && (errno == ENOENT)) {
        (void)snprintf(msg_buf, sizeof(msg_buf), "No object at position %s",
        	randsock_file[i]);
        msg_buf[sizeof(msg_buf) - 1] = '\0';
        error_fatal(msg_buf, errno);
        killfail = 1;
        continue;
      }
      if (!IS_UNIX_SOCKET(randsock_file_stat.st_mode)) {
	(void)snprintf(msg_buf, sizeof(msg_buf), "Will not touch %s: no socket",
		randsock_file[i]);
	msg_buf[sizeof(msg_buf) - 1] = '\0';
	error_error(msg_buf, errno);
	killfail = 1;
	continue;
      }

      if ((kill_socket = unix_connect(randsock_file[i], 0, 1)) < 0) {
	    (void)snprintf(msg_buf, sizeof(msg_buf),
	    	"cannot kill peer at %s: no daemon", randsock_file[i]);
	    msg_buf[sizeof(msg_buf) - 1] = '\0';
	    error_error(msg_buf, 0);
	    killfail = 1;
      }
      else {
	killpeer(kill_socket);
	my_shutdown(kill_socket, SHUT_RDWR);
	(void)close(kill_socket);
      }
    }

    for (i = 0; i < numsock_tcp; i++) {
      if ((kill_socket = tcp_connect(randsock_tcp[i], 0, 1)) < 0) {
	(void)snprintf(msg_buf, sizeof(msg_buf),
		       "cannot kill peer at %s: no daemon", randsock_tcp[i]);
	msg_buf[sizeof(msg_buf) - 1] = '\0';
	error_error(msg_buf, 0);
	/*
	 * Do no exit because there may be more work to do!
	 */
      }
      else {
	killpeer(kill_socket);
	my_shutdown(kill_socket, SHUT_RDWR);
	(void)close(kill_socket);
      }
    }
    /*
     * Everything has been done!
     */
    if (killfail)
      exit(EXIT_FAILURE);
    else
      exit(EXIT_SUCCESS);
  }


  /*
   * Set up the UNIX domain sockets to service entropy requests
   */
  for (i = 0; i < numsock_file; i++) {

    /*
     * Check out what is already available at the place of the future
     * egd-socket. If we cannot reach the place to be with "stat()",
     * do not continue, as something may be seriously wrong. If the
     * entry does already exist, remove it, but only if it is of type
     * socket! If removal fails for whatever reason, stop, as something
     * may be seriously wrong.
     */
    statval = stat(randsock_file[i], &randsock_file_stat);
    if ((statval < 0) && (errno != ENOENT)) {
      (void)snprintf(msg_buf, sizeof(msg_buf), "Cannot stat socket position %s",
      	randsock_file[i]);
      msg_buf[sizeof(msg_buf) - 1] = '\0';
      error_fatal(msg_buf, errno);
      exit(EXIT_FAILURE);
    }
    else if (!statval) {
      if (!IS_UNIX_SOCKET(randsock_file_stat.st_mode)) {
	(void)snprintf(msg_buf, sizeof(msg_buf), "Will not touch %s: no socket",
		randsock_file[i]);
	msg_buf[sizeof(msg_buf) - 1] = '\0';
	error_fatal(msg_buf, errno);
	exit(EXIT_FAILURE);
      } else {
	if ((kill_socket = unix_connect(randsock_file[i], 0, 1)) >= 0) {
	  (void)snprintf(msg_buf, sizeof(msg_buf),
		  "socket %s already used by another process, exiting.\n",
		  randsock_file[i]);
	  msg_buf[sizeof(msg_buf) - 1] = '\0';
	  error_fatal(msg_buf, 0);
	  exit(EXIT_FAILURE);
	}

	/*
         * Now there is a socket without service listening on it: we can
	 * remove it.
	 */
	if (unlink(randsock_file[i]) < 0) {
	  (void)snprintf(msg_buf, sizeof(msg_buf),
	  	"Cannot remove already existing entry %s", randsock_file[i]);
	  msg_buf[sizeof(msg_buf) - 1] = '\0';
	  error_fatal(msg_buf, errno);
	  exit(EXIT_FAILURE);
	}
      }
    }

    service_socket[service_count++] = unix_listen(randsock_file[i], 1, 1);
    if (chmod(randsock_file[i], socketmode) < 0) {
      (void)snprintf(msg_buf, sizeof(msg_buf), "Could not chmod socket to %04o",
		     socketmode);
      msg_buf[sizeof(msg_buf) - 1] = '\0';
      error_fatal(msg_buf, errno);
      exit(EXIT_FAILURE);
    }
    count_openfd++;
  }

  /*
   * Set up the TCP sockets to service entropy requests
   */
  for (i = 0; i < numsock_tcp; i++) {
    service_socket[service_count++] = tcp_listen(randsock_tcp[i], 1, 1);
    count_openfd++;
  }


  /*
   * If not in debugging mode, make myself a daemon 
   */
  if (!debug)
  {
    ret = fg ? fg : daemon(0, 0);
    if (ret < 0)
    {
      error_fatal("Could not daemonize", errno);
      exit(EXIT_FAILURE);
    }

    if ((pwent = getpwuid(getuid())) != NULL)
      username = pwent->pw_name;
    /*
     * The setup is finished, and we are daemonized, so we must now switch
     * to syslog...
     */
    (void)snprintf(msg_buf, sizeof(msg_buf),
    	"prngd %s (%s) started up for user %s", PRNGD_VERSION, PRNGD_DATE,
	username);
    msg_buf[sizeof(msg_buf) - 1] = '\0';
    error_notice(msg_buf, 0);
    count_openfd++;
    (void)snprintf(msg_buf, sizeof(msg_buf),
    	"have %d out of %d filedescriptors open", count_openfd, limit_openfd);
    msg_buf[sizeof(msg_buf) - 1] = '\0';
    error_notice(msg_buf, 0);
  }

  /*
   * Maybe we have changed our pid, let's add some more of it. 
   */
  seed_internal(NULL);

  if (!killmode && pidfile) {
    memset(pidfilebuffer, 0, sizeof(pidfilebuffer));
    snprintf(pidfilebuffer, sizeof(pidfilebuffer), "%12ld\n", (long)getpid());
    pidfilebuffer[sizeof(pidfilebuffer) - 1] = '\0';
    if (write(pidfilefd, pidfilebuffer, strlen(pidfilebuffer)) < 0) {
      error_fatal("Error writing pidfile", errno);
      exit(EXIT_FAILURE);
    }
    close_pidfile();
    count_openfd--;
  }

  /* Never returns */
  main_loop(service_socket, service_count, seed_stat_interval,
	    seed_ext_interval, entropy_source, MAX_GATHERER_BYTES);

  /*
   * We never get here, but want to make the compiler happy...
   */
  return(EXIT_SUCCESS);
}
