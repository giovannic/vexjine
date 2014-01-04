/*
 * JVMRegistration.cpp
 *
 *  Created on: 28 Jul 2010
 *      Author: nb605
 */

#include "virtualtime_EventNotifier.h"
#include "VTF.h"
#include "JVMTIUtil.h"
#include "JVMProfilingInvalidationEnforcer.h"
#include <cassert>

extern jvmtiEnv *globalJvmti;


/*************************************************************************
 ****
 **** REGISTERING METHODS - EQUIVALENT TO JAVA AGENT ONES
 ****
 ************************************************************************/
/*
 * Register method id to method fqn - everyone will be using the methodId from now - only the output printing methods will map the ids back to the FQNs
 */
JNIEXPORT void JNICALL Java_virtualtime_EventNotifier__1registerMethod(JNIEnv *jni_env, jclass, jstring fqn, jint methodId, jint method0) {
	const char *fqName = (fqn == 0) ? 0 : jni_env->GetStringUTFChars(fqn, 0);

	if (fqName) {
		VEX::eventLogger->registerMethod(fqName, methodId, method0);
		// Cleanup
		jni_env->ReleaseStringUTFChars(fqn, fqName);

	}
}

/*
 * Register time scaling factors for method with id methodId
 */
JNIEXPORT void JNICALL Java_virtualtime_EventNotifier__1registerMethodVirtualTime(JNIEnv *jni_env, jclass, jint methodId, jdouble speedup) {
//	vtflog(true, stderr, "Acceleration factor: for method %d => %.6f\n", methodId, speedup);
	VEX::methodEventsBehaviour->registerMethodTimeScalingFactor(methodId, speedup);

}

/*
 * Register performance describing queueing network for method with id methodId
 */
JNIEXPORT void JNICALL Java_virtualtime_EventNotifier__1registerMethodPerformanceModel(JNIEnv *jni_env, jclass, jint methodId, jstring jmtModelFilename, jint customerClass, jstring javaSourceNodeLabel) {

	const char *modelFilename = (jmtModelFilename == 0) ? 0 : jni_env->GetStringUTFChars(jmtModelFilename, 0);
	const char *sourceNodeLabel = (javaSourceNodeLabel == 0) ? 0 : jni_env->GetStringUTFChars(javaSourceNodeLabel, 0);

	VEX::methodEventsBehaviour->registerMethodPerformanceModel(methodId, modelFilename, sourceNodeLabel, customerClass);

	if (modelFilename != 0) {
		jni_env->ReleaseStringUTFChars(jmtModelFilename, modelFilename);
	}

	if (sourceNodeLabel != 0) {
		jni_env->ReleaseStringUTFChars(javaSourceNodeLabel, sourceNodeLabel);
	}

}

/*
 * This call determines the adaptive profiling policy to be used
 */
JNIEXPORT void JNICALL Java_virtualtime_EventNotifier__1registerProfilingInvalidationPoilcy(JNIEnv *jni_env, jclass, jstring _invalidationPolicyInfo) {
	const char *invalidationPolicyInfo = (_invalidationPolicyInfo == 0) ? 0 : jni_env->GetStringUTFChars(_invalidationPolicyInfo, 0);
	if (invalidationPolicyInfo) {
		VEX::methodEventsBehaviour->registerInvalidationPolicy(invalidationPolicyInfo);
		// Cleanup
		jni_env->ReleaseStringUTFChars(_invalidationPolicyInfo, invalidationPolicyInfo);

	}

}

/*
 * This call allows to exclude threads with a particular name from the simulation - used for JVM system threads
 */
JNIEXPORT void JNICALL Java_virtualtime_EventNotifier__1registerExcludedThreadName(JNIEnv *jni_env, jclass, jstring _excludedThreadName) {
	const char *excludedThreadName = (_excludedThreadName == 0) ? 0 : jni_env->GetStringUTFChars(_excludedThreadName, 0);
	if (excludedThreadName) {
		VEX::threadEventsBehaviour->addThreadToBeExcludedFromSimulation(excludedThreadName);

		// Cleanup
		jni_env->ReleaseStringUTFChars(_excludedThreadName, excludedThreadName);

	}
}

