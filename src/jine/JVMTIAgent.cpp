#include "virtualtime_EventNotifier.h"
#include "VTF.h"
#include "JVMProfilingInvalidationEnforcer.h"
#include "JVMTIUtil.h"

#if REPORT_ALL_METHODS > 0
#include "JVMOptionalCallbacks.h"
#endif

#include <iostream>

using namespace std;

using namespace VEX;

/*************************************************************************
 ****
 **** GLOBAL JINE LOWER-LEVEL LAYER VARIABLES
 ****
 ************************************************************************/
extern jvmtiEnv *globalJvmti;				// Global JVMTI environment pointer declaration
extern bool changingRecursive;				// Flag denoting whether recursive methods should have their instruments removed
extern bool includeJavaSystemThreads;		// Include Java system threads in profiling
extern bool usingJvmtiForMonitorControl;	// Use JVMTI for monitor control - not explicit locking
extern bool includeGcTime;					// Should GC time be included in the simulation

/*************************************************************************
 **** 
 **** AUTOMATIC JVMTI-ENABLED CALLBACKS 
 **** 
 ************************************************************************/
/*
 * Callback when a thread starts
 */

#include <sched.h>

void JNICALL callbackThreadStart(jvmtiEnv *jvmti_env, JNIEnv* jni_env, jthread thread) {
	if (shuttingDown) {
		return;
	}

	jvmtiPhase phase_ptr;
	jvmti_env->GetPhase(&phase_ptr);

	// Control used to disregard internal JVM threads
	if (phase_ptr == JVMTI_PHASE_LIVE) {
		long threadId = getThreadId(jni_env, thread);
		char *threadName = getThreadName(jvmti_env, jni_env, thread);	// basically needed for debugging reasons
		threadEventsBehaviour->onStart(threadId, threadName);

	} else if (includeJavaSystemThreads) {
		// getThreadId and getThreadName do not work before the LIVE phase and so we have to assign custom thread names and ids (negative)
		static int __vex_system_threads_found = 0;
		char threadName[32];
		sprintf(threadName, "jvm_system_thread%d", __vex_system_threads_found++);
		threadEventsBehaviour->onStart(- 10 - __vex_system_threads_found, threadName);	// random negative ids for system threads
	}

	JINE_METHOD_LOG(CALLBACKTHREADSTART)
}

/**
 * Sent when a thread is about to wait on an object. That is, a thread is entering Object.wait(). 
 **/
void JNICALL callbackMonitorWait(jvmtiEnv *jvmti_env, JNIEnv* jni_env, jthread thread, jobject object, jlong timeout) {
	threadEventsBehaviour->onWrappedWaitingStart();
	JINE_METHOD_LOG(CALLBACKMONITORWAIT)
}


/**
 * Callback sent when a thread finishes waiting on an object. That is, a thread is leaving Object.wait().
 * */
void JNICALL callbackMonitorWaited(jvmtiEnv *jvmti_env, JNIEnv* jni_env, jthread thread, jobject object, jboolean timed_out) {
	threadEventsBehaviour->onWrappedWaitingEnd();
	JINE_METHOD_LOG(CALLBACKMONITORWAITED)

}

/**
 * Callback sent when a thread is attempting to enter a Java programming language monitor already acquired by another thread.
 **/
void JNICALL callbackMonitorContendedEnter(jvmtiEnv *jvmti_env, JNIEnv* jni_env, jthread thread, jobject object) {
	threadEventsBehaviour->onWrappedWaitingStart();
	JINE_METHOD_LOG(CALLBACKMONITORCONTENDEDENTER)
}

/**
 * Callback sent when a thread enters a Java programming language monitor after waiting for it to be released by another thread.
 */
void JNICALL callbackMonitorContendedEntered(jvmtiEnv *jvmti_env, JNIEnv* jni_env, jthread thread, jobject object) {
	threadEventsBehaviour->onWrappedWaitingEnd();
	JINE_METHOD_LOG(CALLBACKMONITORCONTENDEDENTERED)
}

/*
 * Callback when a thread ends
 */
void JNICALL callbackThreadEnd(jvmtiEnv *jvmti_env, JNIEnv* jni_env, jthread thread) {

	threadEventsBehaviour->onEnd();
	JINE_METHOD_LOG(CALLBACKTHREADEND)

}



#if TRACK_GARBAGE_COLLECTOR > 0
/*
 * Callback when the stop-the-world phase of GC starts
 */
void JNICALL callbackGCStart(jvmtiEnv *jvmti_env) {
	JINE_METHOD_LOG(CALLBACKGCSTART)
//	cout << "*************************************************************** GC STARTING" << endl;
	threadEventsBehaviour->onBackgroundLoadExecutionStart();


//	VISUALIZE_TIME_EVENT(GC_STARTED, virtualTimeline->getGlobalTime())
//	JINE_METHOD_LOG(CALLBACKGCSTART)
//	//	cout << "*************************************************************** GC STARTING" << endl;
//	registry->forbidForwardLeaps();		// Hack to avoid virtual leaps forward
//	Time::onGarbageCollectionStart();	// Start measuring GC time
}

