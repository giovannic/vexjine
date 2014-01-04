/*
 * ThreadManagerServer.cpp
 *
 *  Created on: 4 Oct 2010
 *      Author: root
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/uio.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/in.h>
#include <netdb.h>
#include <cassert>

#include <sstream>
#include <iostream>

#include "Logger.h"
#include "ThreadQueue.h"
#include "Visualizer.h"
#include "ThreadState.h"
#include "ThreadRegistry.h"
#include "EventLogger.h"
#include "IoSimulator.h"

/* the TCP port that is used for this example */
#define TCP_PORT   6500


using namespace std;
/* function prototypes and global variables */
void *do_chld(void *);
pthread_mutex_t lock;
int	service_count;


#include "ThreadManager.h"
#include "VirtualTimeline.h"
#include "ThreadManagerServer.h"
#include "ThreadManagerCommunicationProtocol.h"


ThreadManagerServer::~ThreadManagerServer() {
	if (logger != NULL) {
		delete logger;
	}
}


void *startMainThread(void *Args) {
	ThreadManagerServer *manager = (ThreadManagerServer *)Args;
	manager->acceptConnections();
	return NULL;
}


void ThreadManagerServer::startManagerServerThread() {
	pthread_create(new pthread_t, NULL, startMainThread, (void *)this);
}

/********* CONNECTION LAYER ************/
bool ThreadManagerServer::init(int port) {

	struct sockaddr_in serv_addr;

	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		fprintf(stderr,"VTF Server: can't open stream socket\n");
		return false;
	}

	memset((char *) &serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(port);

	int optval = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

	if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		fprintf(stderr,"VTF Server: can't bind local address\n");
		return false;
	}

	/* set the level of thread concurrency we desire */
	listen(sockfd, 5);

	return true;
}


struct serverThreadArguments {
	int sockfd;
	ThreadManagerServer *manager;
};


void ThreadManagerServer::acceptClientRequests(int clientSockFd) {
	while (1) {
		SchedulerRequest serverRequest;
		if (serverMessageRead(clientSockFd, &serverRequest, sizeof(SchedulerRequest)) > 0) {
			if (serverRequest.code != REQUEST_SETCURRENT_TIME) {
				LOG(logger, logINFO) << "VTF server: new request " << SchedulerRequest::codeToString(serverRequest.code) << " "  << ((serverRequest.state != NULL)?serverRequest.state->getName():"no thread involved") << endl;
			}
			if (serverRequest.code != PROCESS_EXIT) {
				LOG(logger, logINFO) << "note over clientListener,clientServer: " <<SchedulerRequest::codeToString(serverRequest.code) << " "  << ((serverRequest.state != NULL)?serverRequest.state->getName():"no thread involved") << endl;
			}
			serviceClientRequest(clientSockFd, serverRequest);
		} else {
			close(clientSockFd);
			return;
		}
	}

}

