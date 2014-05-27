#include "ThreadState.h"
#include <unistd.h>
#include <math.h>
#include <cassert>
#include <string>
#include "ThreadManagerRegistry.h"
#include "ThreadManager.h"
#include "Visualizer.h"
#include "ModelHandler.h"

// The per-thread local buffer containing the ThreadState* for each thread.
__thread VexThreadState *VexThreadState::currentThreadState = NULL;

// The permanent thread state is used as a background storage of the DS
// when we want to avoid entering the VEX framework (which happens when we are executing models)
__thread VexThreadState *VexThreadState::permanentThreadState = NULL;


Visualizer *VexThreadState::visualizer = NULL;


ThreadManager *VexThreadState::defaultThreadManager;
ThreadManagerRegistry *VexThreadState::threadManagerRegistry;

using namespace VexThreadStates;

VexThreadState::VexThreadState() {
	init();
	jthreadId = gettid();
	name = new char[16];
	sprintf(name, "LWP-%ld", jthreadId);

}

VexThreadState::VexThreadState(const long &_threadId, char *_name) {
	init();
	jthreadId = _threadId;

	name = new char[strlen(_name)+1];
	strcpy(name, _name);
}

// Set default values to object parameters
void VexThreadState::init() {

	currentState = VexThreadStates::REGISTERING;
	previousState = VexThreadStates::UNKNOWN_STATE;
	jthreadId = 0;
	name = NULL;
	showMethodEntries = false;

	timers = new Timers();
	ioHandler = new IoHandler();
	modelHandler = new ModelHandler();
	scheduling = new Scheduling();

	// Lock used for suspending
	methodLog = new MethodLog();
	stats = new Statistics();

	nativeWaitingCriteria = NativeWaitingCriteriaFactory::getCriteria(scheduling, timers);
	measures = NULL;	// temp buffer of measures per thread

	tid = gettid();

	currentThreadState = this;
	permanentThreadState = this;

	parentThreadWaitingYouToJoin = NULL;
	alreadyUnparked = false;
	parked = false;
	afterInterruptedTimedParking = false;
	waitingInRealTime = false;

	vtfServerStatePtr = NULL;
	vtfClientStatePtr = NULL;
	managingSchedulerFd = 0; 	// denoting current process's scheduler
	timedOut = true;

}



#define PRINT_SIGNAL_STACK_TRACES 0
#if PRINT_SIGNAL_STACK_TRACES == 1
ofstream outputTraces;
static std::map<unsigned int, std::string > signalledTraces;
static std::map<unsigned int, unsigned int > signalledTracesTimes;
void printSignalledTraces() {
	outputTraces.open("signalled_traces", ios::out);
	std::map<unsigned int, std::string >::iterator signalledTracesIterator = signalledTraces.begin();
	while (signalledTracesIterator != signalledTraces.end()) {
		outputTraces << signalledTracesIterator->first << " (" << signalledTracesTimes[signalledTracesIterator->first] << ") => " << signalledTracesIterator->second << endl;
//		cout << signalledTracesIterator->first << " => " << signalledTracesIterator->second << endl;
		++signalledTracesIterator;
	}
	outputTraces.close();
}
#endif

/*
 * All these should take place offline - while the thread is not monitored anymore by the VTF
 */
VexThreadState::~VexThreadState() {
#if PRINT_SIGNAL_STACK_TRACES == 1
	printSignalledTraces();
#endif

	if (name != NULL) {
		delete[] name;
		name = NULL;
	}

	//tid = 0;
	if (timers != NULL) {
		delete timers; timers = NULL;
	}
	if (methodLog != NULL) {
		delete methodLog; methodLog = NULL;
	}
	if (ioHandler != NULL) {
		delete ioHandler; ioHandler = NULL;
	}
	if (modelHandler != NULL) {
		delete modelHandler; modelHandler = NULL;
	}

	// Only local threads have their locks initialized
	if (managingSchedulerFd == 0) {
		//delete scheduling;		//TODO: this should be put back in
	}

	parentThreadWaitingYouToJoin = NULL;
	delete nativeWaitingCriteria;
	currentThreadState = NULL;
	permanentThreadState = NULL;
}


ThreadManager *VexThreadState::getCurrentlyControllingManagerOf() {

	ThreadManager *manager = getThreadCurrentlyControllingManager();
	while (manager == NULL) {
		getPreviouslyControllingManager()->suspendLooseCurrentThread(this, 0);
		manager = getThreadCurrentlyControllingManager();
	}
	return manager;
}

ThreadManager *VexThreadState::getCurrentSchedulerOnVexEntry() {
	inVex = true;
	lockShareResourceAccessKey();
	return scheduling->getThreadCurrentlyControllingManager();
}

ThreadManager *VexThreadState::getCurrentSchedulerOnVexEntry(const long long &startingTime) {
	inVex = true;

	if (ensureThreadIsNotInNativeWaitingStateWhenEnteringVex(startingTime)) {
		updateThreadLocalTimeSinceLastResumeTo(startingTime);// else the thread is updated inside the suspend code for the native waiting thread
	}

	lockShareResourceAccessKey();
	return scheduling->getThreadCurrentlyControllingManager();

}


ThreadManager *VexThreadState::getCurrentSchedulerOnVexEntryWithGuaranteedSuspend(const long long &startingTime) {
	addInvocationPoints();
	//		vtflog(managerDebug & mypow2(15), managerLogfile, "INTERACTION POINT: suspending thread %s (%lld)\n", getName(), tid);
	if (ensureThreadIsNotInNativeWaitingStateWhenEnteringVex(startingTime)) {
		lockShareResourceAccessKey();
		getThreadCurrentlyControllingManager()->suspendCurrentThread(this, startingTime, 0);	// if it is in native waiting do not suspend again
	} else {
		lockShareResourceAccessKey();
	}
	return getThreadCurrentlyControllingManager();	// might have changed since the variable declaration (meaning that we cannot initialize there)
}


void VexThreadState::onVexExitWithBothClocksUpdate() {

	unlockShareResourceAccessKey();
	inVex = false;

	timers->updateClocks();
}

void VexThreadState::onVexExitWithoutTimeUpdate() {
	unlockShareResourceAccessKey();
	inVex = false;
}

void VexThreadState::onVexExitWithCpuTimeUpdate() {
	onVexExitWithoutTimeUpdate();
	timers->updateCpuTimeClock();
}

