#
# $Id$
#

CC	= gcc

PROG	= sortu
OBJS	= sortu.o argv.o compat.o strsep.o table.o

CFLAGS	= -O2 $(CCFLS)
DESTDIR	= /usr/local/local/sbin

all : $(PROG)

clean :
	rm -f a.out core *.o *.t *.cpp
	rm -f $(PROG)

$(PROG) : $(OBJS)
	rm -f $@
	$(CC) $(LDFLAGS) $(OBJS) $(LIBS)
	mv a.out $@

.c.o :
	rm -f $@
	$(CC) $(CFLAGS) $(CPPFLAGS) $(DEFS) $(INCS) -c $< -o $@
