#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/times.h>
#include <unistd.h>
#include <fcntl.h>
#include <syslog.h>
#include <stdarg.h>

#include "config.h"
#include "prngd_version.h"

#ifdef HAVE_GETRUSAGE
#include <sys/resource.h>
#endif
#ifndef HAVE_DAEMON
int daemon(int nochdir, int noclose);
#endif
#ifndef HAVE_SNPRINTF
int snprintf(char *str, size_t count, const char *fmt, ...);
#endif /* !HAVE_SNPRINTF */
#ifndef HAVE_VSNPRINTF
int vsnprintf(char *str, size_t count, const char *fmt, va_list args);
#endif /* !HAVE_SNPRINTF */

typedef struct {
  double rate;
  unsigned int badness;
  unsigned int sticky_badness;
  char *path;
  char *args[5];
  char *cmdstring;
} entropy_source_t;

typedef struct {
  struct tms tmsbuf;
  struct timeval tp;
  pid_t pid;
#ifdef HAVE_GETRUSAGE
  struct rusage usage;
#endif
  int seed_counter;
} seed_t;

extern int debug;
extern int devnull;
extern int limit_openfd;
extern int count_openfd;

extern char *pidfile;

void parse_configfile(const char *cmdpath, entropy_source_t **entropy_source);

void main_loop(int *service_socket, int numsock, time_t seed_stat_interval,
	       time_t seed_ext_interval, entropy_source_t *entropy_source,
	       int max_gatherer_bytes);

void close_seedfile(void);

void seed_internal(seed_t *seed_p);
void seed_stat(void);

void rand_add(const void *buf, int num, double entropy);
int rand_bytes(unsigned char *buf, int num);
int rand_pool(void);
void rand_init(int set_state_size, int set_entropy_needed);

/* What do we exit with when there is an error? */
#ifndef EXIT_FAILURE
#define EXIT_FAILURE 1
#endif

#define SHA_DIGEST_LENGTH 20
/* The SHA block size and message digest sizes, in bytes */

#define SHA_DATASIZE    64
#define SHA_DATALEN     16
#define SHA_DIGESTSIZE  20
#define SHA_DIGESTLEN    5
/* The structure for storing SHA info */

typedef struct sha1_ctx {
  unsigned int digest[SHA_DIGESTLEN];  /* Message digest */
  unsigned int count_l, count_h;       /* 64-bit block count */
  unsigned char block[SHA_DATASIZE];     /* SHA data buffer */
  int index;                             /* index into buffer */
} SHA1_CTX;

void SHA1_init(SHA1_CTX *ctx);
void SHA1_update(SHA1_CTX *ctx, const unsigned char *buffer, unsigned int len);
void SHA1_final(SHA1_CTX *ctx);
void SHA1_digest(SHA1_CTX *ctx, unsigned char *s);
void SHA1_copy(SHA1_CTX *dest, SHA1_CTX *src);

void close_on_exec(int fd, int on);

void non_blocking(int fd, int on);

ssize_t socket_read(int fd, void *buf, size_t num);
ssize_t socket_write(int fd, void *buf, size_t num);

int obtain_limit(void);

/*
 * error_handlers.c
 */
void* my_malloc(size_t);
int my_pipe(int[2]);
void my_dup2(int, int);
void my_gettimeofday(struct timeval*, void*);
void my_shutdown(int, int);

/*
 * error_log.c
 */
#ifndef SYSLOG_FACILITY
#define SYSLOG_FACILITY LOG_DAEMON
#endif
void intitialise_syslog(void);
void error_fatal(const char*, int);
void error_error(const char*, int);
void error_notice(const char*, int);

/*
 * unix_socket.c
 */
int unix_connect(const char *connection, int non_block, int close_exec);
int unix_listen(const char *connection, int non_block, int close_exec);

/*
 * tcp_socket.c
 */
int tcp_connect(const char *connection, int non_block, int close_exec);
int tcp_listen(const char *connection, int non_block, int close_exec);
