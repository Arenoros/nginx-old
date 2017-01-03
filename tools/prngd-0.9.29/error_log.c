#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

#include "prngd.h"


/* functions that are only used by this module */
static void error_syslog(int, const char *, int);
static void initialise_syslog(void);


/* a buffer that is used to copy the message given by the user and and
   message from errno if it is also provided. This is so malloc() doesn't
   have to by relied upon at a later stage. */
#define MSG_BUF_SIZE 1024
static char msg_buf[MSG_BUF_SIZE];

/* indicates if openlog(3) has been called yet */
static int syslog_opened = 0;


static void
initialise_syslog(void)
{
	int rv;

	openlog("prngd", LOG_PID|LOG_CONS|LOG_NOWAIT, SYSLOG_FACILITY);
#ifdef SUNOS4
	rv = on_exit((void (*)()) closelog, NULL);
#else
	rv = atexit(closelog);
#endif

	if (rv != 0) {
#ifdef SUNOS4
		perror("prngd: failed to register closelog(3) with on_exit(3)");
#else
		perror("prngd: failed to register closelog(3) with atexit(3)");
#endif
	}
	else {
		syslog_opened = 1;
	}
}


static void
error_syslog(int priority, const char* msg, int my_errno)
{
	char* errno_msg = NULL;
	int user_msg_size;
	int buffer_left;

	if (! syslog_opened) {
		initialise_syslog();
	}

	user_msg_size = strlen(msg);

	/* if we have errno change it to a error string if there will be
	   enough room in msg_buf to copy the user message and atleast some
	   of the errno msg */
	if (my_errno != 0 && user_msg_size < MSG_BUF_SIZE) {
		errno_msg = strerror(my_errno);
	}

	/* it is possible that errno was out of range */
	if (errno_msg != NULL) {

		/* 1 char for newline and 2 for ": " */
		buffer_left = sizeof(msg_buf) - user_msg_size - 3;
		(void)strncpy(msg_buf, msg, user_msg_size);
		(void)strncpy(&(msg_buf[user_msg_size]), ": ", 3);
		(void)strncpy(&(msg_buf[user_msg_size + 2]), errno_msg,
			buffer_left);
		msg_buf[sizeof(msg_buf) - 1] = '\0';
		syslog(priority, msg_buf);
		(void)fprintf(stderr, "%s\n", msg_buf);
	}
	else {
		syslog(priority, msg);
		(void)fprintf(stderr, "%s\n", msg);
	}
}


void
error_fatal(const char* msg, int my_errno)
{
	error_syslog(LOG_ALERT, msg, my_errno);
}


void
error_error(const char* msg, int my_errno)
{
	error_syslog(LOG_ERR, msg, my_errno);
}


void
error_notice(const char* msg, int my_errno)
{
	error_syslog(LOG_NOTICE, msg, my_errno);
}
