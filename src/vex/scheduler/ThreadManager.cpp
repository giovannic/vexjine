#include "ThreadManager.h"
#include "VirtualTimeline.h"
#include "Visualizer.h"
#include "ThreadState.h"
#include "ThreadQueue.h"
#include "VirtualTimeline.h"
#include "EventLogger.h"
#include "IoSimulator.h"
#include "ObjectRegistry.h"
#include "ThreadRegistry.h"
#include "Logger.h"


#include <sys/time.h>
#include <sys/errno.h>
#include <syscall.h>
#include <unistd.h>
#include <iostream>
#include <signal.h>
#include <time.h>
#include <cassert>
/*************************************************************************
 ****
 **** CONSTRUCTOR / DESTRUCTOR
 ****
 ************************************************************************/
FILE *ThreadManager::managerLogfile = stderr;
ThreadManager *manager = NULL;

#define likely(x) __builtin_expect((x),1)

const int ThreadManager::SUSPEND_OPT_DONT_UPDATE_THREAD_TIME = 1;
const int ThreadManager::SUSPEND_OPT_FORCE_SUSPEND     		 = 2;
const int ThreadManager::SUSPEND_OPT_THREAD_ALREADY_IN_LIST  = 4;
const int ThreadManager::SUSPEND_OPT_DONT_WAKE_UP_SCHEDULER  = 8;
const int ThreadManager::SUSPEND_OPT_DONT_NOTIFY_CLIENT 	 = 16;
const int ThreadManager::SUSPEND_OPT_DONT_MAKE_REQUEST       = 32;
bool ThreadManager::minimizeTimeslotOnNw = false;

void ThreadManager::enableTimeslotMinimazationOnNw() {
	minimizeTimeslotOnNw = true;
}


/* Basic functions to start a ThreadManager thread */
void *pthreadManagerWorker(void *managerPointer) {
	ThreadManager *newManager = (ThreadManager *)managerPointer;
	newManager->start();
	return NULL;
}


void ThreadManager::init() {
	numberOfNativeWaitingThreads = 0;

	runtimeDebuggingEnabled = false;
	showLastContinued = false;

	lastGlobalVirtualTime = 0;
	lastTotalThreadERT = 0;

	pthread_cond_init(&cond, NULL);
	pthread_mutex_init(&mutex, NULL);
	pthread_mutex_init(&threadStatsMutex, NULL);

	freeze = 0;

	// Debugging members
	lastResumed = NULL;
	lastSignaledPtr = 0;

	manager = this;				// pointer used from signal handler - how to multicore this: send the manager ptr address to the signaled thread...
	// Initialize to static value - parameterization might overwrite this value

	managerLogfile = stderr;

	schedulerStats = false;

	setDefaultSchedulerTimeslot(100000000);

	forceMinimumTimeslotSelection = false;

	runningThread = NULL;				// each processor has one running thread
	visualizer = NULL;

	for (int i =0 ; i <100; ++i) {
		lastSignaledTids[i] = 0;
	}
	posixSignalsSent 		= 0;
	normalTimeslotsFinished = 0;
	timesOfMinimumTimeslotForcing = 0;

	aggregateWaitingTime = 0;
	aggregateWaitingTimeSquared = 0;
	waitingTimeSamples = 0;

	decreaseSchedulerSleepingTimeBy = 0;
	currentTimeScalingFactor = 1.0;

	timedOutsSucceeded = 0;
	timedOutsFailed    = 0;
	timesYielding 	   = 0;

	schedulerWaitingForTimeslotExpiry = false;
	pthread_spin_init(&runningThreadSpinLock, 0);
}

ThreadManager::ThreadManager(unsigned int _id, VirtualTimelineController *_globalTimer, ThreadQueue *_runnableThreads, IoSimulator *_ioSimulator, ThreadRegistry *_threadRegistry, ObjectRegistry *_objReg) {
	init();
	managerId = _id;
	virtualTimelineController = _globalTimer;
	ioSimulator = _ioSimulator;
	runnableThreads = _runnableThreads;	// pointer to the queue of runnable threads
	threadRegistry = _threadRegistry;
	objectRegistry = _objReg;

}

ThreadManager::~ThreadManager() {
	pthread_cond_destroy(&cond);
	pthread_mutex_destroy(&mutex);
}


/*************************************************************************
 **** 
 **** AUXILIARRY FUNCTIONS
 **** 
 ************************************************************************/

void ThreadManager::unFreeze() {
	--freeze;
}
void ThreadManager::setFreeze() {
	++freeze;
}

void ThreadManager::setDefaultSchedulerTimeslot(const long &timeslot) {
	if (timeslot > 0) {
		defaultSchedulerTimeslot = timeslot;
		schedulerTimeslot = timeslot;
	}
}


void ThreadManager::minimizeSchedulerTimeslot() {
	if (defaultSchedulerTimeslot > 2000000) {
		schedulerTimeslot = 2000000;
	}
}

void ThreadManager::resetDefaultSchedulerTimeslot() {
	schedulerTimeslot = defaultSchedulerTimeslot;
}

void ThreadManager::setSchedulerTimeslot(const long &timeslot) {
	if (timeslot > 0) {
		schedulerTimeslot = timeslot;
	}
}

void ThreadManager::enableSchedulerStats() {
	schedulerStats = true;
}



// Signal handler for ALARM signal, used to print and enable debugging messages
static bool enablerVariable = false;
static void handler_SIGALRM(int sig) {
	enablerVariable = true;
	VexThreadState *state = VexThreadState::getCurrentThreadState();
	if (state != NULL) {
		cout << "Getting stack trace of thread" << endl;
		state->toggleShowMethodEntries();
		vtfstacktrace(true, stdout, state->getName());
	} else if (manager->schedulerThreadId == gettid()) {
		cout << "====================== NEW STACK TRACE ======================" << endl;
		manager->ps();
		vtfstacktrace(true, stdout, "VTF SCHEDULER THREAD");
		manager->toggleRuntimeDebugging();
		manager->printVirtualTimeForwardLeapSnapshots("/data/vtf_vlf_statistics.csv");
	}
}



/*
 * Thread signal handler
 * All invocations of manager->* methods are protected by the lock that is
 * possessed by the scheduler, which sends the tkill which triggers this 
 * handler invocation.
 * @lock: fully locked up to 3.
 */
static void handler_SIGHUP(int sig) {
	VexThreadState *state = VexThreadState::forceGetCurrentThreadState();
	if (state != NULL) {
		state->onSignalledFromScheduler();
	}
}

/*
 * Signal the scheduler to wakeup
 */
int ThreadManager::wakeup() {
	if (!runnableThreads->empty()) {
		return unconditionalWakeup();
	} else {
		return -1;
	}
}

/*
 * Wakeup scheduler, if the state is not the top of the list
 */
int ThreadManager::conditionalWakeup(VexThreadState *state) {
	if (!runnableThreads->empty() && runnableThreads->top() != state) {
		return unconditionalWakeup();
	} else {
		return -1;
	}
}

int ThreadManager::unconditionalWakeup() {
	lockMutex();
	if (schedulerWaitingForTimeslotExpiry) {
		int rc = pthread_cond_signal(&cond);	// nothing happens if the no-one is waiting at the condition variable
		unlockMutex();
		return rc;
	} else {
		// Reaching this point means that we want to notify the scheduler about a state change, but it is already doing sth (not waiting)
		// For this reason we set this flag to make it skip the next waiting step and make it acknowledge the state change
		schedulerWaitingForTimeslotExpiry = true;
		unlockMutex();
		return 0;
	}
}

// Register signal handlers: SIGHUP-VEX-sched interrupt, SIGALRM-dbg
void ThreadManager::registerSignalHandler() {
	registerSignalHandler(SIGHUP, &handler_SIGHUP);
	registerSignalHandler(SIGALRM, &handler_SIGALRM);
}

void ThreadManager::registerSignalHandler(int sigcode, void (*signalHandler)(int)) {
	struct sigaction action;
    action.sa_handler = signalHandler;				// Set up the structure to specify the new action.
    sigemptyset (&action.sa_mask);
    action.sa_flags = SA_NODEFER | SA_RESTART;	// Signals sent when within the signal handler should not be ignored
    errno = sigaction (sigcode, &action, NULL);
    if (errno != 0) {
    	perror("problem setting signal handler");
    } 

}

void ThreadManager::ignoreSignalHandler() {
	struct sigaction action;
    action.sa_handler = SIG_IGN;				// Set up the structure to specify the new action.
    sigemptyset (&action.sa_mask);
    sigaction (SIGHUP, &action, NULL);
}

void ThreadManager::notifySchedulerForVirtualizedTime(VexThreadState *state, const float &scalingFactor) {

	state->setTimeScalingFactor(scalingFactor);
	_notifySchedulerForVirtualizedTime(state);
}

void ThreadManager::_notifySchedulerForVirtualizedTime(VexThreadState *state) {
	timeWaitingFactorChanged = true;
	unconditionalWakeup();
}
/*************************************************************************
 **** 
 **** DEBUGGING UTILITY FUNCTIONS
 **** 
 ************************************************************************/
/*
 * Utility function for setting the debugging log file
 * @lock: no lock
 */
void ThreadManager::setLog(Log *l) {
	logger = l;
}

void ThreadManager::setVisualizer(Visualizer *viz) {
	if (viz != NULL) {
		visualizer = viz;
	} else {
		fprintf(stderr, "ThreadManager::setVisualizer: could not set event logger\n");fflush(stderr);
	}
}



/*
 * Utility function to print the thread states linked list
 * Note: Do not call any methods to avoid scheduler suspending
 * @lock: no lock - should only be called by scheduler
 */
void ThreadManager::ps() {
	cout << endl << endl << "---------------------------------------------------" << endl;
	printThreadStates();
	cout << "---------------------------------------------------" << endl;
}


void ThreadManager::printThreadStates() {

	int threadListSize = (int)runnableThreads->size();
	cout << "*****************************\nPrinting threadStates list ("<<threadRegistry->getSize()<<") in queue "<<threadListSize<< " at GVT " << getCurrentGlobalTime()/1000000 << " and " << threadRegistry->getThreadsBeingSpawned()<< " spawning threads" << endl;
	printRunningThread();

	cout << endl;
	VexThreadState *topOfQueue = runnableThreads->top();
	if (topOfQueue != NULL) {
		cout << "\t top of queue: " << topOfQueue->getName() << endl;
	}
	threadRegistry->printThreadStates(threadListSize, ioSimulator->areInIoThreadsInRunnablesQueue());
	runnableThreads->print();
}

