#include <jvmti.h>
#include <jni.h>
#include <string.h>

#include "PapiProfiler.h"
#include <iostream>

using namespace std;
static PapiProfiler *profiler;
static __thread char *threadName = NULL;
/*
 * VM init callback
 */
static void JNICALL callbackVMInit(jvmtiEnv *jvmti_env, JNIEnv* env, jthread thread) {
//	cout << profiler->getHardwareInfo() << endl;
//	profiler->onThreadStart();
}

/* Find a JVMTI error code.*/
bool check_jvmti_error(jvmtiEnv *jvmti, jvmtiError& errnum, const char *str) {

	if (errnum != JVMTI_ERROR_NONE) {
		char *errnum_str;
		errnum_str = NULL;
		jvmti->GetErrorName(errnum, &errnum_str);
		#ifndef DISABLE_REPORTING
//		fprintf(stderr, "ERROR: JVMTI: %d(%s): %s\n", errnum, (errnum_str== NULL ? "Unknown" : errnum_str), (str == NULL ? "" : str));
//		fflush(stderr);
		#endif
		return false;
	} else {
		return true;
	}

}


// Get the name of the monitored Java thread
char *getThreadName(jvmtiEnv* jvmti_env, JNIEnv *jni_env, jthread &j_thread) {

	// Get all thread info
	jvmtiError err;

	// Get thread name: optional (here left out) after finding Thread main
	jvmtiThreadInfo info;
	(void)memset(&info, 0, sizeof(info));
	err = jvmti_env->GetThreadInfo(j_thread, &info);

	if (!check_jvmti_error(jvmti_env, err, NULL)) {
		return NULL;
	}

	// Get thread name
	char *name = new char[strlen(info.name)+1];
	strcpy(name,info.name);

	// Correct memory deallocations
	jni_env->DeleteLocalRef(info.thread_group);
	jni_env->DeleteLocalRef(info.context_class_loader);
	jvmti_env->Deallocate((unsigned char *)info.name);

	return name;
}


//static bool firstTime = true;
/*
 * Callback when a thread starts (after EventNotifier.beforeThreadStart)
 */
void JNICALL callbackThreadStart(jvmtiEnv *jvmti_env, JNIEnv* jni_env, jthread thread) {
	threadName = getThreadName(jvmti_env, jni_env, thread);

//	if (firstTime) {
//		sleep(5);
//		firstTime= false;
//	}
//	cout << "Last initiated thread " <<  threadName << endl;

	profiler->onThreadStart();


}


/*
 * Callback when a thread ends
 */
void JNICALL callbackThreadEnd(jvmtiEnv *jvmti_env, JNIEnv* jni_env, jthread thread) {
//	if (threadName == NULL) {
//		cout << "Excluding thread " << getThreadName(jvmti_env, jni_env, thread) << " from statistics" << endl;
//	} else {

	if (threadName != NULL) {
		profiler->onThreadEnd(threadName);
	}
}

/*
 * VM Death callback
 */
static void JNICALL callbackVMDeath(jvmtiEnv *jvmti_env, JNIEnv* jni_env) {
//	profiler->stopMeasurement();
	profiler->getTotalMeasurements("/data/real_hc_prof.csv") ;//<< endl;
}


bool isValidJvmtiCall(const jvmtiError& error) {
	if (error != JVMTI_ERROR_NONE) {
		std::cerr << "Error initializing JVMTI" << endl;
		return false;
	} else {
		return true;
	}
}

/**
 * Called when the agent is first loaded...
 * */
JNIEXPORT jint JNICALL Agent_OnLoad(JavaVM *jvm, char *options, void *reserved) {

	static jvmtiEnv *jvmti = NULL;
	static jvmtiCapabilities capa;

	jvmtiError error;

	jint res;

	jvmtiEventCallbacks callbacks;

	/*  We need to first get the jvmtiEnv* or JVMTI environment */
	res = jvm->GetEnv((void **) &jvmti, JVMTI_VERSION_1_0);
	if (res != JNI_OK || jvmti == NULL) {
		/* This means that the VM was unable to obtain this version of the JVMTI interface, this is a fatal error. */
		printf("ERROR: Unable to access JVMTI Version 1 (0x%x), is your J2SE a 1.5 or newer version? JNIEnv's GetEnv() returned %d\n", JVMTI_VERSION_1, res);
	}

	/* Here we save the jvmtiEnv* for Agent_OnUnload(). */

	(void) memset(&capa, 0, sizeof(jvmtiCapabilities));

	error = jvmti->AddCapabilities(&capa);
	if (!isValidJvmtiCall(error)) {	return false; }



	(void) memset(&callbacks, 0, sizeof(callbacks));

	// Automatic callbacks
	callbacks.VMInit = &callbackVMInit; // JVMTI_EVENT_VM_INIT
	callbacks.VMDeath = &callbackVMDeath; // JVMTI_EVENT_VM_DEATH
	callbacks.ThreadStart = &callbackThreadStart;
	callbacks.ThreadEnd = &callbackThreadEnd;

	// Setting event notification modes for the enabled callbacks
	error = jvmti->SetEventCallbacks(&callbacks, (jint) sizeof(callbacks));
	if (!isValidJvmtiCall(error)) {	return false; }
	error = jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_VM_INIT, (jthread) NULL);
	if (!isValidJvmtiCall(error)) {	return false; }
	error = jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_VM_DEATH, (jthread) NULL);
	if (!isValidJvmtiCall(error)) {	return false; }
	error = jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_THREAD_START, (jthread) NULL);
	if (!isValidJvmtiCall(error)) {	return false; }
	error = jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_THREAD_END, (jthread) NULL);
	if (!isValidJvmtiCall(error)) {	return false; }

	profiler = new PapiProfiler(true);
	/* We return JNI_OK to signify success */
	return JNI_OK;

}


/* Agent_OnUnload: This is called immediately before the shared library is
 *   unloaded. This is the last code executed.
 */
JNIEXPORT void JNICALL Agent_OnUnload(JavaVM *vm) {

}

