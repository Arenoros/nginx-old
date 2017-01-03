 /*
  * Internal seed functions
  */

#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "prngd.h"

static const char *statfiles[] = {
#ifdef PATH_TMP
PATH_TMP,
#endif
#ifdef PATH_VAR_TMP
PATH_VAR_TMP,
#endif
#ifdef PATH_PASSWD
PATH_PASSWD,
#endif
#ifdef PATH_WTMP
PATH_WTMP,
#endif
#ifdef PATH_UTMP
PATH_UTMP,
#endif
#ifdef PATH_SYSLOG
PATH_SYSLOG,
#endif
#ifdef PATH_MAILLOG
PATH_MAILLOG,
#endif
NULL};

 /*
  * seed_internal: internal "cheap" seeding. Call statistic functions that
  * may provide some bits that are hard to predict:
  * - times() utilizes the cumulative computer time in TICKS, so hopefully
  *   the exact amount is not too easy to guess. Might give 1 or 2 bits of
  *   entropy.
  * - gettimeofday() can have the actual time in microseconds, the last
  *   bits might be "random".
  * - getpid() is probably not too hard to guess, but well, it won't hurt.
  * - getrusage(), if available, contains several statistical data that might
  *   be hard to guess.
  * - seed_counter is a simple counter to mix in...
  * This function is especially designed to be computationally cheap, so that
  * it can be called very often, especially every time when an event occurs
  * (a connection is opened etc), so that the usage of the real time values
  * might help increasing the entropy.
  *
  * Be careful anyhow, don't count the bits.
  *
  * This function may also be called to collect the data now and give it back
  * in the supplied memory for later usage.
  */
void seed_internal(seed_t *seed_p)
{
  seed_t seed, *my_seed_p;
  static int seed_counter = 0;

  if (seed_p)
    my_seed_p = seed_p;
  else
    my_seed_p = &seed;

  (void)times(&(my_seed_p->tmsbuf));
  my_gettimeofday(&(my_seed_p->tp), NULL);
  my_seed_p->pid = getpid();
#ifdef HAVE_GETRUSAGE
  (void)getrusage(RUSAGE_SELF, &(my_seed_p->usage));
#endif
  /*
   * add in just a little more variability each time we seed
   * internally by using a counter.
   */
  if (seed_counter == 0)
    seed_counter = (int)getpid();
  my_seed_p->seed_counter = seed_counter++;

  if (!seed_p)
    rand_add(my_seed_p, sizeof(seed_t), 0.0);
}

 /*
  * seed_stat(): Get seed by stat()ing files and directories on the system.
  * This is cheaper than doing an "ls", since no external process is needed.
  * The data returned includes size and access/modification times. The logfiles
  * should change in size quite often, while the TMP directories might
  * are changed quite often.
  * Why have /etc/passwd in the list??? Because a lot of programs need the
  * user information for the uid->name translation, so they must access
  * /etc/passwd (probably via getpwent) and hence impact the access time.
  *
  * We also do not count the bits.
  */
static int pathpoint = 0;

void seed_stat(void)
{
  struct stat buf;

 /*
  * Run over the list of files with often changing status (access/modification
  * time, length) and seed it into the PRNG. Since only certain parts of
  * the information changes, we assume the amount of entropy to be rather
  * small. Use 2bits=1/4byte per check.
  */
  while (statfiles[pathpoint] != NULL) {
    if (stat(statfiles[pathpoint++], &buf) == 0) {
      rand_add(&buf, sizeof(struct stat), 0.0);
      if (statfiles[pathpoint] == NULL)
	pathpoint = 0;
      return;
    }
  }

  pathpoint = 0;
  return;
}
