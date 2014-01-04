include ../../../../../src/Makefile.inc

all:
	g++ -o libnativesim.so -shared -fPIC -I${JAVA_HOME_DIR}/include -I${JAVA_HOME_DIR}/include/linux vtf_tests_forwardleap_utils_NativeWaitingSimulation.cpp -lrt -lpthread

clean:
	rm libnativesim.so