/*
 * Callback when the stop-the-world phase of GC finishes
 */
void JNICALL callbackGCFinish(jvmtiEnv *jvmti_env) {
	threadEventsBehaviour->onBackgroundLoadExecutionEnd();
	JINE_METHOD_LOG(CALLBACKGCFINISH)


	//	managers->progressGlobalTimeBy(Time::getGarbageCollectionRealDuration());
	//	virtualTimeline->addTimeOfThreadExecutingAtUnknownTime(Time::getGarbageCollectionRealDuration());



	// TODO: acknowledge this somehow to TimeLogger (use virtual timeline controller instead of virtual time line)
//	virtualTimeline->addTimeOfUnknownRealTimeDurationAndSynchronize(Time::getGarbageCollectionRealDuration());	// add GC duration as unknown time
//	registry->allowForwardLeaps();
//	cout << "*************************************************************** GC FINISHING" << endl;
//	JINE_METHOD_LOG(CALLBACKGCFINISH)
//	VISUALIZE_TIME_EVENT(GC_FINISHED, virtualTimeline->getGlobalTime())

}
#endif


/*************************************************************************
 **** 
 **** METHODS CALLED ON JVM START - FINISH (INITIALIZATION-FINALIZATION)
 **** 
 ************************************************************************/
/*
 * VM Death callback
 */

static void JNICALL callbackVMDeath(jvmtiEnv *jvmti_env, JNIEnv* jni_env) {

	// Print-out any instrumentation statistics gathered by the upper-JINE layer
	jclass jClass = jni_env->FindClass("virtualtime/statistics/InstrumentationRecordFactory");
	jmethodID jMethodId = jni_env->GetStaticMethodID(jClass, "printStatistics", "()V");
	jni_env->CallStaticVoidMethod(jClass, jMethodId);

	shuttingDown = true;
	JINE_METHOD_LOG(CALLBACKVMDEATH)

	// Print-out the transitions amongst JINE methods
	char filename[160] = {0};
	sprintf(filename,"%s/jine_method_transitions", VEX::outputDir);
	printJineMethodTransitionsMap(filename);

	endSimulator();

}

/*
 * VM init callback
 */
static void JNICALL callbackVMInit(jvmtiEnv *jvmti_env, JNIEnv* env, jthread thread) {
	long id = getThreadId(env, thread);
	threadEventsBehaviour->onThreadMainStart(id);		// main has no parent thread to log it beforehand - therefore we use a special call to the VEX API
}

/*
 * Called when the agent is first loaded...
 */
