/*
 * PapiProfiler.h
 *
 *  Created on: 9 Jun 2011
 *      Author: nb605
 */

#ifndef PAPIPROFILER_H_
#define PAPIPROFILER_H_

#include <string>
#include <ostream>
#include <cstring>
#include <pthread.h>
#include <vector>
#include <sys/resource.h>

class PapiProfileRecords {
public:

	PapiProfileRecords(const char *name, short _measures) {
		measures = _measures;
		papimeasures = new long long[measures];
		for (short i = 0; i<_measures; i++) {
			papimeasures[i]=0;
		}
		cpuTime = 0;
		realTime = 0;
		cpuToReal = 0;
		CPI = 0 ;
		cacheMisses = 0;
		strcpy(threadName, name);
	}

	PapiProfileRecords(const char *name, long long int *_papimeasures, short _measures, long long int _cpuTime, long long int _realTime) {
//		papimeasures = _papimeasures;
		measures = _measures;
		papimeasures = new long long[measures];
		for (short i = 0; i<_measures; i++) {
			papimeasures[i]= _papimeasures[i];
		}
		cpuTime = _cpuTime;
		realTime = _realTime;
		measures = _measures;
		cpuToReal = 0;
		CPI = 0 ;
		cacheMisses = 0;

		strcpy(threadName, name);
	}

	~PapiProfileRecords() {
		delete[] papimeasures;
	}

	void calculateInfo() {
		if (realTime > 0) {
			cpuToReal = (double)cpuTime/(double)realTime;
		} else {
			cpuToReal = 0;
		}
		if (papimeasures[1] > 0) {
			CPI = (double)(papimeasures[0])/(double)(papimeasures[1]) ;
		} else {
			CPI = 0;
		}
		if (papimeasures[3] > 0) {
			cacheMisses = (double)(papimeasures[2])/(double)(papimeasures[3]);
		} else {
			cacheMisses = 0;
		}
	}

	friend std::ostream & operator <<(std::ostream &outs, const PapiProfileRecords &prof) {
		outs << prof.threadName << "," <<  prof.cpuTime/1000000 << "," << prof.realTime/1000000;
		outs << "," << prof.cpuToReal << "," << prof.CPI << "," << prof.cacheMisses;
		//outs << "," << prof.cpuTime/1000000 << "," << prof.realTime/1000000;
		for (short i = 0; i<prof.measures; i++) {
			outs << "," << prof.papimeasures[i];
		}

		return outs;
	}

	PapiProfileRecords &operator+=(const PapiProfileRecords &prof) {
		for(short i=0; i <measures ; i++) {
			papimeasures[i] += prof.papimeasures[i];
		}

		cpuToReal += prof.cpuToReal;
		CPI += prof.CPI;
		cacheMisses += prof.cacheMisses;

		cpuTime += prof.cpuTime;
		realTime += prof.realTime;

		return *this;
	}

	void average(const int &count) {
		for(short i=0; i <measures ; i++) {
			papimeasures[i] /= (double)count;
		}

		cpuToReal /= (double)count;
		CPI /= (double)count;
		cacheMisses /= (double)count;

		cpuTime /= (double)count;
		realTime /= (double)count;
	}

private:
	short measures;
	long long int *papimeasures;
	long long int cpuTime;
	long long int realTime;
	double cpuToReal;
	double CPI;
	double cacheMisses;
	char threadName[64];
};

class PapiProfiler {
public:
	PapiProfiler(bool initializeCounters);

	void addEvents();
	std::string getHardwareInfo();
	std::string getAvailablePapiEvents();
	void getTotalMeasurements();
	void getTotalMeasurements(const char *filename);

	virtual ~PapiProfiler();
	void onThreadStart();
	void onThreadEnd(const char *threadName);
	void onThreadEnd();

private:
	int enum_add_native_events( int *num_events, int **evtcodes );
	void add_single_event(const char *eventName, int &);
	int add_selected_events( int *num_events, int **evtcodes );

	void outputResults(std::ostream &outs);
	void startPapiCounterMeasurement();
	long long int *stopPapiCounterMeasurement();


	int count;
	int totalThreads;
	long long int *totalValues;
	long long int totalCpuTime;
	pthread_mutex_t mutex;

	static __thread long long int startingThreadCpuTime;
	static __thread long long int startingThreadRealTime;
	static __thread int eventSet;
	static __thread struct rusage resourceUsage;

	bool papiInitializedSuccess;
	std::vector<PapiProfileRecords *> data;
};

#endif /* PAPIPROFILER_H_ */
