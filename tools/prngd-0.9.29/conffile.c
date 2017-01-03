#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "prngd.h"

#define WHITESPACE " \t\n"

void parse_configfile(const char *cmdpath, entropy_source_t **entropy_source_p)
{
  FILE *conf;
  int linenum;
  int num_cmds = 64, cur_cmd = 0;
  int arg;
  char msg_buf[1024];
  double est;
  char buffer[1024], cmd[1024], fullpath[1024], *cp;
  entropy_source_t *entropy_source;

  entropy_source = (entropy_source_t*) my_malloc(num_cmds *
  	sizeof(entropy_source_t));

  if (!(conf = fopen(cmdpath, "r"))) {
    (void)snprintf(msg_buf, sizeof(msg_buf),
    	"could not open command-sourcefile %.100s", cmdpath);
    msg_buf[sizeof(msg_buf) - 1] = '\0';
    error_fatal(msg_buf, errno);
    exit(EXIT_FAILURE);
  }
 
  linenum = 0;
  while (fgets(buffer, 1023, conf)) {
    linenum++;
    cp = buffer + strspn(buffer, WHITESPACE);
    if ((*cp == '\0') || (*cp == '#'))
	continue;

    if (*cp != '"') {
      (void)snprintf(msg_buf, 1024, "Bad entropy command %.100s:%d\n", cmdpath,
      	linenum);
      msg_buf[sizeof(msg_buf) - 1] = '\0';
      error_error(msg_buf, 0);
      continue;
    }

    cp = strtok(cp, "\"");
    if (cp == NULL) {
      (void)snprintf(msg_buf, 1024, "missing or bad command string, %.100s:%d -- ignored\n",
              cmdpath, linenum);
      msg_buf[sizeof(msg_buf) - 1] = '\0';
      error_error(msg_buf, 0);
      continue;
    }
    (void)strncpy(cmd, cp, sizeof(cmd));
    cmd[sizeof(cmd)-1] = '\0';

    if ((cp = strtok(NULL, WHITESPACE)) == NULL) {
      (void)snprintf(msg_buf, 1024, "missing command cmdpath, %.100s:%d -- ignored",
	      cmdpath, linenum);
      msg_buf[sizeof(msg_buf) - 1] = '\0';
      error_error(msg_buf, 0);
      continue;
    }

    /* did configure mark this as dead? */
    if (strncmp("undef", cp, 5) == 0)
      continue;
    (void)strncpy(fullpath, cp, sizeof(fullpath));
    fullpath[sizeof(fullpath)-1] = '\0';

    /* third token, entropy rate estimate for this command */
    if ((cp = strtok(NULL, WHITESPACE)) == NULL) {
      (void)snprintf(msg_buf, 1024, "missing entropy estimate, %.100s:%d -- ignored",
	      cmdpath, linenum);
      msg_buf[sizeof(msg_buf) - 1] = '\0';
      error_error(msg_buf, 0);
      continue;
    }
    est = atof(cp);

    /* end of line */
    if ((cp = strtok(NULL, WHITESPACE)) != NULL) {
      (void)snprintf(msg_buf, 1024, "garbage at end of %.100s:%d -- ignored",
	      cmdpath, linenum);
      msg_buf[sizeof(msg_buf) - 1] = '\0';
      error_error(msg_buf, 0);
      continue;
    }

    /*
     * Now that the line is scanned in, fit it into the entropy_cmd table.
     */

    entropy_source[cur_cmd].path = malloc(strlen(fullpath) + 1);
    if (entropy_source[cur_cmd].path == NULL) {
	error_fatal("could not allocate entropy-command-table", errno);
	exit(EXIT_FAILURE);
    }
    (void)strcpy(entropy_source[cur_cmd].path, fullpath);

    entropy_source[cur_cmd].cmdstring = malloc(strlen(cmd) + 1);
    if (entropy_source[cur_cmd].cmdstring == NULL) {
	error_fatal("could not allocate entropy-command-table", errno);
	exit(EXIT_FAILURE);
    }
    (void)strcpy(entropy_source[cur_cmd].cmdstring, cmd);

    /* split the command args */
    cp = strtok(cmd, WHITESPACE);
    arg = 0;
    do {
      char *s = malloc(strlen(cp) + 1);
      if (!s) {
	error_fatal("could not allocate entropy-command-table", errno);
	exit(EXIT_FAILURE);
      }
      (void)strncpy(s, cp, strlen(cp) + 1);
      entropy_source[cur_cmd].args[arg] = s;
      arg++;
    } while ((arg < 5) && (cp = strtok(NULL, WHITESPACE)));
    while (arg < 5)
      entropy_source[cur_cmd].args[arg++] = NULL;

    if (strtok(NULL, WHITESPACE)) {
	(void)snprintf(msg_buf, 1024, "ignored extra command elements (max 5), %.100s:%d",
               cmdpath, linenum);
	msg_buf[sizeof(msg_buf) - 1] = '\0';
      	error_error(msg_buf, 0);
    }

    entropy_source[cur_cmd].rate = est;
    /* Initialise other values */
    entropy_source[cur_cmd].sticky_badness = 1;
    
    cur_cmd++;

    /* If we've filled the array, reallocate it twice the size */
    /* Do this now because even if this we're on the last command,
     * we need another slot to mark the last entry */
    if (cur_cmd == num_cmds) {
      num_cmds *= 2;
      entropy_source = realloc(entropy_source,
			       num_cmds * sizeof(entropy_source_t));
      if (!entropy_source) { 
	error_fatal("could not allocate entropy-command-table", errno);
	exit(EXIT_FAILURE);
      }
    }


  }
  (void)fclose(conf);

  /*
   * Zero out the last entry in the table and resize the array to save some
   * memory, evn though it shouldn't be much.
   */
  (void)memset(entropy_source + cur_cmd, 0, sizeof(entropy_source_t));

  entropy_source = realloc(entropy_source,
			   (cur_cmd + 1)*sizeof(entropy_source_t));
  if (!entropy_source) { 
	error_fatal("could not allocate entropy-command-table", errno);
	exit(EXIT_FAILURE);
  }

  *entropy_source_p = entropy_source;
}