// TODO : refactor the write methods
bool ThreadManagerServer::serviceClientRequest(const int & clientSockFd, SchedulerRequest clientRequest) {

	VexThreadState *state = clientRequest.state;
	VexThreadState *newstate;
	int responseInt;
	bool responseBool;
	long long currentTime, timeDiff;
	long timeout;
	VexThreadStateInfo threadStateInfo;

	switch (clientRequest.code) {
		case REQUEST_GLOBALTIME:
			currentTime = virtualTimelineController->getGlobalTime();
			serverMessageWrite(clientSockFd, &currentTime, sizeof(long long));
			break;

		case REQUEST_ADD_NEW_THREADSTATE:
			newstate = new VexThreadState();

			serverLockMutex();
			serverMessageRead(clientSockFd, &threadStateInfo, sizeof(VexThreadStateInfo));

			LOG(logger, logDEBUG2) << "VTF Server read new threadstate" << endl;
			threadStateInfo.setInto(newstate);
			newstate->leapToGlobalTime(virtualTimelineController->getGlobalTime());
			newstate->setVtfServerStatePtr(newstate);
			newstate->setVtfClientStatePtr(threadStateInfo.clientStatePtr);
			newstate->setLastCPUTime(0);	// should always be zero

			// Let the client know of the server ptr

			LOG(logger, logDEBUG4) << "VTF server: write the server ptr for thread " << newstate->getName() << " to be " << newstate << endl;

			newstate->setRegistering();
			VISUALIZE_EVENT(THREAD_START, newstate);

			threadRegistry->add(newstate);
//			addThreadState(newstate);
			serverMessageWrite(clientSockFd, &newstate, sizeof(VexThreadState *));
			serverUnlockMutex();
			LOG(logger, logDEBUG) << "VTF Server added new remote thread " << newstate->getName() << endl;

			break;

		case REQUEST_WAKEUP:
			responseInt = -1;
			if (!runnableThreads -> empty()) {
				responseInt = pthread_cond_signal(&cond);
			}
			//TODO
			cout << "SHOULD WRITE MESSAGE BACK TO SERVER " << responseInt << endl;
//			serverMessageWrite(clientSockFd, &responseInt, sizeof(int));
			break;

		case REQUEST_CONDITIONAL_WAKEUP:
			responseInt = -1;
			if (!runnableThreads->empty() && runnableThreads->top() != state) {
				responseInt = pthread_cond_signal(&cond);
			}
//			serverMessageWrite(clientSockFd, &responseInt, sizeof(int));
			break;

		case REQUEST_UNCONDITIONAL_WAKEUP:
			pthread_cond_signal(&cond);
			break;

		case REQUEST_NOTIFY_SCHEDULER_FOR_VT:
			state->setTimeScalingFactor(clientRequest.virtualTimeSpeedup);
			notifySchedulerForVirtualizedTime(state, clientRequest.virtualTimeSpeedup);
			break;

		case REQUEST_SHOULD_CURRENT_THREAD_SUSPEND:
//			serverLockMutex();	// called from within the handler
			if (!state -> isSuspended()) {
				responseBool = shouldCurrentThreadSuspend(state);
			} else {
				responseBool = false;
			}
			serverMessageWrite(clientSockFd, &responseBool, sizeof(bool));
//			serverUnlockMutex();
			break;

		case REQUEST_SET_WAITING:
			setWaitingThread(state);
			break;

		case REQUEST_SET_NATIVE_WAITING:
#if DEBUGCLIENTSERVER == 1
			cout << "apo explicit request for thread " << state->getName() << endl;
#endif
			setNativeWaiting(state);
			break;

		case REQUEST_SET_TIMED_WAITING:
			setTimedWaitingThread(state);
			break;

		case REQUEST_SET_IO_THREAD:
			//setIoThread(state);
			break;

		case REQUEST_SET_THREAD_RUNNABLE:
			setSuspendedAndPushToRunnables(state);
			break;
//65-66
		case REQUEST_UPDATE_GLOBAL_TIME_BY:

			timeDiff = clientRequest.timestamp;
			lockingUpdateTimeBy(timeDiff, state);

			break;

		case REQUEST_UPDATECURRENT_TIME:
			currentTime = clientRequest.timestamp;
			//serverLockMutex();
			setCurrentThreadVT(currentTime, state);
			currentTime = state->getEstimatedRealTime();
			//serverUnlockMutex();
//			cout << "VTF server: sending timestamp (update) for " << state->getName() << " = " << state->getEstimatedRealTime() << endl;
			serverMessageWrite(clientSockFd, &currentTime, sizeof(long long));
			break;

		case REQUEST_SETCURRENT_TIME:
			currentTime = clientRequest.timestamp;
			setCurrentThreadVT(currentTime, state);
			currentTime = state->getEstimatedRealTime();

//			cout << "lastCputime of "<< state->getName() << " = " << state->getLastCPUTime() << endl;
//			cout << "VTF server: sending timestamp (set) for " << state->getName() << " = " << state->getEstimatedRealTime() << " = " << currentTime<< endl;
			serverMessageWrite(clientSockFd, &currentTime, sizeof(long long));
			break;

		case REQUEST_SETCURRENT_TIME_FROM_HANDLER:
			currentTime = clientRequest.timestamp;
//			serverLockMutex();
			if (currentTime > 0) {
				// We might be waking up a thread that is blocked in a native pthread_cond_wait. We need to find out about this
				// IMPORTANT: Check time progress here, before the @ timeDiff alteration

				// take into account the speedup
				currentTime *= state->getTimeScalingFactor();

				state->addElapsedTime(currentTime);
				virtualTimelineController->commitCpuTimeProgress(state);//progressGlobalTimeBy(currentTime); // TODO: I BROKE THIS
				//vtflog(managerDebug & mypow2(1), managerLogfile, "accGlobal update: %lld (handler) by %s\n", virtualTimelineController->getGlobalTime(), state->getName());
			}
			if (state->getTimeout() < 0) {
				state->leapToGlobalTime(virtualTimelineController->getGlobalTime());
			}
//			serverUnlockMutex();
//			cout << "VTF server: sending timestamp (handler) for " << state->getName() << " = " << state->getEstimatedRealTime() << endl;


			serverMessageWrite(clientSockFd, &currentTime, sizeof(long long));
			state->setEstimatedRealTime(currentTime);

			break;

		case NOTIFY_THREAD_WAITING_START:
			timeDiff = clientRequest.timestamp;

			serverLockMutex();
			locklessUpdateTimeBy(timeDiff, state);

			state->setWaiting();	// actually all you need to do is not push state back into the runnable queue
			unsetCurrentThreadFromRunningThread(state);

			// Notify the scheduler
			unconditionalWakeup();
			serverUnlockMutex();

			break;

		case REQUEST_SET_GLOBAL_TIME:
			state->leapToGlobalTime(virtualTimelineController->getGlobalTime());
			break;


		case NOTIFY_THREAD_TIMED_WAITING_START:
//ppppppppppppppp
			serverLockMutex();
			// The timestamp here represents the position on the vt timeline after the expiry at the local host
			timeout = clientRequest.timestamp - state->getEstimatedRealTime();
			state->setTimedWaiting(timeout);
			state->setEstimatedRealTime(clientRequest.timestamp);
			LOG(logger, logINFO) << "VTF server: setting " << state->getName() << " to timed waiting with timeout " << timeout << endl;
			setTimedWaitingThread(state);

			unconditionalWakeup();

			serverUnlockMutex();
			break;

		case NOTIFY_THREAD_CONTENDED_ENTER:
			serverLockMutex();
			locklessUpdateTimeBy(clientRequest.timestamp, state);
			state->setTimeout(0);
			state->setWaiting();	// actually all you need to do is not push state back into the runnable queue
			unsetCurrentThreadFromRunningThread(state);
			unconditionalWakeup();
			serverUnlockMutex();

			break;
		case NOTIFY_THREAD_END:

			//onThreadEnd(state);

			serverLockMutex();
			LOG(logger, logINFO) << "*** ENDING thread " << state->getName() << endl;
			//locklessUpdateTimeBy(clientRequest.timestamp, state, false);
			virtualTimelineController->commitCpuTimeProgress(state); //progressGlobalTimeBy(clientRequest.timestamp);// TODO: I BROKE THIS

			if (state == runningThread) {
				runningThread = NULL;				// this denotes that the thread will not be suspended by the scheduler when he wakes up
				pthread_cond_signal(&cond);
				LOG(logger, logDEBUG) << "VTF Server: signalling scheduler to resume next"<< endl;
			}
			state->setDead();

			LOG(logger, logINFO) << "**** ENDED thread " << state->getName() << endl;

			serverUnlockMutex();

			threadRegistry->remove(state);

			VISUALIZE_EVENT(THREAD_END, state);

			if (threadRegistry->noProcessThreadsLeft()) {
				serverLockMutex();
				sendRequestToClientListener(PROCESS_EXIT, state->getManagingSchedulerFd());
				serverUnlockMutex();
			}
//			delete state;
//			state = NULL;
			break;

		case NOTIFY_TIMED_WAITING_END:
			serverLockMutex();
			state->setEstimatedRealTime(clientRequest.timestamp);
			state->setTimeout(-1);
			state->setSuspended();

			runnableThreads->update();

			serverUnlockMutex();
			break;

		case NOTIFY_THAT_THREAD_WAS_INTERRUPTED:
			serverLockMutex();

			onAnyReplacedWaitingInterrupt(state, clientRequest.timestamp);
			serverUnlockMutex();
			break;

		case NOTIFY_THREAD_BEFORE_YIELD:
			state->addInvocationPoints();
			serverLockMutex();
			newstate = runnableThreads->top();
			if (newstate != NULL) {
				state->addElapsedTime(newstate->getEstimatedRealTime() + (schedulerTimeslot));
			}
			serverUnlockMutex();
			break;

		case REQUEST_SET_RUNNING:
			serverLockMutex();
			responseBool = true;
			if (runningThread != NULL && runningThread != state) {
				responseBool = false;
			} else {
				if (state->beforeCurrentGlobalTime(virtualTimelineController->getGlobalTime())) {
					state->leapToGlobalTime(virtualTimelineController->getGlobalTime());
				}
				changeThreadStateToRunning(state);
			}
			serverMessageWrite(clientSockFd, &responseBool, sizeof(bool));
			serverUnlockMutex();
			break;

		case REQUEST_CHANGE_STATE_TO_RUNNING:
			changeThreadStateToRunning(state);
			break;

		case REQUEST_SUSPEND_THREAD:
			state->setSuspended();
			break;

		case REQUEST_TRY_SUSPEND_CURRENT_THREAD:
			LOG(logger, logDEBUG) << "VTF server: will suspend thread " << state->getName() << " at " << clientRequest.timestamp << endl;

			state->setThreadResponseFd(clientSockFd);
//			if ((clientRequest.options & SUSPEND_OPT_EXTERNALLY_LOCKED) != 0) {
//				clientRequest.options ^= SUSPEND_OPT_EXTERNALLY_LOCKED;
//			}
			suspendCurrentThread(state, clientRequest.timestamp, clientRequest.options);
			break;

		case NOTIFY_IO_START:
			state->setIoInvocationPointHashValue( clientRequest.ioInvocationPointHashValue);
			state->setStackTraceHash(clientRequest.stackTraceHash);
			state->setThreadResponseFd(clientSockFd);
			state->setLocalTime(clientRequest.timestamp);

			ioSimulator->startIo(state);
			break;

		case NOTIFY_IO_END:

			LOG(logger, logDEBUG1) << "I/O end start for " << state->getName() << endl;
			onThreadIoEnd(clientRequest.timestamp, state, 0);
			LOG(logger, logDEBUG1) << "I/O end end for " << state->getName() << endl;

			break;

		case REQUEST_PROGRESS_TIME_BY:
//			progressGlobalTimeBy(state, clientRequest.timestamp);	//TODO: I BROKE THIS
			break;

		case REQUEST_SET_GLOBAL_TIME_TO_THREADTIME:
			serverLockMutex();
			commitIoDuration(state, 0);		// TODO: add correct actualIoDuration
			serverUnlockMutex();
			break;

		case REQUEST_TERMINATE:
			LOG(logger, logDEBUG) << "VTF server: sending PROCESS EXIT message to fd " << clientRequest.ioInvocationPointHashValue << endl;
//			sendRequestToClientListener(PROCESS_EXIT, clientRequest.ioInvocationPointHashValue);
//			LOG(logger, logDEBUG) << "VTF server: after sending PROCESS EXIT message to fd " << clientRequest.ioInvocationPointHashValue << endl;
#if TRACK_EVENTS == 1
			if (visualizer != NULL) {
				visualizer->writeToFile("/data/vtf_events.csv");
				visualizer->clear();
				visualizer->clearThreads();
			}
#endif
			close(clientSockFd);

//			exit(0);
	}
	return true;
}



