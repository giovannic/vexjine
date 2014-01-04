/*
 * VirtualTimeline.h: timeline file
 *
 *  Created on: 28 Jun 2010
 *      Author: nb605
 */

#ifndef VIRTUALTIMELINE_H_
#define VIRTUALTIMELINE_H_

#include <iostream>
#include <fstream>

class VexThreadState;
struct vex_and_system_states;
class TimeLoggingBehaviour;

class VirtualTimeline {

public:
	VirtualTimeline();

	void lock();
	void unlock();

	// Any non-manager lock should only be acquired by a thread, after the thread has acquired its controlling manager lock, so that
	// it will not get signalled, while holding the lock essentially blocking its controller => deadlock. The getGlobalTime() is
	// therefore used to avoid acquiring two locks in cases where the exact value of getGlobalTime does not matter.
	inline long long const &getGlobalTime() {
		return globalVirtualTime;
	};

	void leapForwardTo(const long long &forwardTime);
	void addTimeOfUnknownRealTimeDurationAndSynchronize(const long long &forwardTime);
	virtual void addTimeOfThreadExecutingAtUnknownTime(const long long &forwardTime);

	/*
	inline void leapForwardTo(const long long &forwardTime) {
		lock();			// locks used for stupid native waiting threads
		if (forwardTime+unknownParallelTime > globalVirtualTime) {
			globalVirtualTime = forwardTime+unknownParallelTime;
		}
		unknownParallelTime = 0;
		unlock();
	};

	inline void addTimeOfUnknownRealTimeDurationAndSynchronize(const long long &forwardTime) {
		lock();
		globalVirtualTime += forwardTime;
		unlock();
	};

	inline void addTimeOfThreadExecutingAtUnknownTime(const long long &forwardTime) {
		lock();
		globalVirtualTime += forwardTime;
		unknownParallelTime += forwardTime;
		unlock();
	}
	*/


	virtual void reset();
	virtual ~VirtualTimeline();

protected:
	// The current global virtual time
	long long globalVirtualTime;

	// Time attributed to events not scheduled by VEX - like native waiting threads
	long long unknownParallelTime;

	pthread_spinlock_t spinlock;
    int spinint;
    pthread_mutex_t mutex;
};


/***
 * Used for loose synchronizing schedulers
 */
class MulticoreVirtualTimeline : public VirtualTimeline {
public:
	MulticoreVirtualTimeline(const int &schedulers) : VirtualTimeline() {
		localCoreVirtualTimes = new long long[schedulers];
		for (int i =0 ; i<schedulers;i++) {
			localCoreVirtualTimes[i] = 0;//LLONG_MAX;
		}
		processors = schedulers;
	}
	~MulticoreVirtualTimeline();
	void reset();

	long long const &getLocalTime(const int &managerId) {
		return localCoreVirtualTimes[managerId];
	}

	virtual void addTimeOfThreadExecutingAtUnknownTime(const long long &forwardTime);
	virtual void leapForwardTo(const long long &time, const int &localTimelineId);
	virtual void addTimeOfUnknownRealTimeDurationAndSynchronize(const long long &forwardTime);
	virtual void updateGlobalTimeTo(const long long &time);
	virtual bool shouldBlockCoreProgress(const long long &schedulerTimeslot, const int &localTimelineId);
	virtual bool disableLocalTimeline(const int &id);
	virtual long long const &getLocalTimelineActivationTime(const long long &threadTime, const int &id);

	virtual long long &getUpdatedGlobalTime();

protected:
	long long *localCoreVirtualTimes;			// Local time per scheduler
	int processors;
};


/***
 * Used for strictly synchronizing schedulers (single timeline)
 */
class DisablingMulticoreVirtualTimeline : public MulticoreVirtualTimeline {
public:
	DisablingMulticoreVirtualTimeline(const int &schedulers) : MulticoreVirtualTimeline(schedulers) {
		localCoreActive = new bool[schedulers];
		for (int i =0 ; i<schedulers;i++) {
			localCoreActive[i]=false;
		}
		for (int i =0 ; i<schedulers;i++) {
			localCoreActive[i]=false;
		}
		processors = schedulers;
	}


	~DisablingMulticoreVirtualTimeline();
	void reset();

	virtual void updateGlobalTimeTo(const long long &time);
	virtual bool disableLocalTimeline(const int &id);
	virtual long long const &getLocalTimelineActivationTime(const long long &threadTime, const int &id);

	virtual long long &getUpdatedGlobalTime();
protected:
	bool *localCoreActive;

};