void VexThreadState::onWrappedWaitingStart() {

	long long startingTime = timers->getThreadTimeBeforeMethodInstrumentation();
	ThreadManager *manager = getCurrentSchedulerOnVexEntry(startingTime);

	addInvocationPoints();

	setTimedOut(true);
	setTimeout(-1);
	setAwakeningFromJoin(false);		// used to avoid leaps incurred by delayed notifications to parents of joining threads
											// if the waiting was not join-induced nothing happens
	//setTimeoutFlagToDenotePossiblyResumingThread(); // FROM onThreadContendedEntered // The timeout flag is set here to 0 instead of -1. This denotes that the thread might become runnable at any point in the future, a piece of information that can be used to deter virtual leaps from happening
	setWaiting();	// actually all you need to do is not push state back into the runnable queue

	manager->onWrappedWaitingStart(this, startingTime);
	onVexExitWithBothClocksUpdate();
}

void VexThreadState::onWrappedWaitingEnd() {

	ThreadManager *manager = getCurrentSchedulerOnVexEntry();
	if (manager == NULL) {
		manager = getPreviouslyControllingManager();
	}

	long long startingTime = getVirtualTime();
	long long cpuDifference = startingTime - getLastCPUTime();	// we do this because the CPU difference is added twice: one to the waiting time and one to the CPU time.
	long long timeBeforeStartingWaiting = getEstimatedRealTime() + cpuDifference;	// we therefore move the starting point of the waiting time by that difference

	setTimeout(-1);
	setAwakeningFromJoin(false);		// used to avoid leaps incurred by delayed notifications to parents of joining threads
											// if the waiting was not join-induced nothing happens
	setTimedOut(true);

	manager->onWrappedWaitingEnd(this, startingTime);

	updateWaitingTimeFrom(timeBeforeStartingWaiting);

	onVexExitWithCpuTimeUpdate();
}



void VexThreadState::onVexEntry() {
	long long startingTime = getVirtualTime();
	if (ensureThreadIsNotInNativeWaitingStateWhenEnteringVex(startingTime)) {
		updateThreadLocalTimeSinceLastResumeTo(startingTime);// else the thread is updated inside the suspend code for the native waiting thread
	}
	lockShareResourceAccessKey();
}

long long VexThreadState::getTimeOnVexEntry() {
	long long startingTime = getVirtualTime();
	if (ensureThreadIsNotInNativeWaitingStateWhenEnteringVex(startingTime)) {
		updateThreadLocalTimeSinceLastResumeTo(startingTime);// else the thread is updated inside the suspend code for the native waiting thread
	}
	lockShareResourceAccessKey();
	return startingTime;
}

long long VexThreadState::getRealTimeOnVexEntry() {
	long long startingTime = Time::getRealTime();
	updateThreadLocalTimeSinceLastResumeToRealTime(startingTime);
	updateCpuTimeClock();
	ensureThreadIsNotInNativeWaitingStateWhenEnteringVex(0);
	lockShareResourceAccessKey();
	return startingTime;
}


void VexThreadState::onVexExitWithRealTimeUpdate() {
	unlockShareResourceAccessKey();
	inVex = false;
	updateRealTimeClock();		// used to compare real time difference with virtual leap difference (extra guarantee for virtual leaps)
}

bool VexThreadState::ensureThreadIsNotInNativeWaitingStateWhenEnteringVex(const long long &startingTime) {
	bool threadIsNotInNativeWaiting = true;

	inVex = true;

	if (!isRunning() || getThreadCurrentlyControllingManager() == NULL) {
		setCustom2();
		lockShareResourceAccessKey();
		getPreviouslyControllingManager()->suspendLooseCurrentThread(this, startingTime);
		//managers->getDefaultManager()->suspendCurrentThread(state, startingTime, ThreadManager::SUSPEND_OPT_EXTERNALLY_LOCKED | ThreadManager::SUSPEND_OPT_FORCE_SUSPEND | ThreadManager::SUSPEND_OPT_DONT_WAKE_UP_SCHEDULER);
		unlockShareResourceAccessKey();
		threadIsNotInNativeWaiting = false;

		assert(getThreadCurrentlyControllingManager() != NULL);
	}

	if (!isRunning()) {
		cout << "Illegal state for thread " << getName() << ": " << getCurrentStateName() << endl;
	}
	assert(getThreadCurrentlyControllingManager() != NULL);
	return threadIsNotInNativeWaiting;
}



bool VexThreadState::onReplacedTimedWaiting(const long &objectId, const long &timeout) {
	onVexEntry();

	setWaitingObjectId(objectId);
	ThreadManager *manager = getThreadCurrentlyControllingManager();
	setTimedWaiting(timeout);

	manager->onReplacedTimedWaiting(this, timeout);
//	manager->onReplacedTimedWaiting(this, timeout); //stateManager->onThreadTimedWaitingStart(state, correctTimeout);

	assert(isRunning());

	return getTimedOut();
}



// The resumeShouldBeHandledInternally argument is used to distinguish between the case that an external mechanism (like JVMTI and MonitorWaited)
// will notify VEX after the resumption of the waiting thread, or whether this should take place internally in VEX.
bool VexThreadState::onReplacedWaiting(const long &objectId, const bool &resumeShouldBeHandledInternally) {
	onVexEntry();

	setTimedOut(true);
	setWaitingObjectId(objectId);
	setWaiting();

	if (resumeShouldBeHandledInternally) {
		setTimeout(-2);	// TODO: this is just a hack to change the behaviour upon interrupt for correct handling of non-Jvmti using simulations
	}
	ThreadManager *manager = getThreadCurrentlyControllingManager();
	manager->onReplacedWaiting(this);

	if (resumeShouldBeHandledInternally) {
		setTimeout(-1);
	}
	assert(isRunning());
	return getTimedOut();
}



void VexThreadState::onBlockedWaitingInVex() {
//	onVexExitWithRealTimeUpdate();
	unlockShareResourceAccessKey();
	blockHereUntilSignaled();
	lockShareResourceAccessKey();
}



void VexThreadState::onWrappedTimedWaitingStart(const long &objectId, const long long &startingTime, const long &timeout) {
	setWaitingObjectId(objectId);
	waitingInRealTime = true;

	setTimedWaiting(timeout);

	ThreadManager *manager = getCurrentlyControllingManagerOf();
	manager->onWrappedTimedWaitingStart(this, startingTime, timeout);
	onVexExitWithCpuTimeUpdate();
}


