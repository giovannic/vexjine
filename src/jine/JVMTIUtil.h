#ifndef JVMTIUTIL_H_
#define JVMTIUTIL_H_

#include <jvmti.h>
#include "JVMStatistics.h"

/* Get a name for a jthread */
char *getThreadName(jvmtiEnv* jvmti_env, JNIEnv *jni_env, jthread &j_thread);
long getThreadId(JNIEnv *jni_env, jthread& j_thread);
int getObjectHashCode(JNIEnv *jni_env, jobject& object);
bool initializeJineLowerLayer(char *opt);

/* Print the JVM stack frames from native code - useful for debugging with gdb */
void jstack();
void jstack(const char *threadname);
void jstackAll(int max);
#if GDB_USAGE == 1
void addOneTimeArtificialDelayToAllowGdbToConnect();
#endif


/* Bind thread executions to specific cores */
void bindToCoresInRoundRobinbFashion(char *threadName);
bool bindToCore(char *threadName, const int &i);

/* Find a JVMTI error code.*/
inline bool checkJvmtiError(jvmtiEnv *jvmti, jvmtiError& errnum, const char *str) {
	if (errnum != JVMTI_ERROR_NONE) {
		char *errnum_str;
		errnum_str = NULL;
		jvmti->GetErrorName(errnum, &errnum_str);
		fprintf(stderr, "ERROR: JVMTI: %d(%s): %s\n", errnum, (errnum_str== NULL ? "Unknown" : errnum_str), (str == NULL ? "" : str));

		return false;
	} else {
		return true;	
	}
};

/* Describe a JVMTI error */
void describe(jvmtiEnv *jvmti, jvmtiError err);

/* Deprecated methods */
jthread invokeNewJavaLangThread(JNIEnv *env);
void checkThatTimeoutIsInsideLimits(jlong &timeout);
void profileInstrumentationCodes(JNIEnv* jni_env);




#endif /*JVMTIUTIL_H_*/
