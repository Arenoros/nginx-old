/*
 * $Id: prngd-ctl.c,v 1.2 2004/05/17 07:19:49 jaenicke Exp $
 *
 * Author:
 * Thomas Binder <binder@arago.de>
 *
 * Purpose:
 * Command line utility to query an egd-type socket. write-command currently
 * not implemented!
 *
 * Usage:
 * 1. prngd-ctl path-to-socket get
 *    -> prints number of available entropy bits
 *
 * 2. prngd-ctl path-to-socket read number-of-bytes
 *    -> reads number-of-bytes random bytes and prints them (unencoded!)
 *
 * 3. prngd-ctl path-to-socket readb number-of-bytes
 *    -> reads (blocking) number-of-bytes random bytes and prints them
 *       (unencoded!)
 *
 * 4. prngd-ctl path-to-socket pid
 *    -> prints the corresponding prngd's (or egd's) pid
 *
 * License:
 * Copyright (c) 2002 Thomas Binder.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

static int cmd_get(int sd);
static int cmd_read(int sd, size_t bytes);
static int cmd_readb(int sd, size_t bytes);
static int cmd_pid(int sd);

int main(int argc, char *argv[])
{
    int		    sd = -1,
		    socklen,
		    ret;
    long	    bytes;
    char	    *p;
    struct sockaddr sock, *dest;

    if (argc < 3)
    {
usage:
	fprintf(stderr, "Usage: %s path-to-socket cmd [args]\n", argv[0]);
	if (sd != -1)
	    close(sd);
	exit(1);
    }
    sd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sd == -1)
    {
	perror("socket");
	exit(1);
    }

    socklen = strlen(argv[1]) + 1 + sizeof(sock) - sizeof(sock.sa_data);
    if ((dest = (struct sockaddr*)malloc( socklen)) == NULL)
    {
	perror("malloc dest");
	close(sd);
	exit(1);
    }
    memset(dest, 0, socklen);
    dest->sa_family = AF_UNIX;
    strcpy(dest->sa_data, argv[1]);
    if (connect(sd, dest, socklen) == -1)
    {
	perror(argv[1]);
	close(sd);
	exit(1);
    }
    if (!strcasecmp(argv[2], "get"))
    {
	if (argc != 3)
	    goto usage;
	ret = cmd_get(sd);
    }
    else if (!strcasecmp(argv[2], "read"))
    {
	if (argc != 4)
	    goto usage;
	bytes = strtoul(argv[3], &p, 0);
	if (*p || (bytes < 0) || (bytes > 255))
	{
	    fprintf(stderr, "%s: Illegal number\n", argv[3]);
	    close(sd);
	    exit(1);
	}
	ret = cmd_read(sd, (size_t)bytes);
    }
    else if (!strcasecmp(argv[2], "readb"))
    {
	if (argc != 4)
	    goto usage;
	bytes = strtoul(argv[3], &p, 0);
	if (*p || (bytes < 0) || (bytes > 255))
	{
	    fprintf(stderr, "%s: Illegal number\n", argv[3]);
	    close(sd);
	    exit(1);
	}
	ret = cmd_readb(sd, (size_t)bytes);
    }
    else if (!strcasecmp(argv[2], "pid"))
    {
	if (argc != 3)
	    goto usage;
	ret = cmd_pid(sd);
    }
    else
	goto usage;
    close(sd);
    exit(ret);
}

static int cmd_get(int sd)
{
    ssize_t read_bytes,
	    wrote_bytes;
    char    buf[257];
    long    amount;

    buf[0] = 0;
    wrote_bytes = write(sd, buf, 1);
    if (wrote_bytes != (ssize_t)1)
    {
	fprintf(stderr, "write() to socket failed\n");
	return(1);
    }
    read_bytes = read(sd, buf, 4);
    if (read_bytes != (ssize_t)4)
    {
	fprintf(stderr, "read() from socket failed\n");
	return(1);
    }
    amount = ((int)buf[0] & 0xff) * 16777216;
    amount += ((int)buf[1] & 0xff) * 65536;
    amount += ((int)buf[2] & 0xff) * 256;
    amount += (int)buf[3] & 0xff;
    printf("%ld\n", amount);
    return(0);
}

static int cmd_read(int sd, size_t bytes)
{
    ssize_t read_bytes,
	    wrote_bytes;
    size_t  to_read;
    char    buf[257];

    buf[0] = 1;
    buf[1] = (char)bytes;
    wrote_bytes = write(sd, buf, 2);
    if (wrote_bytes != (ssize_t)2)
    {
	fprintf(stderr, "write() to socket failed\n");
	return(1);
    }
    read_bytes = read(sd, buf, 1);
    if (read_bytes != (ssize_t)1)
    {
	fprintf(stderr, "read() from socket failed\n");
	return(1);
    }
    to_read = (size_t)((int)buf[0] & 0xff);
    read_bytes = read(sd, buf, to_read);
    if (read_bytes != to_read)
    {
	fprintf(stderr, "read() from socket failed\n");
	return(1);
    }
    write(STDOUT_FILENO, buf, to_read);
    return(0);
}

static int cmd_readb(int sd, size_t bytes)
{
    ssize_t read_bytes,
	    wrote_bytes;
    char    buf[257];

    buf[0] = 2;
    buf[1] = (char)bytes;
    wrote_bytes = write(sd, buf, 2);
    if (wrote_bytes != (ssize_t)2)
    {
	fprintf(stderr, "write() to socket failed\n");
	return(1);
    }
    while (bytes)
    {
	read_bytes = read(sd, buf, bytes);
	if (read_bytes == -1)
	{
	    fprintf(stderr, "read() from socket failed\n");
	    return(1);
	}
	if (read_bytes == 0)
	    break;
	write(STDOUT_FILENO, buf, read_bytes);
	bytes -= read_bytes;
    }
    return(0);
}

static int cmd_pid(int sd)
{
    ssize_t read_bytes,
	    wrote_bytes;
    size_t  to_read;
    char    buf[257];

    buf[0] = 4;
    wrote_bytes = write(sd, buf, 1);
    if (wrote_bytes != (ssize_t)1)
    {
	fprintf(stderr, "write() to socket failed\n");
	return(1);
    }
    read_bytes = read(sd, buf, 1);
    if (read_bytes != (ssize_t)1)
    {
	fprintf(stderr, "read() from socket failed\n");
	return(1);
    }
    to_read = (size_t)((int)buf[0] & 0xff);
    read_bytes = read(sd, buf, to_read);
    if (read_bytes != to_read)
    {
	fprintf(stderr, "read() from socket failed\n");
	return(1);
    }
    buf[to_read] = 0;
    puts(buf);
    return(0);
}

/* EOF */