void *serviceRequests(void *Args) {
	struct serverThreadArguments *args = (struct serverThreadArguments *)Args;
	int clientSockFd = args->sockfd;
	ThreadManagerServer *manager = args->manager;

	manager->acceptClientRequests(clientSockFd);

	/* close the socket and exit this thread */
	close(clientSockFd);
	pthread_exit((void *)0);
	return NULL;
}


int ThreadManagerServer::createClientListenerConnection(struct sockaddr_in cli_addr) {
	int listenerSockFd = socket(AF_INET, SOCK_STREAM, 0);
	if (listenerSockFd < 0) {
		cerr << "VTF server: ERROR opening socket" << endl;
		return false;
	}

	while (connect(listenerSockFd, (struct sockaddr *)&cli_addr, sizeof(cli_addr)) < 0) {
		cerr << "VTF server: could not connect to client listener. Trying again" << endl;
		sleep(1);
	}

	LOG(logger, logINFO) << "VTF server: successfully connected to client listener at "<< cli_addr.sin_port << endl;
	return listenerSockFd;

}


void ThreadManagerServer::acceptConnections() {

	pthread_t *server_thread;
	int newsockfd;
	socklen_t clilen;
	struct sockaddr_in cli_addr;

	LOG(logger, logINFO) << "VTF Server: starting to accept connections" << endl;

	while (1) {
		clilen = sizeof(cli_addr);
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		if(newsockfd < 0) {
			cerr << "VTF Server: accept error\n";
			return;
		} else {
			LOG(logger, logINFO) << "VTF Server: accepted new client" << endl;
		}

		cout << "VTF Server: new client connecting at " << virtualTimelineController->getGlobalTime() << "...";

		// Create connection to client listener
		int clientListenerThreadPort;
		serverMessageRead(newsockfd, &clientListenerThreadPort, sizeof(int));

		LOG(logger, logINFO) << "VTF server: will connect to client listener at " << clientListenerThreadPort << endl;

		cli_addr.sin_port = htons(clientListenerThreadPort);
		int listenerSockFd = createClientListenerConnection(cli_addr);

		struct serverThreadArguments *args = new struct serverThreadArguments;
		args->sockfd  = newsockfd;
		args->manager = this;
		server_thread = new pthread_t;
		/* create a new thread to process the incoming requests */
		pthread_create(server_thread, NULL, serviceRequests, (void *) args);

		// Notify client about the listener id (to attach it to its VexThreadStates)
		serverMessageWrite(newsockfd, &listenerSockFd, sizeof(int));

		LOG(logger, logINFO) << "VTF server: connected to client at " << clientListenerThreadPort << endl;

		cout << "Connected!" << endl;
		/* the server is now free to accept another socket request */
	}

}



