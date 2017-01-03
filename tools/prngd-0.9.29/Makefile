# CC=cc
# CC=gcc

# HP-UX 10, HP-UX 11 (use -DHPUX10 or -DHPUX11 as appropriate)
# Flags for HP's ANSI C compiler
# CFLAGS=-Ae +w1 +O4 +Oall +DAportable +ESlit -DHPUX10
# Flags for gcc
# CFLAGS=-O -ansi -D_HPUX_SOURCE -Wall -Wstrict-prototypes -Wmissing-prototypes -Wshadow -DHPUX10

# Linux 2
# CFLAGS=-O -ansi -D_GNU_SOURCE -Wall -Wstrict-prototypes -Wmissing-prototypes -Wshadow -DLINUX2

# For Tru64 Unix (version 5). Older versions (reported for 4.0F) may not have
# snprintf in the C library but in libdb, so either remove the HAVE_SNPRINTF
# and HAVE_VSNPRINTF in config.h or link against libdb.
# CFLAGS=-O -DTRU64
# LDFLAGS=-ldb		# Only when snprintf is not supplied otherwise

# Digital's OSF1 Unix
# CFLAGS=-O -DOSF1

# Several Solaris settings. For Solaris up to 2.5.1, no snprintf() is available
# and old versions don't have "socklen_t" but use "int". If you experience
# problems with missing socklen_t or snprintf(), use the SOLARIS_OLD instead
# of the SOLARIS configuration.
# (Based on mixed information from several people and docs.sun.com.)

# Solaris 7 or 8 or 9
# CC=cc
# CFLAGS=-xO4 -KPIC -DSOLARIS
# CC=gcc
# CFLAGS=-O -DSOLARIS
# SYSLIBS=-lsocket -lnsl

# Solaris 2.6
# CC=cc
# CFLAGS=-xO4 -KPIC -DSOLARIS26 -D__EXTENSIONS__
# CC=gcc
# CFLAGS=-O -DSOLARIS26 -D__EXTENSIONS__
# SYSLIBS=-lsocket -lnsl

# Solaris 2.5.1
# CC=cc or gcc
# CFLAGS=-O -DSOLARIS_251 -D__EXTENSIONS__
# SYSLIBS=-lsocket -lnsl

# Old Solaris
# CC=cc or gcc
# CFLAGS=-O -DSOLARIS_OLD -D__EXTENSIONS__
# SYSLIBS=-lsocket -lnsl

# AT&T Unix System V Release 4 (NCR MP-RAS 3.x)
# CC=cc
# CFLAGS=-O -DNCR_MP_RAS3 -Hnocopyr -D_XOPEN_SOURCE -D_XOPEN_SOURCE_EXTENDED=2
# SYSLIBS=-lnsl -lsocket -L/usr/ucblib -lucb

# AT&T Unix System V Release 4 (NCR MP-RAS 2.x)
# CC=cc
# CFLAGS=-O -DNCR_MP_RAS2 -Hnocopyr
# SYSLIBS=-lnsl -lsocket -L/usr/ucblib -lucb

# MacOS X 10.0.x (PowerPC G3|G4) with cc or gcc (doesn't matter since Darwin
# cc is some sort of gcc 2.95.2) ("Atsushi Kobayashi" <loudear@a3.ctktv.ne.jp>)
# CC=cc
# CFLAGS=-O3 -mcpu=750 -traditional-cpp -force_cpusubtype_ALL -Wall \
#	-Wno-precomp -Wno-long-double -DMACOSX
# LDFLAGS=

# FreeBSD 4.x/5.x ("Bjoern A. Zeeb" <bzeeb@zabbadoz.net>)
# CC=gcc
# CFLAGS=-O3 -Wall -DFREEBSD
# SYSLIBS=

# Unixware 1
# CC=cc
# CFLAGS=-O -DUNIXWARE1
# SYSLIBS=-lnsl -lsocket

# NeXTstep 3.3 (m68k) with cc or gcc (doesn't matter since NeXTstep cc is some
# sort of gcc 2.5.8) ("Michael Weiser" <michael@weiser.saale-net.de>)
# Requires "libopenbsd-compat" from the OpenSSH-portable package.
# Do not use "-posix", as it is seriously broken.
# CFLAGS=-O3 -Wall -DNEXTSTEP -D_POSIX_SOURCE
# LDFLAGS=-L. -lopenbsd-compat

