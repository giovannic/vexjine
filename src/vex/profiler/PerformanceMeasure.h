/*
 * PerformanceMeasure.h: Class that receives samples of performance data and calculates the mean of the samples
 *
 *  Created on: Jul 30, 2008
 *      Author: richard
 */

#ifndef PERFORMANCEMEASURE_H_
#define PERFORMANCEMEASURE_H_

#define MAXIMUM_SAMPLES_PER_MEASURE 1
#define INSTRUMENTATION_OVERHEAD_PROFILING_METHOD_ID 50	// Hard-coded method id of the method used for profiling of instrumentation delays

#include <fstream>
#include <iomanip>
#include <vector>
#include <iostream>
#include <typeinfo>

//using namespace std;


/***
 * The SampleStorageBehaviour is an interface that defines how the samples for each performance measure (which can be on a method or stack-trace
 * level) need to be stored. It might be indifferent to store each sample (NoSampleStorage), or we might want a limited number of samples
 * (LimitedSamplesStorage) or an unlimited one (UnlimitedSamplesStorage), though the latter might bear a heavy memory overhead
 */
class SampleStorageBehaviour {
	friend std::ostream & operator<< (std::ostream &outs, const SampleStorageBehaviour &record) {
		return record.printSamples(outs);
	};
public:
	SampleStorageBehaviour() {
		samples = 0;
	}
	virtual void init() = 0;
	virtual void addSample(const long long &sample) = 0;

	unsigned int const &getSamples() const {
		return samples;
	}

	virtual SampleStorageBehaviour& operator+=(SampleStorageBehaviour& ssb) {
		samples += ssb.samples;
		return *this;
	}

	virtual ~SampleStorageBehaviour() {

	}
protected:
	virtual std::ostream &printSamples(std::ostream &outs) const {
		return outs;
	}
	unsigned int samples;
};

class NoSampleStorage : public SampleStorageBehaviour {
public:
	NoSampleStorage() : SampleStorageBehaviour() {};
	void init() {};
	void addSample(const long long &sample) {
		++samples;
	};
};

class LimitedSamplesStorage : public SampleStorageBehaviour {
friend std::ostream & operator<<(std::ostream &outs, const LimitedSamplesStorage &record);

public:
	LimitedSamplesStorage() : SampleStorageBehaviour() {};
	void init() {
		for(int i =0; i < MAXIMUM_SAMPLES_PER_MEASURE; i++) {
			storedSamples[i] = 0;
		}
	}
	void addSample(const long long &sample) {
		if (samples < MAXIMUM_SAMPLES_PER_MEASURE) {
			storedSamples[samples] = sample;
		}
		++samples;
	};

	SampleStorageBehaviour& operator+=(SampleStorageBehaviour& ssb) {
//		try {
			LimitedSamplesStorage &ssb_temp = dynamic_cast<LimitedSamplesStorage &>(ssb);
			int count = samples, count2 = 0;
			while (count < MAXIMUM_SAMPLES_PER_MEASURE) {
				storedSamples[count] 	= ssb_temp.storedSamples[count2];
			}
//		} catch (std::bad_cast exc) {

//		}
		samples += ssb.getSamples();
		return *this;
	}

protected:
	virtual std::ostream &printSamples(std::ostream &outs) const {
		outs << ",,Stored samples:";
		for (unsigned int i = 0; i < samples; i++) {
			outs << "," << storedSamples[i];
		}
		return outs;
	}

	long long storedSamples[MAXIMUM_SAMPLES_PER_MEASURE];
};






class PerformanceMeasure {
	friend std::ostream & operator << (std::ostream &outs, PerformanceMeasure &record) {
		return record.printMeansInMSec(outs);//monitorWaitExcludingCalleeMethods << "," << record.ioWaitExcludingCalleeMethods  << "," << record.monitorWaitIncludingCalleeMethods  << "," << record.ioWaitIncludingCalleeMethods << "," << record.cpuTimeExcludingCalleeMethods << "," << record.estimatedRealTimeExcludingCalleeMethods << "," << record.cpuTimeIncludingCalleeMethods << "," << record.estimatedRealTimeIncludingCalleeMethods << ",";

	};
public:

	PerformanceMeasure(const int &methodId);
	PerformanceMeasure(const int &methodId, PerformanceMeasure *_callingMethod);

	virtual ~PerformanceMeasure();

	/**Add a sample - a VT and an estimated RT. Summed values refer to caller+callee methods */
	void add(const long long &virtual_value, const long long &real_value, const long long &sample_cpuTimeIncludingCalleeMethods, const long long &sample_estimatedRealTimeIncludingCalleeMethods, const long long &monWait, const long long &ioWait, const long long &incl_monWait, const long long &incl_ioWait);
	void moreInfo(long long _callee_time, long long _callee_lost, long long _lost_time);
	PerformanceMeasure& operator+=(PerformanceMeasure& callinfo);

	/**Get the mean of the samples to date.*/
	double getMean(const long long &metric) const;

