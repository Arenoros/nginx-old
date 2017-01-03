
#ifdef NCR_MP_RAS2
typedef int ssize_t;
#define HAVE_GETRUSAGE
#define O_RSYNC		0
#define SOCKLEN_T	int
#define PATH_TMP	"/tmp"
#define PATH_VAR_TMP	"/var/tmp"
#define PATH_PASSWD	"/etc/passwd"
#define PATH_WTMP	"/var/adm/wtmp"
#define PATH_UTMP	"/var/adm/utmp"
#define PATH_SYSLOG	"/etc/.osm"
#define PATH_MAILLOG	"/etc/.osm"
#define rlimit ucb_rlimit
#include <../ucbinclude/sys/resource.h>
#undef rlimit
#define _SYS_RESOURCE_H
#endif

#ifdef NCR_MP_RAS3
#define HAVE_DAEMON
#define HAVE_GETRUSAGE
#define O_RSYNC		0
#define SOCKLEN_T	size_t
#define PATH_TMP	"/tmp"
#define PATH_VAR_TMP	"/var/tmp"
#define PATH_PASSWD	"/etc/passwd"
#define PATH_WTMP	"/var/adm/wtmp"
#define PATH_UTMP	"/var/adm/utmp"
#define PATH_SYSLOG	"/etc/.osm"
#define PATH_MAILLOG	"/etc/.osm"
#endif

#ifdef UNIXWARE1
#define DONT_CHMOD_SOCKET
#define O_RSYNC		0
#define SOCKLEN_T       int
#define PATH_TMP        "/tmp"
#define PATH_VAR_TMP    "/var/tmp"
#define PATH_PASSWD     "/etc/passwd"
#define PATH_WTMP       "/var/adm/wtmpx"
#define PATH_UTMP       "/var/adm/utmpx"
#define PATH_SYSLOG	"/etc/.osm"
#define PATH_MAILLOG	"/var/mail/:log"
#endif

#ifdef LINUX2
#define HAVE_DAEMON
#define HAVE_GETRUSAGE
#define HAVE_SNPRINTF
#define HAVE_VSNPRINTF
#define SOCKLEN_T	socklen_t
#define PATH_TMP	"/tmp"
#define PATH_VAR_TMP	"/var/tmp"
#define PATH_PASSWD	"/etc/passwd"
#define PATH_WTMP	"/var/log/wtmp"
#define PATH_UTMP	"/var/run/utmp"
#define PATH_SYSLOG	"/var/log/messages"
#define PATH_MAILLOG	"/var/log/mail"
#endif

#if defined(HPUX10) || defined(HPUX11)
#define HAVE_GETRUSAGE
#ifdef HPUX11
#define HAVE_SNPRINTF
#define HAVE_VSNPRINTF
#endif
#define SOCKLEN_T	int
#define PATH_TMP	"/tmp"
#define PATH_VAR_TMP	"/var/tmp"
#define PATH_PASSWD	"/etc/passwd"
#define PATH_WTMP	"/var/adm/wtmp"
#define PATH_UTMP	"/etc/utmp"
#define PATH_SYSLOG	"/var/adm/syslog/syslog.log"
#define PATH_MAILLOG	"/var/adm/syslog/mail.log"
#endif

#ifdef TRU64
#define HAVE_GETRUSAGE
#define HAVE_SNPRINTF
#define HAVE_VSNPRINTF
#define SOCKLEN_T	size_t
#define PATH_TMP	"/tmp"
#define PATH_VAR_TMP	"/var/tmp"
#define PATH_PASSWD	"/etc/passwd"
#define PATH_WTMP	"/var/adm/wtmp"
#define PATH_UTMP	"/var/adm/utmp"
#define PATH_SYSLOG	"/var/adm/syslog.dated/current/daemon.log"
#define PATH_MAILLOG	"/var/adm/syslog.dated/current/mail.log"
#endif

#ifdef OSF1
#define HAVE_GETRUSAGE
#define SOCKLEN_T	size_t
#define PATH_TMP	"/tmp"
#define PATH_VAR_TMP	"/var/tmp"
#define PATH_PASSWD	"/etc/passwd"
#define PATH_WTMP	"/var/adm/wtmp"
#define PATH_UTMP	"/var/adm/utmp"
#define PATH_SYSLOG	"/var/adm/syslog.dated/current/daemon.log"
#define PATH_MAILLOG	"/var/adm/syslog.dated/current/mail.log"
#endif

