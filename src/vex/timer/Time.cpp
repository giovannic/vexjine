/*
 * Time.cpp
 *
 *  Created on: 1 Sep 2011
 *      Author: root
 */

#include "Time.h"
#include "Constants.h"

#include <cstdlib>
#include <cstring>

int Time::bytecodeDelay = 0;
int Time::ioBytecodeDelay = 0;
int Time::interactionPointDelay = 0;
int Time::yieldDuration = 0;

long long Time::lastBackgroundLoadExecutionTime = 0;
long long Time::totalBackgroundLoadExecutionTime = 0;
long long Time::totalCollections = 0;

#include <iostream>
#include <fstream>
#include <iomanip>
using namespace std;

Time::Time() {

}

bool Time::onSystemInit() {

	// any code that should be called for the timer when the system is initialized
#if USE_PAPI == 1
	int retval;
	// Initializing PAPI
	if ((retval = PAPI_library_init(PAPI_VER_CURRENT)) != PAPI_VER_CURRENT) {
		printf("PAPI Library initialization error! \n");
		return false;
	}


	if (PAPI_thread_init(pthread_self) != PAPI_OK) {
		printf("PAPI thread Library initialisation error! \n");
		return false;
	}

#elif USE_SYS_CLOCK == 1

#elif USE_PERFCTR == 1
	PERFCTR_initialize();

#endif
	return true;
}

void Time::onSystemClose() {
#if USE_PAPI == 1
	PAPI_shutdown();
#elif USE_SYS_CLOCK == 1

#elif USE_PERFCTR == 1

#endif
}


void Time::onThreadInit() {
	// any code that should be called for the timer when a thread is initialized
#if USE_PAPI == 1
	PAPI_register_thread();
#elif USE_SYS_CLOCK == 1

#elif USE_PERFCTR == 1
	PERFCTR_register_thread();
#endif
}

bool Time::usingPapiTimer() {
	// any code that should be called for the timer when a thread is initialized
#if USE_PAPI == 1
	return true;

#elif USE_SYS_CLOCK == 1
	return false;

#elif USE_PERFCTR == 1
	return true;

#endif
}


int Time::measureYieldDuration() {
	long diff, start;
	int iterations = 10000;
	int possibleValues=2000;
	int *values = new int[possibleValues];

	for (int i =0 ; i<possibleValues; i++) {
		values[i] = 0;
	}

	int rc = 0;
	for (int i =0 ; i<iterations; ++i) {
		start = Time::getVirtualTime();
		for (int j = 0; j<1000; j++) {
			rc += sched_yield();
		}

		diff = Time::getVirtualTime() - start;
		++values[(int)(diff)/1000];
	}

	// Find median

	int sumVals =0 ;
	for (int i =0 ; i<possibleValues; i++) {
		if (sumVals < (iterations)/2) {
			sumVals += values[i];
		} else {

			if (rc >= 0) {
				delete[] values;
				yieldDuration = (i-1);
				return yieldDuration;
			}

		}
	}

	delete[] values;
	yieldDuration = possibleValues;
	return yieldDuration;
}



void Time::profileTimers() {
	int iterations = 10000000;
	long long starttime = Time::getVirtualTime();
	long long temp = 0, temp1, temp2;
	for (int i =0 ; i<iterations;i++) {
		temp1  = Time::getVirtualTime();
		temp2 = Time::getVirtualTime();
		temp += (temp2 - temp1);
	}
	long long endtime = Time::getVirtualTime()-starttime;
	cout << "10000000 getVirtualTime iterations in " << (double)((double)endtime/(double)(2.0*iterations)) << " "  << (temp/(double)iterations) << endl;

	starttime = Time::getVirtualTime();
	temp = 0;
	for (int i =0 ; i<iterations;i++) {
		temp1 = Time::getRealTime();
		temp2 = Time::getRealTime();
		temp += (temp2 - temp1);
	}
	endtime = Time::getVirtualTime()-starttime;
	cout << "10000000 getRealTime iterations in " << (double)((double)endtime/(double)(2.0*iterations)) << " "  << (temp/(double)iterations) << endl;

}