/********* PROCESSING LAYER ************/







/********* MODIFIED SCHEDULER METHODS ************/
void ThreadManagerServer::sendRequestToClientListener(int code, VexThreadState *state, long long timestamp) {
//	LOG(logger, logINFO) << "note over clientListener,clientServer: " << SchedulerRequest::codeToString(code) << " to " << state->getName() << " " << state->getManagingSchedulerFd() << endl;
	SchedulerRequest request(code, state->getVtfClientStatePtr(), timestamp);
	serverMessageWrite(state->getManagingSchedulerFd(), &request, sizeof(SchedulerRequest));
}
void ThreadManagerServer::sendRequestToClientListener(int code, VexThreadState *state) {
//	LOG(logger, logINFO) << "note over clientListener,clientServer: " << SchedulerRequest::codeToString(code) << " to " << state->getName() << " " << state->getManagingSchedulerFd() << endl;
	SchedulerRequest request(code, state->getVtfClientStatePtr());
	serverMessageWrite(state->getManagingSchedulerFd(), &request, sizeof(SchedulerRequest));
}
void ThreadManagerServer::sendRequestToClientListener(int code, int fd) {
//	LOG(logger, logINFO) << "note over clientListener,clientServer: " << SchedulerRequest::codeToString(code) << " to " << " " << fd << endl;
	SchedulerRequest request(code);
	serverMessageWrite(fd, &request, sizeof(SchedulerRequest));
}

