include ../Makefile.inc
PROFILER_INCLUDE_DIRS=$(INCLUDE_DIRS)   $(PAPI_LIBRARIES) 

JNI_INCLUDE_DIRS=-I$(JAVA_HOME_DIR)/include -I$(JAVA_HOME_DIR)/include/linux
all: libpapiprofiler.so libpapiprofilerjvm.so test

libpapiprofiler.so: PapiProfiler.o 	 
	$(CC) $(CFLAGS) -fPIC -shared $< -o $@ $(LIBPAPI_LDDIRS) $(LIBPAPI_WLRPATHS) -lpapi -lpfm 

PapiProfiler.o: PapiProfiler.cpp PapiProfiler.h
	$(CC) $(CFLAGS) $(PROFILER_INCLUDE_DIRS) -fPIC -c $< -o $@
 	
java_agent.o: java_agent.cpp
	$(CC) $(CFLAGS) $(JNI_INCLUDE_DIRS) -fPIC -c $< -o $@
	
libpapiprofilerjvm.so: PapiProfiler.o java_agent.o
	$(CC) $(CFLAGS) -fPIC -shared $^ -o $@ $(LIBPAPI_LDDIRS) $(LIBPAPI_WLRPATHS) -lpapi -lpfm 

test: test.cpp 
	$(CC) $(CFLAGS) $< -o $@ $(LIBPAPI_LDDIRS) $(LIBPAPI_WLRPATHS) -lpapi -lpfm -lpthread -L. -lpapiprofiler
	
clean:
	rm -rf test *.o *.so	
