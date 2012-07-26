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

CC	=ccache gcc -std=gnu99 -g
DEFS	=-D_FORTIFY_SOURCE=2
DEFS	+=-DHAVE_READLINE=1
OPT	=-Os
INCS	=-I.
CFLAGS	=${OPT} -Wall -Wextra -Werror -pedantic -pipe -g ${DEFS} ${INCS}
LDFLAGS	=-g
LDLIBS	=-ldl
LDLIBS	+=-lSegFault
LDLIBS	+=-lreadline -ltermcap

all::	libmtracesh.so

libmtracesh.so: libmtracesh.c
	${CC} ${CFLAGS} -o $@ -shared -fPIC libmtracesh.c

example: example.o
	${CC} ${LDFLAGS} -o $@ example.o ${LDLIBS}

${TARGETS}::

clean::
	${RM} a.out *.o core.* lint tags
	${RM} *.lst
	${RM} *.s
	${RM} example

distclean clobber:: clean
	${RM} libmtracesh.so
	${RM} example

check::	example libmtracesh.so
	-LD_PRELOAD=${PWD}/libmtracesh.so ./example ${ARGS}
	mtrace mtrace.out

tags::
	ctags -R .

ifneq	(${SUBDIRS},)
${TARGETS}::
	${MAKE} TARGET=$@ ${SUBDIRS}

${SUBDIRS}::
	${MAKE} -C $@ ${TARGET}
endif