void ThreadManager::printTimesliceStats() {
	cout << getStats() << endl;
}



std::string ThreadManager::getStats() {
	stringstream str;
	str << "Signals sent from scheduler " << managerId << " to threads asking whether they should suspend: " << posixSignalsSent << endl;
	str << "Regular timeslot duration: " << schedulerTimeslot/1e6 << " ms" << endl;
	str << "Timed-out timeslots of scheduler " << managerId << ": " << normalTimeslotsFinished << endl;
	str << "Interrupted timeslots of scheduler " << managerId << " avg duration: " << durationsOfInterruptedTimeslots.getMean()/1e6 << " ms and stdev " << durationsOfInterruptedTimeslots.getStdev()/1e6 << " ms: " << durationsOfInterruptedTimeslots.getSamples() << endl;
	str << "Minimum duration timeslots for scheduler " << managerId << ": " << timesOfMinimumTimeslotForcing << endl;
	double timedOutsSum = 1.0;
	if (timedOutsSucceeded + timedOutsFailed > 0) {
		timedOutsSum = timedOutsSucceeded + timedOutsFailed;
	}
	str << "Allowed vfl by scheduler " << managerId << ": " << timedOutsSucceeded << " (" << (double)timedOutsSucceeded/(timedOutsSum) << "%)" << endl;
	str << "Forbidden vfl by scheduler " << managerId << ": " << timedOutsFailed << " (" << (double)timedOutsFailed/(timedOutsSum) << "%)" << endl;
	str << "Threads that asked from the scheduler " << managerId << " to yield: " << timesYielding << endl;

	// Virtual forward leaps statistics
//	ofstream vflStatsFile("/data/vtf_vlf_statistics.csv", fstream::out | fstream::app);
//	vflStatsFile << "allowed timeRemaining timeout threadERT underCreation waitingThreads nwThreads activeInIoThreads blockedInIoThreads suspended running" << endl;
//	vector<VirtualTimeForwardLeapSnapshot *>::iterator vit = manager->vflStatistics.begin();
//	while (vit != manager->vflStatistics.end()) {
//		VirtualTimeForwardLeapSnapshot *vv = (VirtualTimeForwardLeapSnapshot *)*vit;
//		vflStatsFile << *vv << endl;
//		++vit;
//	}
//	manager->vflStatistics.clear();
//	vflStatsFile.close();

	return str.str();

}

/*************************************************************************
 **** 
 **** THREAD FUNCTIONS: mostly called by threads from the agent callbacks
 **** 
 ************************************************************************/

//---------------- THREAD STARTING FUNCTIONS ----------------------------//

void ThreadManager::onThreadSpawn(VexThreadState *state) {
	if (state == NULL) {
		return;
	}

//	setThreadToMaximumPriority();
//	virtualTimelineController->updateNewThreadTimestamp(state);
	registerSignalHandler();

	state->acquireThreadControllingLock();
	state->updateCpuTimeClock();			// no lock required in this case, because the ThreadState is not yet visible to the global thread states data structure
	state->setRegistering();

	VISUALIZE_EVENT(THREAD_START, state);
	LOG(logger, logINFO) << " Manager " << managerId << ": " << state->getName() << " spawned" << endl;

	threadRegistry->add(state);			// Populate the index data structure for this Thread state - scheduler still unaware of this thread
	
	//cout << "********added new state of thread " << state->getName() << "  (" << state->getId() << ") at " << state->getEstimatedRealTime()/1e6 << " to virtual core " << managerId <<  endl;
	state->setThreadCurrentlyControllingManager(this);

	// Always wake-up scheduler - what if the running thread gets into a while(condition==true) loop (with the condition becoming false by the other threads)
	state->lockShareResourceAccessKey();
	suspendCurrentThread(state, 0, SUSPEND_OPT_DONT_UPDATE_THREAD_TIME);

	//cout << "********thread " << state->getName() << "  (" << state->getId() << ") started really executing at " << state->getEstimatedRealTime()/1e6 << " to virtual core " << managerId <<  endl;
//	threadRegistry->newThreadStarted();		// update counter used to denote live registering threads, to stop leaps forward in virtual time, if live threads exist

	state->onVexExitWithCpuTimeUpdate();
}


// Object.wait() (with JVMTI), Monitor contention (with JVMTI)
void ThreadManager::onWrappedWaitingStart(VexThreadState *state, const long &startingTime) {
	setCurrentThreadVT(startingTime, state);
	unsetCurrentThreadFromRunningThread(state);

	VISUALIZE_EVENT(WAIT, state);
	LOG(logger, logINFO) << " thread " << state->getName() << " started WAITING by " << managerId << " as controller and timeout " << state->getTimeout() << endl;

	assert(state != runningThread);

	unconditionalWakeup();
}




void ThreadManager::onWrappedWaitingEnd(VexThreadState *state, const long &startingTime) {
	virtualTimelineController->updateBlockedThreadTimestamp(state);

//	suspendCurrentThread(state, 0, SUSPEND_OPT_DONT_UPDATE_THREAD_TIME);	//FROM onThreadContendedEntered
	suspendCurrentThread(state, startingTime, 0);							//FROM onThreadWaitingEnd

	VISUALIZE_EVENT(WAITING_RELEASED, state);
	LOG_LAST_VEX_METHOD(state)

}


void ThreadManager::onWrappedWaitingInterrupt(VexThreadState *state, long interruptionTime) {
	if (interruptionTime == 0) {
		virtualTimelineController->updateInterruptedWaitingThreadTimestamp(state, getCurrentGlobalTime());
	} else {
		virtualTimelineController->updateInterruptedWaitingThreadTimestamp(state, interruptionTime);
	}

	state->beforeThreadInterruptAt(0);

	state->setWaiting();			// we set the thread to the waiting state, so that it updates its state only when it reenters VEX
									// because otherwise it might be considered a suspended thread, but be blocked in native waiting
	state->setTimedOut(false);

//	runnableThreads->update();
	VISUALIZE_EVENT(WAITING_RELEASED, state);
	LOG_LAST_VEX_METHOD(state)

}


//wwwwwwwwwwwwwwwwwwwwwwwwww
// parkNanos, epoll_wait
void ThreadManager::onWrappedTimedWaitingStart(VexThreadState *state, const long &startingTime, const long &timeout) {
	setCurrentThreadVT(startingTime, state);
	setTimedWaitingThread(state);
	unconditionalWakeup();
}

void ThreadManager::onWrappedTimedWaitingEnd(VexThreadState *state) {

//	if (returnValue == -1) {
//		cout << state->getName() << "(" << state-getId() << ") interrupted!!!" << endl;
//		virtualTimelineController->updateBlockedThreadTimestamp(state);
//		runnableThreads->update();
//	}
	state->setSuspended();
	unconditionalWakeup();
	state->onBlockedWaitingInVex();

	ThreadManager *managerAfterResume = state->getThreadCurrentlyControllingManager();
	virtualTimelineController->updateTimedOutWaitingThreadTimestamp(state);
	managerAfterResume->setRunningThread(state);
	if(runnableThreads->find(state)) {
		cout << "crashing for " << state->getName() << " " << state->getCurrentStateName() << endl;
		ps();
		vtfstacktrace(true, stderr, "failure");
		assert(false);
	}
	assert(state->isRunning());

}



// Object.wait() (without JVMTI), Monitor contention (without JVMTI)
void ThreadManager::onReplacedWaiting(VexThreadState *state) {

	unsetCurrentThreadFromRunningThread(state);
	unconditionalWakeup();

	state->onBlockedWaitingInVex();

	ThreadManager *managerAfterResume = state->getThreadCurrentlyControllingManager();
	virtualTimelineController->updateTimedOutWaitingThreadTimestamp(state);
	managerAfterResume->setRunningThread(state);

}


// Thread.sleep()
void ThreadManager::onReplacedTimedWaiting(VexThreadState *state, const long &timeout) {
	setTimedWaitingThread(state);
	unconditionalWakeup();

	state->onBlockedWaitingInVex();

	ThreadManager *managerAfterResume = state->getThreadCurrentlyControllingManager();
	virtualTimelineController->updateTimedOutWaitingThreadTimestamp(state);
	managerAfterResume->setRunningThread(state);
}

////INTERRUPTS - here state != VexThreadState::getCurrentThreadState (it is a different thread that interrupts)
//void ThreadManager::onWrappedWaitingInterrupt(VexThreadState *state, long interruptionTime) {
//
//}

void ThreadManager::onWrappedTimedWaitingInterrupt(VexThreadState *state, const long long &interruptionTime) {
	if (interruptionTime == 0) {
		virtualTimelineController->updateInterruptedWaitingThreadTimestamp(state, getCurrentGlobalTime());
	} else {
		virtualTimelineController->updateInterruptedWaitingThreadTimestamp(state, interruptionTime);
	}
	runnableThreads->update();
}

void ThreadManager::onAnyReplacedWaitingInterrupt(VexThreadState *state, const long long &interruptionTime) {
	//vtflog(managerDebug & mypow2(2), managerLogfile, "SCHEDULER*THREAD: entering onThreadTimedWaitingEnd\n");
	if (state != NULL) {
		virtualTimelineController->updateInterruptedWaitingThreadTimestamp(state, interruptionTime);
		long timeout = state->getTimeout();
		state->beforeThreadInterruptAt(objectRegistry);
//		state->setTimedOut(true);	// so that the scheduler does not wait for the real thread to set its own timedOut to true
//		cout << "onAnyReplacedWaitingInterrupt for thread " << state->getName() << " with timeout1 " << timeout << " and timeout2 " << state->getTimeout() << endl;
		if (timeout != 0 && timeout != -2) {
			runnableThreads->update();			// the scheduler will schedule the interrupted thread when its time comes
		} else {
			runnableThreads->push(state);
		}
	}
}


