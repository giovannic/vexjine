/*
 * ThreadManagerClient.cpp
 *
 *  Created on: 4 Oct 2010
 *      Author: root
 */
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <sys/time.h>
#include <sys/errno.h>
#include <syscall.h>
#include <signal.h>
#include <time.h>
#include <sstream>
#include <pthread.h>
#include <cassert>
#include "ThreadManagerClient.h"
#include "ThreadRegistry.h"
#include "Visualizer.h"

#include <iostream>
using namespace std;

#define BUFSIZE 128


ThreadManagerClient *managerClient = NULL;

inline int tkill(int tid, int sig) {
	// Decided here that no redefinition of sigaction action is needed after r110
//	lastSignaledTids[(lastSignaledPtr++)%100] = tid;
	return (int) syscall(SYS_tkill, tid, sig);
}


/*************************************************************************
 ****
 **** SERVER COMMUNICATION LAYER
 ****
 ************************************************************************/
/*
 * Listener thread: waits for requests from the central scheduler
 */
struct listenerArgs {
	ThreadManagerClient *manager;
	int fd;
};

void *startListenerThread(void *args) {
	struct listenerArgs *largs = (struct listenerArgs *)args;
	ThreadManagerClient *manager = largs->manager;
	manager->acceptServerRequests(largs->fd);

	return NULL;
}

void ThreadManagerClient::clientInit(VirtualTimeline *vt) {
	managerClient = this;
	std::stringstream stemp;
	stemp << "vtf_client_log.txt";
//	logger = new Log(stemp.str().c_str(), vt);//TODO: I BROKE IT
}

int ThreadManagerClient::createListenerThread(int port) {

	int listenerPort = initListenerThread(port+1);
	struct listenerArgs args;
	args.manager = this;
	args.fd = listenerSockFd;
	pthread_t thread;
	pthread_create(&thread, NULL, startListenerThread, (void *)&args);
	return listenerPort;

}

