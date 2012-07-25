#########################################################################
# vim: ts=8 sw=8
#########################################################################
# Author:   tf135c (James Reynolds)
# Filename: Makefile
# Date:     2006-12-11 09:56:05
#########################################################################
# Note that we use '::' rules to allow multiple rule sets for the same
# target.  Read that as "modularity exemplarized".
#########################################################################

PREFIX	:=${HOME}/opt/$(shell uname -m)
BINDIR	=${PREFIX}/bin

.SUFFIXES: .lst

%.s:	%.c
	${CC} ${CFLAGS} -o $@ -S $^

%.lst:	%.o
	objdump -dS $^ >$@

%.lst:	%.so
	objdump -dS $^ >$@

TARGETS	=all clean distclean clobber check install uninstall tags
TARGET	=all

SUBDIRS	=

.PHONY:	${TARGETS} ${SUBDIRS}

CC	=ccache gcc -std=gnu99
DEFS	=-D_FORTIFY_SOURCE=2
DEFS	+=-DHAVE_READLINE=1
OPT	=-Os
INCS	=-I.
CFLAGS	=${OPT} -Wall -Wextra -Werror -pedantic -pipe -g ${DEFS} ${INCS}
LDFLAGS	=-g
LDLIBS	=-ldl
LDLIBS	+=-lSegFault
LDLIBS	+=-lreadline -ltermcap

all::	mtrace-sh

libmtrace-sh.so: mtrace-sh.c
	${CC} ${CFLAGS} -o $@ -shared -fPIC mtrace-sh.c

mtrace-sh: main.o
	${CC} ${LDFLAGS} -o $@ main.o ${LDLIBS}

${TARGETS}::

clean::
	${RM} a.out *.o core.* lint tags
	${RM} *.lst
	${RM} *.s

distclean clobber:: clean
	${RM} libmtrace-sh.so
	${RM} mtrace-sh

check::	mtrace-sh libmtrace-sh.so
	LD_PRELOAD=${PWD}/libmtrace-sh.so ./mtrace-sh ${ARGS}

install:: mtrace-sh
	install -d ${BINDIR}
	install -c -s mtrace-sh ${BINDIR}/

uninstall::
	${RM} ${BINDIR}/mtrace-sh

tags::
	ctags -R .

# ${TARGETS}::
# 	${MAKE} TARGET=$@ ${SUBDIRS}

# ${SUBDIRS}::
# 	${MAKE} -C $@ ${TARGET}