void VexThreadState::onWrappedTimedWaitingEnd(ObjectRegistry *objectRegistry) {
	inVex = true;
	lockShareResourceAccessKey();
	waitingInRealTime = false;

	eraseWaitingObjectIdFromRegistry(objectRegistry);
	setTimedOut(true);

	ThreadManager *manager = getCurrentlyControllingManagerOf();
	manager->onWrappedTimedWaitingEnd(this);
	onVexExitWithRealTimeUpdate();

}






bool VexThreadState::isSomewhereInVex() {
	return inVex && !isWaiting();
}

long long VexThreadState::getCurrentState() {
	return currentState;
}



void VexThreadState::onInvocationPoint() {
	long startingTime = Time::getThreadTimeBeforeInteractionPoint();
	onVexEntry();
	addInvocationPoints();
	ThreadManager *manager = getCurrentlyControllingManagerOf();
	manager->suspendCurrentThread(this, startingTime, 0);
	onVexExitWithCpuTimeUpdate();
}


void VexThreadState::onYield() {
	long long startingTime = Time::getThreadTimeBeforeInteractionPoint(); // used to account for as lost time
	onVexEntry();
	addInvocationPoints();
	ThreadManager *manager = getCurrentlyControllingManagerOf();
	manager->onThreadYield(this, startingTime);
	onVexExitWithCpuTimeUpdate();
}



bool VexThreadState::setIoAndCheckIfOperationShouldBlockSimulationProgress(const bool &learning) {
	if(learning) {
		setLearningIo();
		// The operation may timeout - it will when the state is on the top of the stack
		if (getTimeout() != -1) {
			leapForwardBy(getTimeout());
			return true;
		}
	} else {
		setInIo();
		return true;
	}
	return false;
}


#if PRINT_SIGNAL_STACK_TRACES == 1
void logSignalledTrace(std::string &s, std::string actionOccured) {
	unsigned int hash = 0;
	for (unsigned int i = 0; i<s.length(); i++) {
		hash += s[i];
	}
	std::map<unsigned int, std::string >::iterator signalledTracesIterator = signalledTraces.find(hash);
	if (signalledTracesIterator == signalledTraces.end()) {
		signalledTraces[hash] = s;
		signalledTracesTimes[hash]=1;
//		cout << hash << " => " << s << " ....... " << actionOccured << endl;
	} else {
		++(signalledTracesTimes[hash]);
	}
}
#endif

void VexThreadState::onSignalledFromScheduler() {
	//assert(isRunning());

	// 1. Get current thread CPU time and update global timeline
	long long startingHandlerTime = getVirtualTime();

#if PRINT_SIGNAL_STACK_TRACES == 1
	//std::string stackTrace = getAllCallingMethodsUntil(6);
	std::string stackTraceC = getAllCallingMethodsUntil(10);
	std::stringstream stackTraceJava;
	stackTraceJava << stackTraceC << "#" << methodLog->getCurrentMethodId() << "#";
	std::string stackTrace = stackTraceJava.str();
#endif

//	vtfstacktrace(true, stderr, getName());

	// Polling to see whether model finished
	if (isWaitingRealCodeToCompleteAfterModelSimulation()) {
		assert(isWaitingRealCodeToCompleteAfterModelSimulation());

		//if (getWaitingInNativeVTFcode() == 0 && (startingHandlerTime - getLastCPUTime() < 200000)) {	// observation-based 200microseconds constant means blocked
		if (!inVex && (startingHandlerTime - getLastCPUTime() < 200000)) {	// observation-based 200microseconds constant means blocked
			notifySchedulerForIntention(TO_BE_DISREGARDED);
		} else {
			//setWaitingInNativeVTFcode(0);
			notifySchedulerForIntention(TO_KEEP_ON_RUNNING);
			setLastCPUTime(startingHandlerTime);
		}
#if PRINT_SIGNAL_STACK_TRACES == 1
		logSignalledTrace(stackTrace, "isWaitingRealCodeToCompleteAfterModelSimulation");
#endif
		return;
	}

	long long realHandlerTime = Time::getRealTime();

	assert(!isWaitingRealCodeToCompleteAfterModelSimulation());

	bool threadSuspendedInSystemCall = false;
	// Hack: move last CPU time to the past to include also the real time execution if a system call is run
	if (isInSystemCall()) {
		long long modifiedLastCpuTime = getLastCPUTime() - (realHandlerTime - getLastRealTime());
		setLastCPUTime(modifiedLastCpuTime);
		threadSuspendedInSystemCall = true;
		// The modified time is going to be used both for updating the thread's virtual timestamp,
		// as well as to avoid blocking it in native waiting
	}


	// Native waiting threads should check their total CPU time if polled to decide whether they are still native waiting
	if (isNativeWaiting()) {
		if (!isThreadStillBlockedInNativeWait(startingHandlerTime)) {
//				cout << "previously native waiting " << getName() << " will become suspended because it has run for " << ((startingHandlerTime - getLastCPUTime())) / 1e6 << " ms" << endl;
			ThreadManager *manager = getThreadCurrentlyControllingManager();
			if (manager == NULL) {
				manager = getPreviouslyControllingManager();		// if none, then a default manager (this of core 0) is returned
			}
			manager->suspendLooseCurrentThread(this, startingHandlerTime);
			unlockShareResourceAccessKey();

			LOG_LAST_VEX_METHOD(this)
			updateCpuTimeClock();
		} else {
			clearThreadBeingCurrentlyPolledForNativeWaiting();	// flag used to allow scheduler to poll again in a bit
		}

#if PRINT_SIGNAL_STACK_TRACES == 1
		logSignalledTrace(stackTrace, "isAlreadyInNativeWaiting");
#endif
		return;
	}

	ThreadManager **stateManager = getThreadCurrentlyControllingManagerPtr();
	if (stateManager == NULL) {
#if PRINT_SIGNAL_STACK_TRACES == 1
		logSignalledTrace(stackTrace, "manager NULL");
#endif
		return;
	}

	(*stateManager)->setCurrentThreadVT(startingHandlerTime, this);
	// 2. Disregard thread if in native waiting/suspend it if not and it should be suspended/let it run
	if (isThreadBlockedInNativeWait(realHandlerTime)) {
//		long long time = (*stateManager)->getCurrentGlobalTime()/1000000;

//		cout << "***********" << name << " thread thinks it should native wait" << endl;
//		vtfstacktrace(true, stderr, getName());

		(*stateManager)->setNativeWaiting(this);
		notifySchedulerForIntention(TO_BE_DISREGARDED);
		setResumedLastAt(getEstimatedRealTime());
		getAndResetLocalTime();
#if PRINT_SIGNAL_STACK_TRACES == 1
		logSignalledTrace(stackTrace, "becomes native waiting");
#endif
	//	assert(getWaitingInNativeVTFcode() == 0);
	} else if ((*stateManager)->shouldCurrentThreadSuspend(this)) {
		setSuspended();

		VISUALIZE_EVENT_OF_THIS_THREAD(SUSPEND);

		notifySchedulerForIntention(TO_BE_SUSPENDED);
		// Wait here
		blockHereUntilSignaled();

		lockShareResourceAccessKey();
		(*stateManager)->setRunningThread(this); // lightweight sync with scheduler: if after sleeping the sched doesn't find you running it will not re-suspend you
												 // also needed for updating the barrier of loose synchronisation policies
		if (threadSuspendedInSystemCall) {
			setInSystemCall();
		}
		unlockShareResourceAccessKey();

		VISUALIZE_EVENT_OF_THIS_THREAD(RESUME);
#if PRINT_SIGNAL_STACK_TRACES == 1
		logSignalledTrace(stackTrace, "suspend-resume");
#endif
	} else {

//		cout << getName() << " to keep on running at " << getEstimatedRealTime() << endl;
		// let the scheduler know that you have decided that you should NOT suspend
		notifySchedulerForIntention(TO_KEEP_ON_RUNNING);

		timers->updateLastResumedTo(realHandlerTime);
#if PRINT_SIGNAL_STACK_TRACES == 1
		logSignalledTrace(stackTrace, "keeps on running");
#endif
	}


	//LOG_LAST_VEX_METHOD(this)   // avoid logging from signal handler
	// Update time counters
	if (threadSuspendedInSystemCall) {
		updateClocks();
	} else {
		updateCpuTimeClock();
	}

}