int ThreadManagerClient::initListenerThread(int port) {

	if((listenerSockFd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		fprintf(stderr,"VTF Server: can't open stream socket\n");
		return false;
	}

	int freeport = connectToNextFreePort(port);
	/* set the level of thread concurrency we desire */
	listen(listenerSockFd, 1);

	return freeport;
}


int ThreadManagerClient::connectToNextFreePort(int port) {

	struct sockaddr_in serv_addr;
	memset((char *) &serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(port);

	LOG(logger, logINFO) << "VTF client listener: will connect to port " << port << " (" << listenerSockFd << ")" << endl;
	int optval = 1;
	setsockopt(listenerSockFd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

	while(bind(listenerSockFd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		serv_addr.sin_port = htons(++port);		// connect to the next free port

		LOG(logger, logINFO) << "VTF client listener: will connect to port " << port << " (" << listenerSockFd << ")" << endl;

	}

	return port;
}

void ThreadManagerClient::acceptServerRequests(int fd) {

	// Initializations
	socklen_t servlen;
	struct sockaddr_in server_addr;
	servlen = sizeof(server_addr);

	LOG(logger, logINFO) << "VTF client listener: Starting accepting connections (" << fd << ")" << endl;

	int optval = 1;
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
	listenerSockFd  = accept(fd, (struct sockaddr *) &server_addr, &servlen);
	if (listenerSockFd <= 0) {
		cerr << "FATAL ERROR: VTF client listener: could not read request. Exiting " << endl;
		exit(0);
	}

	int readValue;
	while (1) {
		SchedulerRequest serverRequest;
		if ((readValue = read(listenerSockFd, &serverRequest, sizeof(SchedulerRequest))) > 0) {

			LOG(logger, logINFO) << "VTF client listener: server requested " << SchedulerRequest::codeToString(serverRequest.code)  <<endl;

			if (!serviceServerRequest(serverRequest)) {
				cerr << "Listener terminated" << endl;
				return;
			}
		} else {
			LOG(logger, logINFO) << "VTF client listener: could not read request " << readValue << endl;
			return;
		}
	}
}


/*
 * Connect to server methods
 */
bool ThreadManagerClient::initialize(char *hostname, int port) {

	// Store the listener port to transmit it to the scheduler
	int listenerPort = createListenerThread(port+1);

	if (!connectToServer(hostname, port)) {
		return false;
	}

	registerListenerThreadPort(listenerPort);

	return true;
}


bool ThreadManagerClient::connectToServer(char *hostname, int port) {
	struct sockaddr_in serveraddr;
	struct hostent *server;

	/* socket: create the socket */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		cerr << "VTF client: ERROR opening socket" << endl;
		return false;
	}

	/* gethostbyname: get the server's DNS entry */
	server = gethostbyname(hostname);
	if (server == NULL) {
		fprintf(stderr,"ERROR, no such host as %s\n", hostname);
		return false;
	}

	/* build the server's Internet address */
	bzero((char *) &serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	bcopy((char *)server->h_addr,
	(char *)&serveraddr.sin_addr.s_addr, server->h_length);
	serveraddr.sin_port = htons(port);

	/* connect: create a connection with the server */
	if (connect(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0) {
		cerr << "VTF client: ERROR connecting to "  << hostname << " and port "<< port<< endl;
		return false;
	}

	LOG(logger, logINFO) << "VTF client: Connected to VTF server "  << hostname << ":"<< port<< endl;
	return true;

}

void ThreadManagerClient::registerListenerThreadPort(int listenerPort) {

	if (write(sockfd, &listenerPort, sizeof(int)) < 0) {
		cerr << "VTF client: ERROR writing to socket" << endl;
		return;
	}

	if (read(sockfd, &listenerFdOfClientAtServer, sizeof(int)) < 0) {
		cerr << "VTF client: ERROR reading from socket" << endl;
		return;
	}
}


ThreadManagerClient::~ThreadManagerClient() {
	close(sockfd);

}



// TODO: style fixing
void ThreadManagerClient::sendSchedulerRequest(int code, VexThreadState *state, long long timestamp, const char &c) {
	LOG(logger, logDEBUG4) << "sending request " << SchedulerRequest::codeToString(code) << " for thread " << state->getName() << " with ptr " << state->getVtfServerStatePtr() << endl;
	SchedulerRequest request(code, state->getVtfServerStatePtr(), timestamp, c);
	request.send(sockfd);
}

void ThreadManagerClient::sendSchedulerRequest(int code, VexThreadState *state, long long timestamp, const int &i, const int &s) {
	LOG(logger, logDEBUG4) << "sending request " << SchedulerRequest::codeToString(code) << " for thread " << state->getName() << " with ptr " << state->getVtfServerStatePtr() << endl;
	SchedulerRequest request(code, state->getVtfServerStatePtr(), timestamp, i ,s);
	request.send(sockfd);
}

void ThreadManagerClient::sendSchedulerRequest(int code, float speedup, VexThreadState *state) {
	LOG(logger, logDEBUG4) << "sending request " << SchedulerRequest::codeToString(code) << " for thread " << state->getName() << " with ptr " << state->getVtfServerStatePtr() << endl;
	SchedulerRequest request(code, state->getVtfServerStatePtr(), speedup);
	request.send(sockfd);
}
void ThreadManagerClient::sendSchedulerRequest(int code, VexThreadState *state, long long timestamp) {
	LOG(logger, logDEBUG4) << "sending request " << SchedulerRequest::codeToString(code) << " for thread " << state->getName() << " with ptr " << state->getVtfServerStatePtr() << endl;
	SchedulerRequest request(code, state->getVtfServerStatePtr(), timestamp);
	request.send(sockfd);
}
void ThreadManagerClient::sendSchedulerRequest(int code, VexThreadState *state) {
	LOG(logger, logDEBUG4) << "sending request " << SchedulerRequest::codeToString(code) << " for thread " << state->getName() << " with ptr " << state->getVtfServerStatePtr() << endl;
	SchedulerRequest request(code, state->getVtfServerStatePtr());
	request.send(sockfd);
}

void ThreadManagerClient::sendSchedulerRequest(int code, int fd) {
	SchedulerRequest request(code, NULL, 0, fd, fd);
	request.send(sockfd);
}

void ThreadManagerClient::sendSchedulerRequest(int code) {
	SchedulerRequest request(code);
	request.send(sockfd);
}

int ThreadManagerClient::getSchedulerResponseInt() {
	int response;
	if (read(sockfd, &response, sizeof(int)) < 0) {
		cerr << "VTF client:: error reading int"  << getAllCallingMethodsUntil(5) << endl;
	}
	return response;
}

bool ThreadManagerClient::getSchedulerResponseBool() {
	bool response;
	if (read(sockfd, &response, sizeof(bool)) < 0) {
		cerr << "VTF client:: error reading bool" << getAllCallingMethodsUntil(5) << endl;
	}
	return response;
}

long long ThreadManagerClient::getSchedulerResponseLongLong() {
	long long response;
	if (read(sockfd, &response, sizeof(long long)) < 0) {
		cerr << "VTF client:: error reading long long " << getAllCallingMethodsUntil(5) << endl;
	}
	return response;
}

void ThreadManagerClient::getSchedulerResponseTimes(VexThreadState *state) {
	long long times[2];
	if (read(sockfd, times, 2 *sizeof(long long)) < 0) {
		cerr << "VTF client:: error reading long long " << getAllCallingMethodsUntil(5) << endl;
	}
	state->setEstimatedRealTime(times[0]);
	state->setVirtualTime(times[1]);
}












/*************************************************************************
 ****
 **** THE SIGNAL HANDLER FOR SUSPENDING LOCAL THREADS
 ****
 ************************************************************************/
/*
 * Thread signal handler
 * All invocations of manager->* methods are protected by the lock that is
 * possessed by the scheduler, which sends the tkill which triggers this
 * handler invocation.
 * @lock: fully locked up to 3.
 */
static void handler_SIGHUP_client(int sig) {

	// The scheduler will only continue if this thread is suspended - this means that all threads
	// are suspended once at a time. Thus using unprotected global variables (threadId)
	// BEFORE suspending the thread is valid

	VexThreadState *state = VexThreadState::getCurrentThreadState();
	if (state != NULL) {

		state->updateCurrentLocalTime();

		/////assert(managerClient->runningThread == state);
		short action = managerClient->askServerWhetherThreadShouldSuspend(state);
		LOG(managerClient->getLogger(), logDEBUG) << "VTF Client handler: remote suspend of " << state->getName() << " response is " << action << endl;

		state->notifySchedulerForIntention(action);

		if (action == TO_BE_SUSPENDED) {
			// 2. Set self as suspended - the scheduler will wait until you become suspended and set you back in the threads list
			state->setSuspended();
			LOG(managerClient->getLogger(), logDEBUG) << "thread " << state->getName() << " getting suspended in handler" << endl;

			// 3. Block in native code waiting for a signal from the scheduler
			state->blockHereUntilSignaled();

			state->setAwaken(true);
			LOG(managerClient->getLogger(), logDEBUG) << "thread " << state->getName() << " getting resumed in handler" << endl;

			ThreadManagerClient *newStateManager = (ThreadManagerClient *)state->getThreadCurrentlyControllingManager();
			newStateManager->setRunningThreadToState(state);	// remote
			state->notifySchedulerThatThreadResumed();
//			managerClient->lockMutex();
//			managerClient->setRunningThread(state);
//			managerClient->changeVexThreadStateToRunning(state);
//			managerClient->setRunningThreadToState(state);
//			managerClient->unlockMutex();
			LOG(managerClient->getLogger(), logDEBUG3) << "thread " << state->getName() << " getting resumed in handler has state " << state->stateToString(state->getCurrentThreadState())->c_str() << endl;
		} else if (action == TO_BE_DISREGARDED) {
			managerClient->unsetCurrentThreadFromRunningThread(state);
			state->setNativeWaiting();
			LOG(managerClient->getLogger(), logINFO) << "VTF client signal handler: setting " << state->getName() << " to native waiting because of stack trace" << endl;
		}

		state -> updateCpuTimeClock();

	}

}

// called from the signal handler
short ThreadManagerClient::askServerWhetherThreadShouldSuspend(VexThreadState *state) {
	long long currentRealTime = Time::getRealTime();
	long long buffer[2] = {0,0};

	buffer[0] = state->getAndResetLocalTime();
	if (state->isThreadBlockedInNativeWait(currentRealTime)) {
		long long cpuTimeDifferenceSinceLastResume = (state->getEstimatedRealTime() - state->getResumedLastAt())/state->getTimeScalingFactor(); //inaccurate
		long long realTimeDifferenceSinceLastResume = currentRealTime- state->getResumedLastAtReal();
		LOG(logger, logDEBUG) << "---- NATIVE WAITING BECAUSE " <<cpuTimeDifferenceSinceLastResume << "/" << realTimeDifferenceSinceLastResume << " local=" << state->getLocalTime() <<endl;

		//vtfstacktrace(true, stderr, state->getName());
		buffer[1] = TIME_CODE_DENOTING_THAT_THREAD_IS_NATIVE_WAITING;
	} else {
		buffer[1] = currentRealTime - state->getResumedLastAtReal();
	}
	if (write(managerClient->listenerSockFd, buffer, 2*sizeof(long long)) < 0) {
		cerr << "Failed to ask server whether thread should suspend" << endl;
		assert(false);
	}
	short action;
	if (read(managerClient->listenerSockFd, &action, sizeof(short)) < 0) {
		cerr << "Failed to get response from server when asking whether thread should suspend" << endl;
		assert(false);
	}
	return action;
}



/*************************************************************************
 ****
 **** CALLBACKS TO SERVER THREAD
 ****
 ************************************************************************/
/*
 * Signal the scheduler to wakeup
 */
int ThreadManagerClient::wakeup() {
	sendSchedulerRequest(REQUEST_WAKEUP);
	return 1;
}

int ThreadManagerClient::conditionalWakeup(VexThreadState *state) {
	sendSchedulerRequest(REQUEST_CONDITIONAL_WAKEUP);
	return 1; //getSchedulerResponseInt();
}

int ThreadManagerClient::unconditionalWakeup() {
	sendSchedulerRequest(REQUEST_UNCONDITIONAL_WAKEUP);
	return 1;
}

void ThreadManagerClient::notifySchedulerForVirtualizedTime(VexThreadState *state, const float &scalingFactor) {
	sendSchedulerRequest(REQUEST_NOTIFY_SCHEDULER_FOR_VT, state, state->getTimeScalingFactor());
}


/*************************************************************************
 ****
 **** THREAD FUNCTIONS: mostly called by threads from the agent callbacks
 ****
 ************************************************************************/
//---------------- THREAD STARTING FUNCTIONS ----------------------------//
// Register thread to catch SIGHUP signals with the defined handler
void ThreadManagerClient::registerSignalHandler() {
	struct sigaction action;
    action.sa_handler = handler_SIGHUP_client;			// Set up the structure to specify the new action.
    sigemptyset (&action.sa_mask);
    action.sa_flags = SA_NODEFER | SA_RESTART;	// Signals sent when within the signal handler should not be ignored
    sigaction (SIGHUP, &action, NULL);
}


long long ThreadManagerClient::getCurrentGlobalTime() {
	lockMutex();
	sendSchedulerRequest(REQUEST_GLOBALTIME);
	long long currentTime = getSchedulerResponseLongLong();

//	virtualTimelineController->setGlobalTimeTo(currentTime); // TODO: I BROKE THIS
	unlockMutex();
	return currentTime;
}

void ThreadManagerClient::leapToGlobalTime(VexThreadState *state) {
	sendSchedulerRequest(REQUEST_SET_GLOBAL_TIME, state);
}

/***
 * Virtual Time logging function - Probably the most important part of the code
 * @lock: should be externally locked - DON'T LOCK AGAIN
 */
void ThreadManagerClient::updateCurrentThreadVT(VexThreadState *state, bool dontUseWaitHeuristics) {
	sendSchedulerRequest(REQUEST_UPDATECURRENT_TIME, state, Time::getVirtualTime() - state->getLastCPUTime());
	state->setEstimatedRealTime(getSchedulerResponseLongLong());
//	virtualTimelineController->setGlobalTimeTo(state->getEstimatedRealTime());// TODO: I BROKE THIS
}


/**	*
 * Virtual Time logging function II - Set a previously acquired measurement as the current time
 * of the thread
 * @lock: should be externally locked
 */
void ThreadManagerClient::setCurrentThreadVT(const long long &presetTime, VexThreadState *state) {
	lockMutex();
	sendSchedulerRequest(REQUEST_SETCURRENT_TIME, state, presetTime - state->getLastCPUTime() + state->getAndResetLocalTime());

	state->setEstimatedRealTime(getSchedulerResponseLongLong());
//	virtualTimelineController->setGlobalTimeTo(state->getEstimatedRealTime());// TODO: I BROKE THIS
	unlockMutex();

}


/*
 * Put a new thread into the thread states data structure (whatever it is)
 * @lock: protected
 */
void ThreadManagerClient::addVexThreadState(VexThreadState *state) {

	// The scheduler cannot interrupt you here because it doesn't know your existence
	lockMutex();
	LOG(logger, logINFO) << "VTF client : registering thread "<< state->getName() << endl;
	threadRegistry->add(state);

	sendSchedulerRequest(REQUEST_ADD_NEW_THREADSTATE, state);
	VexThreadStateInfo threadStateInfoToSendToScheduler(state);
	if (write(sockfd, &threadStateInfoToSendToScheduler, sizeof(VexThreadStateInfo)) < 0) {
		cerr << "Failed to notify server about new thread" << endl;
		assert(false);
	}

	VexThreadState *tempState;
	if (read(sockfd, &tempState, sizeof(VexThreadState *)) < 0) {
		cerr << "Failed to receive state struct for new thread from server" << endl;
		assert(false);
	}
	state->setVtfServerStatePtr(tempState);

	LOG(logger, logDEBUG) <<  "VTF client: Thread " << state->getName() << " was registered in index with id value " << state->getUniqueId() << " and server-ptr " << state->getVtfServerStatePtr() << endl;
	unlockMutex();
	// Now the scheduler can interrupt you
}

/*
 * Check whether you should suspend yourself
 * @lock: SHOULD BE EXTERNALLY LOCKED
 */
bool ThreadManagerClient::shouldCurrentThreadSuspend(VexThreadState *state) {
	sendSchedulerRequest(REQUEST_SHOULD_CURRENT_THREAD_SUSPEND, state);
	return getSchedulerResponseBool();
}


/**
 * Set a thread as the currently running thread of this processor
 * @lock: assumed manager lock held when entering
 * @return: true if the thread had to be suspended again before releasing
 */
void ThreadManagerClient::setRunningThread(VexThreadState *state) {
	if (state != NULL) {

		LOG(logger, logDEBUG4) << "VTF client: setting thread to running from " << getAllCallingMethodsUntil(8) << endl;
		sendSchedulerRequest(REQUEST_SET_RUNNING, state);

		state -> setAwaken(false);
		if (getSchedulerResponseBool() == false) {
			suspendCurrentThread(state, 0, SUSPEND_OPT_FORCE_SUSPEND | SUSPEND_OPT_DONT_UPDATE_THREAD_TIME);
			//lockMutex(); //acquire the lock again
		}
		setRunningThreadToState(state);
	}

}

bool ThreadManagerClient::changeVexThreadStateToRunning(VexThreadState *state) {

	LOG(logger, logDEBUG4) << "VTF client: setting thread to running from " << getAllCallingMethodsUntil(8) << endl;
	sendSchedulerRequest(REQUEST_CHANGE_STATE_TO_RUNNING);
	setRunningThreadToState(state);
	return true;	// TODO: added arbitrarily
}

void ThreadManagerClient::setRunningThreadToState(VexThreadState *state) {
	state->setRunning();
	runningThread = state;
}

/*
 * Set a thread into the runnable state:
 * - currentState = SUSPENDED
 * - in the threads list
 *
 * If a thread is already in the threads list it should not be pushed again.
 * This requires special handling according to the I/O simulation policy selection
 */
void ThreadManagerClient::setRunnableThread(VexThreadState *state) {
	vtfstacktrace(true, stderr, "setRunnableThread");
	sendSchedulerRequest(REQUEST_SET_THREAD_RUNNABLE, state);
}


/*
 * Set waiting without re-inserting into the thread list
 */
void ThreadManagerClient::setWaitingThread(VexThreadState *state) {
	vtfstacktrace(true, stderr, "setNativeWaiting");
	sendSchedulerRequest(REQUEST_SET_WAITING, state);
}

/*
 * Set native waiting without re-inserting into the thread list - the thread will resume alone
 */
void ThreadManagerClient::setNativeWaiting(VexThreadState *state) {
	vtfstacktrace(true, stderr, "setNativeWaiting");
	sendSchedulerRequest(REQUEST_SET_NATIVE_WAITING, state);
}


/*
 * Set a thread into an I/O performing state
 *
 * The policies are the following:
 * - IO_SEQUENTIAL		: don't push back into the queue, but freeze scheduler
 * - IO_PARALLEL_STRICT : push back into the queue, when finished just update the queue.
 * - IO_PARALLEL_LAX	: don't push back into the queue, do that when finished
 */
void ThreadManagerClient::setIoThread(VexThreadState *state, bool learning) {
	vtfstacktrace(true, stderr, "setIoThread");
	sendSchedulerRequest(REQUEST_SET_IO_THREAD, state);
}

/*
 * Set a thread into the runnables queue in RUNNING state.
 *
 * Its controlling manager will be frozen and the rest will just do nothing if
 * this thread's state is on the top of the queue. This is used to allow
 * threads to run as long as they like in virtual time without affecting the
 * synchronization of the application
 */
void ThreadManagerClient::setVirtualTimeBlockingThread(VexThreadState *state) {

}

//----------------------- THREAD RUNNING FUNCTIONS ----------------------//
void ThreadManagerClient::onThreadSpawn(VexThreadState *state) {

	if (state == NULL) {
		return;
	}
	// Jump into current time - internal locking
	state->setManagingSchedulerFd(listenerFdOfClientAtServer);
	state->setVtfClientStatePtr(state);

	registerSignalHandler();
	//vtflog(managerDebug & mypow2(13), managerLogfile, "\nregisterCurrentThread: thread %s %lld\n",state->getName(),state->getUniqueId());

	state->waitForResumingThread();	// lock it here to block on the conditional wait in suspend
												// the key will almost always be held by the thread
	state->updateCpuTimeClock();			// no lock required in this case, because the VexThreadState is not yet visible to the global thread states data structure
	state->setRegistering();

	VISUALIZE_EVENT(THREAD_START, state);

	addVexThreadState(state);			// Populate the index data structure for this Thread state - scheduler still unaware of this thread

	// Always wake-up scheduler - what if the running thread gets into a while(condition==true) loop (with the condition becoming false by the other threads)
	suspendCurrentThread(state, 0, SUSPEND_OPT_DONT_UPDATE_THREAD_TIME);
	state->updateCpuTimeClock();

}


//uuuuuuuuuuuuuuuuu
/*
 * suspendCurrentThread method: Suspend the currently running thread
 *
 * A thread that is currently executed by VTF gets suspended
 * The suspending functionality follows a number of options as flags
 * Parameters:
 * state: the VexThreadState of the thread to be suspended
 * startingTime: the timestamp of the virtual timeline when the suspend takes place
 * options: any combination of the following
 * SUSPEND_OPT_DONT_UPDATE_THREAD_TIME	: update the virtual timestamp of the thread (either set to startingTime if !=0, or update from globaltime)
 * SUSPEND_OPT_FORCE_SUSPEND			: always suspend
 * SUSPEND_OPT_THREAD_ALREADY_IN_LIST	: the thread is already in the runnable threads' list
 * SUSPEND_OPT_DONT_WAKE_UP_SCHEDULER	: you should not notify the scheduler that you got suspended
 * SUSPEND_OPT_EXTERNALLY_LOCKED		: the lock is already acquired by the executing process
 *
 */
void ThreadManagerClient::suspendCurrentThread(VexThreadState *state, const long long & startingTime, const char & options) {

	if (state == NULL) {
		return;
	}

	// Never use totalTimeSinceLastUpdate = 0 since it denotes that time should be updated at the server
	long long totalTimeSinceLastUpdate = 1;
	if (!(options & SUSPEND_OPT_DONT_UPDATE_THREAD_TIME)) {
		totalTimeSinceLastUpdate = state->getTimeDifferenceUntil(startingTime);
		LOG(logger, logDEBUG4) << "VTF client: thread " << state->getName() << " updated time to " <<totalTimeSinceLastUpdate<< endl;
	}

	if (!(options & SUSPEND_OPT_DONT_MAKE_REQUEST)) {
		sendSchedulerRequest(REQUEST_TRY_SUSPEND_CURRENT_THREAD, state, totalTimeSinceLastUpdate, options);
	}

	LOG(logger, logDEBUG4) << "VTF client: thread "<< state->getName()<<" requested to be suspended and is waiting for decision" << endl;
	bool shouldThreadSuspend = getSchedulerResponseBool();


	if (shouldThreadSuspend) {
		state->setSuspended();

		unlockMutex();
		state->blockHereUntilSignaled();

		LOG(logger, logDEBUG4) << "VTF client: thread "<< state->getName()<<" resumed correctly 1/2" << endl;
		ThreadManagerClient *newStateManager = (ThreadManagerClient *)state->getThreadCurrentlyControllingManager();
		newStateManager->setRunningThreadToState(state);	// remote
		state->notifySchedulerThatThreadResumed();
		LOG(logger, logINFO) << "VTF client: thread "<< state->getName()<<" resumed correctly" << endl;
		//vtflog(managerDebug & mypow2(13), managerLogfile, "(self) Resuming thread %s (%ld) resuming after receiving msg\n", state->getName(), state->tid);
	} else {
		setRunningThreadToState(state);
		unlockMutex();
	}


	if (!(options & SUSPEND_OPT_DONT_UPDATE_THREAD_TIME) && startingTime == 0) {
		state->updateCpuTimeClock();
	}
}
//----------------------- THREAD WAITING FUNCTIONS ----------------------//
/*
 * A thread sets itself into the WAITING state
 */
void ThreadManagerClient::onThreadWaitingStart(const long long &startingTime, VexThreadState *state) {
	state->addInvocationPoints();

	lockMutex();
	state->updateThreadLocalTimeSinceLastResumeTo(startingTime);
	sendSchedulerRequest(NOTIFY_THREAD_WAITING_START, state, state->getAndResetLocalTime());
	unsetCurrentThreadFromRunningThread(state);
	state->setWaiting();

	VISUALIZE_EVENT(WAIT, state);

	unlockMutex();
}

void ThreadManagerClient::onThreadWaitingStart(VexThreadState *state) {
	state->addInvocationPoints();

	lockMutex();
	state->updateCurrentLocalTime();
	sendSchedulerRequest(NOTIFY_THREAD_WAITING_START, state, state->getAndResetLocalTime());
	unsetCurrentThreadFromRunningThread(state);
	state->setWaiting();
	VISUALIZE_EVENT(WAIT, state);
	unlockMutex();
}


/*
 * A previously WAITING thread sets itself wakes up and sets itself into the suspended threads list
 */
void ThreadManagerClient::onThreadWaitingEnd(VexThreadState *state) {
	if (!running)
		return;

	if (state != NULL) {
		lockMutex();
		// TODO: maybe this can be removed for performance
		leapToGlobalTime(state);
		//TODO: set awakening from join
		//state->setAwakeningFromJoin(false);
		state->setTimeout(-1);
		suspendCurrentThread(state, 0, 0);
	}

}


void ThreadManagerClient::onThreadContendedEnter(VexThreadState *state, const long long & presetTime) {

	lockMutex();
	state->setTimeout(0);
	state->setWaiting();	// actually all you need to do is not push state back into the runnable queue

	state->addInvocationPoints();

	sendSchedulerRequest(NOTIFY_THREAD_CONTENDED_ENTER, state, state->getTimeDifferenceUntil(presetTime));
	unlockMutex();

	state->updateCpuTimeClock();

}

/*
 * A previously WAITING thread sets itself wakes up and sets itself into the suspended threads list
 */
void ThreadManagerClient::onThreadContendedEntered(VexThreadState *state) {
	if (state == NULL) {
		return;
	}

	long long startingTime = state->getVirtualTime();
	state->updateThreadLocalTimeSinceLastResumeTo(startingTime);
	onThreadWaitingEnd(state);
	state->updateCpuTimeClock();
}



/**
 * Register a thread to be interrupted on a virtual timeout (nanoseconds).
 * The thread will add the expected timeout to its estimated real time in order to be scheduler by the scheduler only
 * when the timeout has expired. If
 * @assumptions: lock is NOT held when the method is called
 * @lock: lock
 */
void ThreadManagerClient::onThreadTimedWaitingStart(VexThreadState *state, long &timeout) {

	state->setTimedWaiting(timeout);
	unsetCurrentThreadFromRunningThread(state);

	sendSchedulerRequest(NOTIFY_THREAD_TIMED_WAITING_START, state, state->getEstimatedRealTime());

	unlockMutex();

	state->blockHereUntilSignaled();

	ThreadManagerClient *managerAfterResume = (ThreadManagerClient *)state->getThreadCurrentlyControllingManager();
	managerAfterResume->lockMutex();
	managerAfterResume->setRunningThreadToState(state);
//	setRunningThread(state);
}

/** SAME AS IN THREADMANAGER
 * Scheduler wakes up a timed waiting thread (which is already in the thread queue)
 * @assumptions: lock is NOT held when the method is called
 * @lock: SCHEDULER LOCK SHOULD BE HELD WHEN ENTERED
 * @returns: true if a thread was actually found waiting and was interrupted
 */
void ThreadManagerClient::onThreadTimedWaitingEnd(VexThreadState * state, const long  & interruptTime) {

	if (state != NULL) {
		LOG(logger, logINFO) << "VTF client: notified/interrupted thread " << state->getName() << " at " << interruptTime << " from " << getAllCallingMethodsUntil(4) << endl;
//		state->beforeThreadInterruptAt(interruptTime, objectRegistry);// TODO: I BROKE THIS
		sendSchedulerRequest(NOTIFY_THAT_THREAD_WAS_INTERRUPTED, state, interruptTime);

	}
}


//----------------------------------- THREAD I/O METHODS ----------------------------//
void ThreadManagerClient::onThreadIoStart(VexThreadState *state) {

//	if (!ioSimulator->isParallelLax()) {
//		suspendCurrentThread(state, 0, SUSPEND_OPT_DONT_UPDATE_THREAD_TIME);
//	}
	lockMutex();	// so that you are not suspended here
	sendSchedulerRequest(NOTIFY_IO_START, state, state->getAndResetLocalTime(), state->getIoInvocationPointHashValue(), state->getStackTraceHash());
	getSchedulerResponseBool();
	unlockMutex();
	state->updateClocks();
}


void ThreadManagerClient::onThreadIoEnd(const long long& realTimeValueOnExit, VexThreadState *state, const int &methodId) {

	lockMutex();
	sendSchedulerRequest(NOTIFY_IO_END, state, realTimeValueOnExit);
	suspendCurrentThread(state, 0, SUSPEND_OPT_DONT_UPDATE_THREAD_TIME | SUSPEND_OPT_DONT_MAKE_REQUEST);

//	ioSimulator->endIo(state, realTimeValueOnExit);

	lockMutex();
	sendSchedulerRequest(REQUEST_THREAD_TIMES, state);
	getSchedulerResponseTimes(state);
	unlockMutex();

	state->updateExitingMethodInfo(methodId);

}

/*
 * Yield the CPU to another thread
 *
 * Linux behaviour: wait for all other threads to be scheduled before you re-schedule yourself (according to javamex)
 * Behaviour here: check number of all runnable threads and wait for timeslot * their number
 */
void ThreadManagerClient::onThreadYield(VexThreadState *state, const long long &startingTime) {
	//vtflog(managerDebug & mypow2(3), managerLogfile, "Thread %s %lld will yield at %lld\n", state->getName(), state->getUniqueId(), state->getEstimatedRealTime());

	sendSchedulerRequest(NOTIFY_THREAD_BEFORE_YIELD, state);
	suspendCurrentThread(state, startingTime, 0);

	state->updateCpuTimeClock();

}

/****
 * Very critical method that clear-ups the thread structures on its end
 *
 * First suspect for bug-causes...
 * @lock: lock
 */
void ThreadManagerClient::onThreadEnd(VexThreadState *state) {

	if (state != NULL) {
		long long totalTimeSinceLastCommit = state->getLocalTime()+(state->getVirtualTime()-state->getLastCPUTime());
		lockMutex();
		LOG(logger, logINFO) << "*** EXITING "<< (*state) << " timeDiff=" <<totalTimeSinceLastCommit<< endl;
		sendSchedulerRequest(NOTIFY_THREAD_END, state, totalTimeSinceLastCommit);
		LOG(logger, logDEBUG2) << "*** EXITED thread "<< state->getName() << " removed state"<<endl;
		threadRegistry->remove(state);
		state->setDead();

		unlockMutex();


		generateThreadStats(state);

	}
}


/*************************************************************************
 *
 * BASIC SCHEDULER FUNCTIONS: resuming/suspending running threads
 *
 ************************************************************************/
/**
 * Scheduler suspends a thread
 * @return: true if the thread was suspended, false otherwise
 * @lock: invoked only by scheduler - using its lock
 */
bool ThreadManagerClient::suspendThread(VexThreadState *state) {

	LOG(logger, logINFO) << "VTF client: will try to suspend " << state->getName()<<endl;

	/* Set up the structure to specify the new action. */
	if (runningThread != NULL && runningThread == state && tkill(state->getId(), SIGHUP) == 0) {
		return isSignalledThreadGoingToSuspend(state);

	} else {
		LOG(logger, logINFO) << "VTF client: failed to suspend " << state->getName() << ": it is not the running thread" << endl;
		notifyServerThatThreadCannotBeSuspended();
		return false;
	}

	return true;

}

/*
 * Method used to notify the server that the thread could not be suspended, because:
 * - it is not the running thread (it has changed its runnign state)
 * - it's dead
 */
void ThreadManagerClient::notifyServerThatThreadCannotBeSuspended() {
	long long buffer[2];
	buffer[0] = TIME_CODE_DENOTING_THAT_THREAD_WAS_NOT_SUSPENDED;
	if (write(listenerSockFd, buffer, 2*sizeof(long long)) < 0) {
		cerr << "Failed to notify server that thread cannot be suspended" << endl;
		assert(false);
	}
}

/*
 * Recreates the thread list so that the top element has the minimum estimated virtual time
 */
void ThreadManagerClient::updateThreadList() {
	sendSchedulerRequest(REQUEST_UPDATE_THREAD_LIST);
}




/*
 * THE MAIN SCHEDULER LOOP
 */

bool ThreadManagerClient::serviceServerRequest(SchedulerRequest serverRequest) {

	VexThreadState *state = serverRequest.state;
	char ack = 'a';

	lockMutex();
	switch (serverRequest.code) {
		case SUSPEND_THREAD:
			if (suspendThread(state)) {
				LOG(logger, logINFO) << "Client: successfully suspended thread " << state->getName()<< endl;
			}
			break;

		case RESUME_THREAD:

			LOG(logger, logINFO) << "VTF Client: RESUMING THREAD " << state->getName()<< endl;
//			virtualTimelineController->setGlobalTimeTo(serverRequest.timestamp);// TODO: I BROKE THIS
			state->setResumedLastAt(serverRequest.timestamp);
			state->setEstimatedRealTime(serverRequest.timestamp);
			resumeThread(state);
			state->waitForResumingThread();
			if (write(listenerSockFd, &ack, sizeof(char)) < 0) {
				cerr << "Failed to send ACK" << endl;
				assert(false);
			}
			break;

		case TIMED_WAITING_END_THREAD:
//			virtualTimelineController->setGlobalTimeTo(serverRequest.timestamp);// TODO: I BROKE THIS
			state->setResumedLastAt(serverRequest.timestamp);
//			state->beforeThreadInterruptAt(serverRequest.timestamp, objectRegistry);// TODO: I BROKE THIS
			resumeThread(state);
			break;

		case PROCESS_EXIT:
			LOG(logger, logINFO) << "VTF Client: Received exiting message" << endl;
			close(listenerSockFd);
//			ThreadManager::unconditionalWakeup();
			unlockMutex();
			return false;

	}
	unlockMutex();
	return true;
}




/*
 * Utility function to print the thread states linked list
 * Note: Do not call any methods to avoid scheduler suspending
 * @lock: no lock - should only be called by scheduler
 */
void ThreadManagerClient::ps() {
	sendSchedulerRequest(REQUEST_PS);
}
void ThreadManagerClient::printVexThreadStates() {
	sendSchedulerRequest(REQUEST_PRINTTHREADSTATES);
}
/*
 * Print the scheduler statistics
 */
void ThreadManagerClient::printStats(const char *file_name) {
	sendSchedulerRequest(REQUEST_PRINTSTATS);
}

void ThreadManagerClient::end() {
	running = false;
	lockMutex();
	sendSchedulerRequest(REQUEST_TERMINATE, listenerSockFd);
//	LOG(logger, logINFO) << "Waiting indefinitely to be notified by clientlistener" << endl;
//	indefiniteWait();
	unlockMutex();
//	close(listenerSockFd);

}
