#include <sys/time.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <unistd.h>
#include <errno.h>

#include "prngd.h"

#ifdef NEED_SYS_SELECT_H
#include <sys/select.h>
#endif /* NEED_SYS_SELECT_H */

/*
 * Try to obtain the maximum allowed number of open files.
 * Compare with FD_SETSIZE and return whatever is smaller.
 * Partly inspired by open_limit() function in Wietse Venema's Postfix.
 */
#ifndef RLIMIT_NOFILE
#ifdef RLIMIT_OFILE
#define RLIMIT_NOFILE RLIMIT_OFILE
#endif
#endif

int obtain_limit(void)
{
    int limit = -1;
#ifdef RLIMIT_NOFILE
    struct rlimit rl;
#endif

#ifdef RLIMIT_NOFILE
    if (getrlimit(RLIMIT_NOFILE, &rl) < 0) {
	error_error("getrlimit() failed", errno);
        return (-1);
    }
    limit = rl.rlim_cur;
#endif

#ifndef RLIMIT_NOFILE
    limit = getdtablesize();
#endif

    if (limit> FD_SETSIZE)
      limit = FD_SETSIZE;

    return (limit);

}