void ThreadManager::onWrappedBackgroundLoadExecutionStart() {
	VISUALIZE_TIME_EVENT(GC_STARTED, getCurrentGlobalTime())
	threadRegistry->forbidForwardLeaps();		// Hack to avoid virtual leaps forward
	Time::onBackgroundLoadExecutionStart();	// Start measuring GC time
}

void ThreadManager::onWrappedBackgroundLoadExecutionEnd(const long long &executionDuration) {

	commitBackgroundLoadDuration(executionDuration);

//	virtualTimeline->addTimeOfUnknownRealTimeDurationAndSynchronize(executionDuration);			// add background load time duration as unknown time
	threadRegistry->allowForwardLeaps();

	VISUALIZE_TIME_EVENT(GC_FINISHED, getCurrentGlobalTime())
}

//----------------------- METHODS EXECUTED BY APPLICATIONS THREADS ----------------------//
/*
 * Method enforcing the scheduling policy
 * @lock: should be externally locked
 */
bool ThreadManager::shouldCurrentThreadSuspend(VexThreadState *state) {
	if (state != NULL) {
		ThreadManager *manager; // = state->getThreadCurrentlyControllingManager();
		if (state->getAndResetForcedSuspendFlag() ||
			((manager = state->getThreadCurrentlyControllingManager()) == NULL ||
			manager->anotherThreadAlreadySetRunning(state))) {
			return true;
		} else {
			if (!state->isSuspendingAllowed()) {	// used to avoid interrupting a thread while holding an malloc-related kernel lock
				LOG(logger, logDEBUG4) << "Manager " << managerId << " thread isSuspendingAllowed for \""<< state->getName() << "\" is false ... I'm sorry" << endl;
				//cout << "Manager " << managerId << " thread isSuspendingAllowed for \""<< state->getName() << "\" is false ... I'm sorry" << endl;
				return false;
			}

			if ((state->getEstimatedRealTime() - state->getResumedLastAt()) < 0.75 * schedulerTimeslot) {
				LOG(logger, logDEBUG4) << "Manager " << managerId << " thread \""<< state->getName() << "\" should NOT be suspended because ERT - resumedLastAt = " << state->getEstimatedRealTime()/1000000 << " - " << state->getResumedLastAt()/1000000 << " = " << (state->getEstimatedRealTime() - state->getResumedLastAt()) << endl;
				return false;
			}

			//WORKING VERSION if (nextRunnable != NULL && (state->getEstimatedRealTime() > nextRunnable->estimatedRealTime)) {// (state->getEstimatedRealTime() - nextRunnable->estimatedRealTime) > schedulerTimeslot) {
			if (runnableThreads->isNextRunnableThreadBefore(state->getEstimatedRealTime())) {
				return true;
			} else {
				LOG(logger, logDEBUG4) << "Manager " << managerId << " thread \""<< state->getName() << "\" should NOT be suspended because is before nextThread" << endl;
			}
		}
	} else {
		printError("should suspend found NULL state - something is wrong");
	}
	return false;
}



/*
 * Yield the CPU to another thread
 *
 * Linux behaviour: wait for all other threads to be scheduled before you re-schedule yourself
 * Behaviour here: find thread with the highest estimated time after me and be scheduled after it
 */
void ThreadManager::onThreadYield(VexThreadState *state, const long long &startingTime) {
	//vtflog(managerDebug & mypow2(3), managerLogfile, "Thread %s %lld will yield at %lld\n", state->getName(), state->getUniqueId(), state->getEstimatedRealTime());
	long timeDiff = Time::getYieldDuration() + startingTime - state->getLastCPUTime();

	if (timeDiff > 0) {
		locklessUpdateTimeBy(timeDiff, state);
	} else {
		locklessUpdateTimeBy(Time::getYieldDuration(), state);
	}

	++timesYielding;

	// This check was removed because it made threads stay in yielding mode for a long time if no thread was before it
//	if (threadRegistry->areAnyOtherThreadsActiveInFormerTime(state, NULL) != NONE_ALIVE) {
		// This ensures that all *runnable* threads will run before this one
		long long highestTimeAfterMe = runnableThreads->getHighestRunnableTime(state);
		state->leapTo(highestTimeAfterMe + 1);

		suspendCurrentThread(state, 0, SUSPEND_OPT_DONT_UPDATE_THREAD_TIME | SUSPEND_OPT_FORCE_SUSPEND);
		// mutex is unlocked within suspend
//	}
}

/*****************************************************************
 *
 * RUNNING THREAD RESOURCE HANDLING
 *
 *****************************************************************/
void ThreadManager::lockRunningThread() {
	pthread_spin_lock(&runningThreadSpinLock);
}

void ThreadManager::unlockRunningThread() {
	pthread_spin_unlock(&runningThreadSpinLock);
}

VexThreadState *ThreadManager::getRunningThread() {
	lockRunningThread();
	VexThreadState *currentRunningThread = runningThread;
	unlockRunningThread();
	return currentRunningThread;
}

void ThreadManager::printRunningThread() {
	lockRunningThread();
	if (runningThread != NULL) {
		cout << "\t currently running thread: "<< runningThread->getName() << " last resumed at "<< runningThread->getResumedLastAt()/1000000 <<" [before "<<(getCurrentGlobalTime()-runningThread->getResumedLastAt())/1000000 <<"]" << endl;
	} else {
		cout << "\t No thread set as currently running\n" << endl;
	}
	unlockRunningThread();
}

void ThreadManager::printRunningThreadWithCore() {
	//cout << "Core " << managerId << " ("<<schedulerTid<<") at " << virtualTimelineController->getLocalTimeOfScheduler(managerId)/1000000 << " <<-," << this << ": ";
	cout << "Core " << managerId << " ("<<schedulerTid<<") at <<-," << this << ": ";
	printRunningThread();
}


void ThreadManager::printVirtualTimeForwardLeapSnapshots(const char *filename) {

	ofstream vflStatsFile(filename, fstream::out);

	vflStatsFile << "allowed timeRemaining timeout threadERT underCreation waitingThreads nwThreads activeInIoThreads blockedInIoThreads suspended running" << endl;

	vector<VirtualTimeForwardLeapSnapshot *>::iterator vit = vflStatistics.begin();
	while (vit != vflStatistics.end()) {
		VirtualTimeForwardLeapSnapshot *vv = (VirtualTimeForwardLeapSnapshot *)(*vit);
		vflStatsFile << *vv << endl;
		++vit;
	}
	vflStatistics.clear();
	vflStatsFile.close();
}



bool ThreadManager::anotherThreadAlreadySetRunning(VexThreadState *state) {
	lockRunningThread();
	bool isAnotherThreadAlreadySetRunning = (runningThread != NULL && runningThread != state);
	unlockRunningThread();
	return isAnotherThreadAlreadySetRunning;
}

bool ThreadManager::changeThreadStateToRunning(VexThreadState *state) {
	lockRunningThread();
	if (runningThread != NULL && runningThread != state) {
//		LOG(logger, logINFO) << "Manager " << managerId << " thread \""<< state->getName() << "\" ("<< state->getEstimatedRealTime()/1e6 << ") found " << ((runningThread != NULL)?runningThread->getName():"NULL") << " as running and should be suspended " << getAllCallingMethodsUntil(5) << endl;
		unlockRunningThread();
		return false;
	} else {
		state->setRunning();
		runningThread = state;
		state->setThreadCurrentlyControllingManager(this);

		unlockRunningThread();
		LOG(logger, logINFO) << "Manager " << managerId << " has now thread \""<< state->getName() << "\" ("<< state->getEstimatedRealTime()/1e6 << ") as running" << endl;// at " << getAllCallingMethodsUntil(5) << endl;
		return true;
	}
}

void ThreadManager::unsetCurrentThreadFromRunningThread(VexThreadState *state) {
	lockRunningThread();
    if (runningThread == state) {
		runningThread = NULL;
	}
    unlockRunningThread();
}

float const & ThreadManager::getRunningThreadScalingFactor() {
	lockRunningThread();
	VexThreadState *currentRunningThread = runningThread;
	unlockRunningThread();

	if (currentRunningThread != NULL) {
		return currentRunningThread->getTimeScalingFactor();
	} else {
		return currentTimeScalingFactor;
	}

}
void ThreadManager::unsetCurrentThreadFromRunningThreadAndWakeup(VexThreadState *state) {
	lockRunningThread();
    if (runningThread == state) {
		runningThread = NULL;
		unlockRunningThread();

		unconditionalWakeup();
	} else {
		unlockRunningThread();
	}
}

//void ThreadManager::increaseRunningThreadTimeBy(const long long &duration) {
//
//	lockRunningThread();
//	VexThreadState *state = runningThread;
//	unlockRunningThread();
//	if (state != NULL) {
//		state-> addGlobalTime(duration);
////		cout << state->getName() << " added GC duration " << duration/1000000 << endl;
//	}
//
//}

/**
 * Set a thread as the currently running thread of this processor
 * @lock: assumed thread lock held when entering
 * @return: true if the thread had to be suspended again before releasing
 */
void ThreadManager::setRunningThread(VexThreadState *state) {
	if (state != NULL) {

		state->setAwaken(false);
		if (changeThreadStateToRunning(state)) {
			virtualTimelineController->updateResumingSuspendedThreadTimestamp(state);

			assert(state->getThreadCurrentlyControllingManager() != NULL);
		} else {

			short alreadyInRunnable = 0;
			if (runnableThreads->find(state)) {
				alreadyInRunnable = SUSPEND_OPT_THREAD_ALREADY_IN_LIST;
			}

			suspendCurrentThread(state, 0, SUSPEND_OPT_FORCE_SUSPEND | SUSPEND_OPT_DONT_UPDATE_THREAD_TIME | alreadyInRunnable);

		}
	}

}


long long ThreadManager::getCurrentGlobalTime() {
	return virtualTimelineController->getGlobalTime();
}



/*
 * Set a thread into the runnable state:
 * - currentState = SUSPENDED
 * - in the threads list
 *
 * If a thread is already in the threads list it should not be pushed again.
 */
void ThreadManager::setSuspendedAndPushToRunnables(VexThreadState *state) {
	if (state != NULL) {
		unsetCurrentThreadFromRunningThread(state);
		state->setSuspended();
		state->setTimedOut(true);
		state->setTimeout(-1);
		runnableThreads->push(state);
	}
}

