include ../Makefile.inc

ifeq ($(shell uname -m), i686)
JVMTI_LIBRARY_NAME=libJVMTIAgent_x86.so
JVMTI_LIBRARY_TRACE_NAME=libJVMTIAgent_real_trace_x86.so
JVMTI_LIBRARY_LIGHT_NAME=libJVMTIAgent_light_x86.so

else
JVMTI_LIBRARY_NAME=libJVMTIAgent.so
JVMTI_LIBRARY_TRACE_NAME=libJVMTIAgent_real_trace.so
JVMTI_LIBRARY_LIGHT_NAME=libJVMTIAgent_light.so

endif


ifeq "$(MAKECMDGOALS)" "papi"

JVMTI_LIBRARY_NAME=libJVMTIAgentpapi.so
JVMTI_LIBRARY_TRACE_NAME=libJVMTIAgent_real_tracepapi.so
JVMTI_LIBRARY_LIGHT_NAME=libJVMTIAgent_lightpapi.so

endif

ifeq "$(MAKECMDGOALS)" "nologpapi"

JVMTI_LIBRARY_NAME=libJVMTIAgentpapi.so
JVMTI_LIBRARY_TRACE_NAME=libJVMTIAgent_real_tracepapi.so
JVMTI_LIBRARY_LIGHT_NAME=libJVMTIAgent_lightpapi.so

endif

ifeq "$(MAKECMDGOALS)" "papirelease"

JVMTI_LIBRARY_NAME=libJVMTIAgentpapi.so
JVMTI_LIBRARY_TRACE_NAME=libJVMTIAgent_real_tracepapi.so
JVMTI_LIBRARY_LIGHT_NAME=libJVMTIAgent_lightpapi.so

endif

INCLUDE_DIRS+=-I$(JAVA_HOME_DIR)/include -I$(JAVA_HOME_DIR)/include/linux -I$(VEX_DIR)
CFLAGS+=-c -fPIC $(CPPFLAGS)

JVMTI_LIBRARY_TESTER_NAME=libJVMTIAgent_tester.so
JVMTI_LIBRARY_MINIMAL_NAME=libJVMTIAgent_minimal.so

JVMOBJECTS=JVMTIUtil.o JVMDirectCalls.o JVMMethods.o JVMStatistics.o JVMRegistration.o JVMSockets.o JVMThreadEvents.o JVMProfilingInvalidationEnforcer.o JVMOptionalCallbacks.o JVMOptimizer.o JVMTIAgent.o

all: $(JVMTI_LIBRARY_NAME) 

$(JVMTI_LIBRARY_NAME): $(JVMOBJECTS)
	$(CC) $(LDFLAGS) -o $(JVMTI_LIBRARY_NAME) $(JVMOBJECTS) $(VEX_WLRPATHS) $(VEX_LIBRARY)
	cp $(JVMTI_LIBRARY_NAME) $(OUTPUT_LIB_DIR)

$(JVMTI_LIBRARY_TRACE_NAME): $(OBJECTS_TRACE) JVMTIAgent_real_trace.o
	$(CC) $(LDFLAGS) -o $(JVMTI_LIBRARY_TRACE_NAME) $(OBJECTS_TRACE) JVMTIAgent_real_trace.o $(SHARED_LIBRARY_INCLUSION) $(JAVA_LIBS) 
	cp $(JVMTI_LIBRARY_TRACE_NAME) $(JAVA_LIB_OUT)

$(JVMTI_LIBRARY_TESTER_NAME): $(OBJECTS) JVMTIAgent_tester.o
	$(CC) $(JAVA_LIBS) $(LDFLAGS) -o $(JVMTI_LIBRARY_TESTER_NAME) $(OBJECTS) JVMTIAgent_tester.o $(SHARED_LIBRARY_INCLUSION) 
	cp $(JVMTI_LIBRARY_TESTER_NAME) $(JAVA_LIB_OUT)

$(JVMTI_LIBRARY_MINIMAL_NAME): $(OBJECTS) JVMTIAgent_minimal.o
	$(CC)$(LDFLAGS) -o $(JVMTI_LIBRARY_MINIMAL_NAME) $(OBJECTS) JVMTIAgent_minimal.o $(SHARED_LIBRARY_INCLUSION)  $(JAVA_LIBS) 
	cp $(JVMTI_LIBRARY_MINIMAL_NAME) $(JAVA_LIB_OUT)

$(JVMTI_LIBRARY_LIGHT_NAME): $(OBJECTS) JVMTIAgent_vtonly.o
	$(CC) $(LDFLAGS) -o $(JVMTI_LIBRARY_LIGHT_NAME) $(OBJECTS) JVMTIAgent_vtonly.o $(PAPI_LIBRARIES) $(JAVA_LIBS)  $(SHARED_LIBRARY_INCLUSION) 
	cp $(JVMTI_LIBRARY_LIGHT_NAME) $(JAVA_LIB_OUT)

JVMTIAgent.o: JVMTIAgent.cpp
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) $< -o $@ 

JVMDirectCalls.o: JVMDirectCalls.cpp
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) $< -o $@

JVMMethods.o: JVMMethods.cpp
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) $< -o $@

JVMOptionalCallbacks.o: JVMOptionalCallbacks.cpp
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) $< -o $@

JVMProfilingInvalidationEnforcer.o: JVMProfilingInvalidationEnforcer.cpp JVMProfilingInvalidationEnforcer.h
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) $< -o $@

JVMRegistration.o: JVMRegistration.cpp
	$(CC) $(CFLAGS)  $(INCLUDE_DIRS) $< -o $@

JVMStatistics.o: JVMStatistics.cpp JVMStatistics.h
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) $< -o $@

JVMSockets.o: JVMSockets.cpp
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) $< -o $@

JVMThreadEvents.o: JVMThreadEvents.cpp
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) $< -o $@

JVMOptimizer.o: JVMOptimizer.cpp
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) $< -o $@

JVMTIUtil.o: JVMTIUtil.cpp JVMTIUtil.h
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) $< -o $@

JVMTIAgent_real_trace.o: JVMTIAgent_real_trace.cpp
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) $< -o $@

JVMTIAgent_tester.o: JVMTIAgent_tester.cpp
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) $< -o $@

JVMTIAgent_minimal.o: JVMTIAgent_minimal.cpp
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) $< -o $@

JVMTIAgent_vtonly.o: JVMTIAgent_vtonly.cpp
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) $< -o $@

clean:
	rm -rf *.o *.jnilib *.so runner runner.* VTFServer