void VexThreadState::haltSuspendForAwhile() {
	scheduling->haltSuspendForAwhile();
}


//unordered_map<int, float> *methodTimeScalingFactors
bool VexThreadState::onVexMethodEntry(const int & methodId, const float &methodTimeScalingFactor) {
	long long startingTime = getThreadTimeBeforeMethodInstrumentation();
	if (ensureThreadIsNotInNativeWaitingStateWhenEnteringVex(startingTime)) {
		updateThreadLocalTimeSinceLastResumeTo(startingTime);	// otherwise time is updated in suspend
	}

	assert(getThreadCurrentlyControllingManager() != NULL);
	onEntry(methodId);

	bool shouldChangeVTFactorOnExit = false;
	// Change accelerator factor, only if it was previously unset, and right after logging time without any virtual factor
	if (getTimeScalingFactor() == 1 && methodTimeScalingFactor != 1.0 && methodTimeScalingFactor != 0) {
		lockShareResourceAccessKey();
		assert(getThreadCurrentlyControllingManager() != NULL);
		getThreadCurrentlyControllingManager()->notifySchedulerForVirtualizedTime(this, methodTimeScalingFactor);
		unlockShareResourceAccessKey();

		shouldChangeVTFactorOnExit = true;
	}

	return logMethodEntry(methodId, shouldChangeVTFactorOnExit);
}



bool VexThreadState::logMethodEntry(const int &methodId, const bool &shouldChangeVTFactorOnExit) {
	MethodCallInfo *callinfo = getNextMethodCallInfo(methodId);
	if (callinfo == NULL) {
		fprintf(stderr, "Could not get next method info call in logMethodEntry\n");fflush(stderr);
		return false;
	}
	callinfo->setShouldResetVTFactor(shouldChangeVTFactorOnExit);


	pushMethodEntryEventIntoThreadMethodStack(callinfo);
	//	setCurrentMethodInfo(callinfo);

	//	if (methodId == 1623255268  || methodId == 295164315) {
	if (shouldShowMethodEntries()) {
		cout << getName() << "(" << getId() << ") entering " << methodId << endl;
	}


	// This is needed for synchronized tracking of the number of threads that are accessing a method at any point in time.
	// Reloading the method can only be accomplished if a single thread is entering the method and no other thread is running it
	//	unordered_map<int, MethodData *>::iterator rit = registeredMethodNames.find(methodId);
	//	if (rit != registeredMethodNames.end()) {
//	callinfo->setGlobalMethodDataOnEntry(registeredMethodNames[methodId]);
	//	}

	// this is the callinfo to be updated
	// for anything that happens within that method call from the calling thread
	onVexExit();
	return callinfo->isRecursive();
}


PerformanceMeasure *VexThreadState::logMethodExit() {

	MethodCallInfo *exitingMethodInfo = getExitingMethodInfo();
	if (exitingMethodInfo == NULL) {
		return NULL;
	}

	MethodCallInfo *callingMethodInfo = NULL;	// the return value
	stack<MethodCallInfo*>* threadStack = getThreadMethodStack();

	if (threadStack != NULL) {

		// Pointer to this thread's stack
		if (!threadStack->empty()) {
			MethodCallInfo *entryCallinfo = threadStack->top();

			if (shouldShowMethodEntries()) {
				cout << getName() << "(" << getId() << ") exiting " << exitingMethodInfo->getMethodId() << endl; //": " << (registeredMethodNames[exitingMethodInfo->getMethodId()])->getName() << endl;
			}
//			if (exitingMethodInfo->getMethodId() == 1623255268  || exitingMethodInfo->getMethodId() == 295164315) {
//				cout << "exiting " << exitingMethodInfo->getMethodId() << ": " << (registeredMethodNames[exitingMethodInfo->getMethodId()])->getName() << endl;
//			}

//			///assert(entryCallinfo->getMethodId() == exitingMethodInfo->getMethodId());
			if (entryCallinfo->getMethodId() != exitingMethodInfo->getMethodId()) {
				// TODO: why do we get 4 of these in the beginning? SPECjvm2008 Derby creates this
				// The next error can still be legally thrown by uncaught exceptions
//				fprintf(stderr, "Wrong method name on thread %ld, previous: %d (%s) and currently leaving %d (%s)\n", getUniqueId(), entryCallinfo->getMethodId(), registeredMethodNames[entryCallinfo->getMethodId()]->getName(), exitingMethodInfo->getMethodId(), registeredMethodNames[exitingMethodInfo->getMethodId()]->getName());

//				entryCallinfo->decreaseThreadsExecutingMethodCounter();
//assert(false);
				// TODO: Attempt to discard frames that were lost for any reason
				threadStack->pop();
				return logMethodExit();	// no match exit
			}

			//pop the entry time off the stack & calculate the execution time.
			threadStack->pop();

			// if this method was called by another update the latter's callee_time
			if (!threadStack->empty()) {
				callingMethodInfo = threadStack->top();
			} else {
				callingMethodInfo = NULL;	// no calling method
			}

			PerformanceMeasure *measure = getExitingMethodPerformanceMeasure();
			exitingMethodInfo->logTimesOnMethodExit(measure, entryCallinfo, callingMethodInfo);

			setCurrentMethodInfo(callingMethodInfo);
			decreaseNextMethodInfoToUse();

			// we don't delete the entryCallinfo, but let them stack to reuse them with the help of the nextMethodInfoToUse index
			return measure;

		} else {

			//The FOLLOWING MESSAGE CAN BE PRINTED IF WE PERFORME EXTERNAL INSTRUMENTATION
			//cout << getName() << " found an empty stack during exit - the last method entered was " << getCurrentMethodId() << " which is " << registeredMethodNames[getCurrentMethodId()] << endl;


			//assert(false);
//			fprintf(stderr, "Found thread %ld, but MethodCallInfo stack empty, during log of %d\n",getUniqueId(), exitingMethodInfo->getMethodId());
		}
	} else {
		fprintf(stderr, "Couldn't find thread MethodCallInfo stack!\n");
		assert(false);
	}

	setCurrentMethodInfo(callingMethodInfo);
	return NULL;
}