/*
 * Runnable queue management
 */
void ThreadManager::pushIntoRunnableQueue(VexThreadState *state) {
	runnableThreads->push(state);
}


void ThreadManager::updateRunnableQueue() {
	runnableThreads->update();
}



/*
 * Set waiting without re-inserting into the thread list
 */
void ThreadManager::setWaitingThread(VexThreadState *state) {
	if (state != NULL) {
		unsetCurrentThreadFromRunningThread(state);
		state->setWaiting();
		//vtflog(managerDebug & mypow2(3), managerLogfile, "Thread %s %lld became explicitly waiting\n", state->getName(), state->getUniqueId());
	}
}

/*
 * Set native waiting without re-inserting into the thread list - the thread will resume alone
 */
void ThreadManager::setNativeWaiting(VexThreadState *state) {
	if (state != NULL) {
		unsetCurrentThreadFromRunningThread(state);
		state->setNativeWaiting();
		threadRegistry->setNativeWaiting(state);
		VISUALIZE_EVENT(SET_NATIVE_WAITING, state);
	}
}


/*
 * Set waiting and re-insert into the thread list to be awakened when the timeout ends
 */
void ThreadManager::setTimedWaitingThread(VexThreadState *state) {
	if (state !=  NULL) {
		unsetCurrentThreadFromRunningThread(state);
		state->setWaiting();

		VISUALIZE_EVENT(SPECIAL_TIMED_WAIT, state);	// SPECIAL as in use global virtual time as timestamp

//		if (state->getTimeout() > 0) {
		runnableThreads->push(state);
//		}
	}
}


/*
 * Refactored internal I/O state setting method
 */
void ThreadManager::_setIoThread(VexThreadState *state, const bool &learning) {
	unsetCurrentThreadFromRunningThread(state);

	if (state->setIoAndCheckIfOperationShouldBlockSimulationProgress(learning)) {
		pushIntoRunnableQueue(state);
	}

}
/*
 * Set a thread into an I/O performing state
 */
void ThreadManager::setIoThread(VexThreadState *state, const bool &learning) {
	if (state != NULL) {
		_setIoThread(state, learning);
	}
}

/*
 * Set a thread in a system call performing state: using real time for measurement
 * but not allowing parallel execution and allowing to be suspended by scheduler
 */
void ThreadManager::setSystemCallThread(VexThreadState *state) {
	if (state != NULL) {
		state->setInSystemCall();
	}
}



/***
 * A version of suspendCurrentThread that does not use any locking, since a loose thread cannot be interrupted by
 * any scheduler. Timers and runnableThread queues have their own internal keys
 */
void ThreadManager::suspendLooseCurrentThread(VexThreadState *state, const long long & startingTime) {
	if (startingTime == 0) {
		updateCurrentThreadVT(state);
	} else {
		setCurrentThreadVT(startingTime, state);
	}

	threadRegistry->unsetNativeWaiting(state);

	if (minimizeTimeslotOnNw) {
		--numberOfNativeWaitingThreads;
		if (numberOfNativeWaitingThreads <= 0) {
			resetDefaultSchedulerTimeslot();
			cout << "resetting to default" << endl;
			numberOfNativeWaitingThreads = 0;
		}
	}

	if (!runnableThreads->empty()) {
		setSuspendedAndPushToRunnables(state);
		VISUALIZE_EVENT(SUSPEND_SELF, state);

		blockCurrentThread(state);
		LOG(logger, logINFO) << " Manager " << state->getThreadCurrentlyControllingManager()->getId() << ": suspendCurrentThread - " << state->getName() << " resumed at " << state->getEstimatedRealTime() << "/" << Time::getRealTime() << " AND GVT: " << virtualTimelineController->getGlobalTime() << endl;

		state->updateCpuTimeClock();
		VISUALIZE_EVENT(RESUME, state);

	} else {
		setRunningThread(state);
	}

}


/*
 * suspendCurrentThread method: Suspend the currently running thread
 *
 * A thread that is currently executed by VTF gets suspended
 * The suspending functionality follows a number of options as flags
 * Parameters:
 * state: the ThreadState of the thread to be suspended
 * startingTime: the timestamp of the virtual timeline when the suspend takes place
 * options: any combination of the following
 * SUSPEND_OPT_DONT_UPDATE_THREAD_TIME	: update the virtual timestamp of the thread (either set to startingTime if !=0, or update from globaltime)
 * SUSPEND_OPT_FORCE_SUSPEND			: always suspend
 * SUSPEND_OPT_THREAD_ALREADY_IN_LIST	: the thread is already in the runnable threads' list
 * SUSPEND_OPT_DONT_WAKE_UP_SCHEDULER	: you should not notify the scheduler that you got suspended
 *
 */
void ThreadManager::suspendCurrentThread(VexThreadState *state, const long long & startingTime, const char & options) {
	assert (state != NULL);

	// Update the thread's VT
	if (!(options & SUSPEND_OPT_DONT_UPDATE_THREAD_TIME)) {
		if (startingTime == 0) {
			updateCurrentThreadVT(state);
		} else {
			setCurrentThreadVT(startingTime, state);
		}
	}

	// Suspend only if you are forced to (but if at least one runnable or thread under creation exist) or there exists another thread whose VT < your updated(VT)
	if (((options & SUSPEND_OPT_FORCE_SUSPEND) &&
			(!runnableThreads->empty() || threadRegistry->atLeastOneThreadBeingSpawned()))
			|| shouldCurrentThreadSuspend(state)) {

		if (options & SUSPEND_OPT_THREAD_ALREADY_IN_LIST) {
			unsetCurrentThreadFromRunningThread(state);
			state->setSuspended();
		} else {
			setSuspendedAndPushToRunnables(state);
		}

		if (!(options & SUSPEND_OPT_DONT_WAKE_UP_SCHEDULER)) {
			unconditionalWakeup();
			LOG(logger, logDEBUG2) << " Manager " << managerId << ": suspendCurrentThread - " << state->getName() << " signals scheduler at " << state->getEstimatedRealTime()/1e6 << endl;
		} else {
			LOG(logger, logDEBUG2) << " Manager \"" << managerId << "\" : suspendCurrentThread - " << state->getName() << " NOT signalling scheduler at " << state->getEstimatedRealTime()/1e6 << endl;
		}


		LOG(logger, logINFO) << " thread " << state->getName() << " SUSPENDED self while under control of " << managerId << " at " << state->getEstimatedRealTime() << "/" << Time::getRealTime() << " AND GVT: " << virtualTimelineController->getGlobalTime() << " with runningthread = " << ((runningThread==NULL)?"NONE":runningThread->getName()) << endl; //<< " from " << getAllCallingMethodsUntil(8) << endl;

		assert(!state->wasAwaken());
		// This key has protected the thread being interrupted, while accessing resources: runningThread, threadRegistry, runnableThreads, manager->mutex


		VISUALIZE_EVENT(SUSPEND_SELF, state);

		blockCurrentThread(state);
		//vtflog(managerDebug & mypow2(13), managerLogfile, "(self) Resuming thread %s (%ld) resuming after receiving msg\n", state->getName(), state->tid);

		LOG(logger, logINFO) << " Manager " << state->getThreadCurrentlyControllingManager()->getId() << ": suspendCurrentThread - " << state->getName() << " resumed at " << state->getEstimatedRealTime() << "/" << Time::getRealTime() << " AND GVT: " << virtualTimelineController->getGlobalTime() << endl;

//		cout << "RESUMED " << state->getName() << " at " << state->getEstimatedRealTime() << "/" << Time::getRealTime() << endl;
//		assert(!keyHeldBy(threadId));

		if (!(options & SUSPEND_OPT_DONT_UPDATE_THREAD_TIME) && startingTime == 0) {
			state->updateCpuTimeClock();
		}

		VISUALIZE_EVENT(RESUME, state);

/*
		if(runnableThreads->find(state)) {
			cout << "Assertion failing from " << getAllCallingMethodsUntil(8) << " for thread " << state->getName() << " with options " << (int)options << " and timeout " << state->getTimeout() << " kai prediction " << state->getLastIoPrediction() << endl;
			cout << (*state) << endl;
			ps();
			assert(false);
			//assert(!runnableThreads->find(state));
		}
		*/
	} else if (state != getRunningThread()) {

		if (options & SUSPEND_OPT_THREAD_ALREADY_IN_LIST) {

//			if (runnableThreads->top() == state) {
			// -------------- Because here another manager can add a thread!!! --------------
//				runnableThreads->getNext();
//			} else {
				runnableThreads->erase(state);
//			}
		}

		// If for some reason - like just finished strict/normal IN IO state
		setRunningThread(state);

		if(runnableThreads->find(state)) {
			cout << "crashing for " << state->getName() << " " << state->getCurrentStateName() << endl;
			ps();
			vtfstacktrace(true, stderr, "failure");
			assert(false);
		}

	}

//	 else {
//		cout << "STO DISCARDED GIA TON  " << state->getName() << " (" << state->getCurrentStateName() << ") " << endl;
//		if (options & SUSPEND_OPT_THREAD_ALREADY_IN_LIST) {
//			runnableThreads->erase(state);
//		}
//	}


}


/*
 * When a thread finished simulating a model (or the model simulation for it finished by the scheduler)
 * put it back into the queue, unsetting the previous blocker state.
 */
void ThreadManager::suspendModelSimulationFinishingThread(VexThreadState *state) {
	char options = ThreadManager::SUSPEND_OPT_DONT_UPDATE_THREAD_TIME; // | ThreadManager::SUSPEND_OPT_FORCE_SUSPEND
	if (state->isWaitingRealCodeToCompleteAfterModelSimulation()) {
		options |= ThreadManager::SUSPEND_OPT_THREAD_ALREADY_IN_LIST;
	}
	suspendCurrentThread(state, 0, options);
}


void ThreadManager::commitIoDuration(VexThreadState *state, const long long &actualIoDuration) {
	virtualTimelineController->commitIoTimeProgress(state, actualIoDuration);
}


void ThreadManager::commitBackgroundLoadDuration(const long long &backgroundLoadDuration) {
	virtualTimelineController->commitBackgroundLoadExecutionTime(backgroundLoadDuration);
}


