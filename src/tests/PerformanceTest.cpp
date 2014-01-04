/*
 * PerformanceTest.cpp
 *
 * Measure VEX's overhead on method and I/O method instrumentation.
 * Investigate sensitivity of various overhead compensation values for 3 different short methdods within a loop.
 *
 *  Created on: 12 Nov 2011
 *      Author: nb605
 *
 */

#include "PerformanceTest.h"
#include "AgentTester.h"
#include <cstdlib>

#include "../profiler/PapiProfiler.h"
#define PERFORMANCE_TEST_METHOD_ID 101

PerformanceTest::PerformanceTest() {

}

PerformanceTest::~PerformanceTest() {

}

// Classes to measure the overhead
void MethodsPerformanceTest::run(int loops) {
	for (int i = 0; i<loops; i++) {
		methodEventsBehaviour->afterMethodEntry(PERFORMANCE_TEST_METHOD_ID);
		methodEventsBehaviour->beforeMethodExit(PERFORMANCE_TEST_METHOD_ID);
	}
}

void IoPerformanceTest::run(int loops) {
	for (int i = 0; i<loops; i++) {
		methodEventsBehaviour->afterIoMethodEntry(4, 101010, false);
		methodEventsBehaviour->beforeIoMethodExit(4);
	}
}


// Loop-classes to show the effect of time compensation
double InstrLoopsTest::getNext(int i) {
	methodEventsBehaviour->afterMethodEntry(102);
	double temp = (double)(i * i) / (i+1);
	methodEventsBehaviour->beforeMethodExit(102);
	return temp;
}

void InstrLoopsTest::run(int loops) {
	int methodId = 101;
	eventLogger->registerMethod("loopingMethodPerformanceTest", methodId);
	methodEventsBehaviour->afterMethodEntry(methodId);
	e = 1.0;
	for (int i = 1; i<loops; i++) {
		e += getNext(i);
	}
	methodEventsBehaviour->beforeMethodExit(methodId);

}

double LoopsTest::getNext(int i) {
	double temp = (double)(i * i) / (i+1);
	return temp;
}

void LoopsTest::run(int loops) {
	e = 1.0;
	for (int i = 1; i<loops; i++) {
		e += getNext(i);
	}
}


static unsigned long next = 1;

// And the effect of different code to the compensation itself
double InstrLoopsTest2::getNext(int i) {
	methodEventsBehaviour->afterMethodEntry(102);
	next = next * 1103515245 + 12345;
	double temp = ((unsigned)(next/65536) % 32768) % i;
	methodEventsBehaviour->beforeMethodExit(102);
	return temp;
}

double LoopsTest2::getNext(int i) {
	next = next * 1103515245 + 12345;
	double temp = ((unsigned)(next/65536) % 32768) % i;
	return temp;
}

double InstrLoopsTest3::getNext(int i) {
	methodEventsBehaviour->afterMethodEntry(102);
	double temp = rand();
	methodEventsBehaviour->beforeMethodExit(102);
	return temp;
}

double LoopsTest3::getNext(int i) {
	double temp = rand();
	return temp;
}


int main(int argc, char **argv) {

	if (argc != 2 && argc != 3) {
		cout << "Syntax: ./perftest <methods|io|loops[1-3]> [set to enable VEX|\"profiling_HPC\" for real time]" << endl;
		cout << "methods:\t\tTest 1: overhead of VEX's method instrumentation (afterMethodEntry and beforeMethodExit) in nanoseconds" << endl;
		cout << "io:\t\t\tTest 2: overhead of VEX's I/O method instrumentation (afterIoMethodEntry and beforeIoMethodExit) in nanoseconds" << endl;
		cout << "loops1, loops2, loops3:\tTest 3: VEX for a short method (1 to 3) within a loop to demonstrate how sensitive these cases are to overhead compensation" << endl;

		return -1;
	}

	long long start = 0, end = 0;

	char options[] = {"file=options"};
	VEX::initializeSimulator(options);

	eventLogger->registerMethod("main", 100);
	threadEventsBehaviour->onThreadMainStart(12521);
	methodEventsBehaviour->afterMethodEntry(100);

	PerformanceTest *ptest;
	if (strncmp(argv[1], "loops", 5) == 0) {

		// "loops*" && argc == 2 is real execution of loopstest
		if (argc == 2 || strcmp(argv[2], "profiling_HPC") == 0) {
			PapiProfiler *profiler = NULL;
			if (argc > 2 && strcmp(argv[2], "profiling_HPC") == 0) {
				profiler = new PapiProfiler(true);
				profiler->onThreadStart();
			}


			if (strcmp(argv[1], "loops1") == 0) {
				ptest = new LoopsTest();
			} else if (strcmp(argv[1], "loops2") == 0) {
				ptest = new LoopsTest2();
			} else {
				ptest = new LoopsTest3();
			}
			start = Time::getRealTime();
			ptest->run(100000000);
			end = Time::getRealTime();
			cout << (end-start)/1e9 << " sec (real)" << endl;
			if (profiler != NULL) {
				profiler->onThreadEnd("main");
				string profilerHpcFilename(argv[1]);
				profilerHpcFilename.append("_profiling_HPC.csv");
				profiler->getTotalMeasurements(profilerHpcFilename.c_str());
			}

		// "loops*" && argc == 3 is VEX execution of loopstest
		} else {
			if (strcmp(argv[1], "loops1") == 0) {
				ptest = new InstrLoopsTest();
			} else if (strcmp(argv[1], "loops2") == 0) {
				ptest = new InstrLoopsTest2();
			} else {
				ptest = new InstrLoopsTest3();
			}
			start = threadEventsBehaviour->getTime();
			ptest->run(10000000);
			end = threadEventsBehaviour->getTime();
			cout << (end-start)/1e9 << " sec (VEX)" << endl;
			methodEventsBehaviour->beforeMethodExit(100);
		}
		delete ptest;
		return 2;

	} else {
		if (strcmp(argv[1], "methods") == 0) {
			ptest = new MethodsPerformanceTest();

		} else if (strcmp(argv[1], "io") == 0) {
			ptest = new IoPerformanceTest();

		} else {
			cout << "Wrong value for test: acceptable values \"methods\" or \"io\" or \"loops\"" << endl;
			return -1;
		}

		eventLogger->registerMethod("performanceTestMethod", PERFORMANCE_TEST_METHOD_ID);

		if (argc == 2) {
			// argc == 2 is a real time execution
			start = Time::getRealTime();
			ptest->run(10000000);
			end = Time::getRealTime();

		} else {
			// argc == 3 is VEX execution
			start = threadEventsBehaviour->getTime();	// the getTime returns VEX time
			ptest->run(10000000);						// for these empty-bodied methods the total VEX time should be 0
			end = threadEventsBehaviour->getTime();		// the fact that it is not is the compensation that needs to be added

		}

		methodEventsBehaviour->beforeMethodExit(100);
		cout << (end-start)/(10000000) << endl;

	}

	delete ptest;

	if (argc == 3 && strcmp(argv[2], "profiling_HPC") != 0) {
		threadEventsBehaviour->onEnd();
		VEX::endSimulator();
	}

	return 0;
}
