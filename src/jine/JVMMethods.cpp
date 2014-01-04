/*
 * JVMMethods.cpp
 *
 *  Created on: 28 Jul 2010
 *      Author: nb605
 */

#include <jni.h>
#include "virtualtime_EventNotifier.h"
#include "JVMProfilingInvalidationEnforcer.h"
#include "JVMTIUtil.h"
#include "VTF.h"

/*************************************************************************
 ****
 **** ON METHOD ENTRIES AND METHOD EXITS (FOR IO AND NON-IO)
 ****
 ************************************************************************/
using namespace VEX;

extern bool changingRecursive;
/*
 * afterMethodEntry: enters the profiled method (methodId)
 * Log the time that the entry took place and adjust the VT factors if needed
 */
JNIEXPORT void JNICALL Java_virtualtime_EventNotifier__1afterMethodEntry(JNIEnv *jni_env, jclass jc, jint methodId) {

#if GDB_USAGE == 1
	addOneTimeArtificialDelayToAllowGdbToConnect();
#endif
	bool isRecursive = methodEventsBehaviour->afterMethodEntry(methodId);
	if (isRecursive && changingRecursive) {

		threadEventsBehaviour->haltSuspendForAwhile();	// this is the best we can do: entirely forbidding suspend might lead to deadlock
		JVMProfilingInvalidationEnforcer::getInstance(jni_env)->reinstrumentRecursiveMethod(jni_env, methodId);
		JINE_METHOD_LOG(REINSTRUMENTRECURSIVEMETHOD);
		threadEventsBehaviour->ensureThreadIsNotInNativeWaitingStateWhenEnteringVex();
	}
}


/****
 * beforeMethodExit: a thread (with threadId) exits the profiled method (methodId)
 *
 * Add the difference between current (virtual) time and method entry point to the results for this methodId
 */
JNIEXPORT void JNICALL Java_virtualtime_EventNotifier__1beforeMethodExit(JNIEnv *jni_env, jclass, jint methodId) {
	bool shouldBeInvalidated = methodEventsBehaviour->beforeMethodExit(methodId);
	if (shouldBeInvalidated) {

		// it is not a good time to suspend the thread, whilst holding the upper-level locks.
		// Hold suspend for a while - this is the best we can do: entirely forbidding suspend might lead to deadlock
		threadEventsBehaviour->haltSuspendForAwhile();

		JVMProfilingInvalidationEnforcer::getInstance(jni_env)->invalidateProfilingOf(jni_env, methodId);
		JINE_METHOD_LOG(INVALIDATEPROFILINGOF);

		threadEventsBehaviour->ensureThreadIsNotInNativeWaitingStateWhenEnteringVex();
	}
}


/*
 * afterIoMethodEntry: a thread (with threadId) enters a profiled I/O method (methodId)
 *
 * Log the time that the entry took place and adjust the VT factors if needed
 */
JNIEXPORT void JNICALL Java_virtualtime_EventNotifier__1afterIoMethodEntry(JNIEnv *jni_env, jclass, jint methodId, jint invocationPointHashValue) {
	methodEventsBehaviour->afterIoMethodEntry(methodId, invocationPointHashValue, false);
}


JNIEXPORT void JNICALL Java_virtualtime_EventNotifier__1afterWritingIoMethodEntry(JNIEnv *, jclass, jint methodId, jint invocationPointHashValue, jint fd,  jint bytesToWrite) {
	methodEventsBehaviour->afterIoMethodEntry(methodId, invocationPointHashValue, false);
}


/*
 * beforeIoMethodExit: a thread (with threadId) exits a profiled I/O method (methodId)
 *
 * The duration is estimated through *real* time measurement between this call and afterIoMethodEntry. Real time measurement is
 * valid, because:
 * - since the instrumentation takes place on the lowest level, we know that no other methods may be involved between these two methods
 * - the scheduler does not suspend a thread performing an I/O operation
 * Therefore the real time is a good approximation of the actual duration.
 * A predictive scheme is used (after a couple of initial measurements are done), to allow more than one threads to perform I/O
 * operations in parallel.
 */
JNIEXPORT void JNICALL Java_virtualtime_EventNotifier__1beforeIoMethodExit(JNIEnv *jni_env, jclass, jint methodId) {
	methodEventsBehaviour->beforeIoMethodExit(methodId);
}



/*
 * Method called before entering a method whose performance is described by a model
 */
JNIEXPORT void JNICALL Java_virtualtime_EventNotifier__1enterPerfModel(JNIEnv *, jclass, jint methodId) {
	methodEventsBehaviour->afterMethodEntryUsingPerformanceModel(methodId);

}


/*
 * Method called before exiting a method whose performance is described by a model
 */
JNIEXPORT void JNICALL Java_virtualtime_EventNotifier__1exitPerfModel(JNIEnv *, jclass, jint methodId) {
	methodEventsBehaviour->beforeMethodExitUsingPerformanceModel(methodId);
}