#ifdef SOLARIS
#define HAVE_SNPRINTF
#define HAVE_VSNPRINTF
#define HAVE_GETRUSAGE
#define SOCKLEN_T       socklen_t
#define PATH_TMP        "/tmp"
#define PATH_VAR_TMP    "/var/tmp"
#define PATH_PASSWD     "/etc/passwd"
#define PATH_WTMP       "/var/adm/wtmpx"
#define PATH_UTMP       "/var/adm/utmpx"
#define PATH_SYSLOG     "/var/adm/messages"
#endif

#ifdef SOLARIS26
#define HAVE_SNPRINTF
#define HAVE_VSNPRINTF
#define HAVE_GETRUSAGE
#define SOCKLEN_T       size_t
#define PATH_TMP        "/tmp"
#define PATH_VAR_TMP    "/var/tmp"
#define PATH_PASSWD     "/etc/passwd"
#define PATH_WTMP       "/var/adm/wtmpx"
#define PATH_UTMP       "/var/adm/utmpx"
#define PATH_SYSLOG     "/var/adm/messages"
#endif

#ifdef SOLARIS_251
#define HAVE_GETRUSAGE
#define SOCKLEN_T       int
#define PATH_TMP        "/tmp"
#define PATH_VAR_TMP    "/var/tmp"
#define PATH_PASSWD     "/etc/passwd"
#define PATH_WTMP       "/var/adm/wtmpx"
#define PATH_UTMP       "/var/adm/utmpx"
#define PATH_SYSLOG     "/var/adm/messages"
#define IS_UNIX_SOCKET(m)  (S_ISSOCK(m) || S_ISFIFO(m))
#include <sys/resource.h>
int getrusage(int who, struct rusage *rusage);
#endif

#ifdef SOLARIS_OLD
#define HAVE_GETRUSAGE
#define SOCKLEN_T       int
#define PATH_TMP        "/tmp"
#define PATH_VAR_TMP    "/var/tmp"
#define PATH_PASSWD     "/etc/passwd"
#define PATH_WTMP       "/var/adm/wtmp"
#define PATH_UTMP       "/var/adm/utmp"
#define PATH_SYSLOG     "/var/adm/messages"
#endif

#ifdef MACOSX
#define HAVE_GETRUSAGE
/* not supported */
#define O_RSYNC 0
#define O_SYNC 0
/* in ansi string.h but undefined in -posix */
#define bzero(b,len) memset(b,0,len)
#ifndef FD_CLOEXEC
#define FD_CLOEXEC	1
#endif
#define PATH_TMP        "/private/tmp"
#define PATH_PASSWD     "/etc/passwd"
#define PATH_WTMP       "/private/var/log/wtmp"
#define PATH_UTMP       "/private/var/run/utmp"
#define PATH_SYSLOG     "/private/var/log/system.log"
#endif

#ifdef FREEBSD
#define HAVE_GETRUSAGE
/* not yet supported */
#define O_RSYNC	0
#ifndef O_SYNC
#define O_SYNC	O_FSYNC
#define PATH_TMP        "/tmp"
#define PATH_VAR_TMP    "/var/tmp"
#define PATH_PASSWD     "/etc/passwd"
#define PATH_WTMP       "/var/log/wtmp"
#define PATH_UTMP       "/var/run/utmp"
#define PATH_SYSLOG     "/var/log/messages"
#endif

#endif

#ifdef NEXTSTEP
#define HAVE_GETRUSAGE
/* not supported */
#define SA_RESTART 0
#define O_RSYNC 0
/* replace with corresponding bsd call */
#define setsid() setpgrp(0, getpid())
/* in ansi string.h but undefined in -posix */
#define bzero(b,len) memset(b,0,len)
#ifndef FD_CLOEXEC
#define FD_CLOEXEC	1
#endif
#define PATH_TMP        "/private/tmp"
#define PATH_PASSWD     "/etc/passwd"
#define PATH_WTMP       "/private/adm/wtmp"
#define PATH_UTMP       "/private/etc/utmp"
#define PATH_SYSLOG     "/private/adm/messages"
#endif

