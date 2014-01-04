#ifndef THREADMANAGER_H_
#define THREADMANAGER_H_

#include <map>
#include <vector>
#include <pthread.h>

class Visualizer;
class VexThreadState;
class ThreadQueue;
class VirtualTimeline;
class EventLogger;
class IoSimulator;
class ObjectRegistry;
class ThreadRegistry;
class Log;
class VirtualTimelineController;
class VirtualTimeForwardLeapSnapshot;

#include <syscall.h>
#include <unistd.h>
#include "QStats.h"

using namespace std;

void *pthreadManagerWorker(void *);

/***
 * The class that implements the scheduler per core that
 * synchronizes with the next suspending and resuming thread.
 * It has to be notified for thread state changes to trigger the virtual timeline updates (through the time controller)
 * and find the next runnable thread.
 *
 * About the design: the idea of assigning so many responsibilities to the same class
 * is to allow extension of the framework for multicore/distributed code by subclassing this,
 * without changing the interface to the instrumentation framework.
 */
class ThreadManager {
public:

	ThreadManager(unsigned int _id, VirtualTimelineController *_globalTimer, ThreadQueue *runnableThreads, IoSimulator *, ThreadRegistry *, ObjectRegistry *);
	virtual ~ThreadManager();

	// The methods that perform the actions upon thread states changes
	virtual void setSuspended(VexThreadState * state);
	virtual void setRunningThread(VexThreadState * state);
    virtual void setSuspendedAndPushToRunnables(VexThreadState * state);
    virtual void setIoThread(VexThreadState * state, const bool &learning);
    void setInModelSimulation(VexThreadState *state, const bool &afterModelSimulation);

    virtual void setWaitingThread(VexThreadState * state);

    virtual void setTimedWaitingThread(VexThreadState * state);
    virtual void setNativeWaiting(VexThreadState * state);

    virtual void setSystemCallThread(VexThreadState *state);


    // The methods that are triggered in the various events of the thread lifecycle
    virtual void onThreadSpawn(VexThreadState * state);
    virtual void onThreadEnd(VexThreadState * state);

    virtual void onThreadYield(VexThreadState * state, const long long & startingTime);


    virtual void onWrappedWaitingStart(VexThreadState *state, const long &startingTime);
    virtual void onWrappedWaitingEnd(VexThreadState *state, const long &startingTime);
    virtual void onWrappedWaitingInterrupt(VexThreadState *state, long interruptionTime);

    virtual void onWrappedTimedWaitingStart(VexThreadState *state, const long &startingTime, const long &timeout);
    virtual void onWrappedTimedWaitingEnd(VexThreadState *state);

    virtual void onWrappedTimedWaitingInterrupt(VexThreadState *state, const long long &interruptionTime);

    virtual void onReplacedWaiting(VexThreadState *state);
    virtual void onReplacedTimedWaiting(VexThreadState *state, const long &timeout);
    virtual void onAnyReplacedWaitingInterrupt(VexThreadState *state, const long long &interruptionTime);

    virtual void onWrappedBackgroundLoadExecutionStart();
    virtual void onWrappedBackgroundLoadExecutionEnd(const long long &executionDuration);

//    virtual void onThreadWaitingStart(const long long &startingTime, VexThreadState *state);
//    virtual void onThreadWaitingStart(VexThreadState * state);
//    virtual void onThreadWaitingStartLockless(VexThreadState *state);
//    virtual void onThreadWaitingEnd(VexThreadState *state);
//    virtual void onThreadNativeWaitingStart(const long long &startingTime, VexThreadState *state);
//    virtual void onThreadNativeWaitingEnd(VexThreadState *state);
//    virtual void onThreadTimedWaitingStart(VexThreadState * state, long &timeout);
//    virtual void onThreadContendedEnter(VexThreadState * state, const long long & presetTime);
//    virtual void onThreadContendedEntered(VexThreadState *state);
//    void onThreadExplicitlySetWaiting(const long long &startingTime, VexThreadState *state);
//	void onThreadExplicitlyManageTimedWaitingAfterReturningFromRealTime(VexThreadState *state, const int &returnValue);
//    void onThreadExplicitlySetTimedWaiting(const long long &startingTime, VexThreadState *state, const long &timeout);
//    virtual void onThreadTimedWaitingEnd(VexThreadState * state, const long  & interruptTime);



