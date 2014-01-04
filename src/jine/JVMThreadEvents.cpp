/*
 * JVMThreadEvents.cpp: File that defines all methods that are used for handling the
 * synchronization events that take place in the lifecycle of a thread.
 *
 *  Created on: 28 Jul 2010
 *      Author: nb605
 */

#include "virtualtime_EventNotifier.h"
#include "VTF.h"
#include "JVMTIUtil.h"

using namespace VEX;

/*
 * Notify a thread that waits on objectId on our simulated wait
 * @return: true if a thread has been found on a simulated wait and has been notified - otherwise false
 */
JNIEXPORT jboolean JNICALL Java_virtualtime_EventNotifier__1notifyTimedWaitingThreads(JNIEnv *jni_env, jclass, jint objectId) {
	bool aThreadWasSignalled = threadEventsBehaviour->onSignallingOnObject(objectId);
	JINE_METHOD_LOG(NOTIFYTIMEDWAITINGTHREADS);
	return aThreadWasSignalled;
}

/*
 * Notify all thread that wait on objectId on our simulated wait
 * @return: true if a thread has been found on a simulated wait and has been notified - otherwise false
 */
JNIEXPORT jboolean JNICALL Java_virtualtime_EventNotifier__1notifyAllTimedWaitingThreads(JNIEnv *, jclass, jint objectId) {
	bool aThreadWasSignalled = threadEventsBehaviour->onBroadcastingOnObject(objectId);
	JINE_METHOD_LOG(NOTIFYALLTIMEDWAITINGTHREADS);
	return aThreadWasSignalled;

}


/***
 * This call handles Thread.sleep(timeout) and Object.wait(timeout) invocations depending on whether objectId == 0 or not respectively.
 * If we are handling Object.wait(timeout), then we need to release the related monitors on the VEX level and make the thread block
 * in a VEX related conditional variable. The original design where these actions took place on the Java application level (in the instrumented
 * code) showed synchronization problems
 */
JNIEXPORT jboolean JNICALL Java_virtualtime_EventNotifier__1waitInVirtualTime(JNIEnv *jni_env, jclass, jobject object, jint objectId, jlong threadId, jlong timeout, jint timeoutNanos) {
	if (object != 0) {
		// Timed - Waiting
		if (threadEventsBehaviour->beforeReleasingLockOfAnyReplacedWaiting(objectId, false)) {		// TODO: use subclass of threadEventsBehaviour

			JINE_METHOD_LOG(INTERRUPTONVIRTUALTIMEOUT);	// put here to minimize effect on time counters of beforeReleasingLockOfAnyReplacedWaiting

			bool hasThreadTimedOut = false;
			int initialMonitorCounter = 0;
			while (jni_env->MonitorExit(object) == JNI_OK) {	// the monitor on the same object might have been entered multiple times
				++initialMonitorCounter;
			}
			jni_env->ExceptionClear();

			// JNI calls may get into a native waiting state (safepoint) - we need to recheck for scheduler inside the method
			long timeoutL = (long)(timeout);
			hasThreadTimedOut = threadEventsBehaviour->onReplacedTimedWaiting(objectId, timeoutL, timeoutNanos, false);	// stay blocked in here until the virtual timeout expires

			// Resume here after timeout or after being interrupted and reacquire the object monitors (as many times as it was entered before)
			for (int l =0 ; l<initialMonitorCounter; l++) {
				jni_env->MonitorEnter(object);
			}

			return hasThreadTimedOut;
		} else {

			JINE_METHOD_LOG(INTERRUPTONVIRTUALTIMEOUT);
			return false;
		}
	} else {

		// Sleeping
		long timeoutL = (long)(timeout);
		bool hasThreadTimedOut = threadEventsBehaviour->onSleep(timeoutL, timeoutNanos);
		JINE_METHOD_LOG(INTERRUPTONVIRTUALTIMEOUT);	// put here to minimize effect on time counters of onSleep
		return hasThreadTimedOut;

	}
}

/*
 * Interrupt thread with id (in this case Java id) interruptedThreadId
 */
JNIEXPORT jboolean JNICALL Java_virtualtime_EventNotifier__1beforeThreadInterrupt(JNIEnv *jni_env, jclass, jlong interruptedThreadId) {
	long interruptedThreadIdL = (long)interruptedThreadId;
	bool returnValue = threadEventsBehaviour->onInterrupt(interruptedThreadIdL);
	JINE_METHOD_LOG(BEFORETHREADINTERRUPT);
	return returnValue;
}