class VirtualTimeSnapshot {
public:
	VirtualTimeSnapshot() {
		globalTime 		= 0;
		localTimelineId = 0;
		localTime 		= 0;
	}

	VirtualTimeSnapshot & operator=(const long long &_globalTime) {
		globalTime = _globalTime;
		localTimelineId = 0;
		localTime = _globalTime;
		return *this;
	}

	void set(const long long &_globalTime, const long long &_localTime, const int &_localTimelineId) {
		globalTime = _globalTime;
		localTime = _localTime;
		localTimelineId = _localTimelineId;
	}

	long long const &getGlobalTime() const {
		return globalTime;
	}

	long long const &getLocalTime() const {
		return localTime;
	}

	int const &getLocalTimelineId() const {
		return localTimelineId;
	}

	VirtualTimeSnapshot & operator-=(const VirtualTimeSnapshot &rhs);
private:
	long long globalTime;
	int localTimelineId;
	long long localTime;

};


class VirtualTimeForwardLeapSnapshot {
public:

	VirtualTimeForwardLeapSnapshot(bool _allowed, long long _timeRemaining, long _timeout, long long _threadERT, int _underCreation, const struct vex_and_system_states &_vass);
	friend std::ostream & operator <<(std::ostream &outs, const VirtualTimeForwardLeapSnapshot &record);
	virtual ~VirtualTimeForwardLeapSnapshot();

protected:
	bool allowed ;
	long long timeRemaining ;
	long timeout ;
	long long threadERT ;
    int underCreation ;
    struct vex_and_system_states *vass;
};





class VirtualTimelineController {

public:
	virtual void commitCpuTimeProgress(VexThreadState *state) = 0;
	virtual void commitIoTimeProgress(VexThreadState *state, const long long &actualIoDuration) = 0;
	virtual void commitTimedWaitingProgress(VexThreadState *state) = 0;
	virtual void commitNativeWaitingProgress(VexThreadState *state) = 0;
	virtual void commitModelSimulationProgress(VexThreadState *state) = 0;
	virtual void commitTimedOutIoProgress(VexThreadState *state) = 0;
	virtual void commitBackgroundLoadExecutionTime(const long long &backgroundLoadExecutionTime) = 0;

//	virtual void updateNewThreadTimestamp(VexThreadState *state) = 0;
	virtual void updateResumingSuspendedThreadTimestamp(VexThreadState *state) = 0;
	virtual void updateResumingSuspendedThreadResumedLastTimestamp(VexThreadState *state) = 0;
	virtual void updateTimedOutWaitingThreadTimestamp(VexThreadState *state) = 0;
	virtual void updateInterruptedWaitingThreadTimestamp(VexThreadState *state, const long long &interruptedTime) = 0;
	virtual void updateBlockedThreadTimestamp(VexThreadState *state) = 0;

	virtual long long getHowFarAheadInVirtualTimeTheThreadIs(VexThreadState *state) = 0;
	virtual bool shouldBlockCoreProgress(const long long &schedulerTimeslot) = 0;
	virtual bool disableCore() = 0;
	virtual long long const &getGlobalTime() = 0;

	virtual void getCurrentTimeSnapshot(VirtualTimeSnapshot &vts) = 0;
	// TIME ADDITION STATISTICS - THIS IS THE MOST IMPORTANT BIT FOR ONCE AND FORALL FIXES
//	void addStatsSample(const int &position, const long long &time) {
//		timestats.addSample(position, time);
//	};

	// OUTPUT METHODS
//	void printStats(char *filename);

protected:


};



class SingleVirtualTimelineController : public VirtualTimelineController {

public:
	SingleVirtualTimelineController(VirtualTimeline *_timeline) {
		virtualTimeline = _timeline;
	};

	virtual void commitCpuTimeProgress(VexThreadState *state);
	virtual void commitIoTimeProgress(VexThreadState *state, const long long &actualIoDuration);
	void commitTimedWaitingProgress(VexThreadState *state);
	void commitNativeWaitingProgress(VexThreadState *state);
	void commitBackgroundLoadExecutionTime(const long long &backgroundLoadExecutionTime);
	void commitModelSimulationProgress(VexThreadState *state);
	void commitTimedOutIoProgress(VexThreadState *state);

//	void updateNewThreadTimestamp(VexThreadState *state);
	void updateResumingSuspendedThreadTimestamp(VexThreadState *state);
	void updateResumingSuspendedThreadResumedLastTimestamp(VexThreadState *state);
	void updateTimedOutWaitingThreadTimestamp(VexThreadState *state);
	void updateInterruptedWaitingThreadTimestamp(VexThreadState *state, const long long &interruptedTime);
	void updateBlockedThreadTimestamp(VexThreadState *state);

