		# Makefile for BF layer
		#for widows use \ in INCDIR
LIB	= bf
INCDIR	= ${MINIREL_HOME}\h
INCS	= 
SRCS	= 
TESTS	= bftest.c
OBJS	= ${SRCS:.c=.o}
LIBS	= lib${LIB}.a

#############################################################################
# This macro definition can be overwritten by command-line definitions.
CC	= gcc
CFLAGS	= -g -ansi -pedantic
#CFLAGS	= -O -ansi -pedantic
#############################################################################

all: lib${LIB}.a ${LIB}test

freelist:
	$(CC) $(CFLAGS) -o freelist -I ..\h

${LIB}test: ${LIB}test.o lib${LIB}.a
	$(CC) $(CFLAGS) -o $@ $< ${LIBS}

lib${LIB}.a:$(OBJS)	
	ar cr lib${LIB}.a $(OBJS)
	ranlib lib${LIB}.a

$(OBJS): ${INCS}

clean:
	rm -f lib${LIB}.a *.o ${LIB}test *.bak *~

.c.o:; $(CC) $(CFLAGS) -c $< -I. -I$(INCDIR)

