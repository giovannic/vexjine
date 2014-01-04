CC=gcc

CFLAGS=-O3 -fPIC -shared -Wall
ifeq "$(MAKECMDGOALS)" "gdb"
CFLAGS=-O0 -g3 -fPIC -shared -Wall
endif

ifeq ($(shell uname -m), i686)
PERFCTR_LIBRARY_NAME=libperfdirect_x86.so
else
PERFCTR_LIBRARY_NAME=libperfdirect.so
endif


#all: x86.o perfdirect.o $(PERFCTR_LIBRARY_NAME)
all: perfdirect.o $(PERFCTR_LIBRARY_NAME)

gdb: 
nologgdb: 
nologpapi: 
papi: 
papirelease: 
release: 

perfctr: all
nologperfctr: all
perfctrrelease: all
viz: all
bit32: all

x86.o: x86.c arch.h
	$(CC) $(CFLAGS) -c x86.c 

perfdirect.o: perfdirect.c perfdirect.h
	$(CC) $(CFLAGS) -c perfdirect.c 

#$(PERFCTR_LIBRARY_NAME):
#	$(CC) $(CFLAGS) -Wl,-soname,$(PERFCTR_LIBRARY_NAME).1 -o $(PERFCTR_LIBRARY_NAME).1 x86.o perfdirect.o -lperfctr
#	ln -s $(PERFCTR_LIBRARY_NAME).1 $(PERFCTR_LIBRARY_NAME)

$(PERFCTR_LIBRARY_NAME):
	$(CC) $(CFLAGS) -Wl,-soname,$(PERFCTR_LIBRARY_NAME).1 -o $(PERFCTR_LIBRARY_NAME).1 perfdirect.o 
	ln -s $(PERFCTR_LIBRARY_NAME).1 $(PERFCTR_LIBRARY_NAME)

clean:
	rm -f *.o *.so* 
