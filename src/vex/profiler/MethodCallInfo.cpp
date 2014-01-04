/*
 * MethodCallInfo.cpp
 *
 *  Created on: 17 Feb 2010
 *      Author: nb605
 */

#include "MethodCallInfo.h"
#include <cassert>

MethodCallInfo::MethodCallInfo() {
	setInfo(0, 0, 0, 0, 0);
	callingMethod = NULL;
	methodData = NULL;
}

MethodCallInfo::MethodCallInfo(const int &_methodId, const long long &virtualTime, const long long &estimatedRealTime, const long long &monitorWaitingTime, const long long &ioWaitingTime) {

	setInfo(_methodId, virtualTime, estimatedRealTime, monitorWaitingTime, ioWaitingTime);
	callingMethod = NULL;
	methodData = NULL;
}

void MethodCallInfo::setInfo(const int &_methodId, const long long &virtualTime, const long long &estimatedRealTime, const long long &monitorWaitingTime, const long long &ioWaitingTime) {

	methodId = _methodId;

	entry_timestamp = virtualTime;
	real_entry_timestamp = estimatedRealTime;

	monitor_waiting_timestamp = monitorWaitingTime;
	io_waiting_timestamp = ioWaitingTime;

	//time spent in callee methods
	callee_time = 0;
	real_callee_time = 0;

	lost_time = 0;
	lost_time_callee = 0;

	callee_monitor_waiting_timestamp = 0;
	callee_io_waiting_timestamp = 0;

	shouldResetVTFactor = false;	// used to let know when the VT factor should be reset to 1
	// the methodName check is no longer used since 2.55

	maximumPredictedVT = 0;		// used to keep correct accounting when callee methods
	maximumEstimatedRealTime = 0; // make I/O predictions and their methods overlap in execution

	callee_monitor_waiting_timestamp = 0;
	callee_io_waiting_timestamp = 0;

	recursive = false;
}


// Used to find the difference between two timestamps
MethodCallInfo& MethodCallInfo::operator-= ( const MethodCallInfo& callinfo ) {
	entry_timestamp = real_entry_timestamp - callinfo.entry_timestamp - callinfo.lost_time;
	real_entry_timestamp = real_entry_timestamp - callinfo.real_entry_timestamp - callinfo.lost_time;
	return *this;
}

int MethodCallInfo::getMethodId() {
	return methodId;
}

long long MethodCallInfo::getTime() {
	return entry_timestamp - lost_time;
}

long long MethodCallInfo::getRealTime() {
	return real_entry_timestamp - lost_time;
}


//int *MethodCallInfo::createNewCurrentStackTrace(int *stackDepth) {
//	MethodCallInfo *temp = callingMethod;
//	*stackDepth = 0;
//	while (temp != NULL) {
//		++(*stackDepth);
//		temp = temp -> callingMethod;
//	}
//
//	if (*stackDepth != 0) {
//		int *currentMethodStackTrace = new int[*stackDepth];
//		int i = 0;
//		temp = callingMethod;
//		while (temp != NULL) {
//			currentMethodStackTrace[i++] = temp -> methodId;
//			temp = temp -> callingMethod;
//		}
//
//		return currentMethodStackTrace;
//	}
//	return NULL;
//}