#ifdef IRIX62
#define HAVE_GETRUSAGE
#define SOCKLEN_T       int
#define PATH_TMP        "/tmp"
#define PATH_VAR_TMP	"/var/tmp"
#define PATH_PASSWD     "/etc/passwd"
#define PATH_WTMP       "/var/adm/wtmpx"
#define PATH_UTMP       "/var/adm/utmpx"
#define PATH_SYSLOG     "/var/adm/SYSLOG"
#endif

#ifdef IRIX65
#define HAVE_GETRUSAGE
#define HAVE_SNPRINTF
#define HAVE_VSNPRINTF
#define SOCKLEN_T       int
#define PATH_TMP        "/tmp"
#define PATH_VAR_TMP	"/var/tmp"
#define PATH_PASSWD     "/etc/passwd"
#define PATH_WTMP       "/var/adm/wtmpx"
#define PATH_UTMP       "/var/adm/utmpx"
#define PATH_SYSLOG     "/var/adm/SYSLOG"
#endif

#ifdef AIX32
extern void closelog();
#define HAVE_GETRUSAGE
#define NEED_SYS_SELECT_H
#define SOCKLEN_T       int
#define O_RSYNC         0
#define PATH_TMP        "/tmp"
#define PATH_VAR_TMP    "/var/tmp"
#define PATH_PASSWD     "/etc/passwd"
#define PATH_WTMP       "/var/adm/wtmp"
#define PATH_UTMP       "/etc/utmp"
#define PATH_SYSLOG     "/var/spool/mqueue/syslog"
#endif

#ifdef AIX43
#define HAVE_GETRUSAGE
#define SOCKLEN_T	socklen_t
#define PATH_TMP	"/tmp"
#define PATH_VAR_TMP	"/var/tmp"
#define PATH_PASSWD	"/etc/passwd"
#define PATH_WTMP	"/var/adm/wtmp"
#define PATH_UTMP	"/etc/utmp"
#define PATH_SYSLOG	"/var/adm/messages"
#define PATH_MAILLOG	"/var/spool/mqueue/syslog"
#endif

#ifdef AIX41
#define HAVE_GETRUSAGE
#define NEED_SYS_SELECT_H
#define SOCKLEN_T	int
#define O_RSYNC		0
#define PATH_TMP	"/tmp"
#define PATH_VAR_TMP	"/var/tmp"
#define PATH_PASSWD	"/etc/passwd"
#define PATH_WTMP	"/var/adm/wtmp"
#define PATH_UTMP	"/etc/utmp"
#define PATH_SYSLOG	"/var/adm/messages"
#define PATH_MAILLOG	"/var/spool/mqueue/syslog"
#endif

#ifdef AIX51
#define HAVE_GETRUSAGE
#define SOCKLEN_T     socklen_t
#define PATH_TMP      "/tmp"
#define PATH_VAR_TMP  "/var/tmp"
#define PATH_PASSWD   "/etc/passwd"
#define PATH_WTMP     "/var/adm/wtmp"
#define PATH_UTMP     "/etc/utmp"
#define PATH_SYSLOG   "/var/log/messages"
#endif

#if defined(ULTRIX)
typedef int     ssize_t;
extern void closelog(void);
#include <sys/syslog.h>
#define HAVE_GETRUSAGE
#define SOCKLEN_T       int
#define SA_RESTART      0
#define O_RSYNC         0
#define PATH_TMP        "/tmp"
#define PATH_VAR_TMP    "/var/tmp"
#define PATH_PASSWD     "/etc/passwd"
#define PATH_WTMP       "/var/adm/wtmp"
#define PATH_UTMP       "/etc/utmp"
#define PATH_SYSLOG     "/var/spool/mqueue/syslog"
#endif

#ifdef QNX
//typedef int     ssize_t;
extern void closelog(void);
#include <syslog.h>
#include <sys/types.h>
#include <sys/socket.h>
#define HAVE_GETRUSAGE
#define SOCKLEN_T       int
#define SA_RESTART      0
#define PATH_TMP        "/tmp"
#define PATH_VAR_TMP    "/var/tmp"
#define PATH_PASSWD     "/etc/passwd"
#define PATH_WTMP       "/var/log/wtmp"
#define PATH_UTMP       "/var/log/utmp"
#define PATH_SYSLOG     "/dev/slog"
#endif