PerformanceMeasure *VexThreadState::onVexMethodExit(const int &methodId) {
	long long startingTime = getThreadTimeBeforeMethodInstrumentation();
	if (ensureThreadIsNotInNativeWaitingStateWhenEnteringVex(startingTime)) {
		updateThreadLocalTimeSinceLastResumeTo(startingTime);	//otherwise time updated internally in suspend
	}

	//cleanupTimeScalingOfThisMethod(state);
	if (getCurrentMethodInfo() != NULL) {
		if (getCurrentMethodInfo()->getShouldResetVTFactor()) {
			lockShareResourceAccessKey();
			getThreadCurrentlyControllingManager()->notifySchedulerForVirtualizedTime(this, 1.0);
			unlockShareResourceAccessKey();
		}
	}

	onExit(methodId);
	updateExitingMethodInfo(methodId);

	return logMethodExit();


}


void VexThreadState::onVexIoMethodEntry(const int &methodId, const int &invocationPointHashValue, const bool &possiblyBlocking) {

	long long startingTime = getThreadTimeBeforeIoMethodInstrumentation();
	if (ensureThreadIsNotInNativeWaitingStateWhenEnteringVex(startingTime)) {
		updateThreadLocalTimeSinceLastResumeTo(startingTime);	//otherwise time updated internally in suspend
	}

	onIoEntry(possiblyBlocking);
	// Temp solution to update times and suspend before I/O
	setLastCPUTime(startingTime);

	setIoInvocationPointHashValue(invocationPointHashValue);

}


void VexThreadState::onVexIoMethodExit() {

	long long startingTime = getThreadTimeBeforeIoMethodInstrumentation();		// a bit inexact
	inVex = true;
	if (isIgnoringIo()) {
		updateThreadLocalTimeSinceLastResumeTo(startingTime);
		//myfile << eventLogger->getMethodName(methodId) << " " << state->getName() << " " << (startingTime - state->getLastCPUTime()) << " vs real " << (realTimeValueOnExit-state->getLastRealTime()) << endl;
	}

	updateIoCPUTimeTo(startingTime);
	setIoFinishedBeforeLogging(true);	// used to avoid invalidating the I/O in real time

	//vtflog(agentDebug & mypow2(1), logFile, "beforeIoMethodExit: %s exited %d from point %d %lld (%lld)\n", state->getName(), methodId, state->getIoInvocationPointHashValue()*state->getStackTraceHash(), virtualTimelineController->getGlobalTime(), Time::getVirtualTime());
	addVtfInvocationPoint();	// used to avoid declaring threads NATIVE_WAITING if they mainly waited for acquisition of the managers->mutex lock
}



void VexThreadState::onSynchronizeOnModelExit(const int &methodId) {
	inVex = true;
	//if (modelSchedulerSim) {	// deprecated - only scheduler simulated
	blockUntilModelSimulationEnd();
	endModelSimulation();

	onExit(methodId);
	updateExitingMethodInfo(methodId);

	logMethodExit();

	//		VISUALIZE_METHOD_EVENT(METHOD_EXIT, state, methodId);
	updateCpuTimeClock();
	lockShareResourceAccessKey();
	ThreadManager *currentManager = getThreadCurrentlyControllingManager();
	if (currentManager == NULL) {
		getAndResetLocalTime();
		getPreviouslyControllingManager()->suspendLooseCurrentThread(this, 0);
	} else {
		currentManager->suspendModelSimulationFinishingThread(this);
	}
	unlockShareResourceAccessKey();

	assert(VexThreadState::forceGetCurrentThreadState() != NULL && VexThreadState::getCurrentThreadState() != NULL);
	onVexExit();
}





















bool VexThreadState::isThreadBlockedInNativeWait() {
	return isThreadBlockedInNativeWait(Time::getRealTime());
}

bool VexThreadState::isThreadBlockedInNativeWait(const long long &currentRealTime) {
	if (!isInSystemCall() && !inVex && !isThreadSetToBeForcefullySuspended() && nativeWaitingCriteria->isNativeWaiting(currentRealTime)) {
//		cout << "starting native waiting sleep "<<endl;
//		sleep(7);
//		cout << "ending native waiting sleep "<<endl;
//		long long cpuTimeDifferenceSinceLastResume = (timers->getEstimatedRealTime() - timers->getLastTimeInHandler());
//		long long realTimeDifferenceSinceLastResume = currentRealTime - timers->getLastRealTimeInHandler();

//		cout << name << " set to native waiting ####################" << endl;//realTimeDifferenceSinceLastResume << " and timediff =  " << cpuTimeDifferenceSinceLastResume << " and " << (0.001 * realTimeDifferenceSinceLastResume - cpuTimeDifferenceSinceLastResume) << " after " << scheduling->getConsecutiveTimeslots() << endl;
//		vtfstacktrace(true, stdout, name);
		return true;
	} else {

//		cout << name << " NOT NW inVex=" << inVex << " " << stats->lastVEXMethodInvoked << endl;
//		if (strcmp(name, "FlushManager") == 0) {
			//cout << name << " not set in NW because difference between CPU and rt is " << timers->getDifferenceBetweenCpuAndRealTime(currentRealTime) << " with ERT=" << timers->getEstimatedRealTime() << " LERT" << timers->getLastTimeInHandler() << " RT=" << currentRealTime << " and LRT=" << timers->getLastRealTimeInHandler() << endl;
//		}

		return false;
	}
}