# NeXTstep 3.3 (HPPA) with cc or gcc (doesn't matter since NeXTstep cc is some
# sort of gcc 2.5.8) ("Jacques Distler" <distler@golem.ph.utexas.edu>)
# Requires "libopenbsd-compat" from the OpenSSH-portable package.
# Do not use "-posix", as it is seriously broken.
# CFLAGS=-O3 -Wall -DNEXTSTEP -D_POSIX_SOURCE
# LDFLAGS=-L. -lopenbsd-compat

# OpenStep 4.2 (m68k) with cc or gcc (doesn't matter since NeXTstep cc is
# a modified 2.7.x series.) ("Ben Lindstrom" <mouring@eviladmin.org>)
# Requires "libopenbsd-compat" from the OpenSSH-portable package.
# Do not use "-posix", as it is seriously broken.
# CFLAGS=-O3 -Wall -DNEXTSTEP -D_POSIX_SOURCE
# LDFLAGS=-L../ossh/openbsd-compat/ -lopenbsd-compat

# IRIX 6.2 w/gcc ("Michael Weiser" <michael@weiser.saale-net.de>)
# CFLAGS=-O3 -Wall -DIRIX62

# IRIX 6.2 w/cc ("Michael Weiser" <michael@weiser.saale-net.de>)
# CFLAGS=-O3 -fullwarn -DIRIX62 -woff 1506

# IRIX 6.5 w/gcc ("Ayamura Kikuchi" <ayamura@ayamura.org>)
# CFLAGS=-O3 -Wall -DIRIX65

# IRIX 6.5 w/cc (/etc/compiler.defaults: Default ABI, ISA and processor options
# configuration file)  ("Ayamura Kikuchi" <ayamura@ayamura.org>)
# CFLAGS=-O3 -fullwarn -DIRIX65

# UNIXWARE7 w/cc ("George Walsh" <gjmwalsh@netscape.net>)
# CFLAGS=-O -DOPEN_SOURCE -DUNIXWARE7
# SYSLIBS=-lsocket -lnsl

# UNIXWARE7 w/gcc ("John Hughes" <john@Calva.COM>)
# CFLAGS=-O -Wall -Wstrict-prototypes -Wmissing-prototypes -Wshadow -DUNIXWARE7
# SYSLIBS=-lsocket -lnsl

# OpenUNIX8 w/cc ("Boyd Gerber" <gerberb@zenez.com>)
# CFLAGS=-Xb -O -DOPEN_SOURCE -DOpenUNIX8
# SYSLIBS=-lsocket -lnsl

# OpenUNIX8 w/gcc ("Boyd Gerber" <gerberb@zenez.com>)
# CFLAGS=-O -Wall -Wstrict-prototypes -Wmissing-prototypes -Wshadow -DOpenUNIX8
# SYSLIBS=-lsocket -lnsl

# OpenServer 5 w/gcc ("Boyd Gerber" <gerberb@zenez.COM>)
# CFLAGS=-O -Wall -Wstrict-prototypes -Wmissing-prototypes -Wshadow -DSCO
# SYSLIBS=-lsocket -lnsl

# SCO 3 w/gcc (Gert Doering <gert@greenie.muc.de>)
# CFLAGS=-O -Wall -Wstrict-prototypes -Wmissing-prototypes -Wshadow -DSCO -DSCO3
# SYSLIBS=-lsocket -lc -lx

# SCO 3.2v4.2 w/gcc (Vikash Badal - PCS <VikashB@ComparexAfrica.co.za>)
# CFLAGS=-O -Wall -Wstrict-prototypes -Wmissing-prototypes -Wshadow -DSCO -DSCO3
# SYSLIBS=-lsocket -lc -lcA -lx

# AIX 3.2 w/cc (Bernhard Simon <simon@zid.tuwien.ac.at>)
# CFLAGS=-O -DAIX32
# SYSLIBS=

# AIX 3.2 w/gcc (Bernhard Simon <simon@zid.tuwien.ac.at>)
# CFLAGS=-O2 -DAIX32
# SYSLIBS=

# AIX 4.3 w/cc (Joerg Petersen <j.petersen@msh.de>)
# Please also check out contrib/AIX-4.3/00README.aix-src
# CFLAGS= -O3 -qmaxmem=32768 -qro -qroconst -qinline -qsuppress=1500-036 -DAIX43
# SYSLIBS=

# AIX 4.1
# CFLAGS= -O -DAIX41
# SYSLIBS=