	/**Get the number of samples to date.*/
	unsigned int const &getMethodInvocations() const;
	long long getCpuTimeExcludingCalleeMethods();
	long long getCpuTimeIncludingCalleeMethods();
	long long getEstimatedRealTimeIncludingCalleeMethods();


	const char *getMethodNameString() {
		if (methodName != NULL) {
			return methodName;
		} else {
			return "Unknown method from PM";
		}
	}

	int getMethodId();
	void setMethodId(const int &_methodId);

	double getMeanERT() {
		return getMean(estimatedRealTimeIncludingCalleeMethods);
	}

	std::ostream & printMeansInMSec(std::ostream &outs) {
		// This fix has to do with the fact that I/O is also added as localTime, which increases CPU time as well - we compensate for this here
		cpuTimeExcludingCalleeMethods -= ioWaitExcludingCalleeMethods; if (cpuTimeExcludingCalleeMethods < 0) cpuTimeExcludingCalleeMethods = 0;
		long long suspendedTimeExcludingCalleeMethods = estimatedRealTimeExcludingCalleeMethods - cpuTimeExcludingCalleeMethods - monitorWaitExcludingCalleeMethods - ioWaitExcludingCalleeMethods;

		// This fix has to do with the fact that I/O is also added as localTime, which increases CPU time as well - we compensate for this here
		cpuTimeIncludingCalleeMethods -= ioWaitIncludingCalleeMethods; if (cpuTimeIncludingCalleeMethods < 0) cpuTimeIncludingCalleeMethods = 0;
		long long suspendedTimeIncludingCalleeMethods = estimatedRealTimeIncludingCalleeMethods - cpuTimeIncludingCalleeMethods - monitorWaitIncludingCalleeMethods - ioWaitIncludingCalleeMethods;

		outs << std::fixed << std::setprecision(3) <<
			getMean(estimatedRealTimeExcludingCalleeMethods)/1e6 << "," <<
			getMean(cpuTimeExcludingCalleeMethods)/1e6 << "," <<
			getMean(monitorWaitExcludingCalleeMethods)/1e6 << "," <<
			getMean(ioWaitExcludingCalleeMethods)/1e6  << "," <<
			getMean(suspendedTimeExcludingCalleeMethods)/1e6  << "," <<

			getMean(estimatedRealTimeIncludingCalleeMethods)/1e6 << "," <<
			getMean(cpuTimeIncludingCalleeMethods)/1e6 << "," <<
			getMean(monitorWaitIncludingCalleeMethods)/1e6  << "," <<
			getMean(ioWaitIncludingCalleeMethods)/1e6 << "," <<
			getMean(suspendedTimeIncludingCalleeMethods)/1e6 << *ertSamplesStorage;
		return outs;
	};

	inline bool isTheInstrumentationProfilingMethod() {
		return methodId == INSTRUMENTATION_OVERHEAD_PROFILING_METHOD_ID;
	};

	void addSubmethodTrace(PerformanceMeasure *submethodTraceMeasure) {
		if (subMethods == NULL) {
			subMethods = new std::vector<PerformanceMeasure *>();
		}
		subMethods->push_back(submethodTraceMeasure);
		submethodTraceMeasure->setCallingMethod(this);
	}

	std::vector<PerformanceMeasure *> *initGetSubMethods() {
		if (subMethods == NULL) {
			subMethods = new std::vector<PerformanceMeasure *>;
		}
		return subMethods;
	}

	bool isRoot() {
		return callingMethod == NULL;
	}

	void setCallingMethod(PerformanceMeasure *_callingMethod) {
		callingMethod = _callingMethod;
	}

	PerformanceMeasure *getCallingMethod() {
		return callingMethod;
	}

	std::vector<PerformanceMeasure *> *getSubMethods() {
		return subMethods;
	}

	void setSubMethods(std::vector<PerformanceMeasure *> *_subMethods) {
		subMethods = _subMethods;
	}

	// Related to whether the method described by this measure is recursive or not
	void setRecursive(bool value);
	bool isRecursive();

	const char *getMethodName() {
		return methodName;
	}

	void setMethodName(const char *_methodName) {
		methodName = _methodName;
	}
protected:
	void init(const int &_methodId);

	const char *methodName;
	int methodId;

	PerformanceMeasure *callingMethod;
	std::vector<PerformanceMeasure *> *subMethods;

	// Flags used for recursive adaptations
	bool recursive;
	bool alreadyAdapted;

	SampleStorageBehaviour *ertSamplesStorage;

	long long cpuTimeExcludingCalleeMethods;
	long long estimatedRealTimeExcludingCalleeMethods;
	long long monitorWaitExcludingCalleeMethods;
	long long ioWaitExcludingCalleeMethods;

	long long cpuTimeIncludingCalleeMethods;
	long long estimatedRealTimeIncludingCalleeMethods;
	long long monitorWaitIncludingCalleeMethods;
	long long ioWaitIncludingCalleeMethods;

	long long callee_time;
	long long callee_lost_time;
	long long lost_time;

};






































#endif /* PERFORMANCEMEASURE_H_ */