//-------------------------------------------- GLOBAL TIME-UPDATING METHODS -------------------------------------------//
/*
 * Such wrappers are used to facilitate the distributed version implementation - various threadmanager classes behave differently
 */

/*
 * Private methods to update the thread and global time called from external updating time methods
 */
void ThreadManager::locklessUpdateTimeBy(const long long &timeDiff, VexThreadState *state) {
	// FROM SINGLE CORE
//	state->addElapsedTime(timeDiff);
//	progressGlobalTimeByLockless(timeDiff+state->getAndResetLocalTime(), state);


	state->addLocalTime(timeDiff);	// if you ever replace this with add-elapsed time remember that scaling factor adjustments are in thread class only now
	if (state->isNativeWaiting()) {
//		cout << "VEX TIME UPDATE: thread " << state->getName() << " IS NATIVE WAITING AND SO updates local time by " << (state->getLocalTime())/1000000 << " on " << managerId << " at " << virtualTimelineController->getGlobalTime()/1000000 << endl;

//cout << state->getName() << " committing NATIVE WAITING time " << state->getLocalTimeSinceLastResume() << " at " <<virtualTimelineController->getGlobalTime()/1000000 << endl;
		virtualTimelineController->commitNativeWaitingProgress(state);
//cout << state->getName() << " committed NATIVE WAITING time " << state->getLocalTimeSinceLastResume() << " updating time to " <<virtualTimelineController->getGlobalTime()/1000000 << endl;
		//virtualTimelineController->progressGlobalTimeBy(state->getLocalTimeSinceLastResume());
	} else {
//		cout << state->getName() << " " << state->getResumedLastAt() << " " << state->getLocalTimeSinceLastResume() << endl;
//		cout << "VEX TIME UPDATE: thread " << state->getName() << " started at " << state->getResumedLastAt()/1000000 << " updates time to " << (state->getResumedLastAt()+state->getLocalTime())/1000000 << " on " << managerId << " at " << virtualTimelineController->getGlobalTime()/1000000 << endl;

		//virtualTimelineController->tryForwardTimeLeap(state->getResumedLastAt()+state->getLocalTimeSinceLastResume());
		virtualTimelineController->commitCpuTimeProgress(state);
	}

}

void ThreadManager::lockingUpdateTimeBy(const long long &timeDiff, VexThreadState *state) {
	locklessUpdateTimeBy(timeDiff, state);
}

/***
 * Virtual Time logging function - Probably the most important part of the code
 * @lock: should be externally locked
 */
void ThreadManager::updateCurrentThreadVT(VexThreadState *state) {
	long long currentTime = state->getVirtualTime();
	long long timeDiff = currentTime - state->getLastCPUTime();

	// Check whether the Virtual Time has progressed
	if (timeDiff > 0) {
		locklessUpdateTimeBy(timeDiff, state);
	}
	state->setLastCPUTime(currentTime);
}


/***
 * Virtual Time logging function II - Set a previously acquired measurement as the current time
 * of the thread
 * @lock: should NOT be externally locked
 */
// Lockless version of the previous call without the timeout condition
void ThreadManager::setCurrentThreadVT(const long long &startingTime, VexThreadState *state) {
	long timeDiff = startingTime - state->getLastCPUTime();
	// Check whether the Virtual Time has progressed

	if (timeDiff > 0) {
		// take into account the speedup
		locklessUpdateTimeBy(timeDiff, state);
	} else if (timeDiff < 0) {
//		cout << "TO NEGATIVE TIMEDIFF EINAI " << timeDiff << endl;
//		vtfstacktrace(true, stderr, "FOUND NEGATIVE TIMEDIFF");
//		assert(false);
//	} else {
//		cout << " VRIKA MIDENIKO timediff STO ThreadManager " << __LINE__ << endl;
	}
}



//----------------------- THREAD WAITING FUNCTIONS ----------------------//


void ThreadManager::blockCurrentThread(VexThreadState *state) {

	state->onBlockedWaitingInVex();

	// It could be a different manager
	ThreadManager *managerControllingThreadAfterResume = state->getThreadCurrentlyControllingManager();
	managerControllingThreadAfterResume->setRunningThread(state);	// if setRunning leads to a suspend (due to another thread running)
																	// then you unlock the key of the possibly new manager

}
//----------------------- THREAD ENDING FUNCTIONS ----------------------//
void ThreadManager::generateThreadStats(VexThreadState *state) {
 	if (schedulerStats) {
		pthread_mutex_lock(&threadStatsMutex);
		char *threadName = new char[256];
		strcpy(threadName, state->getName());
		pthread_mutex_unlock(&threadStatsMutex);
	}

}

/****
 * Very critical method that clear-ups the thread structures on its end
 * 
 * First suspect for bug-causes...
 */ 
void ThreadManager::onThreadEnd(VexThreadState *state) {

	updateCurrentThreadVT(state);
	state->setDead();

	threadRegistry->remove(state);	// Moved this from here - before notifying scheduler to resume next: this notifies the join thread first
	unsetCurrentThreadFromRunningThreadAndWakeup(state);

	LOG(logger, logINFO) << state->getName() << "("<< state->getId() << ") terminating execution as " << *state << endl;
	state->onVexExitWithoutTimeUpdate();

	VISUALIZE_EVENT(THREAD_END, state);
	generateThreadStats(state);

}


/************************************************************************* 
 *
 * BASIC SCHEDULER FUNCTIONS: resuming/suspending running threads and waiting
 *
 ************************************************************************/
 /*
 * Keeps the scheduler thread waiting (timed or forever)
 * @lock: releasing/acquiring the lock on sleep/wakeup
 * @return: true if time has expired false otherwise
 */
void ThreadManager::doControllerWait() {
	struct timespec ts;
	struct timeval tp;
	struct timeval current_tp,diff;
	long totalRemainingWaitingTime;

	int rc;
	long nsecAlreadyWaited = 0;
	float timeWaitingFactor;
	float previousTimeWaitingFactor = 1.0;

	clearPendingNotifications();

	do {
		totalRemainingWaitingTime = schedulerTimeslot;
//		if (!(runnableThreads->empty() || schedulerTimeslot <0)) {
		if (schedulerTimeslot > 0) {

			bool alreadyWaited = false;
			forceMinimumTimeslotSelection = false;

			do {
				if (alreadyWaited) {
					gettimeofday(&current_tp, NULL);
					timersub(&current_tp, &tp, &diff);
					nsecAlreadyWaited = (long)((double)(1000000000 * diff.tv_sec + 1000 * diff.tv_usec));// / (double)timeWaitingFactor);	//essentially previousTimeWaitingFactor
					previousTimeWaitingFactor = timeWaitingFactor;
				}

				timeWaitingFactor = getRunningThreadScalingFactor();
				assert (timeWaitingFactor > 0);

				nsecAlreadyWaited += decreaseSchedulerSleepingTimeBy;
				decreaseSchedulerSleepingTimeBy = 0;

				alreadyWaited = true;
				timeWaitingFactorChanged = false;

				if (forceMinimumTimeslotSelection) {
					totalRemainingWaitingTime = 1000000;
				} else {
					totalRemainingWaitingTime = (long)((double)((totalRemainingWaitingTime - nsecAlreadyWaited)*previousTimeWaitingFactor)/ (double)timeWaitingFactor) ;
				}

				if (totalRemainingWaitingTime>0) {
					clock_gettime(CLOCK_REALTIME, &ts);

					tp.tv_sec = ts.tv_sec;
					tp.tv_usec = ts.tv_nsec/1000;
					ts.tv_nsec += totalRemainingWaitingTime;

					while (ts.tv_nsec > 1000000000) {
						ts.tv_nsec -= 1000000000;
						ts.tv_sec += 1;
					}

					lockMutex();
                    if (likely(running) && likely(!schedulerWaitingForTimeslotExpiry)) {
                    	schedulerWaitingForTimeslotExpiry = true;
                    	rc = pthread_cond_timedwait(&cond, &mutex, &ts);
                    } else {

                    	schedulerWaitingForTimeslotExpiry = false;
						unlockMutex();
						return;
                    }

					if (schedulerStats) {
						if (rc == EINTR) {
							gettimeofday(&current_tp, NULL);
							timersub(&current_tp, &tp, &diff);
							durationsOfInterruptedTimeslots.addSample(1000000000 * diff.tv_sec + 1000 * diff.tv_usec);
						} else {
							++normalTimeslotsFinished;
						}
					}
					schedulerWaitingForTimeslotExpiry = false;
					unlockMutex();


				}

			} while (timeWaitingFactorChanged);
		} else {
			assert(false);
			rc = indefiniteWait();
		}
	} while (noThreadsToSchedule());

}

bool ThreadManager::noThreadsToSchedule() {
	return (runnableThreads->empty() && runningThread == NULL) || freeze!=0;
}

int ThreadManager::indefiniteWait() {
	return pthread_cond_wait(&cond, &mutex);
}


void ThreadManager::setSuspended(VexThreadState *state) {

	lastSuspended = runningThread;
	if (runningThread == state) {
		runningThread = NULL;
	}
	state->setSuspended();
	runnableThreads->push(state);	// thread might already be in the suspended state - it would not be normally pushed

}


/*
 * Set a thread in model simulation mode.
 * If the afterModelSimulation is true, then just insert a blocker into the queue
 */
void ThreadManager::setInModelSimulation(VexThreadState *state, const bool &afterModelSimulation) {

	if (afterModelSimulation) {
		// Model already simulated by the thread itself
		locklessUpdateTimeBy(0, state);
		state->ignoreThreadFromVEX();
		state->waitForRealCodeToComplete();

	} else {
		long long remainingTimeslice = schedulerTimeslot - (state->getEstimatedRealTime()-state->getLastTimeInHandler());
		if (remainingTimeslice > 0 && state->getTimeout() <= 0) {	// waiting threads should not continue immediately
			unsetCurrentThreadFromRunningThread(state);

			_simulateModel(state, remainingTimeslice);
			unconditionalWakeup();
			return;
		}
	}

	unsetCurrentThreadFromRunningThread(state);
	runnableThreads->push(state);
	unconditionalWakeup();

	// After this point the thread returns to executing real code
}