#ifdef UNIXWARE7
#define HAVE_GETRUSAGE
#define HAVE_SNPRINTF
#define HAVE_VSNPRINTF
#define SOCKLEN_T	size_t
#define O_RSYNC		0
#define PATH_TMP        "/tmp"
#define PATH_VAR_TMP    "/var/tmp"
#define PATH_PASSWD     "/etc/passwd"
#define PATH_WTMP       "/var/adm/wtmp"
#define PATH_UTMP       "/etc/utmp"
#define PATH_SYSLOG     "/var/adm/syslog"
/*
 * Strange thing on Unixware 7, sockets manifest itself as FIFOs
 * John Hughes <john@Calva.COM>
 */
#define IS_UNIX_SOCKET(m)  (S_ISSOCK(m) || S_ISFIFO(m))
#endif

#ifdef OpenUNIX8
#define HAVE_GETRUSAGE
#define HAVE_SNPRINTF
#define HAVE_VSNPRINTF
#define SOCKLEN_T	size_t
#define O_RSYNC		0
#define PATH_TMP        "/tmp"
#define PATH_VAR_TMP    "/var/tmp"
#define PATH_PASSWD     "/etc/passwd"
#define PATH_WTMP       "/var/adm/wtmp"
#define PATH_UTMP       "/etc/utmp"
#define PATH_SYSLOG     "/var/adm/syslog"
#define IS_UNIX_SOCKET(m)  (S_ISSOCK(m) || S_ISFIFO(m))
#endif

#ifdef SCO
#ifdef SCO3
typedef	int	ssize_t;
extern void closelog(void);
#define ftruncate	chsize
#define SA_RESTART	0
#define SOMAXCONN 5
#define NO_UNIX_SOCKET
#else
#define HAVE_GETRUSAGE
#define HAVE_SNPRINTF
#define HAVE_VSNPRINTF
#endif
#define SOCKLEN_T	int
#define O_RSYNC		0
#define PATH_TMP        "/tmp"
#define PATH_VAR_TMP    "/var/tmp"
#define PATH_PASSWD     "/etc/passwd"
#define PATH_WTMP       "/var/adm/wtmp"
#define PATH_UTMP       "/etc/utmp"
#define PATH_SYSLOG     "/var/adm/syslog"
#ifdef S_ISSOCK
#define IS_UNIX_SOCKET(m)  (S_ISSOCK(m) || S_ISFIFO(m))
#else
#define IS_UNIX_SOCKET(m)  S_ISFIFO(m)
#endif
#endif

#ifdef SUNOS4
#define HAVE_GETRUSAGE
#define SOCKLEN_T	int
#define SA_RESTART	0
#define O_RSYNC		0
#define PATH_TMP        "/tmp"
#define PATH_VAR_TMP    "/var/tmp"
#define PATH_PASSWD     "/etc/passwd"
#define PATH_WTMP       "/var/adm/wtmp"
#define PATH_UTMP       "/etc/utmp"
#define PATH_SYSLOG     "/var/adm/messages"
#define PATH_MAILLOG    "/var/log/syslog"

typedef int ssize_t;

#ifndef EXIT_SUCCESS
#define	EXIT_SUCCESS	0
#endif

#define difftime(time1,time0) ((double)(time1) - (double)(time0))
#define memmove(dst,src,len) bcopy((char *)(src), (char *)(dst), (int)(len))

extern char *sys_errlist[];

#define strerror(n) (sys_errlist[(n)])

/* explicit declaration of functions */

#include <memory.h>

extern int accept(int, struct sockaddr *, int *);
extern int bind(int, struct sockaddr *, int);
extern void bcopy(char *, char *, int);
extern void bzero(char *, int);
extern int connect(int, struct sockaddr *, int);
extern int fchmod(int, int);
extern int fclose(/* FILE * */);
extern int fprintf(/* FILE *, const char *, ... */);
extern int getrlimit(/* int, struct rlimit * */);
extern int getrusage(/* int, struct rusage * */);
extern int ftruncate(int, off_t);
extern int gettimeofday(struct timeval *, struct timezone *);
extern int listen(int, int);
extern void perror(char *);
extern int remove(const char *);
extern int select(int, fd_set *, fd_set *, fd_set *, struct timeval *);
extern int setsockopt(/* int, int, int, char *, int */);
extern int shutdown(int, int);
extern int socket(int, int, int);
extern int sscanf(const char *, const char *, ...);
extern long time(long *);
extern int on_exit(void (*)(), caddr_t arg);
extern openlog(const char *, int, int);
extern syslog(int, const char *, ...);
extern closelog(void);
#endif

