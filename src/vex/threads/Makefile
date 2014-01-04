include ../../Makefile.inc

all: AggregateStateCounters.o IoHandler.o ModelHandler.o MethodLog.o Scheduling.o State.o Statistics.o NativeWaitingCriteria.o Timers.o ThreadState.o 

CFLAGS+=-c -fPIC $(CPPFLAGS)

ThreadState.o: ThreadState.cpp  ThreadState.h  
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) $< -o $@

AggregateStateCounters.o: AggregateStateCounters.cpp  AggregateStateCounters.h  
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) $< -o $@

ModelHandler.o: ModelHandler.cpp  ModelHandler.h  
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) $(INCLUDE_CINQS_DIRS) $< -o $@

IoHandler.o: IoHandler.cpp  IoHandler.h  
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) $< -o $@

NativeWaitingCriteria.o: NativeWaitingCriteria.cpp  NativeWaitingCriteria.h  
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) $< -o $@

MethodLog.o: MethodLog.cpp  MethodLog.h  
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) $< -o $@
	
Scheduling.o: Scheduling.cpp  Scheduling.h  
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) $< -o $@	

State.o: State.cpp  State.h  
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) $< -o $@
	
Statistics.o: Statistics.cpp  Statistics.h  
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) $< -o $@
	
Timers.o: Timers.cpp Timers.h  
	$(CC) $(CFLAGS) $(CPPFLAGS_TIME) $(INCLUDE_DIRS) $< -o $@
	
clean:
	rm -rf *.o
