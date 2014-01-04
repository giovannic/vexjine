#ifndef PASSIVEMANAGER_H_
#define PASSIVEMANAGER_H_

#include <map>
#include <pthread.h>

#include "Visualizer.h"
#include "ThreadManager.h"
#include "ThreadState.h"
#include "ThreadQueue.h"
#include "EventLogger.h"
#include "IoSimulator.h"
#include "ObjectRegistry.h"
#include "ThreadRegistry.h"

using namespace std;

class PassiveManager : public ThreadManager {
public:
	PassiveManager(VirtualTimelineController *_globalTimer, ThreadQueue *runnableThreads, IoSimulator *ioSim, ThreadRegistry *reg, ObjectRegistry *_objReg) : ThreadManager(0, _globalTimer, runnableThreads, ioSim, reg, _objReg) {};
	~PassiveManager();


	/*** Methods for changing the state of a thread (from the VTF-scheduler perspective) ******/
	void setRunningThread(VexThreadState *state);
	void setSuspendedAndPushToRunnables(VexThreadState *state);
	void setWaitingThread(VexThreadState *state);
	void setTimedWaitingThread(VexThreadState *state);
	void setNativeWaiting(VexThreadState *state);
	void setIoThread(VexThreadState *state, bool learning);

	/*** Method to be called throughout the thread lifecycle ******/
	void onThreadSpawn(VexThreadState *state);

	// Called to indicate that the given thread is waiting
	void onThreadWaitingStart(const long long &startingTime, VexThreadState *state);

	// Called when a thread restarts after waiting
	void onThreadWaitingEnd(VexThreadState *state);
    void onThreadContendedEntered(VexThreadState *state);

	// Called to notify the thread manager that the thread is about to execute an action in which it will be suspended
	// like waiting or sleeping) that expires after a timeout. This should be correctly simulated in virtual time
    void suspendLooseCurrentThread(VexThreadState *state, const long long & startingTime);
	void onThreadTimedWaitingStart(VexThreadState *state, long &timeout);

	// Either when the timed action finishes or when it gets interrupted
	void onThreadTimedWaitingEnd(VexThreadState *state, const long &interruptTime);

	void onThreadYield(VexThreadState *state, const long long &startingTime);
	void onThreadEnd(VexThreadState *state);

	// Called just before an interaction point
	void onThreadInteractionPointEncounter(VexThreadState *state, const long long& startingTime);



	/*** Methods to update the running thread's clocks ****/
	void updateCurrentThreadVT(VexThreadState *state);
	void setCurrentThreadVT(const long long &presetTime, VexThreadState *state);	// used to avoid lost times
    void setCurrentThreadVTLockless(const long long &startingTime, VexThreadState *state);

    int unconditionalWakeup();

	/**Suspends the current thread - if the shouldCurrentThreadSuspend conditions are fulfilled.*/
	void suspendCurrentThread(VexThreadState *state, const long long & startingTime, const char & options);
	/**Check if the current thread that was asked to be suspended should actually suspend**/
	bool shouldCurrentThreadSuspend(VexThreadState *state);

	void blockCurrentThread(VexThreadState *state);	/* Immediately block thread - suspend after awaking*/

	void onWrappedTimedWaitingEnd(VexThreadState *state);

	void onReplacedWaiting(VexThreadState *state);
	void onReplacedTimedWaiting(VexThreadState *state, const long &timeout);

	virtual void start();
	virtual void end();

};

#endif /*PASSIVEMANAGER_H_*/