bool ThreadManager::keyHeldBy(const int &threadId) {
	return mutex.__data.__owner == threadId;
}

/*
 * Communicate with the signal handler code of the currently running thread to find out whether it should suspend according to the
 * scheduling policy and its performance counters
 */
//bool ThreadManager::isSignalledThreadGoingToSuspend(VexThreadState *state) {
//
//	// 2. ask the thread whether it should suspend (according to its CPU time)
//	++posixSignalsSent;
//	short threadsIntention = state->getSuspendingThreadIntention();
//	if (threadsIntention == TO_BE_SUSPENDED) {	// 3.1. thread should suspend, according to the scheduling policy
//		setSuspended(state);
//		LOG(logger, logINFO) << " thread " << state->getName() << " SUSPENDED by " << this->managerId << " as controller at " << state->getEstimatedRealTime()/1e6 << endl;
//		return true;
//
//	} else if (threadsIntention == TO_BE_DISREGARDED) {	// 3.2. thread is blocked though this was not identified by VTF->native code blocked
//		LOG(logger, logINFO) << " thread " << state->getName() << " set NATIVE WAITING by " << this->managerId << " as controller at " << state->getEstimatedRealTime()/1e6 << endl;
//		return true;
//
//	} else {	// 3.3. thread should continue running
//
//		// The thread state should not change (RUNNING->RUNNING or IN_SYSTEM_CALL->IN_SYSTEM_CALL)
//		lockRunningThread();
//		runningThread = state;
//		state->setThreadCurrentlyControllingManager(this);
//		unlockRunningThread();
//		LOG(logger, logINFO) << "Manager " << managerId << " has now thread \""<< state->getName() << "\" ("<< state->getEstimatedRealTime()/1e6 << ") as running" << endl;// at " << getAllCallingMethodsUntil(5) << endl;
//
////		state->increaseConsecutiveTimeslots();
////		decreaseSchedulerSleepingTimeBy = state->getEstimatedRealTime() - state->getResumedLastAt();	- crashes, because thread might have already continued and exited
//
//		decreaseSchedulerSleepingTimeBy += schedulerTimeslot/2;	// NEW in 415: limit timeslot, if thread decides to keep on running
//		return false;
//
//	}
//}
void ThreadManager::onIntentionToBeDisregarded(VexThreadState *state) {
	if (minimizeTimeslotOnNw) {
		++numberOfNativeWaitingThreads;
		if (numberOfNativeWaitingThreads > 0) {
			cout << "minimizing scheduler timeslot" << numberOfNativeWaitingThreads << endl;
			minimizeSchedulerTimeslot();
		}
	}
	LOG(logger, logINFO) << " thread " << state->getName() << " set NATIVE WAITING by " << this->managerId << " as controller at " << state->getEstimatedRealTime()/1e6 << endl;
}

void ThreadManager::onIntentionToKeepOnRunning(VexThreadState *state) {

	// The thread state should not change (RUNNING->RUNNING or IN_SYSTEM_CALL->IN_SYSTEM_CALL)
	lockRunningThread();
	runningThread = state;
	state->setThreadCurrentlyControllingManager(this);
	unlockRunningThread();
	LOG(logger, logINFO) << "Manager " << managerId << " has now thread \""<< state->getName() << "\" ("<< state->getEstimatedRealTime()/1e6 << ") as running" << endl;// at " << getAllCallingMethodsUntil(5) << endl;

//		state->increaseConsecutiveTimeslots();
//		decreaseSchedulerSleepingTimeBy = state->getEstimatedRealTime() - state->getResumedLastAt();	- crashes, because thread might have already continued and exited

	decreaseSchedulerSleepingTimeBy += schedulerTimeslot/2;	// NEW in 415: limit timeslot, if thread decides to keep on running
}

bool ThreadManager::isSignalledThreadGoingToSuspend(VexThreadState *state) {

	// 2. ask the thread whether it should suspend (according to its CPU time)
	++posixSignalsSent;
	short threadsIntention = state->getSuspendingThreadIntention();
	if (threadsIntention == TO_BE_SUSPENDED) {	// 3.1. thread should suspend, according to the scheduling policy
		setSuspended(state);
		LOG(logger, logINFO) << " thread " << state->getName() << " SUSPENDED by " << this->managerId << " as controller at " << state->getEstimatedRealTime()/1e6 << endl;
		return true;

	} else if (threadsIntention == TO_BE_DISREGARDED) {	// 3.2. thread is blocked though this was not identified by VTF->native code blocked

		onIntentionToBeDisregarded(state);
		return true;

	} else {

		onIntentionToKeepOnRunning(state);
		return false;

	}
}


/*
 * Process the response of a signalled thread, when asked if it is still running
 */
bool ThreadManager::isSignalledThreadStillRunning(VexThreadState *state) {
	++posixSignalsSent;
	short threadsIntention = state->getSuspendingThreadIntention();
	if (threadsIntention == TO_BE_DISREGARDED) {
		return false;
	} else {
		return true;
	}
}

/**
 * Scheduler suspends a thread
 * @return: true if the thread was suspended, false otherwise
 * @lock: invoked only by scheduler - using its lock, locking thread lock
 */
bool ThreadManager::suspendThread(VexThreadState *state) {
	state->lockShareResourceAccessKey();
	bool isThreadSuspended = _suspendThread(state);
	state->unlockShareResourceAccessKey();
	return isThreadSuspended;
}

/**
 * Internal protected by suspending lock thread suspend method
 * @return: true if the thread was suspended or no thread was found running, false otherwise
 * @lock: locking/unlocking runningThread lock
 */
bool ThreadManager::_suspendThread(VexThreadState *state) {
	if (state != NULL && state == getRunningThread()) {
		//vtflog(managerDebug & mypow2(14), managerLogfile, "***SUSPENDING THREAD %s (%lld) at %lld\nSCHEDULER THREAD: before sending suspend signal to %s (%lld)\n", state->getName(), state->tid, getRealTime(jvmti_env), state->getName(), state->tid);
		// 1. Send async signal to make thread call its suspend routine
		if (tkill(state->getId(), SIGHUP) == 0) {

			return isSignalledThreadGoingToSuspend(state);

		} else {
			printError("suspendThread: problem sending async signal");
			runnableThreads->erase(state);
			return false;
		}
	} else {
		return true; // since no thread was found you can schedule someone else
	}
}

/**
 * Scheduler resumes a thread
 * @return: 1 if the thread has been successfully resumed, 0 otherwise
 * @assumption: the lock should be held when this is invoked  
 * @lock: locked 
 */
void ThreadManager::resumeThread(VexThreadState *state) {
	// always resume suspended threads regardless of running variable value
					
	// 2. Wait for the thread to release its controlling key when it atomically enters wait...
	state->waitForThreadToBlock();

	lastResumed = state;

	// NOTE: forbidden to set running here - this might lead to locking a locked thread
	// TODO: too much delegation to state - both managerId and pointer passed;
	state->onThreadResumeByManager(managerId);
	virtualTimelineController->updateResumingSuspendedThreadResumedLastTimestamp(state);
	state->setThreadCurrentlyControllingManager(this);

	LOG(logger, logINFO) << " thread " << state->getName() << " RESUMED by " << this->managerId << " as controller at " << state->getEstimatedRealTime()/1e6 << endl;
	state->signalBlockedThreadToResume();
	state->allowSignalledThreadToResume();

	currentTimeScalingFactor = state->getTimeScalingFactor();
	//vtflog(managerDebug & mypow2(14), managerLogfile,	"SCHEDULER*THREAD: resumeThread: RESUMED SUCCESFFULLY %s (%ld) after receiving ack\n", state->getName(), state->tid );

}


/***
 * A thread is explicitly set as waiting (probably deprecated)
 */
//void ThreadManager::onThreadExplicitlySetWaiting(const long long &startingTime, VexThreadState *state) {
//	setCurrentThreadVT(startingTime, state);
//	setWaitingThread(state);
//	wakeup();
//	// from here on you are *not* part of the VTF
//}
//
///*
// * An explicitly timed-waiting thread will be pushed into the queue and
// * remain there until the virtual time expires - even though it might return sooner than that from real time
// */
//void ThreadManager::onThreadExplicitlySetTimedWaiting(const long long &startingTime, VexThreadState *state, const long &timeout) {
//	setCurrentThreadVT(startingTime, state);
//	state->setExplicitWaiting();
//	state->setTimedWaiting(timeout);
//	setTimedWaitingThread(state);
//	unconditionalWakeup();
//}
//
///**
// * This call is made when an explicitly timed-waiting thread returns from real time.
// * If the thread was interrupted then just update its timestamp, move it possibly higher in the runnables list and let it be rescheduled.
// * Otherwise, wait until notified in virtual time
// */
//void ThreadManager::onThreadExplicitlyManageTimedWaitingAfterReturningFromRealTime(VexThreadState *state, const int &returnValue) {
//
//	state->unsetExplicitWaiting();
//
//	state->beforeThreadInterruptAt(objectRegistry);
//	state->setTimedOut(true);
//	if (returnValue == -1) {
//		cout << state->getName() << "(" << state-getId() << ") interrupted!!!" << endl;
//		virtualTimelineController->updateBlockedThreadTimestamp(state);
//		runnableThreads->update();
//	}
//
//	unconditionalWakeup();
//	state->unlockShareResourceAccessKey();
//
//	state->blockHereUntilSignaled();
//
//	// the key is atomically acquired here
//	// if scheduler interferes now: wait until currentState is running
//	state->lockShareResourceAccessKey();
//	ThreadManager *managerAfterResume = state->getThreadCurrentlyControllingManager();
//	virtualTimelineController->updateTimedOutWaitingThreadTimestamp(state);
//	managerAfterResume->setRunningThread(state);
//	assert(state->isRunning());
//}



/*
 * Underprediction handling - keep invalidating I/O predictions by doubling the predicted time
 */
void ThreadManager::recursivelyUpdatePendingIoRequests(VexThreadState *state, const long long &currentRealTime) {
	VexThreadState *nextThread = runnableThreads->top();
	if (nextThread != NULL) {
		if (nextThread->inIoPredictionPhase() && !nextThread->isIoPredictionStillValid(currentRealTime)) {
			nextThread = runnableThreads->getNext();
			recursivelyUpdatePendingIoRequests(nextThread, currentRealTime);
			runnableThreads->push(nextThread);
		}
	}
	state->invalidateIoPrediction();
}

