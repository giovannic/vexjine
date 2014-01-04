/*
 * NativeWaitingCriteria.h
 *
 *  Created on: 15 Aug 2011
 *      Author: root
 */

#ifndef NATIVEWAITINGCRITERIA_H_
#define NATIVEWAITINGCRITERIA_H_

#include "Timers.h"
#include "Scheduling.h"

class NativeWaitingCriteria {
public:
	NativeWaitingCriteria();
	virtual bool isNativeWaiting(const long long &currentRealTime) = 0;
	virtual ~NativeWaitingCriteria();
};


/**
 * Class for the cases where no native waiting is possible (i.e. no virtual machines - C++)
 */
class NoNativeWaitingCriteria : public NativeWaitingCriteria {
public:
	NoNativeWaitingCriteria();
	bool isNativeWaiting(const long long &currentRealTime);
	virtual ~NoNativeWaitingCriteria();
};


/**
 * Class using timing and scheduling counters from VEX to decide whether a thread is native waiting
 */
class VexCountersBasedCriteria : public NativeWaitingCriteria {
public:
	VexCountersBasedCriteria(Scheduling *threadSchedulingInfo, Timers *threadTimingInfo);
	bool isNativeWaiting(const long long &currentRealTime);
	virtual ~VexCountersBasedCriteria();

private:
	Scheduling *threadSchedulingInfo;
	Timers *threadTimingInfo;
};


/**
 * Class using the stack trace to see if the current method trace
 * includes vex.so or jvmtiagent.so
 */
class StackTraceBasedCriteria : public NativeWaitingCriteria {
public:
	StackTraceBasedCriteria();
	bool isNativeWaiting(const long long &currentRealTime);
	bool isNativeWaiting();
	virtual ~StackTraceBasedCriteria();
};


/**
 * Class using timing and scheduling counters from VEX to decide whether a thread is native waiting
 */
class CombinedCriteria : public VexCountersBasedCriteria {
public:
	CombinedCriteria(Scheduling *threadSchedulingInfo, Timers *threadTimingInfo) : VexCountersBasedCriteria(threadSchedulingInfo, threadTimingInfo) {};
	bool isNativeWaiting(const long long &currentRealTime);
	virtual ~CombinedCriteria();
};


class NativeWaitingCriteriaFactory {
public:

	static NativeWaitingCriteria *getCriteria(Scheduling *scheduling, Timers *timers);
	static void setCriteriaType(const char *);

private:
	NativeWaitingCriteriaFactory();
	~NativeWaitingCriteriaFactory();
	static short criteriaType;
};



#endif /* NATIVEWAITINGCRITERIA_H_ */