    // Thread lifecycle related
    virtual void suspendLooseCurrentThread(VexThreadState *state, const long long & startingTime);
    virtual void suspendCurrentThread(VexThreadState * state, const long long & startingTime, const char & options);
    virtual bool shouldCurrentThreadSuspend(VexThreadState * state);
    virtual void blockCurrentThread(VexThreadState * state);
    virtual void resumeThread(VexThreadState * state);
    virtual bool changeThreadStateToRunning(VexThreadState * state);
    virtual void notifySchedulerForVirtualizedTime(VexThreadState * state, const float &scalingFactor);
    void unsetCurrentThreadFromRunningThread(VexThreadState * state);
    void handleIOPerformingThread(VexThreadState * state);
    virtual void interruptTimedWaitingThread(VexThreadState * state);


    virtual std::string getStats();

    inline void lockMutex() {
    	pthread_mutex_lock(&mutex);
    };

    inline void unlockMutex() {
    	pthread_mutex_unlock(&mutex);
    };

    // Scheduler lifecycle related
    virtual void start();
    virtual void end();

    // Scheduler timeslot handling - other classes (like I/O handling ones) are allowed to dynamically adapt this
    void setSchedulerTimeslot(const long  & timeslot);
    void setDefaultSchedulerTimeslot(const long &timeslot);
    void minimizeSchedulerTimeslot();
    void limitNextTimeslice();
    void resetDefaultSchedulerTimeslot();

    // Signal the scheduler thread
    virtual int wakeup();
    virtual int conditionalWakeup(VexThreadState * state);
    virtual int unconditionalWakeup();
    void unFreeze();
    void setFreeze();
    int indefiniteWait();

    // Time updating related
    virtual void updateCurrentThreadVT(VexThreadState * state);	// The next three use virtual locklessUpdateTimeBy() so overriding it might be enough
    virtual void setCurrentThreadVT(const long long &startingTime, VexThreadState *state);
    virtual void commitIoDuration(VexThreadState * state, const long long &actualIoDuration);
    virtual long long getCurrentGlobalTime();
    virtual void commitBackgroundLoadDuration(const long long &backgroundLoadDuration);

//    void increaseRunningThreadTimeBy(const long long &duration);

    void suspendModelSimulationFinishingThread(VexThreadState *state);

    // Debugging related
    void setLog(Log *);
    void setVisualizer(Visualizer *viz);


    // Related to the queue with the runnable threads
    void pushIntoRunnableQueue(VexThreadState * state);			// invoked explicitly by model handling code
    void updateRunnableQueue();

    // Outputs for debugging reasons
    void ps();
    void printThreadStates();
    void printRunningThread();
    void printRunningThreadWithCore();
    void printVirtualTimeForwardLeapSnapshots(const char *filename);
    void printTimesliceStats();

    void toggleRuntimeDebugging() {
    	runtimeDebuggingEnabled ^= 1;		// debugging messages to be toggled on/off during the simulation by sending SIGALRM to a scheduler thread
    }

    void enableSchedulerStats();	// general stats that are printed out in the end of the simulation
	inline bool areSchedulerStatsEnabled() {
		return schedulerStats;
	};



    static FILE *managerLogfile;
    static const int SUSPEND_OPT_DONT_UPDATE_THREAD_TIME;
    static const int SUSPEND_OPT_FORCE_SUSPEND;
    static const int SUSPEND_OPT_THREAD_ALREADY_IN_LIST;
    static const int SUSPEND_OPT_DONT_WAKE_UP_SCHEDULER;
    static const int SUSPEND_OPT_DONT_NOTIFY_CLIENT;
    static const int SUSPEND_OPT_DONT_MAKE_REQUEST;

    void increasePendingNotifications() {
    	++pendingNotifications;
    }
    void clearPendingNotifications() {
		pendingNotifications = 0;
    }

    unsigned int schedulerThreadId;

    
    unsigned long getPosixSignalsSent() {
    	return posixSignalsSent;
    };
    unsigned int const & getId() const {
    	return managerId;
    };

    bool keyHeldBy(const int &threadId);

    bool anotherThreadAlreadySetRunning(VexThreadState * state);

	void _notifySchedulerForVirtualizedTime(VexThreadState *state);

	bool testIsValidToLeapInVirtualTimeTo(VexThreadState *state);

	vector<VirtualTimeForwardLeapSnapshot *> vflStatistics;

	bool runtimeDebuggingEnabled;	// used for debugging (from SIGALRM handler)
	static void enableTimeslotMinimazationOnNw();
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
protected:
	static bool minimizeTimeslotOnNw;		//TODO: this is a hack - make nice class and unite with I/O induced minimization

