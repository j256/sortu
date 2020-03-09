CC	= cc

PROG	= sortu
OBJS	= sortu.o argv.o strsep.o table.o

CFLAGS	= -g -Wall -O2 $(CCFLS)
DESTDIR	= /usr/local/sbin

all : $(PROG)

clean :
	rm -f a.out core *.o *.t *.cpp
	rm -f $(PROG)

tests :
	sh test_sortu.sh

$(PROG) : $(OBJS)
	rm -f $@
	$(CC) $(LDFLAGS) $(OBJS) $(LIBS)
	mv a.out $@

.c.o :
	rm -f $@
	$(CC) $(CFLAGS) $(CPPFLAGS) $(DEFS) $(INCS) -c $< -o $@

argv.o: argv.c strsep.h argv.h argv_loc.h
sortu.o: sortu.c argv.h table.h
strsep.o: strsep.c
table.o: table.c table.h table_loc.h
