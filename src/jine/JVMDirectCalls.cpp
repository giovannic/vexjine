/*
 * JVMDirectCalls.cpp
 *
 *  Created on: 28 Jul 2010
 *      Author: nb605
 */

#include "virtualtime_EventNotifier.h"
#include "VTF.h"
#include "JVMTIUtil.h"
#include <cassert>
#include <unistd.h>
/*************************************************************************
 ****
 **** NATIVE METHODS OF virtualtime.EventNotifier
 ****
 ************************************************************************/
/*
 * Re-initialization function
 */
JNIEXPORT void JNICALL Java_virtualtime_EventNotifier_resetPerformanceMeasures(JNIEnv *, jclass) {
	VEX::resetSimulator();
}

/*
 * Output current VEX results to directory outputDirectory
 * This will only print the merged profiles of all *terminated* threads
 * If no threads have terminated there is no output
 */
JNIEXPORT void JNICALL Java_virtualtime_EventNotifier_writePerformanceMeasures(JNIEnv *jni_env, jclass, jstring outputDirectory) {
	const char *outputDir = (outputDirectory == 0) ? 0 : jni_env->GetStringUTFChars(outputDirectory, 0);
	if (outputDir) {
		VEX::printResults(const_cast<char *>(outputDir));
		jni_env->ReleaseStringUTFChars(outputDirectory, outputDir); // no exception
	} else {
		fprintf(stderr, "Error: could not convert name of output directory in writePerformanceMeasures\n");
	}
}

/*
 * Get the real global time
 */
JNIEXPORT jlong JNICALL Java_virtualtime_EventNotifier__1getPapiRealTime(JNIEnv *jni_env, jclass) {
	return Time::getRealTime();
}

JNIEXPORT jlong JNICALL Java_virtualtime_EventNotifier__1getPapiCpuTime(JNIEnv *jni_env, jclass) {
	return Time::getVirtualTime();
}


/*
 * Measure the thread execution time of a newly starting thread (before it initializes its hardware perf counters) in real time
 */
JNIEXPORT void JNICALL Java_virtualtime_EventNotifier_getTimeBeforeCreatingThread(JNIEnv *, jclass, jlong threadToBeSpawnedId) {
	VEX::threadEventsBehaviour->beforeCreatingThread(threadToBeSpawnedId);
	JINE_METHOD_LOG(GETTIMEBEFORECREATINGTHREAD);

}

JNIEXPORT void JNICALL Java_virtualtime_EventNotifier_getTimeAfterCreatingThread(JNIEnv *, jclass) {
	VEX::threadEventsBehaviour->afterCreatingThread();
	JINE_METHOD_LOG(GETTIMEAFTERCREATINGTHREAD);

}

/***
 * Called when current thread invokes join on the thread with id joiningThreadId.
 * Used to coordinate the two threads and avoid virtual leaps
 */
JNIEXPORT void JNICALL Java_virtualtime_EventNotifier_logJoiningThread(JNIEnv *, jclass, jlong joiningThreadId) {
	long joiningThreadIdL = static_cast<long>(joiningThreadId);
	VEX::threadEventsBehaviour->onJoin(joiningThreadIdL);
	JINE_METHOD_LOG(LOGJOININGTHREAD);
}

/*
 * Get the estimated real time in nanoseconds according to the simulation
 */
JNIEXPORT jlong JNICALL Java_virtualtime_EventNotifier__1getVtfTime(JNIEnv *, jclass) {
	long currentTime = static_cast<long>(VEX::threadEventsBehaviour->getTime());
	JINE_METHOD_LOG(GETVTFTIME);
	return currentTime;
}


/*
 * Get the estimated real time in milliseconds according to the simulation
 */
JNIEXPORT jlong JNICALL Java_virtualtime_EventNotifier__1getVtfTimeInMillis(JNIEnv *, jclass) {
	long currentTime = static_cast<long>(VEX::threadEventsBehaviour->getTime()/1e6);
	JINE_METHOD_LOG(GETVTFTIMEINMILLIS);
	return currentTime;

}

/*
 * Set the time delays that will be used to compensate for the JINE and VEX instrumentation overheads
 */
JNIEXPORT void JNICALL Java_virtualtime_EventNotifier_setInstrumentationDelays(JNIEnv *env, jclass c, jlong methodDelay, jlong ioMethodDelay, jlong ipDelay) {
	Time::setDelays(methodDelay/2, ioMethodDelay/2, ipDelay);
	Java_virtualtime_EventNotifier_resetPerformanceMeasures(env, c);
}


JNIEXPORT void JNICALL Java_virtualtime_EventNotifier__1simulateIoDuration(JNIEnv *, jclass, jint timeout) {
	usleep(1000*timeout);
}


JNIEXPORT jboolean JNICALL Java_virtualtime_EventNotifier_forcingNoWaitBecauseNotUsingScheduling(JNIEnv *, jclass) {
	return !VEX::usingVtfScheduler;
}

extern bool usingJvmtiForMonitorControl;
JNIEXPORT jboolean JNICALL Java_virtualtime_EventNotifier_notRelyingOnJvmtiForMonitorHandling(JNIEnv *, jclass) {
	return !usingJvmtiForMonitorControl;
}

//NOTE: @deprecated: was used to remove JINE instrumentation delays
JNIEXPORT void JNICALL Java_virtualtime_EventNotifier__1onInstrumentationStart(JNIEnv *, jclass) {
	assert(false);
//	VexThreadState *state = VexThreadState::getCurrentThreadState();
//	if (state != NULL) {
//		// Update time
//		managers->setCurrentThreadVT(Time::getVirtualTime(), state);
//		state->setPerformingInstrumentation(true);
//		LOG_LAST_VEX_METHOD(state)
//	}
}

//NOTE: @deprecated: was used to remove JINE instrumentation delays
JNIEXPORT void JNICALL Java_virtualtime_EventNotifier__1onInstrumentationEnd(JNIEnv *, jclass) {
	assert(false);
//	VexThreadState *state = VexThreadState::getCurrentThreadState();
//	if (state != NULL) {
//		state->setPerformingInstrumentation(false);
//		LOG_LAST_VEX_METHOD(state)
//		state->updateCpuTimeClock();
//	}
}