/**
 * Methods used to explicit denote monitor acquisition and release to avoid leaps
 */
JNIEXPORT void JNICALL Java_virtualtime_EventNotifier__1beforeAcquiringMonitor(JNIEnv *, jclass, jint objectId) {
	threadEventsBehaviour->onRequestingLock(objectId);
	JINE_METHOD_LOG(REQUESTINGLOCK);
}


JNIEXPORT void JNICALL Java_virtualtime_EventNotifier__1beforeReleasingMonitor(JNIEnv *, jclass, jint objectId) {
	threadEventsBehaviour->onReleasingLock(objectId);
	JINE_METHOD_LOG(RELEASINGLOCK);
}

JNIEXPORT jboolean JNICALL Java_virtualtime_EventNotifier__1waitInVirtualTimeWithoutJvmti(JNIEnv *jni_env, jclass, jobject object, jint objectId, jlong threadId, jlong timeout, jint timeoutNanos, jboolean handleLowLevelMonitors) {

	if (object != 0) {
		// Timed - Waiting
		if (threadEventsBehaviour->beforeReleasingLockOfAnyReplacedWaiting(objectId, true)) {
			JINE_METHOD_LOG(INTERRUPTONVIRTUALTIMEOUT);	// put here to minimize effect on time counters of beforeReleasingLockOfAnyReplacedWaiting

			bool hasThreadTimedOut = false;
			int initialMonitorCounter = 0;
			if (handleLowLevelMonitors) {
				while (jni_env->MonitorExit(object) == JNI_OK) {	// the monitor on the same object might have been entered multiple times
					++initialMonitorCounter;
				}
				jni_env->ExceptionClear();
			}


			if ((timeout+timeoutNanos) > 0) {
				long timeoutL = (long)(timeout);
				hasThreadTimedOut = threadEventsBehaviour->onReplacedTimedWaiting(objectId, timeoutL, timeoutNanos, true);	// stay blocked in here until the virtual timeout expires
			} else {
				threadEventsBehaviour->onReplacedWaiting(objectId, true);
			}

			if (handleLowLevelMonitors) {
				// Resume here after timeout or after being interrupted and reacquire the object monitors (as many times as it was entered before)
				for (int l =0 ; l<initialMonitorCounter; l++) {
					jni_env->MonitorEnter(object);
				}
			}

			return hasThreadTimedOut;
		} else {

			JINE_METHOD_LOG(INTERRUPTONVIRTUALTIMEOUT);
			return false;
		}
	} else {

		// Sleeping
		long timeoutL = (long)(timeout);
		bool hasThreadTimedOut = threadEventsBehaviour->onSleep(timeoutL, timeoutNanos);
		JINE_METHOD_LOG(INTERRUPTONVIRTUALTIMEOUT);	// put here to minimize effect on time counters of onSleep
		return hasThreadTimedOut;

	}

}


JNIEXPORT void JNICALL Java_virtualtime_EventNotifier_park(JNIEnv *, jclass, jboolean isAbsoluteTime, jlong timeout) {
	threadEventsBehaviour->onPark(isAbsoluteTime, timeout);

}

JNIEXPORT void JNICALL Java_virtualtime_EventNotifier_parked(JNIEnv *, jclass) {
	threadEventsBehaviour->onParked();

}

// Plays the role of interrupting a parked thread
JNIEXPORT void JNICALL Java_virtualtime_EventNotifier_unpark(JNIEnv *, jclass, jlong threadId) {
	threadEventsBehaviour->onUnpark(threadId);

}


/*************************************************************************
 ****
 **** THREAD SYNCHRONIZATION POINTS EXPLICIT TRAPPING
 ****
 ************************************************************************/
JNIEXPORT void JNICALL Java_virtualtime_EventNotifier__1interactionPoint(JNIEnv *, jclass) {
	threadEventsBehaviour->onInteractionPoint();
	JINE_METHOD_LOG(INTERACTIONPOINT);
}


JNIEXPORT void JNICALL Java_virtualtime_EventNotifier__1yield (JNIEnv *jni_env, jclass j_class) {
	threadEventsBehaviour->onYield();
	JINE_METHOD_LOG(YIELD);
}