JNIEXPORT jint JNICALL Agent_OnLoad(JavaVM *jvm, char *options, void *reserved) {

	static jvmtiEnv *jvmti = NULL;
	static jvmtiCapabilities capa;

	jvmtiError error;
	jint res;
	jvmtiEventCallbacks callbacks;

	//  We need to first get the jvmtiEnv* or JVMTI environment
	res = jvm->GetEnv((void **) &jvmti, JVMTI_VERSION_1_0);
	if (res != JNI_OK || jvmti == NULL) {
		// This means that the VM was unable to obtain this version of the JVMTI interface, this is a fatal error.
		fprintf(stderr, "Error: Unable to access JVMTI Version 1 (0x%x), is your J2SE a 1.5 or newer version? JNIEnv's GetEnv() returned %d\n", JVMTI_VERSION_1, res);
	}

	// Initializing the options of VEX
	if (!initializeSimulator(options)) {
		fprintf(stderr, "Error parsing VEX simulator options. Exiting...\n");
		return -1;
	}

	// Initialize the options of JINE lower-level instrumentation layer
	if (!initializeJineLowerLayer(options)) {
		fprintf(stderr, "Error parsing JINE lower layer options. Exiting...\n");
		return -1;
	}


	// Here we save the jvmtiEnv* for Agent_OnUnload().
	(void) memset(&capa, 0, sizeof(jvmtiCapabilities));

	capa.can_signal_thread = 1;
	capa.can_get_owned_monitor_info = 1;

#if REPORT_ALL_METHODS > 0
	capa.can_generate_method_entry_events = 1;
	capa.can_generate_method_exit_events = 1;
#endif

#if CHANGE_NATIVE_METHODS > 0
	capa.can_generate_native_method_bind_events = 1;
#endif

#if TRACK_GARBAGE_COLLECTOR > 0
	if (includeGcTime) {
		capa.can_generate_garbage_collection_events = 1;
	}
#endif

#if TRIGGER_CLASS_HOOKS > 0
	capa.can_generate_all_class_hook_events = 1;
#endif

#if TRACK_JIT > 0
	capa.can_generate_compiled_method_load_events = 1;
#endif

	if (usingVtfScheduler) {
		capa.can_generate_monitor_events = 1;
	}

	capa.can_suspend = 1;
	capa.can_tag_objects = 1;
	capa.can_set_native_method_prefix = 1;

	error = jvmti->AddCapabilities(&capa);
	checkJvmtiError(jvmti, error, "Unable to get necessary JVMTI capabilities.");

	(void) memset(&callbacks, 0, sizeof(callbacks));

	// Automatic callbacks
	callbacks.VMInit = &callbackVMInit; // JVMTI_EVENT_VM_INIT
	callbacks.VMDeath = &callbackVMDeath; // JVMTI_EVENT_VM_DEATH
	callbacks.ThreadStart = &callbackThreadStart;
	callbacks.ThreadEnd = &callbackThreadEnd;

	if (usingJvmtiForMonitorControl) {
		callbacks.MonitorWait = &callbackMonitorWait; // Sent when a thread is about to wait on an object.
		callbacks.MonitorWaited = &callbackMonitorWaited; // Sent when a thread finishes waiting on an object.
		callbacks.MonitorContendedEnter = &callbackMonitorContendedEnter; // Sent when a thread is attempting to enter a Java programming language monitor already acquired by another thread.
		callbacks.MonitorContendedEntered = &callbackMonitorContendedEntered;// Sent when a thread enters a Java programming language monitor after waiting for it to be released by another thread.
	}

	// Optional automatic callbacks
#if REPORT_ALL_METHODS > 0
	callbacks.MethodEntry = &callbackMethodEntry;
	callbacks.MethodExit = &callbackMethodExit;
#endif
#if CHANGE_NATIVE_METHODS > 0
	callbacks.NativeMethodBind = &callbackNativeMethodBind;
#endif

#if TRACK_GARBAGE_COLLECTOR > 0
	if (includeGcTime) {
		callbacks.GarbageCollectionStart = &callbackGCStart;
		callbacks.GarbageCollectionFinish = &callbackGCFinish;
	}
#endif

#if TRIGGER_CLASS_HOOKS > 0
	callbacks.ClassFileLoadHook = &callbackCFLHook;
#endif

#if TRACK_JIT > 0
	callbacks.CompiledMethodLoad = &callbackCompiledMethodLoad;
#endif

	error = jvmti->SetEventCallbacks(&callbacks, (jint) sizeof(callbacks));
	checkJvmtiError(jvmti, error, "Cannot set jvmti callbacks");

	// Setting event notification modes for the enabled callbacks
	error = jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_VM_INIT, (jthread) NULL);
	error = jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_VM_DEATH, (jthread) NULL);
	error = jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_THREAD_START, (jthread) NULL);
	error = jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_THREAD_END, (jthread) NULL);

	//	usingVtfScheduler = false;
	if (usingVtfScheduler) {
		error = jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_MONITOR_CONTENDED_ENTER, (jthread) NULL);
		error = jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_MONITOR_CONTENDED_ENTERED, (jthread) NULL);
		error = jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_MONITOR_WAIT, (jthread) NULL);
		error = jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_MONITOR_WAITED, (jthread) NULL);
	}

#if TRACK_GARBAGE_COLLECTOR > 0
	if (includeGcTime) {
		error = jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_GARBAGE_COLLECTION_START, NULL);
		checkJvmtiError(jvmti, error, "Cannot set event notification JVMTI_EVENT_GARBAGE_COLLECTION_START");
		error = jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_GARBAGE_COLLECTION_FINISH, NULL);
		checkJvmtiError(jvmti, error, "Cannot set event notification JVMTI_EVENT_GARBAGE_COLLECTION_FINISH");
	}
#endif

	// Setting the native method prefix for instrumented I/O methods
	error = jvmti->SetNativeMethodPrefix("__vtf_native_prefix_");
	checkJvmtiError(jvmti, error, "Problem setting native method prefix");

#if REPORT_ALL_METHODS > 0
	error = jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_METHOD_ENTRY, (jthread) NULL);
	checkJvmtiError(jvmti, error, "Cannot set event notification JVMTI_EVENT_METHOD_ENTRY");
	error = jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_METHOD_EXIT, (jthread) NULL);
	checkJvmtiError(jvmti, error, "Cannot set event notification JVMTI_EVENT_METHOD_EXIT");
#endif

#if CHANGE_NATIVE_METHODS > 0
	error = jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_NATIVE_METHOD_BIND, NULL);
	checkJvmtiError(jvmti, error, "Cannot set event notification JVMTI_EVENT_NATIVE_METHOD_BIND");
#endif

#if TRIGGER_CLASS_HOOKS > 0
	error = jvmti -> SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_CLASS_FILE_LOAD_HOOK, NULL);
	checkJvmtiError(jvmti, error, "Cannot set event notification JVMTI_EVENT_CLASS_FILE_LOAD_HOOK");
#endif

#if TRACK_JIT > 0
	error = jvmti -> SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_COMPILED_METHOD_LOAD, NULL);
	checkJvmtiError(jvmti, error, "Cannot set event notification JVMTI_EVENT_COMPILED_METHOD_LOAD");
#endif

	globalJvmti = jvmti;

	// We return JNI_OK to signify success
	return JNI_OK;

}


/*
 * Agent_OnUnload: This is called immediately before the shared library is unloaded.
 * This is the last code executed.
 */
JNIEXPORT void JNICALL Agent_OnUnload(JavaVM *vm) {

}