void ThreadManagerServer::getClientListenerTimeResponse(const int & fd, long long *buf) {
	serverUnlockMutex();
	bzero(buf, 2*sizeof(long long));
	if (serverMessageRead(fd, buf, 2*sizeof(long long)) < 0) {
		cerr << "VTF server: error while reading" << endl;
	}
	// What happened in the meantime?
	serverLockMutex();
}

char ThreadManagerServer::getClientResponse(const int & fd) {
	serverUnlockMutex();
	char ack;
	if (serverMessageRead(fd, &ack, sizeof(char)) < 0) {
		cerr << "VTF server: error while reading" << endl;
	}
	// What happened in the meantime?
	serverLockMutex();
	return ack;
}

bool ThreadManagerServer::serverSuspendThread(VexThreadState *state) {

	if (state -> isManagedLocally()) {
		return suspendThread(state);
	} else {
		LOG(logger, logDEBUG2) << "VTF Server: will try to remotely suspend " << state->getName() << " through sockfd=" << state -> getManagingSchedulerFd() << endl;

		long long buffer[2];
		short response;

		if (state != NULL  && state == runningThread) {
			sendRequestToClientListener(SUSPEND_THREAD, state);

			// we are releasing the lock here, otherwise obvious deadlock if a client's thread is holding its local manager's lock and waiting for a server response
			getClientListenerTimeResponse(state->getManagingSchedulerFd(), (long long *)(&buffer[0]));
			// and reacquiring it before reaching this point - anything (state change, termination) can happen in the meantime

			if (signalCouldNotBeDeliveredToThread(buffer[0])) {
				LOG(logger, logDEBUG2) << "VTF Server: signal could not be delivered to " << state->getName() << " exiting server suspend without sending response" << endl;
				return true;
			}

			if (threadDidNotChangeStateOrDiedWhileWaitingForClientResponse(state)) {
				LOG(logger, logDEBUG4) << "VTF Server: remotely suspending " << state->getName() << " with time diffs vt: " << buffer[0] << " real:" << buffer[1] << endl;

				long long virtualTimeDiff = buffer[0];
				long long realTimeDifference = buffer[1];

				locklessUpdateTimeBy(virtualTimeDiff, state);		// update of global time
				if (realTimeDifference != TIME_CODE_DENOTING_THAT_THREAD_IS_NATIVE_WAITING) {
					if (shouldCurrentThreadSuspend(state)) {
						response = TO_BE_SUSPENDED;
					} else {
						response = TO_KEEP_ON_RUNNING;
					}
				} else {
					response = TO_BE_DISREGARDED;	// no message needed here the thread already knows this
				}


				if (response == TO_BE_SUSPENDED) {
					lastSuspended = runningThread;
					// 3. set the thread's state as suspended to know that it should be resumed if needed
					if (runningThread == state) {
						runningThread = NULL;
					}
					state -> setSuspended();
					runnableThreads -> push(state);

					VISUALIZE_EVENT(SUSPEND, state);
					LOG(logger, logDEBUG2) << "VTF Server: remotely suspended " << state->getName() << endl;

					serverMessageWrite(state->getManagingSchedulerFd(), &response, sizeof(short));

					LOG(logger, logDEBUG3) << "VTF Server: remote suspend of " << state->getName() << " response is TO_BE_SUSPENDED"  << endl;

					return true;
				} else if (response == TO_BE_DISREGARDED) {
					setNativeWaiting(state);

					serverMessageWrite(state->getManagingSchedulerFd(), &response, sizeof(short));
					LOG(logger, logDEBUG3) << "VTF Server: remote suspend of " << state->getName() << " response is TO_BE_DISREGARDED" << endl;

					return true;
				} else {
					changeThreadStateToRunning(state);

					serverMessageWrite(state->getManagingSchedulerFd(), &response, sizeof(short));
					LOG(logger, logDEBUG3) << "VTF Server: remote suspend of " << state->getName() << " response is TO_KEEP_RUNNING" << endl;
					return false;
				}
			} else {
				LOG(logger, logWARNING) << "VTF Server: will not suspend, because " << state->getName() << " changed its status unexpectedly" << endl;

				response = TO_KEEP_ON_RUNNING;
				serverMessageWrite(state->getManagingSchedulerFd(), &response, sizeof(short));
			}
		}
		return false;
	}
}