void MethodCallInfo::logTimesOnMethodExit(PerformanceMeasure *measure, MethodCallInfo *entryCallinfo, MethodCallInfo *callingMethodInfo) {

	// get time difference of exiting to entry event
	long long methodCpuTimeIncludingCallee				= getInclusiveCpuTimeDifferenceFrom(entryCallinfo);
	long long methodEstimatedRealTimeIncludingCallee 	= getInclusiveRealTimeDifferenceFrom(entryCallinfo);
//	long long totalLostTime 							= entryCallinfo->getTotalLostTime();

	long long monitorWaitingTimeIncludingCallee			= getInclusiveMonitorWaitDifferenceFrom(entryCallinfo);
	long long ioWaitingTimeIncludingCallee 				= getInclusiveIoWaitDifferenceFrom(entryCallinfo);

	if (callingMethodInfo != NULL) {
		callingMethodInfo -> updateCalleeTime(methodCpuTimeIncludingCallee, methodEstimatedRealTimeIncludingCallee, 0, monitorWaitingTimeIncludingCallee, ioWaitingTimeIncludingCallee);
	}
//	if (totalLostTime > 0) {
//		assert(false);
//		cout << totalLostTime << endl;
//	}
	long long methodCpuTimeIncludingCalleeWithoutLost   		= methodCpuTimeIncludingCallee 	 			;//- totalLostTime;
	long long methodEstimatedRealTimeIncludingCalleeWithoutLost = methodEstimatedRealTimeIncludingCallee	;//- totalLostTime;
	long long methodCpuTimeExcludingCallee						= methodCpuTimeIncludingCallee		  		- (entryCallinfo -> callee_time 	 );//+ entryCallinfo->lost_time);
	long long methodEstimatedRealTimeExcludingCallee 			= methodEstimatedRealTimeIncludingCallee  	- (entryCallinfo -> real_callee_time );//+ entryCallinfo->lost_time);

	long long monitorWaitingTimeExcludingCallee			= monitorWaitingTimeIncludingCallee - entryCallinfo->callee_monitor_waiting_timestamp;
	long long ioWaitingTimeExcludingCallee 				= ioWaitingTimeIncludingCallee 		- entryCallinfo->callee_io_waiting_timestamp;

	// ExitingCallInfo has all necessary information
	if(measure != NULL) {
		measure->add(methodCpuTimeExcludingCallee, methodEstimatedRealTimeExcludingCallee, methodCpuTimeIncludingCalleeWithoutLost, methodEstimatedRealTimeIncludingCalleeWithoutLost, monitorWaitingTimeExcludingCallee, ioWaitingTimeExcludingCallee, monitorWaitingTimeIncludingCallee, ioWaitingTimeIncludingCallee);
		measure->moreInfo(entryCallinfo -> callee_time, 0, 0);//entryCallinfo->lost_time_callee, entryCallinfo->lost_time);
//		entryCallinfo->decreaseThreadsExecutingMethodCounter();

	} else {
		assert(false);
	}

}

//
//void MethodCallInfo::decreaseThreadsExecutingMethodCounter() {
//	if (methodData != NULL) {
//		assert(false);
//		methodData->decreaseThreadsExecutingMethodCounter();
//		cout << "decreased counter: " << methodData->getGlobalEntryCount() << endl;
//	} else {
//		if (methodId > 30) {
////			cout << " ERROR:v COULD NOT FIND METHOD-DATA FOR METHODID: " << methodId << endl;
////			assert(false);
//		}
//	}
//}
//
//void MethodCallInfo::setGlobalMethodDataOnEntry(MethodData *data) {
//	assert(false);
//	if (data != NULL && methodId > 30) {
//		methodData = data;
//		methodData->increaseThreadsExecutingMethodCounter();
//	} else {
//		if (methodId > 30) {
////			cout << " ERROR: COULD NOT FIND METHOD-DATA FOR METHODID: " << methodId << endl;
//		}
//	}
//}

MethodCallInfo::~MethodCallInfo() {
	//nothing to do here
}


void StackTraceInfo::setInfo(const int &_methodId, const long long &virtualTime, const long long &estimatedRealTime, const long long &monitorWaitingTime, const long long &ioWaitingTime, PerformanceMeasure *_correspondingStackTraceMeasure) {
	correspondingStackTraceMeasure = _correspondingStackTraceMeasure;
	MethodCallInfo::setInfo(_methodId, virtualTime, estimatedRealTime, monitorWaitingTime, ioWaitingTime);
}

StackTraceInfo::~StackTraceInfo() {

}


void PrimeNumberStackTraceInfo::setInfo(const int &_methodId, const long long &virtualTime, const long long &estimatedRealTime, const long long &monitorWaitingTime, const long long &ioWaitingTime, const long double &_traceId) {
	traceId = _traceId;
	MethodCallInfo::setInfo(_methodId, virtualTime, estimatedRealTime, monitorWaitingTime, ioWaitingTime);
}

PrimeNumberStackTraceInfo::~PrimeNumberStackTraceInfo() {

}