void Time::printDelays() {
	#if REMOVE_DELAY_TYPE == BYTECODE_DELAY_TYPE
	fprintf(stderr, "Lost times: per bytecode: %d - per IP: %d\n", bytecodeDelay, interactionPointDelay);
	fflush(stderr);
	#else
	fprintf(stderr, "Lost times: per method: %d - per IP: %d\n", methodDelay, interactionPointDelay);
	fflush(stderr);
	#endif
}

void Time::printDelaysToFile(std::ofstream &myfile) {
	myfile << "Compensation for instrumentation delay," << bytecodeDelay << endl;
	myfile << "Compensation for IP delay," << interactionPointDelay << endl;
	myfile << "Compensation for I/O instrumentation delay," << ioBytecodeDelay << endl;
	myfile << "Compensation for sched_yield system call," << yieldDuration << endl;
	myfile << "Total GC time [ms]," << totalBackgroundLoadExecutionTime/1e6 << endl;
	myfile << "Total collections," << totalCollections << endl;
	myfile << "Mean GC time," << (totalBackgroundLoadExecutionTime/1e6)/totalCollections << endl;
}

void Time::concisePrinting(std::ostream &out, const long long &timeValue, bool htmlEncoding) {
	if (timeValue < 1000) {
		out << timeValue << "ns";
	} else if (timeValue < 1000000) {
		out << fixed << setprecision(3) << (double)timeValue/1000.0;
		if (htmlEncoding) {
			out << "&#956s";
		} else {
			out << "\u03BCs";
		}
	} else if (timeValue < 1000000000) {
		out << fixed << setprecision(3) << (double)timeValue/1000000.0 << "ms";
	} else {
		out << fixed << setprecision(3) << (double)timeValue/1000000000.0 << "s";
	}
}



long long Time::profileDelayPerMeasurement() {
	// Profiling of cost per VT time measurement
	long long start;
	long long startingCallbackTime ;
	start = getVirtualTime();
	for(int z = 0; z<1000; z++) {
		startingCallbackTime = getVirtualTime();
	}

	// using startingCallbackTime to keep the compiler from complaining
	return (startingCallbackTime  + getVirtualTime() - start - startingCallbackTime)/1000;
}



void Time::onBackgroundLoadExecutionStart() {
	lastBackgroundLoadExecutionTime = getRealTime();
}

long long Time::getBackgroundLoadExecutionRealDuration() {
//	cout << "adding garbage collection time " << (getRealTime() - lastBackgroundLoadExecutionTime) /1e6 << " ms" <<endl;
	long long differenceTime = getRealTime() - lastBackgroundLoadExecutionTime;
	totalBackgroundLoadExecutionTime += differenceTime;
	++totalCollections;
	return differenceTime;
}


/*
 * Open file with sample delays of bytecode instrumentation and get the delay values
 */
bool Time::parseDelaysFile(const char *filename) {
	ifstream dfile(filename);
	if (dfile.is_open()) {
		string line;

		getline(dfile, line);
		bytecodeDelay = atoi(line.c_str());
		getline(dfile, line);
		interactionPointDelay = atoi(line.c_str());
		getline(dfile, line);
		ioBytecodeDelay = atoi(line.c_str());
		getline(dfile, line);
		yieldDuration = atoi(line.c_str());
		dfile.close();

	} else if (strcmp(filename, DEFAULT_DELAYS_FILE) != 0){
		fprintf(stderr, "\nError: Unable to open delays file: %s\n", filename); fflush(stderr);

	} else {
		setDelays(0, 0, 0);
	}
	return true;
}


void Time::writeDelaysToFile(const char *filename) {
	ofstream dfile(filename);
	if (dfile.is_open()) {
		dfile << bytecodeDelay << endl;
		dfile << interactionPointDelay << endl;
		dfile << ioBytecodeDelay << endl;
		dfile << yieldDuration << endl;
		dfile.close();
	}

}
