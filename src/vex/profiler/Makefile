include ../../Makefile.inc

all: ProfilingInvalidationPolicy.o PerformanceMeasure.o EventLogger.o  MethodCallInfo.o MethodData.o

CFLAGS+=-c -fPIC $(CPPFLAGS)

ProfilingInvalidationPolicy.o: ProfilingInvalidationPolicy.cpp  ProfilingInvalidationPolicy.h  
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) $< -o $@

PerformanceMeasure.o: PerformanceMeasure.cpp  PerformanceMeasure.h  
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) $< -o $@

MethodData.o: MethodData.cpp  MethodData.h  
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) $< -o $@

EventLogger.o: EventLogger.cpp EventLogger.h
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) $< -o $@

MethodCallInfo.o: MethodCallInfo.cpp MethodCallInfo.h  
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) $< -o $@

clean:
	rm -rf *.o
