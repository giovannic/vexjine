/*
 * JVMSockets.cpp: This file includes all methods that are performed in real time (so without
 * changing their sequence or setting timeouts in virtual time), but rely on the virtual schedule
 * of the simulation (i.e. the virtual timestamp order) to be completed. The strategy here is always
 * the same: we wrap the operations before they begin and after they finish and control within VEX,
 * when the corresponding thread will be allowed to be resumed by the VEX simulator.
 *
 *  Created on: 28 Jul 2010
 *      Author: nb605
 */

#include "virtualtime_EventNotifier.h"

#include "JVMTIUtil.h"
#include "VTF.h"

#include <errno.h>

using namespace VEX;


/**
 * Method: _beforeSocketRead: treated as a normal I/O method - we rely on the virtual schedule
 * to determine the time that each thread will perform a socket communication. The termination
 * of the reading operation is captured by beforeIoMethodExit
 */
JNIEXPORT void JNICALL Java_virtualtime_EventNotifier__1beforeSocketRead(JNIEnv *jni_env, jclass jC, jint methodId, jint invocationPointHashValue, jint timeout) {
	methodEventsBehaviour->afterIoMethodEntry(methodId, invocationPointHashValue, true);
}

JNIEXPORT void JNICALL Java_virtualtime_EventNotifier__1beforeSocketRead__IIIII(JNIEnv *, jclass, jint methodId, jint invocationPointHashValue, jint fd, jint bytesToRead, jint timeout) {
	methodEventsBehaviour->afterIoMethodEntry(methodId, invocationPointHashValue, true);
}


/*
 * Socket accepts are treated as blocking (waiting) calls and are wrapped by JINE to notify VEX,
 * when the accept call has returned. The virtual time schedule will determine the time that this
 * will happen.
 */
JNIEXPORT void JNICALL Java_virtualtime_EventNotifier_beforeSocketAccept(JNIEnv *, jclass) {
	threadEventsBehaviour->onWrappedWaitingStart();
}

JNIEXPORT void JNICALL Java_virtualtime_EventNotifier_afterSocketAccept(JNIEnv *, jclass) {
	threadEventsBehaviour->onWrappedWaitingEnd();
}


/*
 * Asynchronous I/O operations are wrapped by VEX (instruments before and after), but are actually performed in real time:
 * again we rely on the virtual time schedule to determine the timing that the asynchronous call might
 * be interrupted. A real-time timeout at t=virtual_timestamp_of(thread) is handled within VEX as follows:
 * while t is the minimum virtual timestamp of the system the simulator remains idle,
 * waiting for the operation to timeout in real time
 */
JNIEXPORT jboolean JNICALL Java_virtualtime_EventNotifier__1beforeEpollWaitingOn(JNIEnv *, jclass, jint fd, jlong timeout) {
	long lfd = fd;
	long _timeout = timeout;
	int flag = 0;
	threadEventsBehaviour->onWrappedTimedWaitingStart(lfd, _timeout, flag);
	return false;
}

JNIEXPORT void JNICALL Java_virtualtime_EventNotifier_afterEpollWaiting(JNIEnv *, jclass) {
	threadEventsBehaviour->onWrappedTimedWaitingEnd();
}

/*
 * Interrupt a timed-waiting thread on the unique id: fd + 1
 */
JNIEXPORT void JNICALL Java_virtualtime_EventNotifier__1beforeInterruptingFd(JNIEnv *, jclass, jint fd) {
	long objectId = fd + 1;	// that's the way the mapping takes place in HotSpot
	threadEventsBehaviour->onWrappedTimedWaitingInterrupt(objectId);
}





/*
#include <sys/epoll.h>
#include <dlfcn.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <cassert>

#define RESTARTABLE(_cmd, _result) do { \
  do { \
    _result = _cmd; \
  } while((_result == -1) && (errno == EINTR)); \
} while(0)
#define jlong_to_ptr(a) ((void*)(int)(a))


static int
iepoll(int epfd, struct epoll_event *events, int numfds, jlong timeout)
{
    jlong start, now;
    int remaining = timeout;
    struct timeval t;
    int diff;

    gettimeofday(&t, NULL);
    start = t.tv_sec * 1000 + t.tv_usec / 1000;

    for (;;) {
        int res = epoll_wait(epfd, events, numfds, timeout);
        if (res < 0 && errno == EINTR) {
            if (remaining >= 0) {
                gettimeofday(&t, NULL);
                now = t.tv_sec * 1000 + t.tv_usec / 1000;
                diff = now - start;
                remaining -= diff;
                if (diff < 0 || remaining <= 0) {
                    return 0;
                }
                start = now;
            }
        } else {
            return res;
        }
    }
}

jint throwIOExceptionWithLastError( JNIEnv *env, const char *message ) {
	jclass exClass;
	const char *className = "java/io/IOException" ;

	exClass = env->FindClass(className);
	assert (exClass != NULL);
	return env->ThrowNew(exClass, message);
}


//
 * We trap epoll_wait within JINE so that we do not have to call JNI again.
 * This is not only done for performance reasons, but also because calling JNI might lead the thread to NW,
 * while the rest of the simulation is expecting it to set the waitingInRealTime flag back to false
 *
JNIEXPORT jint JNICALL Java_virtualtime_EventNotifier_epollWaiting(JNIEnv *env, jobject object, jlong address, jint numfds, jlong timeout, jint epfd) {
//jint vex_Java_sun_nio_ch_EPollArrayWrapper_epollWait(JNIEnv *env, jlong address, jint numfds, jlong timeout, jint epfd)
//{


	cout << "brand new and fresh epollWaiting" << endl;
//	long longTimeout = timeout * 1e6;
//	threadEventsBehaviour->onExplicitTimedWaitingStart(epfd, longTimeout);
	threadEventsBehaviour->onWrappedTimedWaitingStart(epfd, timeout, 0);


	struct epoll_event *events = (struct epoll_event *)jlong_to_ptr(address);
    int res;

    if (timeout <= 0) {           // Indefinite or no wait
        RESTARTABLE(epoll_wait(epfd, events, numfds, timeout), res);
    } else {                      // Bounded wait; bounded restarts
        res = iepoll(epfd, events, numfds, timeout);
    }

	threadEventsBehaviour->onWrappedTimedWaitingEnd();

    if (res < 0) {
    	cout << "brand new and fresh epollWaiting out WITH EXCEPTION due to  "<< res  << endl;
        throwIOExceptionWithLastError(env, "epoll_wait failed");
    }
    cout << "brand new and fresh epollWaiting out with "<< res  << endl;
    return res;
}

*/


//JNIEXPORT void JNICALL Java_virtualtime_EventNotifier_resumedSynchronizeOnFdInVirtualTime(JNIEnv *, jclass, jint returnValue) {
////	threadEventsBehaviour->onWrappedTimedWaitingEnd(returnValue);
//	threadEventsBehaviour->onWrappedTimedWaitingEnd(returnValue);
//}





//JNIEXPORT jboolean JNICALL Java_virtualtime_EventNotifier__1beforeSocketAcceptOnFd (JNIEnv *, jclass, jint fd) {
//
//	threadEventsBehaviour->onWrappedWaitingStart(fd);
//}
//JNIEXPORT void JNICALL Java_virtualtime_EventNotifier__1beforeSocketConnectOnFd(JNIEnv *, jclass, jint) {
////	threadEventsBehaviour->on
//}


