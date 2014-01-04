/*
 * MethodCallInfo.h
 *
 *  Created on: 17 Feb 2010
 *      Author: nb605
 */

#ifndef METHODCALLINFO_H_
#define METHODCALLINFO_H_

#include "PerformanceMeasure.h"
#include "MethodData.h"

class MethodCallInfo {
public:
	void setRecursive() {
		recursive = true;
	}

	bool isRecursive() {
		return recursive;
	}

	MethodCallInfo();
	MethodCallInfo(const int &_methodId, const long long &virtualTime, const long long &estimatedRealTime, const long long &monitorWaitingTime, const long long &ioWaitingTime);

	void setInfo(const int &_methodId, const long long &virtualTime, const long long &estimatedRealTime, const long long &monitorWaitingTime, const long long &ioWaitingTime);
	MethodCallInfo& operator-= ( const MethodCallInfo& callinfo );
	virtual ~MethodCallInfo();
	int getMethodId();

	long long getTime();
	long long getRealTime();

	long long getTotalLostTime() {
		return lost_time + lost_time_callee;
	}

//	int *createNewCurrentStackTrace(int *stackDepth);


	inline void setShouldResetVTFactor(bool value) {
		shouldResetVTFactor = value;
	}

	inline bool getShouldResetVTFactor() {
		return shouldResetVTFactor;
	}

	inline void updateCalleeTime(const long long &diff, const long long &real_diff, const long long &callee_lost_time, const long long &callee_monitor_wait, const long long &callee_io_wait) {
		callee_time		 += diff;
		real_callee_time += real_diff;
//		lost_time_callee += callee_lost_time;
		callee_monitor_waiting_timestamp 	+= callee_monitor_wait;
		callee_io_waiting_timestamp 		+= callee_io_wait;
	};

	inline long long getInclusiveCpuTimeDifferenceFrom(MethodCallInfo *start) {
		return entry_timestamp - start->entry_timestamp;
	};

	inline long long getInclusiveRealTimeDifferenceFrom(MethodCallInfo *start) {
		return real_entry_timestamp - start->real_entry_timestamp;
	};

	long long getInclusiveMonitorWaitDifferenceFrom(MethodCallInfo *start) {
		return monitor_waiting_timestamp - start->monitor_waiting_timestamp;
	};

	long long getInclusiveIoWaitDifferenceFrom(MethodCallInfo *start) {
		return io_waiting_timestamp - start->io_waiting_timestamp;
	};

	void logTimesOnMethodExit(PerformanceMeasure *measure, MethodCallInfo *entryCallinfo, MethodCallInfo *callingMethodInfo);
//	void decreaseThreadsExecutingMethodCounter();
//	void setGlobalMethodDataOnEntry(MethodData *data);
protected:
	//virtual timestamp at method entry
	long long entry_timestamp;

	//estimated real timestamp at method entry
	long long real_entry_timestamp;

	//CPU time spent in callee methods
	long long callee_time;

	//estimated real time spent in callee methods
	long long real_callee_time;

	// maximum predicted times from callee methods - VT from callee methods lower than this should
	// not be taken into account for this method - since 2.8
	long long maximumPredictedVT;
	long long maximumEstimatedRealTime;

	// time waiting for a monitor acquisition
	long long monitor_waiting_timestamp;
	// time waiting for a I/O completion
	long long io_waiting_timestamp;

	long long callee_monitor_waiting_timestamp;
	// time waiting for a I/O completion
	long long callee_io_waiting_timestamp;

	long long lost_time;
	long long lost_time_callee;
	int methodId;

	int method_trace_ptr;				// 4 bytes pointing to the length of the string in the current_method_trace_bythread
										// that corresponds to this events method starting occurance (used for pop)

	bool shouldResetVTFactor;	// set on the method that induced the change of the VT factor
	bool recursive;
	MethodCallInfo *callingMethod;
	MethodData *methodData;
};




class StackTraceInfo : public MethodCallInfo {
public:
	StackTraceInfo() : MethodCallInfo() {
		correspondingStackTraceMeasure = 0;
	}

	StackTraceInfo(const int &_methodId, const long long &virtualTime, const long long &estimatedRealTime, const long long &monitorWaitingTime, const long long &ioWaitingTime, PerformanceMeasure *_correspondingStackTraceMeasure) :
		MethodCallInfo(_methodId, virtualTime, estimatedRealTime, monitorWaitingTime, ioWaitingTime) {
		correspondingStackTraceMeasure = _correspondingStackTraceMeasure;
	}

	void setInfo(const int &_methodId, const long long &virtualTime, const long long &estimatedRealTime, const long long &monitorWaitingTime, const long long &ioWaitingTime, PerformanceMeasure *_correspondingStackTraceMeasure);

	~StackTraceInfo();

	inline PerformanceMeasure *getCorrespondingStackTraceMeasure() {
		return correspondingStackTraceMeasure;
	}

private:
	PerformanceMeasure *correspondingStackTraceMeasure;
};



// Extends MethodCallInfo to keep the long double that denotes the path to the trace
class PrimeNumberStackTraceInfo : public MethodCallInfo {
public:
	PrimeNumberStackTraceInfo() : MethodCallInfo() {
		traceId = 0;
	}

	PrimeNumberStackTraceInfo(const int &_methodId, const long long &virtualTime, const long long &estimatedRealTime, const long long &monitorWaitingTime, const long long &ioWaitingTime, const long double &_traceId) :
		MethodCallInfo(_methodId, virtualTime, estimatedRealTime, monitorWaitingTime, ioWaitingTime) {
		traceId = _traceId;
	}

	~PrimeNumberStackTraceInfo();
	void setInfo(const int &_methodId, const long long &virtualTime, const long long &estimatedRealTime, const long long &monitorWaitingTime, const long long &ioWaitingTime, const long double &_traceId);

private:
	long double traceId;
};


#endif /* METHODCALLINFO_H_ */