#ifdef RELIANTUNIX
#define O_RSYNC		0
#undef HAVE_SNPRINTF
#undef HAVE_VSNPRINTF
#define HAVE_GETRUSAGE
#define SOCKLEN_T       size_t
#define PATH_TMP        "/tmp"
#define PATH_VAR_TMP    "/var/tmp"
#define PATH_PASSWD     "/etc/passwd"
#define PATH_WTMP       "/var/adm/wtmpx"
#define PATH_UTMP       "/var/adm/utmpx"
#define PATH_SYSLOG     "/var/adm/log/messages"
#define IS_UNIX_SOCKET(m)  (S_ISSOCK(m) || S_ISFIFO(m))
#endif

#ifdef UNICOS
#define HAVE_DAEMON
#define HAVE_SNPRINTF
#define HAVE_VSNPRINTF
#define O_RSYNC         0
#define SA_RESTART 0
#define PATH_TMP        "/tmp"
#define PATH_VAR_TMP    "/usr/tmp"
#define PATH_PASSWD     "/etc/passwd"
#define PATH_WTMP       "/etc/wtmp"
#define PATH_UTMP       "/etc/utmp"
/* System logging on Unicos is different, so the following settings probably
 * are not perfect. */
#define PATH_SYSLOG "/usr/adm/syslog/daylog"
#define PATH_MAILLOG "/usr/spool/mqueue/syslog"
#endif

#ifdef TANDEM
#define O_RSYNC         0
#define SA_RESTART      0
#define IS_UNIX_SOCKET(m)  (S_ISSOCK(m) || S_ISFIFO(m))
#ifdef HAVE_FLOSS_H
#include <floss.h>
#endif
#endif

#ifndef RANDSAVENAME
#define RANDSAVENAME	"prngd-seed"
#endif
#ifndef CONFIGFILE
#define CONFIGFILE	"prngd.conf"
#endif
#ifndef SOCKETMODE
#define SOCKETMODE	0777
#endif


#define BITS_PER_BYTE	8

/*
 * If not available, we cannot perform the check and will always succeed.
 */
#ifndef IS_UNIX_SOCKET
# ifdef S_ISSOCK
#  define IS_UNIX_SOCKET(m)	(S_ISSOCK(m))
# else
#  define IS_UNIX_SOCKET(m)	(1)
# endif
#else
# ifndef S_ISSOCK
#  define S_ISSOCK		(1)
# endif
#endif

/*
 * Some platforms don't have the SHUT_* macros
 */
#ifndef SHUT_RDWR
#define SHUT_RDWR	2
#endif

/*
 * If SOCKLEN_T is not defined, use a default.
 */
#ifndef SOCKLEN_T
#define SOCKLEN_T	int
#endif

/*
 * The pool size of the OpenSSL PRNG
 */
#define PRNGD_STATE_SIZE	(1024 * 4)

#ifndef SEED_STAT_INTERVAL
#define SEED_STAT_INTERVAL	17
#endif
#ifndef SEED_EXT_INTERVAL
#define SEED_EXT_INTERVAL	49
#endif

#define MAX_GATHERER_BYTES	100000

/*
 * Define the minimum ENTROPY_NEEDED to be 256, as one can retrieve 255 bytes
 * with on egd-query.
 */
#ifndef ENTROPY_NEEDED
#define ENTROPY_NEEDED		256
#endif

/*
 * Define the minimum of entropy we want to have in the pool on a regular
 * basis. If we come below this threshold, the gatherer processes are fired
 * up continously until we come back over the threshold.
 */
#ifndef THRESHOLD
#define THRESHOLD		4
#endif
#define ENTROPY_THRESHOLD	(ENTROPY_NEEDED * BITS_PER_BYTE * THRESHOLD)

/*
 * We will only accept a certain limit of open connections. We define a
 * reserve of open filedescriptors since we want to be able to open the
 * config file for re-reading and since from startup descriptors may
 * still be open.
 */
#ifndef OPEN_FD_RESERVE
#define OPEN_FD_RESERVE		10
#endif
