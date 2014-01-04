
/*
 * JVMProfilingInvalidationEnforcer.h
 *
 *  Created on: Jun 01, 2010
 *      Author: Nick
 */

#ifndef PROFILINGINVALIDATIONENFORCER_H_
#define PROFILINGINVALIDATIONENFORCER_H_

#include <jvmti.h>
#include "VTF.h"
using namespace std;

/*
 * Class used to invoke methods from Java classes to check whether a thread should be profiled (so as to exclude system threads)
 * and to enforce the invalidation of method profiling, by reloading classes and removing their method entry/exit instruments.
 * Implementing the singleton pattern
 */
class JVMProfilingInvalidationEnforcer {
public:

	static JVMProfilingInvalidationEnforcer *getInstance(JNIEnv* jni_env) {
		static JVMProfilingInvalidationEnforcer jvmProfilingInvalidationEnforcer(jni_env);
		return &jvmProfilingInvalidationEnforcer;
	}

	void invalidateProfilingOf(JNIEnv *jni_env, const int & methodId);
	void reinstrumentRecursiveMethod(JNIEnv *jni_env, const int & methodId);

	static void setProfilingInvalidationPrintouts(const bool &printingFoundRecursiveCalls);
private:
	JVMProfilingInvalidationEnforcer(JNIEnv* jni_env);

	// Cached values
	jclass javaEventNotifierClass;
	jmethodID invalidateMethodMethodId;
	jclass javaClassTransformerClass;
	jmethodID adaptRecursiveMethodId;

	jclass javaLangThreadClass;
	jmethodID currentThreadMethodId;
	jmethodID getIdMethodId;
	jmethodID getVtfProfiledMethodId;

	static bool printingFoundRecursiveCalls;

	void check_out_of_memory(jobject aClass);
	void check_error(bool condition, const char *problemDescription);

	void createGlobalReferenceForClass(JNIEnv* jni_env, const char *className, jclass &jClass);
	void cacheStaticMethodId(JNIEnv* jni_env, jclass &jClass, const char *methodName, const char *methodDesc, jmethodID &jMethodId);
	void cacheMethodId(JNIEnv* jni_env, jclass &jClass, const char *methodName, const char *methodDesc, jmethodID &jMethodId);

};

 
 

#endif /* PROFILINGINVALIDATIONENFORCER_H_ */
