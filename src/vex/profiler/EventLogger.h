/*
 * EventLogger.h
 *
 *  Created on: Jul 02, 2010
 *      Author: nb605
 */


#ifndef EVENTLOGGER_H_
#define EVENTLOGGER_H_

#include <tr1/unordered_map>

#include "Constants.h"
class VexThreadState;
class MethodData;
class ProfilingInvalidationPolicy;
class PerformanceMeasure;
#include <pthread.h>
#include <fstream>
#include <vector>
//#include "ProfilingInvalidationPolicy.h"


//using namespace std;
//using namespace tr1;

class EventLogger {
	
public:
	EventLogger();
	virtual ~EventLogger();

	virtual void reset();

	// Move to OutputWriter.class
	void writeData(const char *filename, const int &outputFormat, long long totalTime);
	virtual void writeDataVTFStyle(const char *filename, long long totalTime);
	void writeDataHprofStyle(const char *filename, long long totalTime);
	void writeMethods(const char *filename);

	void setPrunePercentage(double pp);

	void registerMethod(const char *methodName, int methodId);
	void registerMethod(const char *_fqName, int methodId, int methodId0);

	int getRegisteredMethodId(const char *_fqName);
	const char *getMethodName(const int &methodId);
	int getMethod0(const int &methodId);		// return the methodId of the first method of the class that is instrumented

	void pmethod(int methodId);

	void printMethodRegistry(const char *filename, ProfilingInvalidationPolicy *invalidationPolicyUsed);

	long long createMeasures();
	virtual void createMeasuresForThread(VexThreadState *state);
	virtual void onThreadEnd(VexThreadState *state);

	MethodData *getMethodDataOf(const int &methodId);

//	unordered_map<int, MethodData *> *getRegisteredMethodNames();

	static const int TYPICAL_VTF_OUTPUT;
	static const int HPROF_LIKE_OUTPUT;

	void setRegistryStats(const unsigned int & _totalThreads, const unsigned long &_posixSignalsSent, const unsigned long &_allThreadsInvocationPoints);
protected:
	pthread_cond_t cond;
	pthread_mutex_t mutex;

	// Global event map data structure
	int measuresLock();
	int measuresUnlock();

	long allThreadsInvocationPoints;
	unsigned int maximumStackDepth;
	
	void printExperimentLegend(std::ofstream &myfile, const long long &estimatedExecutionTime, const unsigned long &summedInvocations);
	long long serializedExecutionTime;
	std::tr1::unordered_map<int, MethodData *> registeredMethodNames;

	double prunePercentage;

	unsigned int totalThreadsControlled;
	unsigned long posixSignalsSent;
	long long startingRealTime;

private:
	// long: threadId	     int: methodId
	std::tr1::unordered_map<long, std::tr1::unordered_map<int, PerformanceMeasure*> *> allThreadMeasures;
	std::tr1::unordered_map<int, PerformanceMeasure*> measures;


};


/*
 * Subclass used when stack trace is enabled
 */
class StackTraceLogger : public EventLogger {

public:
	StackTraceLogger() : EventLogger() {
		measuresRoots = NULL;
	};
	virtual ~StackTraceLogger();

	virtual void writeDataVTFStyle(const char *filename, long long totalTime);

	virtual void createMeasuresForThread(VexThreadState* state);
	virtual void reset();

	virtual void onThreadEnd(VexThreadState *state);

private:
	void outputStackTraceLevel(std::vector<PerformanceMeasure *> *submethodsMeasures, unsigned int level, std::ofstream & output);
	void toHTML(std::vector<PerformanceMeasure *> *submethodsMeasures, unsigned int &lastNodeId, std::ofstream & output);

	// long: threadId	     int: methodId
	std::tr1::unordered_map<long, std::tr1::unordered_map<long double, PerformanceMeasure*> *> allThreadMeasures;
	std::vector<PerformanceMeasure *> *measuresRoots;

	// Recursive methods to parse the measures tree
	void mergeMeasures(std::vector<PerformanceMeasure *> &globalNode, std::vector<PerformanceMeasure *> *threadNode);
	unsigned long sortSubmethodsAndGetSamples(std::vector<PerformanceMeasure *> *methodsMeasures);
};

#endif /* EVENTLOGGER_H_ */