/**
 * Scheduler resumes a thread
 * @return: 1 if the thread has been successfully resumed, 0 otherwise
 * @assumption: the lock should be held when this is invoked
 * @lock: locked
 */
void ThreadManagerServer::resumeThread(VexThreadState *state) {
	if (state->isManagedLocally()) {

		state->waitForThreadToBlock();
		lastResumed = state;

		// NOTE: forbidden to set running here - this might lead to locking a locked thread

//		state -> onThreadResumeByManager(virtualTimelineController->getGlobalTime(), managerId);
		state->onThreadResumeByManager(managerId);
		virtualTimelineController->updateResumingSuspendedThreadResumedLastTimestamp(state);

		state->signalBlockedThreadToResume();
		state->allowSignalledThreadToResume();

	} else {

		if (!anotherThreadAlreadySetRunning(state)) {
			LOG(logger, logINFO) << "VTF Server: resuming remote " << state->getName() << " at " << state->getEstimatedRealTime() << endl;
			sendRequestToClientListener(RESUME_THREAD, state, virtualTimelineController->getGlobalTime());
			setRunningThread(state);
			getClientResponse(state->getManagingSchedulerFd());	// synchronize with client
			//LOG(logger, logINFO) << "VTF Server: resumed " << state->getName() << " at " << state->getEstimatedRealTime() << endl;
			VISUALIZE_EVENT(RESUME, state);	// might be a bit inaccurate now, but the thread itself notifies that it is actually running
		} else {
			runnableThreads->push(state);
		}

	}
}



/*
 * Used to interrupt timed-out threads in I/O
 */
void ThreadManagerServer::interruptTimedOutIoThread(VexThreadState *state) {

}



/*
 * Set a thread into an I/O performing state
 *
 * The policies are the following:
 * - IO_SEQUENTIAL		: don't push back into the queue, but freeze scheduler
 * - IO_PARALLEL_STRICT : push back into the queue, when finished just update the queue.
 * - IO_PARALLEL_LAX	: don't push back into the queue, do that when finished
 */
void ThreadManagerServer::setIoThread(VexThreadState *state, const bool &learning) {
	if (state != NULL) {
		_setIoThread(state, learning);

		// ACK to set the requesting thread to start the I/O without suffering
		// the I/O-communication-cost
		if (!state->isManagedLocally()) {
			serverMessageWrite(state->getThreadResponseFd(), &learning, sizeof(bool));
		}
	}
}


/*
 * Called at the end of an I/O operation
 *
 * @return: bool whether the I/O operation finished normally in VTF and a new measurement was appended
 */
void ThreadManagerServer::onThreadIoEnd(const long long& realTimeValueOnExit, VexThreadState *state, const int &methodId) {

	long long times[2];

	if (state->inIo()) {

		ioSimulator->endIo(state, realTimeValueOnExit);

		times[0] = state->getVirtualTime();
		times[1] = state->getEstimatedRealTime();
	} else {
		// Recovery
		times[0] = state->getVirtualTime();
		times[1] = state->getEstimatedRealTime();

		serverLockMutex();
		state->leapToGlobalTime(virtualTimelineController->getGlobalTime());		// forget what happened - just skip to current time
		int options = SUSPEND_OPT_FORCE_SUSPEND  | SUSPEND_OPT_DONT_UPDATE_THREAD_TIME;
		// failed I/O sth happened while in I/O, don't use this measurement
		if (runnableThreads -> find(state)) {
			options |= SUSPEND_OPT_THREAD_ALREADY_IN_LIST;
		}
		suspendCurrentThread(state, 0, options);
	}

	if (state->isManagedLocally()) {
		// Only one exiting call info - no need to malloc/free each time
		state->getExitingMethodInfo()->setInfo(methodId, times[0], times[1], 0, 0);	// TODO: missing io/monitor waiting times
	} else {
		serverLockMutex();
		serverMessageWrite(state->getThreadResponseFd(), times, 2 * sizeof(long long));
		serverUnlockMutex();
	}
}

/**
 * Scheduler wakes up a timed waiting thread (which is already in the thread queue)
 * @assumptions: lock is NOT held when the method is called
 * @lock: SCHEDULER LOCK SHOULD BE HELD WHEN ENTERED
 * @returns: true if a thread was actually found waiting and was interrupted
 */
