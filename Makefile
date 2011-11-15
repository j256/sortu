#
# $Id$
#

CC	= cc

PROG	= sortu
OBJS	= sortu.o argv.o strsep.o table.o

PORTFLS	= README.txt LICENSE.txt Makefile ChangeLog test_sortu.sh \
	sortu.c strsep.[ch] argv.[ch] argv_loc.h table.[ch] table_loc.h

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

port :: port.t

port.t :: $(PORTFLS)
	rm -rf $@
	mkdir $@
	cp $(PORTFLS) $@
	@ echo ''
	@ echo 'Please rename $@ to a version and tar up file'
