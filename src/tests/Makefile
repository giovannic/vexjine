include ../Makefile.inc

OBJECTS=MethodOperations.o MethodSimulation.o Tester.o AgentTester.o ThreadExecutionPattern.o
TESTS=randomprogram testmanagertest perftest lftest verifysequence pagefault queue vlock

SHARED_LD=$(LDDIRS) $(LDLIBS)

all: $(OBJECTS) $(TESTS)

AgentTester.o: AgentTester.cpp AgentTester.h
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) -c AgentTester.cpp
	
Tester.o: Tester.cpp Tester.h
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) -c Tester.cpp
	
MethodSimulation.o: MethodSimulation.cpp MethodSimulation.h
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) -c MethodSimulation.cpp

MethodOperations.o: MethodOperations.cpp MethodOperations.h
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) -c MethodOperations.cpp
 		
ThreadExecutionPattern.o: ThreadExecutionPattern.cpp ThreadExecutionPattern.h
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) -c ThreadExecutionPattern.cpp
	
randomprogram: RunRandomProgram.cpp RunRandomProgram.h
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) $(OBJECTS) RunRandomProgram.cpp -o randomprogram  $(VEX_WLRPATHS) $(VEX_LIBRARY) $(SHARED_LD)
	
verifysequence: SequenceVerification.cpp SequenceVerification.h
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) $(OBJECTS) SequenceVerification.cpp -o verifysequence $(VEX_WLRPATHS) $(VEX_LIBRARY) $(SHARED_LD)

pagefault: PageFaultTest.cpp PageFaultTest.h
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) $(OBJECTS)  PageFaultTest.cpp -o pagefault  $(VEX_WLRPATHS) $(VEX_LIBRARY) $(SHARED_LD)
	
queue: Queueing.cpp Queueing.h
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) $(OBJECTS)  Queueing.cpp -o queue  $(VEX_WLRPATHS) $(VEX_LIBRARY) $(SHARED_LD)
		
vlock: ValidateLock.cpp
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) $(OBJECTS)  ValidateLock.cpp -o vlock  $(VEX_WLRPATHS) $(VEX_LIBRARY) $(SHARED_LD)
				
testmanagertest: ThreadManagerTester.cpp ThreadManagerTester.h ThreadExecutionPattern.o AgentTester.o
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) $(OBJECTS)  ThreadManagerTester.cpp -o testmanagertest $(VEX_WLRPATHS) $(VEX_LIBRARY) $(SHARED_LD) 

perftest: PerformanceTest.cpp PerformanceTest.h
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) $(OBJECTS)   PerformanceTest.cpp -o perftest $(VEX_WLRPATHS) $(VEX_LIBRARY) $(SHARED_LD) $(LDDIRS) $(LDLIBS)
	
lftest: ForwardLeapTest.cpp ForwardLeapTest.h
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) $(OBJECTS)   ForwardLeapTest.cpp -o lftest $(VEX_WLRPATHS) $(VEX_LIBRARY) $(SHARED_LD)
				
clean:
	rm -rf *.o randomprogram tmt testmanagertest perftest lftest verifysequence pagefault queue vlock
