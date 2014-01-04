include ../../Makefile.inc

all: ObjectRegistry.o ThreadRegistry.o ThreadManagerRegistry.o LockRegistry.o

CFLAGS+=-c -fPIC $(CPPFLAGS)

ObjectRegistry.o: ObjectRegistry.cpp  ObjectRegistry.h  
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) $< -o $@

ThreadRegistry.o: ThreadRegistry.cpp  ThreadRegistry.h  
	$(CC) $(CFLAGS) $(CPPFLAGS) $(INCLUDE_DIRS) $< -o $@

ThreadManagerRegistry.o: ThreadManagerRegistry.cpp  ThreadManagerRegistry.h  
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) $< -o $@

LockRegistry.o: LockRegistry.cpp  LockRegistry.h  
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) $< -o $@

clean:
	rm -rf *.o
