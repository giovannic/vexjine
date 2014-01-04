include ../../../Makefile.inc

all: ThreadManagerClient.o  ThreadManagerServer.o 

CFLAGS+=-c -fPIC $(CPPFLAGS)

ThreadManagerClient.o: ThreadManagerClient.cpp  ThreadManagerClient.h  ThreadManagerCommunicationProtocol.h
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) $< -o $@

ThreadManagerServer.o: ThreadManagerServer.cpp  ThreadManagerServer.h  ThreadManagerCommunicationProtocol.h
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) $< -o $@

clean:
	rm -rf *.o
