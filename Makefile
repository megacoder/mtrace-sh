PREFIX	=/opt
BINDIR	=${PREFIX}/bin

SUBDIRS	=

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

all::	libmtrace-sh.so

libmtrace-sh.so: mtrace-sh.c
	${CC} ${CFLAGS} -o $@ -shared -fPIC mtrace-sh.c

mtrace-sh: main.o
	${CC} ${LDFLAGS} -o $@ main.o ${LDLIBS}

${TARGETS}::

clean::
	${RM} a.out *.o core.* lint tags
	${RM} *.lst
	${RM} *.s
	${RM} mtrace-sh

distclean clobber:: clean
	${RM} libmtrace-sh.so
	${RM} mtrace-sh

check::	mtrace-sh libmtrace-sh.so
	-LD_PRELOAD=${PWD}/libmtrace-sh.so ./mtrace-sh ${ARGS}
	mtrace mtrace.out

tags::
	ctags -R .

ifneq	(${SUBDIRS},)
${TARGETS}::
	${MAKE} TARGET=$@ ${SUBDIRS}

${SUBDIRS}::
	${MAKE} -C $@ ${TARGET}
endif