void ThreadManagerServer::resumeWaitingThread(VexThreadState *state, const long &interruptTime) {

	if (state->isManagedLocally()) {
//		ThreadManager::resumeWaitingThread(state, 0);
		virtualTimelineController->commitTimedWaitingProgress(state);
		onAnyReplacedWaitingInterrupt(state, 0);

//		state->beforeThreadInterruptAt(interruptTime, objectRegistry);
//		updateThreadList();			// the scheduler will schedule the interrupted thread when it's time comes
//		resumeThread(state);

	} else {
		state->leapTo(interruptTime);
		LOG(logger, logDEBUG2) << "VTF server will resume thread " << state->getName() << " after waiting finished" << endl;
		sendRequestToClientListener(TIMED_WAITING_END_THREAD, state, state->getEstimatedRealTime());
		setRunningThread(state);
		VISUALIZE_EVENT(RESUME, state);
	}
}

/**
 * Set a thread as the currently running thread of this processor
 * @lock: assumed manager lock held when entering
 * @return: true if the thread had to be suspended again before releasing
 *
void ThreadManagerServer::setRunningThread(VexThreadState *state) {
	if (state != NULL) {

		state -> setAwaken(false);
		if (runningThread != NULL && runningThread != state) {
//			cout << state->getName() << " piga na ksekinisw [epeidi o lastResumed einai " << lastResumed->name << "] alla vrika ayton na trexei akomi " << runningThread->name << endl;
			suspendCurrentThread(state, 0, SUSPEND_OPT_FORCE_SUSPEND | SUSPEND_OPT_DONT_UPDATE_THREAD_TIME  | SUSPEND_OPT_DONT_NOTIFY_CLIENT);
			serverLockMutex(); //acquire the lock again
		} else {

			if (state->beforeCurrentGlobalTime(virtualTimelineController->getGlobalTime())) {
				state->leapToGlobalTime(virtualTimelineController->getGlobalTime());
			}
	//		if (runningThread != NULL) {
	//			vtflog(true, managerLogfile, "xxxxxxxxxxxxxxxxxx - setRunningThread thread %s %lld while another thread is already running\n", state->getName(), state->getUniqueId());
	//			vtfstacktrace(true, managerLogfile, "duplicate running trace\n");
	//		}

	//		int states_count[POSSIBLE_THREADSTATES];
	//		threadRegistry -> getStatesSnapshot((int *)(&states_count[0]));
	//		if ((states_count[RUNNING] == 1 && state!=runningThread) || states_count[RUNNING] > 1) {
	//			manager->ps();
	//		}

//			cout << "o " << state->getName() << " egine kanonika o runningThraed" << endl;
			changeVexThreadStateToRunning(state);
			//vtflog(managerDebug & mypow2(3), managerLogfile, "Thread %s %lld resumed and set as the running thread of scheduler\n", state->getName(), state->getUniqueId());
		}
	}

}

*/