	long long getHowFarAheadInVirtualTimeTheThreadIs(VexThreadState *state);
	bool shouldBlockCoreProgress(const long long &schedulerTimeslot);
	bool disableCore();
	long long const &getGlobalTime();

	void getCurrentTimeSnapshot(VirtualTimeSnapshot &vts);
protected:
	VirtualTimeline *virtualTimeline;

};


class PassiveVirtualTimelineController : public SingleVirtualTimelineController {

public:
	PassiveVirtualTimelineController(VirtualTimeline *_timeline) : SingleVirtualTimelineController(_timeline) {};

	void commitCpuTimeProgress(VexThreadState *state);
	void commitIoTimeProgress(VexThreadState *state, const long long &actualIoDuration);
};

class MultipleVirtualTimelinesController : public VirtualTimelineController {

public:
	MultipleVirtualTimelinesController(MulticoreVirtualTimeline *_timelines, const int &_managerId) {
		multipleTimelines	= _timelines;
		localTimelineId 	= _managerId;
	};

	void commitCpuTimeProgress(VexThreadState *state);
	void commitIoTimeProgress(VexThreadState *state, const long long &actualIoDuration);
	void commitTimedWaitingProgress(VexThreadState *state);
	void commitNativeWaitingProgress(VexThreadState *state);
	void commitModelSimulationProgress(VexThreadState *state);
	void commitTimedOutIoProgress(VexThreadState *state);
	void commitBackgroundLoadExecutionTime(const long long &backgroundLoadExecutionTime);

//	void updateNewThreadTimestamp(VexThreadState *state);
	void updateResumingSuspendedThreadTimestamp(VexThreadState *state);
	void updateResumingSuspendedThreadResumedLastTimestamp(VexThreadState *state);
	void updateTimedOutWaitingThreadTimestamp(VexThreadState *state);
	void updateInterruptedWaitingThreadTimestamp(VexThreadState *state, const long long &interruptedTime);
	void updateBlockedThreadTimestamp(VexThreadState *state);

	long long getHowFarAheadInVirtualTimeTheThreadIs(VexThreadState *state);
	bool shouldBlockCoreProgress(const long long &schedulerTimeslot);
	bool disableCore();
	long long const &getGlobalTime();

	void getCurrentTimeSnapshot(VirtualTimeSnapshot &vts);
protected:
	MulticoreVirtualTimeline *multipleTimelines;	// one global and many local
	int localTimelineId;							// this controller refers to the id-th local timeline

};


class StatisticsEnabledVirtualTimelineController : public VirtualTimelineController {

public:
	StatisticsEnabledVirtualTimelineController(VirtualTimelineController *_controller, TimeLoggingBehaviour *_timeLogging) : controller(_controller), timeLogging(_timeLogging) {}

	void commitCpuTimeProgress(VexThreadState *state);
	void commitIoTimeProgress(VexThreadState *state, const long long &actualIoProgress);
	void commitTimedWaitingProgress(VexThreadState *state);
	void commitNativeWaitingProgress(VexThreadState *state);
	void commitModelSimulationProgress(VexThreadState *state);
	void commitTimedOutIoProgress(VexThreadState *state);
	void commitBackgroundLoadExecutionTime(const long long &backgroundLoadExecutionTime);

//	void updateNewThreadTimestamp(VexThreadState *state);
	void updateResumingSuspendedThreadTimestamp(VexThreadState *state);
	void updateResumingSuspendedThreadResumedLastTimestamp(VexThreadState *state);
	void updateTimedOutWaitingThreadTimestamp(VexThreadState *state);
	void updateInterruptedWaitingThreadTimestamp(VexThreadState *state, const long long &interruptedTime);
	void updateBlockedThreadTimestamp(VexThreadState *state);

	long long getHowFarAheadInVirtualTimeTheThreadIs(VexThreadState *state);
	bool shouldBlockCoreProgress(const long long &schedulerTimeslot);
	bool disableCore();
	long long const &getGlobalTime();

	void getCurrentTimeSnapshot(VirtualTimeSnapshot &vts);

protected:
	VirtualTimelineController *controller;
	TimeLoggingBehaviour *timeLogging;
};


#endif /* TIMER_H_ */
