include ../../Makefile.inc

all: AdaptiblePQueue.o  ThreadQueue.o 

CFLAGS+=-c -fPIC $(CPPFLAGS)

AdaptiblePQueue.o: AdaptiblePQueue.cpp  AdaptiblePQueue.h  
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) $< -o $@

ThreadQueue.o: ThreadQueue.cpp  ThreadQueue.h  
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) $< -o $@

clean:
	rm -rf *.o