void ThreadManagerServer::notifyRemoteThreadForSchedulerDecisionOnIfItShouldSuspend(VexThreadState *state, bool shouldSuspend, const char & options) {

	if (!state->isManagedLocally() && !(options & SUSPEND_OPT_DONT_NOTIFY_CLIENT)) {
		serverMessageWrite(state->getThreadResponseFd(), &shouldSuspend, sizeof(bool));
		if (shouldSuspend) {
			LOG(logger, logINFO) << "VTF Server: will suspend " << state->getName() << " that requested to do so" << endl;
		} else {
			LOG(logger, logINFO) << "VTF Server: will allow " << state->getName() << " that requested to suspend to keep on running " << endl;
		}

	}
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
 *
 */
void ThreadManagerServer::suspendCurrentThread(VexThreadState *state, const long long & startingTime, const char & options) {
	if (state == NULL) {
		return;
	}

	// Protecting - lock
//	if (!(options & SUSPEND_OPT_EXTERNALLY_LOCKED)) {
//		//lockMutex(state);
//		serverLockMutex();
//		LOG(logger, logDEBUG4) << "VTF Server: Self-suspending " << state->getName() << " acquired lock" << endl;
//	}

	LOG(logger, logDEBUG4) << "VTF Server: Self-suspending " << state->getName() << " flags: " << (unsigned int)options << " at " << getAllCallingMethodsUntil(5) << endl;

	// Update the thread's VT
	if (!(options & SUSPEND_OPT_DONT_UPDATE_THREAD_TIME)) {

		if (startingTime == 0) {
			// TODO: maybe this could become a method in state to be subclassed for distributed threads
			updateCurrentThreadVT(state);
		} else {
			// TODO: maybe this could become a method in state to be subclassed for distributed threads
			setCurrentThreadVT(startingTime, state);	// for distributed threads startingTime is essentially a timeDiff
																// but since lastCPUtime is always 0 and virtualTimeSpeedup 1,
																// and localClock 0, we can still use our original calls
		}
	}


	// Suspend only if you are forced to (new thread) or there exists another thread whose VT < your updated(VT)
	if (((options & SUSPEND_OPT_FORCE_SUSPEND) && runnableThreads->size() != 0) || shouldCurrentThreadSuspend(state)) {

		// 1. Notify scheduler to choose the next thread to run.. - this thread's stat
		if (options & SUSPEND_OPT_THREAD_ALREADY_IN_LIST) {
			if (runningThread == state) {
				runningThread = NULL;
			}
			LOG(logger, logDEBUG3) << "VTF Server: Self-suspending thread " << state->getName() << " is already in runnables list*****" << endl;
			state->setSuspended();
		} else {
			if (state->isRegistering()) {
				threadRegistry -> newThreadStarted();
				LOG(logger, logDEBUG3) << "VTF Server: Self-suspending thread " << state->getName() << " was REGISTERING" << endl;
			}
			setSuspendedAndPushToRunnables(state);

		}

		assert(runnableThreads->find(state));

		if (!(options & SUSPEND_OPT_DONT_WAKE_UP_SCHEDULER)) {
			LOG(logger, logDEBUG3) << "VTF Server: Self-suspending thread " << state->getName() << " signals main scheduler" << endl;
			pthread_cond_signal(&cond);
		}


		LOG(logger, logDEBUG4) << "VTF Server: Self-suspending thread " << state->getName() << "("<< state->isManagedLocally() <<") releasing lock" << endl;


		if (state->isManagedLocally()) {
			serverUnlockMutex();

			VISUALIZE_EVENT(SUSPEND_SELF, state);
			blockCurrentThread(state);

			if (!(options & SUSPEND_OPT_DONT_UPDATE_THREAD_TIME) && startingTime == 0) {
				state->updateCpuTimeClock();
			}

			VISUALIZE_EVENT(RESUME, state);

		} else {


			notifyRemoteThreadForSchedulerDecisionOnIfItShouldSuspend(state, true, options);
			serverUnlockMutex();

			VISUALIZE_EVENT(SUSPEND_SELF, state);

		}

	} else if (state != runningThread) {

		// If for some reason - like just finished strict/normal IN IO state
		setRunningThread(state);

		notifyRemoteThreadForSchedulerDecisionOnIfItShouldSuspend(state, false, options);


		if (options & SUSPEND_OPT_THREAD_ALREADY_IN_LIST) {
			if (runnableThreads -> top() == state) {
				runnableThreads -> getNext();
			} else {
				runnableThreads -> erase(state);
			}
		}
		assert(!runnableThreads -> find(state));

		serverUnlockMutex();

		//vtflog(managerDebug & mypow2(13), managerLogfile, "Self-suspending: %s (%ld) decided NOT to suspend because......\n",state->getName(), state->tid);
		///assert( ! runnableThreads -> find(state) ) ;
	} else {

		notifyRemoteThreadForSchedulerDecisionOnIfItShouldSuspend(state, false, options);
		serverUnlockMutex();
	}


}

void ThreadManagerServer::suspendRunningResumeNext() {

//	LOG(logger, logINFO) << "note right of clientServer: lockMutex() at " << getCallingMethod()  << endl;
//	LOG(logger, logINFO) << "activate clientServer" << endl;
	if (runningThread == NULL || serverSuspendThread(runningThread)) {
		VexThreadState *temp = runnableThreads->top();
		if (temp!=NULL) {
			LOG(logger, logINFO) << "continuing thread " << temp->getName() << " which is now " << temp->getCurrentStateName() << endl;
		}
		continueThread(runnableThreads -> getNext());
	}
//	LOG(logger, logINFO) << "note right of clientServer: unlockMutex() at " << getCallingMethod()  << endl;
//	LOG(logger, logINFO) << "deactivate clientServer" << endl;
}


/********* PROGRAM ************/
int main(int argc, char **argv) {

	IoSimulator *ioSimulator = new IoSimulator(new IoProtocolNormal(), new StateAwareIoPrediction(new PredictionMethodFactory("avg", 30)));
	VirtualTimeline *virtualTimeline = new VirtualTimeline();
	ThreadManagerServer *manager = new ThreadManagerServer(0, new SingleVirtualTimelineController(virtualTimeline), new ThreadQueue(), ioSimulator, new ThreadRegistry(1), new ObjectRegistry());

	EventLogger *eventLogger = new EventLogger();		// TODO: this is probably to be accessed remotely - no need for an event logger here

	if (argc == 1) {
		manager->init(TCP_PORT);
	} else {
		manager->init(atoi(argv[1]));
	}
	manager->setVisualizer(new Visualizer(virtualTimeline, eventLogger));
	long long timeslot = 1000000;
	if (argc == 3) {
		manager->setSchedulerTimeslot(atoi(argv[2]) * timeslot);
	}
	Time::onSystemInit();

	// TODO this is for a single CPU
	pthread_create(new pthread_t, NULL, pthreadManagerWorker, NULL);

	manager->setLog(new Log("vtf_server_log.txt", virtualTimeline));

	manager->acceptConnections();
	return 0;
}