bool VexThreadState::isThreadStillBlockedInNativeWait(const long long &cpuTimeAtBeginningOfSignalHandler) {
	return ((cpuTimeAtBeginningOfSignalHandler - getLastCPUTime()) < MAXIMUM_CPU_TIME_PROGRESS_WHILE_IN_NATIVE_WAITING && stackTraceBasedCriteria.isNativeWaiting());
}



// This function is required for built-in STL list functions like sort
bool VexThreadState::operator<(const VexThreadState &rhs) const {
   if( this->getEstimatedRealTime() < rhs.getEstimatedRealTime()) return true;
   return false;
}


//VexThreadState& VexThreadState::operator=(const VexThreadState &rhs) {
//   this->setLastCPUTime(rhs.getLastCPUTime());
//   //this->setVirtualTime(rhs.getVirtualTime());
//   this->setVirtualTime(rhs.getVirtualTime());
//   return *this;
//}

bool VexThreadState::operator==(const VexThreadState &rhs) const {
   if( this->getLastCPUTime() != rhs.getLastCPUTime()) return 0;
   if( this->getVirtualTime() != rhs.getVirtualTime()) return 0;
   return 1;
}

const char *VexThreadState::getCurrentStateName() {
	return stateToString(currentState)->c_str();
}

const char *VexThreadState::getPreviousStateName() {
	return stateToString(previousState)->c_str();
}

MethodCallInfo *VexThreadState::getNextMethodCallInfo(const int &methodId) {
	MethodCallInfo* callinfo;

	if (methodLog->getNextMethodInfoToUse() < methodLog->getMethodInfoStorage()->size()) {
		callinfo = (MethodCallInfo *)methodLog->getMethodInfoStorage()->at(methodLog->getNextMethodInfoToUse());
		callinfo -> setInfo(methodId, timers->getCurrentCpuTime(), getEstimatedRealTime(), timers->getMonitorWaitingTime(), timers->getIoWaitingTime());

	} else {

		//// Initializing event - protect "new" operator that makes use of low-level locks
		scheduling->lockShareResourceAccessKey();
		callinfo = new MethodCallInfo(methodId, timers->getCurrentCpuTime(), getEstimatedRealTime(), timers->getMonitorWaitingTime(), timers->getIoWaitingTime());
		scheduling->unlockShareResourceAccessKey();
		if (callinfo == NULL) {
			fprintf(stderr, "afterMethodEntry could not allocate new memory for MethodCallInfo\n");fflush(stderr);
			return NULL;
		}
		methodLog->getMethodInfoStorage()->push_back(callinfo);
	}

	methodLog->increaseNextMethodInfoToUse();

	return callinfo;
}

/*
 * Checking the status of the suspendflag of a thread to see whether it could be suspended
 */
bool VexThreadState::isTimedWaiting() {
	if (currentState == VexThreadStates::WAITING && getTimeout() > 0) {
		return true;
	} else {
		return false;
	}
}

bool VexThreadState::isModelTimedWaiting() {
	if (currentState == VexThreadStates::IN_MODEL && getTimeout() > 0) {
		return true;
	} else {
		return false;
	}
}


/*
 * Return the map where a thread keeps the performance durations of the methods it calls
 */
unordered_map<int, PerformanceMeasure*>* VexThreadState::getThreadPerformanceMeasures() {

	if (measures == NULL) {
		// this should only be executed on the first method call of each method
		scheduling->lockShareResourceAccessKey();
		measures = new unordered_map<int, PerformanceMeasure *>;	//hash map from methodId to perf measure
		scheduling->unlockShareResourceAccessKey();

		if (measures == NULL) {
			return NULL;
		}
	}
	return measures;
}

PerformanceMeasure *VexThreadState::getExitingMethodPerformanceMeasure() {
	int methodId = getExitingMethodInfo()->getMethodId();

	PerformanceMeasure *measure;
	measures = getThreadPerformanceMeasures();
	unordered_map<int, PerformanceMeasure*>::iterator measures_iterator = measures->find(methodId);
	if(measures_iterator != measures->end()) {
		measure = measures_iterator->second;
	} else {
		scheduling->lockShareResourceAccessKey();
		measure = new PerformanceMeasure(methodId);
		if (measure == NULL) {
			scheduling->unlockShareResourceAccessKey();
			return NULL;
		}
		(*measures)[methodId] = measure;	// local thread accounting - no sync needed
		scheduling->unlockShareResourceAccessKey();
	}
	return measure;
}



void VexThreadState::setCurrentQueueingNode(CinqsNode *node) {
	modelHandler->setCurrentNode(node);
}

void VexThreadState::setInModelSimulation(CinqsNode *source, Customer *customer) {
	setInModelSimulation();
	setCurrentQueueingNode(source);
	modelHandler->setCustomer(customer);
}
bool VexThreadState::resumeSimulation() {
	return modelHandler->resumeModelSimulation();	//returns if the simulation finished
}

void VexThreadState::initiateLocalResourceConsumption(const long long &totalTime) {
	modelHandler->initiateLocalResourceConsumption(totalTime);
}





/*
 * Utility function that transforms a VTF state to string
 * bool printCurrentState: print the current state (true) or the previous state (false)
 * @lock: no lock
 */
string *VexThreadState::stateToString(bool printCurrentState) {

	string *state = new string;
	if (state == NULL) {
		fprintf(stderr,"MEMORY ERROR\n");
		fflush(stderr);
		return NULL;
	}

	VexThreadStates::Code stateOfInterest = previousState;
	if (printCurrentState) {
		stateOfInterest = currentState;
	}

	(*state) = getStateName(stateOfInterest);
	return state;
}


