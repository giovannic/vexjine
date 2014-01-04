include ../../Makefile.inc

all: ThreadManager.o StrictParallelThreadManager.o LooseParallelThreadManager.o PassiveManager.o distributed/ThreadManagerServer.o distributed/ThreadManagerClient.o

CFLAGS+=-c -fPIC $(CPPFLAGS)

ThreadManager.o: ThreadManager.cpp  ThreadManager.h  
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) $< -o $@

StrictParallelThreadManager.o: StrictParallelThreadManager.cpp  StrictParallelThreadManager.h  
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) $< -o $@

LooseParallelThreadManager.o: LooseParallelThreadManager.cpp  LooseParallelThreadManager.h  
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) $< -o $@

PassiveManager.o: PassiveManager.cpp  PassiveManager.h  
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) $< -o $@

distributed/ThreadManagerServer.o: distributed/ThreadManagerServer.cpp distributed/ThreadManagerServer.h distributed/ThreadManagerCommunicationProtocol.h
	cd distributed/ ; $(MAKE) $(MAKECMDGOALS)

distributed/ThreadManagerClient.o: distributed/ThreadManagerClient.cpp distributed/ThreadManagerClient.h distributed/ThreadManagerCommunicationProtocol.h
	cd distributed/ ; $(MAKE) $(MAKECMDGOALS)

clean:
	rm -rf *.o
	cd distributed/ ; $(MAKE) clean
