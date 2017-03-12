# Makefile for PF layer
CIN	= ci -l
RDIFF	= rcsdiff

LIB	= pf
INCDIR	= ${MINIREL_HOME}/h
INCS	= pferrors.h pftypes.h
SRCS	= pf.c pferrors.c pfstats.c
TESTS	= pftest.c pftest-ifvega.c
OBJS	= ${SRCS:.c=.o}
LIBS	= lib${LIB}.a ../bf/libbf.a

#############################################################################
# This macro definition can be overwritten by command-line definitions.
CC	= gcc
#CC	= gcc2723
#CC	= /usr/local/gnu-2.7.2.3/bin/gcc
#CFLAGS	= -g -ansi -pedantic -DDEBUG
#CFLAGS	= -g -ansi -pedantic -pg
#CFLAGS	= -g -ansi -pedantic -D_GNU_SOURCE -DDIRECTIO
#CFLAGS	= -g -ansi -pedantic -DMEMCHECK
CFLAGS	= -g -ansi -pedantic
#CFLAGS	= -O -ansi -pedantic
#CFLAGS	= -O -ansi -pedantic -U_FORTIFY_SOURCE
DIFLAGS	= -O -ansi -pedantic -D_GNU_SOURCE -DDIRECTIO
#############################################################################

newall: lib${LIB}.a ${LIB}test

all: lib${LIB}.a ${LIB}test ${LIB}test-ifvega

dio: clean
	${MAKE} CFLAGS="${DIFLAGS}" all dioset clean

dioset:
	cp lib${LIB}.a lib${LIB}-dio.a
	cp pftest pftest-dio; cp ${LIB}test-ifvega ${LIB}test-ifvega-dio

${LIB}test: ${LIB}test.o ${LIBS}
	$(CC) $(CFLAGS) -o $@ $< ${LIBS}

${LIB}test-ifvega: ${LIB}test-ifvega.o ${LIBS}
	$(CC) $(CFLAGS) -o $@ $< ${LIBS}

lib${LIB}.a:$(OBJS)
	ar cr lib${LIB}.a $(OBJS)
	ranlib lib${LIB}.a
$(OBJS): ${INCS}

ciall:
	${CIN} Makefile ${SRCS} ${INCS} ${TESTS}

diff:
	${RDIFF} Makefile ${SRCS} ${INCS} ${TESTS}

valgrind:
	valgrind --leak-check=full --show-reachable=yes -v pftest

xref:
	ctags ${INCS} ${SRCS}
	calltree -g ${INCS} ${SRCS} > ${LIB}.calltree

clean:
	rm -f lib${LIB}.a *.o ${LIB}test *.bak *~

.c.o:; $(CC) $(CFLAGS) -c $< -I. -I$(INCDIR)

depend:	$(SRCS)
	makedepend -I$(INCDIR) -- $(CFLAGS) -- $(SRCS)
# DO NOT DELETE THIS LINE -- make depend depends on it.