string *VexThreadState::stateToString(VexThreadStates::Code stateOfInterest) {
	string *state = new string;
	*state = getStateName(stateOfInterest);
	return state;
}


// Called vefore a thread is interrupted to clear the registry from a monitor that is being waited on
void VexThreadState::eraseWaitingObjectIdFromRegistry(ObjectRegistry *objectRegistry) {
	if (timedWaitingObjectId != 0 && objectRegistry != NULL) {
		objectRegistry->erase(timedWaitingObjectId, getUniqueId());
	}
}

// Called vefore a thread is interrupted to clear the registry from a monitor that is being waited on
void VexThreadState::beforeThreadInterruptAt(ObjectRegistry *objectRegistry) {
	eraseWaitingObjectIdFromRegistry(objectRegistry);

	if (parked) {
		if (getTimeout() > 0) {
			afterInterruptedTimedParking = true;
		}
		parked = false;
	}

//	setEstimatedRealTime(interruptTime);	// has been moved to virtualTimelineController
	// timeout from -1 => to 0 to restart the counter
	// timeout from t  => remains t

	if (getTimeout() < 0) {		//  the second condition is used for interruptedTimedParkingValidationTest: it forbids an interrupted parked thread to be resumed before it executes the proper VEX wrapper after the park() method
		setTimeoutFlagToDenotePossiblyResumingThread(); 	// Non time-out waiting - will become -1 at Waited JVMTI call
	}
	// WHEN THE NEXT THREE LINES WHERE REMOVED THEN IT DID NOT WORK FOR AN INTERRUPTED THREAD,
	// AS THE MANAGER COULD ACQUIRE THE LOCK BEFORE THE THREAD AND BLOCK WAITING FOR IT
//	else {
//		setTimeout(-1);	// Time-out waiting
//	}

	setSuspended();
	setTimedOut(false);	// not timed-out, but interrupted
}


void VexThreadState::invalidateIoPrediction() {
	// If next thread is too far, you shouldn't allow it to run (so don't do nextThread -> estimatedRealTime)
	// If next threads are too close, you shouldn't allow all of them to run (problem of over-prediction)
	// Doubling the last prediction is probably a good compromise between the two
	timers->leapForwardBy(ioHandler->extendPredictionPeriod());	//2x then 6x then 14x
	setInIoInvalidation();
}

void VexThreadState::updateEstimatedRealTimeAfterIoPrediction(const long long &actualIoDuration) {
	// Every time an I/O prediction is invalidated we are doubling the prediction time used. This leads to the following correction:

//	cout << name << " actual: " << actualIoDuration << " total pred:" << ioHandler->getTotalTimePredicted() << endl;
	timers->leapForwardBy(actualIoDuration - ioHandler->getTotalTimePredicted());
}

//int VexThreadState::getIoHashValue() {
//	return methodLog->getCurrentMethodId();
//}


/*
 * Method used to check whether the last I/O prediction the thread made is still valid (at this REAL time point)
 */
long long VexThreadState::getPredictionError(const long long &currentRealTime) {
	return (currentRealTime - getLastRealTime() - ioHandler->getTotalTimePredicted());
}
//
//void VexThreadState::resetTimeCounters() {
//	timers->resetCounters();
//	clearStackTraceInfo();
//}

void VexThreadState::clearStackTraceInfo() {

}


bool VexThreadState::isIoPredictionStillValid(const long long &currentRealTime) {
	if (getPredictionError(currentRealTime) > 0) {
//cout << name << " not valid prediction " << (currentRealTime - lastRealTime) << " > " << lastIoPrediction << " pred started at " << lastRealTime << endl;
		return false;
	}
	return true;
}

/*
 * Method simulating a model by a thread manager one node at a time,
 * until the scheduler timeslot expires
 *
 * managerId: the id of the controlling manager
 * totalExecutionTime: the execution time to run in total (initially = scheduler timeslot)
 *
 * returns: true if the simulation is finished
 */
bool VexThreadState::resumeModelSimulation(long long &totalExecutionTime) {

	// Resume simulation from last node where it stopped for the duration of a scheduler timeslot
	bool modelSimulationFinished = false;
	long long localTimeServiced;
	do {

		long long initialThreadErt = getEstimatedRealTime();

		// Run any remaining service time
		if ((localTimeServiced = modelHandler->simulateLocalServiceTime(totalExecutionTime)) != 0) {
			addLocalTime(localTimeServiced);

		} else {
			modelSimulationFinished = modelHandler->resumeModelSimulation();

		}
		totalExecutionTime -= (getEstimatedRealTime()-initialThreadErt);

	} while (!modelSimulationFinished && isActiveInModel() && getTimeout() <= 0 && totalExecutionTime > 0);


	// Update global virtual time depending on whether the model refers to an internal or external source

	return modelSimulationFinished;

}



/***
 * Get string with name of thread state
 */
const char *VexThreadState::getStateName(VexThreadStates::Code stateOfInterest) {
	switch (stateOfInterest) {
		case RUNNING: 		return "RUNNING";
		case WAITING:		return "WAITING";
		case SUSPENDED: 	return "SUSPENDED";
		case LEARNING_IO: 	return "LEARNING_IO";
		case IN_IO: 		return "IN_IO";
		case TIMED_WAITING:	return "TIMED_WAITING";
		case IN_NATIVE:		return "IN_NATIVE";
		case IN_MODEL:		return "IN_MODEL";
		case REGISTERING: return "REGISTERING";
		case SUSPENDED_SELF: 	return "SUSPENDED_SELF";
		case NATIVE_WAITING: return "NATIVE_WAITING";
		case IN_IO_STALE:		return "IO_STILL_VALID";
		case NATIVE_IO_WAITING:		return "NATIVE_IO_WAITING";
		case IN_IO_INVALIDATION: return "IO_INVALIDATED";
		case RECOVERED_AFTER_FAILED_IO: return "RECOVERED_AFTER_FAILED_IO";
		case VEX_ZOMBIE: return "TERMINATING";
		case AWAKING_FROM_WAITING: return "AWAKING_FROM_WAITING";
		case WAITING_INTERNAL_SOCKET_READ: return "WAITING_TO_READ_FROM_SOCKET";
		case CUSTOM1: return "CUSTOM1";
		case CUSTOM2: return "CUSTOM2";
		case CUSTOM3: return "CUSTOM3";
		case IN_MODEL_WAITING: return "IN_MODE_WAITING";
		case AFTER_MODEL_SIM_WAITING_FOR_REAL_CODE: return "AFTER_MODEL_WAIT_REAL";
		case IN_SYSTEM_CALL: return "IN_SYSTEM_CALL";
		default: return "Unknown state";

	}
}




