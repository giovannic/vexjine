/*
 * ThreadManagerRegistry.h
 *
 *  Created on: 31 Mar 2011
 *      Author: root
 */

#ifndef THREADMANAGERREGISTRY_H_
#define THREADMANAGERREGISTRY_H_

class ThreadManager;
class Visualizer;
class VexThreadState;
class Log;

#include <pthread.h>

class ThreadManagerRegistry {
public:
	ThreadManagerRegistry(const unsigned int &_maximumCpus);
	bool addThreadManager(ThreadManager *);
	ThreadManager *getDefaultManager();
	ThreadManager *getCurrentThreadManager();
    long long getGlobalTime();

    void onThreadSpawn(VexThreadState * state);
    void suspendCurrentThread(VexThreadState *state, const long long & startingTime, const char & options);

	void enableSchedulerStats();
	void end();

	void writeStats(const char *filename);

	// Global setters
	void setLog(Log *);
	void setDefaultSchedulerTimeslot(const long long &schedulerTimeslot);
	void setVisualizer(Visualizer *visualizer);


    // Methods aggregating info from all managers
    unsigned long getTotalPosixSignalsSent();
    void printRunningThread();
    bool holdsAnyKeys(VexThreadState *state);

    void resetDefaultSchedulerTimeslot();
	virtual ~ThreadManagerRegistry();

protected:

	ThreadManager **getCurrentThreadManagerOf(VexThreadState *state);
	// Global activators

	// Methods used to first locate the manager of the calling thread and then perform the operation
//    void onThreadWaitingStart(const long long &startingTime, VexThreadState *state);
//    void onThreadWaitingStart(VexThreadState * state);
//    void onThreadWaitingEnd(VexThreadState *state);
//
//    void onThreadTimedWaitingStart(VexThreadState * state, long &timeout);
//    void onThreadTimedWaitingEnd(VexThreadState * state, const long  & interruptTime);
//    void onThreadYield(VexThreadState * state, const long long & startingTime);
//    void onThreadEnd(VexThreadState * state);
//    void onThreadInteractionPointEncounter(VexThreadState * state, const long long & startingTime);
//    void onThreadContendedEnter(VexThreadState * state, const long long & presetTime);
//    void onThreadContendedEntered(VexThreadState *state);
//	void onThreadExplicitlySetWaiting(const long long & startingTime, VexThreadState *state);

//    void lockMutex(VexThreadState *state);
//    void unlockMutex(VexThreadState *state);

//    void setCurrentThreadVT(const long long &presetTime, VexThreadState *state);


    // Methods that additionally have a potential global interest
//    void notifySchedulerForVirtualizedTime(VexThreadState *state, const float &speedup);



private:
	pthread_spinlock_t spinlock;
	unsigned int maximumCpus;
	unsigned int currentlyRegistered;
	ThreadManager **managerList;			// all managers are pointed by this array of pointers
	unsigned int nextCpuToStartThread;		// assign the starting of threads to cores in a round robin fashion
};


#endif /* THREADMANAGERREGISTRY_H_ */