# AIX 5.1 w/gcc (Rich Holland <holland@pobox.com>)
# CFLAGS= -O2 -DAIX51
# SYSLIBS=

# SunOS 4.1.4 w/gcc ("Shigeki miyu Sugiura" <miyu@ky.xaxon.ne.jp>)
# Flags for GCC only.
# CFLAGS=-O -Wall -DSUNOS4
# SYSLIBS=

# Reliant Unix
# CC=cc
# CFLAGS=-O -DRELIANTUNIX
# SYSLIBS=-lsocket -lnsl

# Ultrix V4.5 w/cc (Bernhard Simon <simon@zid.tuwien.ac.at>)
# CFLAGS=-O -DULTRIX
# SYSLIBS=

# Ultrix V4.5 w/gcc (Bernhard Simon <simon@zid.tuwien.ac.at>)
# CFLAGS=-O2 -DULTRIX
# SYSLIBS=

# Unicos (Marcin Mogielnicki <mar@ghandalf.pism.pl>)
# CFLAGS=-O3 -DUNICOS
# SYLIBS=

# Tandem Himalaya NSK (Simon Goldrei <simon.goldrei@webpay.com.au>)
# CC = c89
# CFLAGS= -Ww -DTANDEM -D__TANDEM -D_XOPEN_SOURCE -D_XOPEN_SOURCE_EXTENDED=1 -D_TANDEM_SOURCE -DB_ENDIAN
# SYSLIBS=

# nsr-tandem-nsk ("Bates, Tom" <tom.bates@hp.com>)
# CC=gcc
# CFLAGS= -O2 -DHAVE_FLOSS_H
# SYSLIBS=

# QNX 6.5.0 armle (Sergey Zhuravlev <arenoros@gmail.com>)
# CC=arm-unknown-nto-qnx6.5.0-gcc
# CFLAGS="-DQNX -O3 -ffunction-sections -fdata-sections -funroll-loops"
# SYSLIBS=-lsocket

LIBS=$(SYSLIBS)

SRC = bsd-daemon.c bsd-snprintf.c unix_socket.c tcp_socket.c close_on_exec.c conffile.c error_handlers.c error_log.c non_blocking.c obtain_limit.c prngd.c rand.c seed.c serverloop.c sha1.c sockio.c

# Move default locations
#DEFS=-DRANDSAVENAME=\"/tmp/prngd-seed\" -DCONFIGFILE=\"/tmp/prngd.conf\"
# or
#DEFS=-DRANDSAVENAME=\"/usr/local/etc/prngd/prngd-seed\" -DCONFIGFILE=\"/usr/local/etc/prngd/prngd.conf\"
# or

COMPLETE_CPPFLAGS=-I. $(CPPFLAGS)

OBJS=prngd.o serverloop.o bsd-daemon.o unix_socket.o tcp_socket.o seed.o rand.o conffile.o sha1.o close_on_exec.o non_blocking.o sockio.o obtain_limit.o error_log.o error_handlers.o bsd-snprintf.o

.c.o:
	$(CC) $(CFLAGS) $(COMPLETE_CPPFLAGS) $(DEFS) -c $<

prngd: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS) $(LIBS)

prngd.man: prngd.pod
	pod2man -c "" -r "" -s 1 prngd.pod > prngd.man

prngd.cat: prngd.man
	nroff -man prngd.man > prngd.cat

clean:
	rm -f prngd $(OBJS)

lint:
	lint -u -x $(CFLAGS) $(LIBS) $(SRC)

prngd.o: prngd.c config.h prngd.h prngd_version.h
serverloop.o: serverloop.c config.h prngd.h
seed.o: seed.c config.h prngd.h
bsd-daemon.o: bsd-daemon.c config.h prngd.h
bsd-snprintf.o: bsd-snprintf.c config.h prngd.h
rand.o: rand.c config.h prngd.h
conffile.o: conffile.c config.h prngd.h
sha1.o: sha1.c config.h prngd.h
close_on_exec.o: close_on_exec.c config.h prngd.h
non_blocking.o: non_blocking.c config.h prngd.h
sockio.o: sockio.c config.h prngd.h
obtain_limit.o: obtain_limit.c config.h prngd.h
error_log.o: error_log.c config.h prngd.h
error_handlers.o: error_handlers.c config.h prngd.h
tcp_socket.o: tcp_socket.c config.h prngd.h
unix_socket.o: unix_socket.c config.h prngd.h