void ThreadManager::handleIOPerformingThread(VexThreadState *state) {

	if (state->inIoPredictionPhase()) {

		if (!state->getIoFinishedBeforeLogging()) {
			long long currentRealTime = Time::getRealTime();

			if (state->isIoPredictionStillValid(currentRealTime)) {

				state->setInIoStale();

				// TODO: better to update scheduler time slices - does this even take effect?
				forceMinimumTimeslotSelection = true;		// used to wake-up as soon as possible and poll whether the I/O has finished
				++timesOfMinimumTimeslotForcing;
				// just put the thread state back
			} else {
				runnableThreads->invalidateExpiredIoPredictions(currentRealTime);
			}
		}

	} else {

		if (state->getTimeout() == -1) {
			cout << "Assertion failed: thread " << (*state) << " found learning IO with timeout =  " << state->getTimeout() << endl;
			assert(false); 	//otherwise a learning thread should not be here - this is only to enforce virtual timeouts
			//assert(state->getTimeout() != -1);
		}
		interruptTimedOutIoThread(state);

	}

}

/*
 * Very important method for allowing virtual leaps in real time. We would like to be able to leap forward
 * in virtual time, if nothing is to happen between the current time and the leaping position. For example
 * in the case where the only thread of the application is main and it is sleeping for 10sec, we can just
 * update the global time to +10sec and move on immediately. Similarly, if we have a simulated execution
 * (models) we may just leap forward.
 *
 * By far the most troublesome method: we must be able to leap forward into virtual time, if no other thread can make progress, but:
 * - not if new threads have been created but are not part of the VTF scheduler yet
 * - notifications have been recently sent (we don't know if they are actually delivered to some waiting thread)
 * - threads that are not in the runnable list are performing non-blocking I/O
 *
 * Threads on the list being in NATIVE_WAITING state are the most obscure ones, as we have no idea what
 * they are doing.
 * In cases like that, where we are not sure what the thread is doing we use some timing heuristics like:
 * - the leap is less than one scheduler timeslot (no huge error can be inflicted by the leap)
 * - the real duration since the thread started waiting exceed the virtual time. We use a factor according to the size of the leap to increase
 * the real time that should pass, if the leap is huge.
 */
bool ThreadManager::isValidToLeapInVirtualTimeTo(VexThreadState *state) {
	short aliveThreads;

	bool returnValue = false;

//	long long minimumMaybeAliveTime = LONG_LONG_MAX;	// the minimum time that a thread of maybe_alive state (like NATIVE_WAITING) has
	long long remainingTime = virtualTimelineController->getHowFarAheadInVirtualTimeTheThreadIs(state);

//	ps();
	if (state->isWaitingInRealTime()) {
		return returnValue;		// wait until timeout expires in real time
//		return false;

	} else if (remainingTime < 0) { //schedulerTimeslot) {
		returnValue = true;
		return returnValue;
//		return true;

//	} else if (Time::getRealTime() - state->getLastRealTime() < state->getTimeout()) {
//		return false;

	} else if (pendingNotifications == 0 && (aliveThreads = threadRegistry->areAnyOtherThreadsActiveInFormerTime(state, &remainingTime)) != AT_LEAST_ONE_ALIVE) {// && threadRegistry->isMainStillAlive()) {
		if (aliveThreads == MAYBE_ONE_ALIVE) {
			if (lastTotalThreadERT == 0) {
				lastTotalThreadERT = threadRegistry->getSummedErtOfAllThreads();
				lastGlobalVirtualTime = Time::getRealTime();
						//virtualTimelineController->getGlobalTime();
			} else {
				if (Time::getRealTime() - lastGlobalVirtualTime < 200000000) {
					returnValue = false;
					return returnValue;
				}

				long long temp = threadRegistry->getSummedErtOfAllThreads(); //virtualTimelineController->getGlobalTime();
				if (temp == lastTotalThreadERT && threadRegistry->areAllNativeWaitingThreadsBlockedAccordingToSystemState(state)) {
					if (remainingTime > defaultSchedulerTimeslot) {
//						threadRegistry->areAnyOtherThreadsActiveInFormerTimepollNativeWaitingThreads();
						cout << state->getName() << " leaping up: GVT INCREASE IS ZERO. Leap by " << remainingTime/1e6 << "ms leaping because aliveThreads = " << aliveThreads << " " << lastTotalThreadERT << " " << temp << " at " << lastGlobalVirtualTime << " and " << Time::getRealTime() << " and timeslot is " << defaultSchedulerTimeslot << endl;
						ps();
						cout << "============================" << endl;
					}
					lastTotalThreadERT = 0;

					returnValue = true;
					return returnValue;
//					return true;				// there is no progress whatsoever!
				} else {

//					cout << state->getName() << " NOT leaping because GVT progressed by " << (temp - lastGlobalVirtualTime)/1e6 << "ms" << endl;
					//TODO: threadRegistry->pollToFindWhichSupposedlyNativeWaitingThreadsAreMakingProgress();
					lastTotalThreadERT = 0;//temp;
				}
			}
			returnValue = false; //true;//false;
			return returnValue;
//			return false;
		} else {
			if (remainingTime > defaultSchedulerTimeslot) {
				cout << state->getName() << " leaping BECAUSE NONE_ALIVE THREAD by " << remainingTime/1e6 << "ms because aliveThreads = " << aliveThreads << " and timeslot is " << defaultSchedulerTimeslot << endl;
				ps();
				cout << "============================" << endl;
			}
			returnValue = true;
			return returnValue;
//			return true;
		}

//		if (aliveThreads == NONE_ALIVE || (aliveThreads == MAYBE_ONE_ALIVE && state->hasTimeoutExpiredInRealTime(minimumMaybeAliveTime, remainingTime) )) {


/*
//			if (remainingTime > 4000000000) {
			if (remainingTime > 40000000) {
				cout << "HUGE LEAP IN VIRTUAL TIME BY THREAD " << state->getName() << " by " << remainingTime << endl;
//				if (remainingTime > 1000000000000) {
					ps();
//					threadRegistry->forceNativeWaitingPrintTheirStackTraces();
//					exit(0);
//				}

			}
*/

//			return true;
//		}
	} else {
		lastTotalThreadERT = 0;
//		lastGlobalVirtualTime = 0;
		//returnValue = true;		// WTF IS THIS?????????????????
		returnValue = false;
	}

//	return false;
	return returnValue;
}
//
//bool ThreadManager::isValidToLeapInVirtualTimeTo(VexThreadState *state) {
//
//	threadRegistry->lockRegistry();
//	bool forbidden = threadRegistry->leapForbiddingRulesApply(state);
//
//	if (schedulerStats) {
//		struct vex_and_system_states vass;
//		int threadsUnderCreation = threadRegistry->getRegistryThreadsSystemAndVexStates(state, vass);
//		threadRegistry->unlockRegistry();
//
//		long long remainingTime = virtualTimelineController->getHowFarAheadInVirtualTimeTheThreadIs(state);
//		VirtualTimeForwardLeapSnapshot *vflnew = new VirtualTimeForwardLeapSnapshot(!forbidden,
//						remainingTime,
//						state->getTimeout(),
//						state->getEstimatedRealTime(),
//						threadsUnderCreation,
//						vass);
//		vflStatistics.push_back(vflnew);
//		cout << *vflnew << endl;
//
//		if (!forbidden && (remainingTime/1e6 > 30000.0)) {
//			cout << "MAS GAMISES PALI VRWMOPOUSTA" << endl;
//		}
//
//	} else {
//		threadRegistry->unlockRegistry();
//	}
//	return !forbidden;
//
//}


bool ThreadManager::testIsValidToLeapInVirtualTimeTo(VexThreadState *state) {
	return isValidToLeapInVirtualTimeTo(state);
}

/*
 * Used to interrupt timed-out threads in I/O
 */
void ThreadManager::interruptTimedOutIoThread(VexThreadState *state) {
	if (isValidToLeapInVirtualTimeTo(state)) {

		virtualTimelineController->commitTimedOutIoProgress(state);
//		virtualTimelineController->tryForwardTimeLeap(state->getEstimatedRealTime());

		//onThreadTimedWaitingEnd(state, state->getEstimatedRealTime());
		tkill(state->getId(), SIGALRM);
		state->setTimedOut(true);

		cout << "*****I/O operation timed out in virtual time" << endl;
		sleep(3);
		tkill(state->getId(), SIGALRM);
		cout << "*****I/O operation timed out in virtual time 2" << endl;
	} else {

		runnableThreads->push(state);

	}
}


/*
 * Interrupt a thread that is waiting for "timeout" time in virtual time
 *
 */
void ThreadManager::interruptTimedWaitingThread(VexThreadState *state) {

	if (isValidToLeapInVirtualTimeTo(state)) {
		virtualTimelineController->commitTimedWaitingProgress(state);
		++timedOutsSucceeded;
		onAnyReplacedWaitingInterrupt(state, 0);
		state->setTimedOut(true);
		resumeThread(state);

		VISUALIZE_EVENT(WAITING_TIMEOUT, state);

	} else {
		state->setCustom3();
		++timedOutsFailed;
		runnableThreads->push(state);

	}
}


bool ThreadManager::hasResumedModelSimulationRunToCompletion(VexThreadState *state, long long &totalExecutionTime) {
	state->setAwaken(false);

	bool modelSimulationFinished = state->resumeModelSimulation(totalExecutionTime);

	// Increases global time based on model simulation
	virtualTimelineController->commitModelSimulationProgress(state);

	if (state->isModelTimedWaiting()) {
		VISUALIZE_EVENT(WAIT, state);
	} else {
		VISUALIZE_EVENT(SUSPEND, state);
	}

	return modelSimulationFinished;

}