/*
 * Get the thread state as logged in the /proc/<pid>/status file
 */
char VexThreadState::getSystemThreadState() {
	char filename[32];
	sprintf(filename, "/proc/%ld/status", tid);

	char result = 'Z';
	ifstream ifile (filename);
	try {
		if (ifile.is_open()) {
			string line;
			while (getline(ifile, line)) {
				if (line.substr(0, 6) == "State:") {
					result = line.at(7);
					break;
				}
			}
		}
	} catch (...) {

	}
	ifile.close();
	return result;
}



bool StackTraceThreadState::methodAlreadyCalledInStackTrace(PerformanceMeasure *traceInfo, const int &methodId) {
	if (traceInfo == NULL) {
		return false;
	} else {
		if (traceInfo->getMethodId() == methodId) {
			return true;
		} else {
			return methodAlreadyCalledInStackTrace(traceInfo->getCallingMethod(), methodId);
		}
	}
}


StackTraceInfo *StackTraceThreadState::getNextMethodCallInfo(const int &methodId) {
	StackTraceInfo* callinfo;

	bool isRecursive = methodAlreadyCalledInStackTrace(currentStackTraceInfo, methodId);
	PerformanceMeasure *stackTraceInfo = getCurrentStackTraceInfo(methodId);

	if (methodLog->getNextMethodInfoToUse() < methodLog->getMethodInfoStorage()->size()) {
		callinfo = (StackTraceInfo *)methodLog->getMethodInfoStorage()->at(methodLog->getNextMethodInfoToUse());
		callinfo -> setInfo(methodId, timers->getCurrentCpuTime(), getEstimatedRealTime(), timers->getMonitorWaitingTime(), timers->getIoWaitingTime(), stackTraceInfo);
	} else {
		//// Initializing event
		scheduling->lockShareResourceAccessKey();
		callinfo = new StackTraceInfo(methodId, timers->getCurrentCpuTime(), getEstimatedRealTime(), timers->getMonitorWaitingTime(), timers->getIoWaitingTime(), stackTraceInfo);
		if (callinfo == NULL) {
			fprintf(stderr, "getNextMethodCallInfo could not allocate new memory for StackTraceInfo\n");fflush(stderr);
			scheduling->unlockShareResourceAccessKey();
			exit(0);
		}
		methodLog->getMethodInfoStorage()->push_back(callinfo);
		scheduling->unlockShareResourceAccessKey();
	}

	if (isRecursive) {
		callinfo -> setRecursive();
	}

	methodLog->increaseNextMethodInfoToUse();

	return callinfo;
}


/*
 * Return the map where a thread keeps the performance durations of the methods it calls
 * LOCKED EXTERNALLY
 */
PerformanceMeasure *StackTraceThreadState::getCurrentStackTraceInfo(const int &methodId) {
	if (currentStackTraceInfo == NULL) {
		// this should only be executed on the first method call of each method
		if (rootStackTraceInfos == NULL) {
			scheduling->lockShareResourceAccessKey();
			rootStackTraceInfos = new vector<PerformanceMeasure *>;
			scheduling->unlockShareResourceAccessKey();
		}

		vector<PerformanceMeasure *>::iterator viter = rootStackTraceInfos->begin();
		while (viter != rootStackTraceInfos->end()) {
			currentStackTraceInfo = *viter;
			if (currentStackTraceInfo->getMethodId() == methodId) {
				return currentStackTraceInfo;
			}
			++viter;
		}

		scheduling->lockShareResourceAccessKey();
		currentStackTraceInfo = new PerformanceMeasure(methodId);	//hash map from methodId to perf measure - no calling method
		rootStackTraceInfos -> push_back(currentStackTraceInfo);
		scheduling->unlockShareResourceAccessKey();
	} else {
		vector<PerformanceMeasure *> *currentSubmethods = currentStackTraceInfo->initGetSubMethods();
		vector<PerformanceMeasure *>::iterator viter = currentSubmethods->begin();
		while (viter != currentSubmethods->end()) {

			if ((*viter)->getMethodId() == methodId) {
				currentStackTraceInfo = (*viter);
				return currentStackTraceInfo;
			}
			++viter;
		}

		scheduling->lockShareResourceAccessKey();
//		cout << "new pm (down) for " << methodId << endl;
		currentStackTraceInfo = new PerformanceMeasure(methodId, currentStackTraceInfo);	// set father and move pointer in the same statement
		currentSubmethods->push_back(currentStackTraceInfo);								// add new pointer under the subtree of the father
		scheduling->unlockShareResourceAccessKey();
	}
	return currentStackTraceInfo;

}

PerformanceMeasure *StackTraceThreadState::getExitingMethodPerformanceMeasure() {
	PerformanceMeasure *exitingMethodPerfMeasure = ((StackTraceInfo *)getCurrentMethodInfo())->getCorrespondingStackTraceMeasure();
	currentStackTraceInfo = exitingMethodPerfMeasure->getCallingMethod();

	return exitingMethodPerfMeasure;
}

unsigned int DJBHash(const std::string& str) {
   unsigned int hash = 5381;

   for(std::size_t i = 0; i < str.length(); i++) {
      hash = ((hash << 5) + hash) + str[i];
   }

   return hash;
}


void StackTraceThreadState::clearStackTraceInfo() {
	if (rootStackTraceInfos != NULL) {
		delete rootStackTraceInfos;
		rootStackTraceInfos = NULL;
	}

	if (currentStackTraceInfo != NULL) {
		delete currentStackTraceInfo;
		currentStackTraceInfo = NULL;
	}
}


int StackTraceThreadState::getIoHashValue() {
	int ipStackTraceHash = 0;
	PerformanceMeasure *pm = currentStackTraceInfo;
	while (pm != NULL) {
		ipStackTraceHash += DJBHash(pm -> getMethodNameString());
		pm = pm -> getCallingMethod();
	}
	cout << ipStackTraceHash ;
	pm = currentStackTraceInfo;
	while (pm != NULL) {
		cout << ": " << (pm -> getMethodNameString());
		pm = pm -> getCallingMethod();
	}
	cout << endl;
	return ipStackTraceHash;
}

StackTraceThreadState::~StackTraceThreadState() {

}