    VexThreadState *lastSuspended;
    ThreadQueue *runnableThreads;

    Visualizer *visualizer;
    IoSimulator *ioSimulator;
    ObjectRegistry *objectRegistry;
    ThreadRegistry *threadRegistry;

	Log *logger;

    pthread_mutex_t threadStatsMutex;

    // The currently running Thread is treated as a shared resource
    VexThreadState *runningThread;
    pthread_spinlock_t runningThreadSpinLock;
    void lockRunningThread();
    void unlockRunningThread();
    VexThreadState *getRunningThread();

    pthread_mutex_t mutex;
    bool running;

    virtual void registerSignalHandler();
    //void registerSignalHandler(int sigcode, void (*f)(int));
    void registerSignalHandler(int sigcode, void (*f)(int));

    void updateThreadList();

    void resumeModelSimulation(VexThreadState *state);
    void pollThreadToCheckIfActuallyRunning(VexThreadState *state);

    virtual bool continueThread(VexThreadState *state);
    bool suspendThread(VexThreadState * state);
    virtual bool noThreadsToSchedule();
    virtual void afterModelSimulation();

    virtual bool isSignalledThreadGoingToSuspend(VexThreadState *state);
    virtual void onIntentionToBeDisregarded(VexThreadState *state);
    virtual void onIntentionToKeepOnRunning(VexThreadState *state);

    bool isSignalledThreadStillRunning(VexThreadState *state);

    bool schedulerStats;
    long normalTimeslotsFinished;
    unsigned int timesOfMinimumTimeslotForcing;
    QStats<long long> durationsOfInterruptedTimeslots;

    pthread_cond_t cond;
    int freeze;
    long schedulerTimeslot;
    long defaultSchedulerTimeslot;
    bool forceMinimumTimeslotSelection;

    long long lastGlobalVirtualTime;
    long long lastTotalThreadERT;	// a sum of all current virtual timestamps (to show whether any progress is made)

    // Main scheduling algorithm methods
    bool isValidToLeapInVirtualTimeTo(VexThreadState *state);

    void doControllerWait();
    virtual void suspendRunningResumeNext();
    virtual void interruptTimedOutIoThread(VexThreadState *state);
    void _simulateModel(VexThreadState *state, long long &totalExecutionTime);
    virtual bool _suspendThread(VexThreadState *state);

    void setThreadToMaximumPriority();

    void generateThreadStats(VexThreadState *state);

    bool hasResumedModelSimulationRunToCompletion(VexThreadState *state, long long &totalExecutionTime);


    // Debugging members
    VexThreadState *lastResumed;
    int lastSignaledTids[100];
    int lastSignaledPtr;

    inline int tkill(int tid, int sig) {
    	// Decided here that no redefinition of sigaction action is needed after r110
    	lastSignaledTids[(lastSignaledPtr++)%100] = tid;
    	return (int) syscall(SYS_tkill, tid, sig);
    };

    virtual void locklessUpdateTimeBy(const long long &timeDiff, VexThreadState *state);

    unsigned int pendingNotifications;

    // Internal factored methods
    void _onThreadWaitingStart(VexThreadState *state);
    void _setIoThread(VexThreadState *state, const bool &learning);


    // Timeslice monitoring
    bool schedulerWaitingForTimeslotExpiry;
    long long aggregateWaitingTime;
    long long aggregateWaitingTimeSquared; 
    unsigned int waitingTimeSamples;
    unsigned int managerId;
    unsigned long schedulerTid;
    unsigned long posixSignalsSent;

    unsigned int timedOutsFailed;
    unsigned int timedOutsSucceeded;
    unsigned int timesYielding;

	void lockingUpdateTimeBy(const long long &timeDiff, VexThreadState *state);

	inline float const & getRunningThreadScalingFactor();
	void unsetCurrentThreadFromRunningThreadAndWakeup(VexThreadState *state);

	long long decreaseSchedulerSleepingTimeBy;

	VirtualTimelineController *virtualTimelineController;

	float currentTimeScalingFactor;

	bool showLastContinued;
	bool showHandling;

private:
    void init();		// initialize members

    bool timeWaitingFactorChanged;

    int numberOfNativeWaitingThreads;
    void recursivelyUpdatePendingIoRequests(VexThreadState * state, const long long &currentRealTime);
    void ignoreSignalHandler();

};


#endif /*THREADMANAGER_H_*/