void ThreadManager::_simulateModel(VexThreadState *state, long long &totalExecutionTime) {
	// Handle model after this node(s) simulation
	if (hasResumedModelSimulationRunToCompletion(state, totalExecutionTime)) {

		state->updateCpuTimeAddingSimulatedVirtualTime();
		LOG(logger, logDEBUG2) << "Manager " << managerId << " denoting the end of model simulation for " << state->getName() << endl;
		state->waitForRealCodeToComplete();
		runnableThreads->push(state);
		state->notifyModelSimulationEnd();

	} else {
		// Threads waiting forever (blocked in queue) in a model should not be pushed back
		if (state->isActiveInModel()) {
			runnableThreads->push(state);
		}
	}
}


/*
 * Interrupt a thread that is waiting for "timeout" time in virtual time
 * Executed by a scheduler itself.
 */
void ThreadManager::resumeModelSimulation(VexThreadState *state) {


	LOG(logger, logDEBUG2) << "Manager " << managerId << " resuming model simulation for " << state->getName() << " at " << state->getEstimatedRealTime() << " and GVT " << getCurrentGlobalTime() << endl;
//	cout << "Manager " << managerId << " resuming model simulation for " << state->getName() << " at " << state->getEstimatedRealTime()/1000000 << " and GVT " << getCurrentGlobalTime()/1000000 << endl;
//	ps();
	if (state->isModelTimedWaiting()) {


		if (!isValidToLeapInVirtualTimeTo(state)) {
//			cout << "not valid jump of " << state->getName() << " because " << endl;
//			ps();
			runnableThreads->push(state);
			return;
		}
//		cout << state->getName() << " committing its waiting time " << state->getEstimatedRealTime()/1e6 << " at " << getCurrentGlobalTime()/1e6 << endl;
		virtualTimelineController->commitTimedWaitingProgress(state);
		long clearTimeout = -1;
		state->setTimeout(clearTimeout);

		VISUALIZE_EVENT(WAITING_TIMEOUT, state);
	} else {
		virtualTimelineController->updateResumingSuspendedThreadTimestamp(state);
		VISUALIZE_EVENT(RESUME, state);
	}

	long long totalExecutionTime = 1.2 * schedulerTimeslot;
	// Update timers
	state->setThreadCurrentlyControllingManager(this);
	state->onThreadResumeByManager(managerId);

	virtualTimelineController->updateResumingSuspendedThreadResumedLastTimestamp(state);

	_simulateModel(state, totalExecutionTime);

	if (state->isActiveInModel() && !state->isModelTimedWaiting() ) {
		afterModelSimulation();
	}
}

/*
 * This optimization affects scheduling with real threads - threads IN_MODEL may monopolize the
 * scheduler by keeping the lock constantly - reducing the timeslice to 1ms after simulating the model
 * gives threads enough time to be put into the queue (by themselves, other schedulers etc).
 */
void ThreadManager::afterModelSimulation() {
		decreaseSchedulerSleepingTimeBy = schedulerTimeslot - 1000000;	// do not sleep
}


/*
 * Signal a thread that is executing the real code of a method described by a performance model,
 * to identify its status. If the model is still running (its CPU time is increased since the
 * beginning of model simulation or since the last poll), then wait for it (blocking the entire
 * simulation - =all schedulers - if that thread's estimated real time in on top of queue).
 * Otherwise, it should be set as NATIVE WAITING and be disregarded, since it is probably blocked somewhere.
 *
 * If the thread is finishing as its getting signalled, then make sure that the thread acquires this
 * scheduler's lock in order to change its currentState to SUSPENDED.
 */
void ThreadManager::pollThreadToCheckIfActuallyRunning(VexThreadState *state) {
	if (tkill(state->getId(), SIGHUP) == 0) {
		LOG(logger, logDEBUG2) << "Manager " << managerId << " polling thread modelled in real code " << state->getName() << endl;

		if (!isSignalledThreadStillRunning(state)) {
			LOG(logger, logDEBUG2) << "Manager " << managerId << " blocked thread in modelled real code " << state->getName() << endl;
			runnableThreads->setThreadToNextBiggestErt(state);
			return;

		} // else do nothing, you will poll again in schedulerTimeslot ns
	} else {
		runnableThreads->erase(state);
	}
}


/*
 * Method that determines the course of action for the thread on the top of the runnable list
 * according to its state: SUSPENDED, I/O or TIMED_WAITING
 * @lock: no - invoked only by scheduler - using its lock
 */
bool ThreadManager::continueThread(VexThreadState *state) {
	
	if (state != NULL) {
		state->lockShareResourceAccessKey();

		VexThreadState *nextThreadState = runnableThreads->getNextIfEqualsElseReturnTop(state, this);	// loose sync - check nothing changed in the meantime
		if (state != nextThreadState) {
			LOG(logger, logDEBUG2) << "Manager " << managerId << " something changed in the meantime " << endl;//state->getName() << " != " << ((nextThreadState!=NULL)?nextThreadState->getName():"NULL") << endl;
			state->unlockShareResourceAccessKey();
			return continueThread(nextThreadState);
		}


		if (runtimeDebuggingEnabled) {
			switch (state->getState()) {
				case VexThreadStates::LEARNING_IO:  cout << "Handling thread " << state->getName() << "("<<state->getId() <<") at " << state->getEstimatedRealTime()/1e6 << "ms  learning I/O for method " << state->getCurrentMethodId() << endl; break;
				case VexThreadStates::IN_IO:  cout << "Handling thread " << state->getName() << "("<<state->getId() <<") at " << state->getEstimatedRealTime()/1e6 << "ms  performing I/O for method " << state->getCurrentMethodId() << " with prediction " << state->getLastIoPrediction() << endl; break;
				case VexThreadStates::WAITING: cout << "Handling thread " << state->getName() << "("<<state->getId() <<") at " << state->getEstimatedRealTime()/1e6 << "ms  in " << state->getCurrentStateName() << (state->isWaitingInRealTime()?"-EXPL ":" ") << "with timeout " << state->getTimeout() << endl; break;
				default: cout << "Handling thread " << state->getName() << "("<<state->getId() <<") at " << state->getEstimatedRealTime()/1e6 << "ms  in " << state->getCurrentStateName() << endl;
			}
		}

		LOG(logger, logDEBUG2) << "Manager " << managerId << " popped from runnables state " << state->getName() << " in " << state->getCurrentStateName() << endl;

		if (state->isSuspended()) {
			if (state->getTimedOut() || (state->getTimeout() != 0 && state->isNotAterInterruptedTimedParking())) {
				resumeThread(state);
				if (showLastContinued) {
					cout << "Resumed thread "  << state->getName() << " in " << state->getCurrentStateName() << endl;
				}

			} else {
				runnableThreads->push(state);
			}
		} else if (state->inIo()) {
			handleIOPerformingThread(state);
			if (showLastContinued) {
				cout << "Handled I/O thread "  << state->getName() << " in " << state->getCurrentStateName() << endl;
			}
		} else if (state->isTimedWaiting()) {
			interruptTimedWaitingThread(state);
			if (showLastContinued) {
				cout << "Interrupted waiting thread " << state->getName() << " in " << state->getCurrentStateName() << endl;
			}
		} else if (state->isSimulatingModel()) {
			resumeModelSimulation(state);

		} else if (state->isWaitingRealCodeToCompleteAfterModelSimulation()) {

			pollThreadToCheckIfActuallyRunning(state);

		} else if (state->isRunning()) {

			// This can happen in loosely synchronized multicore simulations
			// If the continued thread is in RUNNING state, then this means that this scheduler
			// has just resumed a time-scaled thread (which inserted a "blocker" ThreadState in the runnable
			// thread list to synchronize with the other cores). No need to do anything

			//--- THIS IS NOT CORRECT: OTHER THREADS MIGHT SURPASS THE BLOCKER WHILE YOU ARE JOKING WITH YOUR OWN THREAD
			//DO THIS WITHIN THE LIST: DO NOT REMOVE THE BLOCKER IN GETNEXT CALLS
			LOG(logger, logDEBUG2) << "Manager " << managerId << " CCCCCC " << *state << endl;
//			runnableThreads->push(state);
			assert(false);
		}

		state->unlockShareResourceAccessKey();
	}
	else {
//		// For some reason we tried to continue a NULL thread - sleep for 10% of the normal timeslice;
//		decreaseSchedulerSleepingTimeBy = 0.9 * schedulerTimeslot;
		limitNextTimeslice();
//		LOG(logger, logDEBUG2) << "Manager " << managerId << " setting 10% delay " << decreaseSchedulerSleepingTimeBy << " due to NULL thread found" << endl;
	}

	return true;
}


// Limit upcoming sleeping time to promptly detect if you are allowed to progress
void ThreadManager::limitNextTimeslice() {
	decreaseSchedulerSleepingTimeBy = 0.75 * schedulerTimeslot;
}

void ThreadManager::end() {
    lockMutex();
    running = false; // all Scheduler methods will from now exit immediately
    pthread_cond_signal(&cond);
    pthread_cond_wait(&cond, &mutex);
    unlockMutex();
}


void ThreadManager::setThreadToMaximumPriority() {
	struct sched_param param;
	param.__sched_priority = sched_get_priority_max(SCHED_RR);
	sched_setscheduler(0, SCHED_RR, &param);
}



void ThreadManager::suspendRunningResumeNext() {
	VexThreadState *currentRunningThread = getRunningThread();
	if ((currentRunningThread == NULL || suspendThread(currentRunningThread))) {
		continueThread(runnableThreads->top());
	}
}

/**
 * THE MAIN SCHEDULER LOOP
 */
void ThreadManager::start() {

	// Initializations
	Time::onThreadInit();
//	setThreadToMaximumPriority();

//
//	cpu_set_t set;
//	CPU_ZERO (&set);
//	CPU_CLR (0, &set);
//	CPU_SET (1, &set);
//	cout << "handler " << managerId << " BOUND to 1" << endl;
//	sched_setaffinity (0, sizeof (cpu_set_t), &set);

	schedulerThreadId = gettid();

	registerSignalHandler();
//	cout << "Registering SIGARLM for scheduler thread " << schedulerThreadId << endl;

	running = true;
	// The scheduling algorithm
	while (running) {
		doControllerWait();
		threadRegistry->pollNativeWaitingThreads();
		suspendRunningResumeNext();
	}

	lockMutex();
    pthread_cond_signal(&cond);     // notify terminator that you exited
    unlockMutex();

}
