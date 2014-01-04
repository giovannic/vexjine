#include "JVMTIUtil.h"
#include "JVMProfilingInvalidationEnforcer.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <climits>
#include <cassert>
#include <iostream>

using namespace std;

jvmtiEnv *globalJvmti;				// Global JVMTI environment pointer declaration
bool changingRecursive;				// Flag denoting whether recursive methods should have their instruments removed
bool includeJavaSystemThreads;		// Include Java system threads in profiling
bool usingJvmtiForMonitorControl;	// Use JVMTI for monitor control - not explicit locking
bool includeGcTime;					// Should GC time be included in the simulation

void describe(jvmtiEnv *jvmti, jvmtiError err) {
	jvmtiError err0;
	char *descr;
	err0 = jvmti->GetErrorName(err, &descr);
	if (err0 == JVMTI_ERROR_NONE) {
		printf("%s\n", descr);
	} else {
		printf("error [%d]", err);
	}
}

/*
 * Parses string 'options' to find option 'option'. If not it returns the default value defaultValue
 */
bool getOptionValue(const char *options, const char *option, const bool &defaultValue) {
	std::string optionsString(options);
	if (optionsString.find(option) != std::string::npos) {
		return !defaultValue;
	}
	return defaultValue;
}

/*
 * Parameter parsing utility method - as these options are flags without a legal value range we
 * always return true
 */
bool initializeJineLowerLayer(char *opt) {
	if (opt != NULL) {
		changingRecursive 			= getOptionValue(opt, "no_recursive_change", 	true);
		JVMProfilingInvalidationEnforcer::setProfilingInvalidationPrintouts(getOptionValue(opt, "print_recursion", false));
		includeJavaSystemThreads 	= getOptionValue(opt, "monitoring_jvm_thr", 	false);
		usingJvmtiForMonitorControl = getOptionValue(opt, "no_jvmti", 				true);
		includeGcTime 				= getOptionValue(opt, "no_gc_time", 			true);
	}
	return true;
}


/*
 * Creates a new jthread
 */
jthread invokeNewJavaLangThread(JNIEnv *env) {
	jclass thrClass;
	jmethodID cid;
	jthread res;

	thrClass = env->FindClass("java/lang/Thread");
	cid = env->GetMethodID(thrClass, "<init>", "()V");
	res = env->NewObject(thrClass, cid);
	return res;
}


/*
 * Bind threads to the cores of a dual core machine in round-robin fashion
 */
void bindToCoresInRoundRobinbFashion(char *threadName) {
	static bool latchForBindingToCpu = false;
	int i;
	if (latchForBindingToCpu) {
		i = 0;
	} else {
		i = 1;
	}
	if (bindToCore(threadName, i)) {
		latchForBindingToCpu ^= 1;	// do not change core if this one has failed for some reason
	}
}

/*
 * Bind a thread's execution to a specific core.
 * Printout a warning message in case of failure
 */
bool bindToCore(char *threadName, const int &i) {
	cpu_set_t set;
	CPU_ZERO (&set);        /* clear all CPUs */
	CPU_SET(i, &set);

	if (sched_setaffinity(0, sizeof (cpu_set_t), &set) == 0) {
		return true;
	} else {
		cerr << "Warning: " << " Failed to bind " << threadName << " to core " << i << endl;
		return false;
	}

}

/*
 * Used inside the module
 */
void charDeallocate(char *charToDeallocate) {
	globalJvmti->Deallocate(reinterpret_cast<unsigned char *>(charToDeallocate));
}

/*
 * Utility function to print the stack trace of the current thread in the JVM
 */
void jstack() {
	jvmtiFrameInfo frames[20];
	jint count;
	jvmtiError err;

	char *declaringClassName;

	err = globalJvmti->GetStackTrace(NULL, 0, 20, (jvmtiFrameInfo *)&frames, &count);
	printf("\n-----------------------\nExecuting method: ");
	if (err == JVMTI_ERROR_NONE && count >= 1) {
		char *methodName;
		char *sig;

		for(int i =0 ; i<count; i++) {
			jclass methodClass;
			globalJvmti->GetMethodDeclaringClass(frames[i].method, &methodClass);

			err = globalJvmti->GetClassSignature(methodClass, &declaringClassName, NULL);

			err = globalJvmti->GetMethodName(frames[i].method, &methodName, &sig, NULL);
			if (err == JVMTI_ERROR_NONE) {
				printf("%s -> %s%s\n", declaringClassName, methodName,sig);
			}

		}
		printf("\n\n");
		charDeallocate(methodName);
		charDeallocate(sig);
	}
	charDeallocate(declaringClassName);
}


/*
 * Utility function to print the stack trace of the current thread in the JVM printing also its thread name
 */
void jstack(const char *threadname) {
	jvmtiFrameInfo frames[20];
	jint count;
	jvmtiError err;

	err = globalJvmti->GetStackTrace(NULL, 0, 20, (jvmtiFrameInfo *)&frames, &count);
	printf("\n-----------------------\nThread %s executing stack trace:\n", threadname);
	if (err == JVMTI_ERROR_NONE && count >= 1) {
		char *methodName;
		char *sig;
		for(int i =0 ; i<count; i++) {
			err = globalJvmti->GetMethodName(frames[i].method, &methodName, &sig, NULL);
			if (err == JVMTI_ERROR_NONE) {
				printf("%s%s\n", methodName, sig);
			} else {
				printf("Error getting methodName from jstack!\n");
			}

		}
		printf("\n\n");
		charDeallocate(methodName);
		charDeallocate(sig);
	}
}

/*
 * Utility function to print the JVM stack traces of all threads
 */
void jstackAll(int max) {
	jvmtiStackInfo *stack_info;
	jint thread_count;
	int ti;
	jvmtiError err;

	err = globalJvmti->GetAllStackTraces(max, &stack_info, &thread_count);
	if (err != JVMTI_ERROR_NONE) {
		return;
	}

	for (ti = 0; ti < thread_count; ++ti) {
		jvmtiStackInfo *infop = &stack_info[ti];
		jthread thread = infop->thread;

		jvmtiFrameInfo *frames = infop->frame_buffer;
		int fi;

		jvmtiThreadInfo info;
		(void)memset(&info, 0, sizeof(info));
		err = globalJvmti->GetThreadInfo(thread, &info);

		if (!checkJvmtiError(globalJvmti, err, NULL)) { //"Register - NewThread: Error getting thread info\n")) {
			return;
		}

		char *methodName;
		char *sig;

		fprintf(stderr, "%s: ", info.name);
		for (fi = infop->frame_count-1; fi >= 0; fi--) {
			//	      myFramePrinter(frames[fi].method, frames[fi].location);
			err = globalJvmti->GetMethodName(frames[fi].method, &methodName, &sig, NULL);
			if (err == JVMTI_ERROR_NONE) {
				if (fi > 0) {
					fprintf(stderr, "%s%s ==> ", methodName, sig);
				} else {
					fprintf(stderr, "%s%s", methodName, sig);
				}
			}
		}

		// Correct memory deallocations
		charDeallocate(methodName);
		charDeallocate(sig);
		globalJvmti->Deallocate(reinterpret_cast<unsigned char *>(&info));
	}
	globalJvmti->Deallocate(reinterpret_cast<unsigned char *>(stack_info));
}



/*
 * Get the name of the Java thread referenced by j_thread argument
 */
char *getThreadName(jvmtiEnv* jvmti_env, JNIEnv *jni_env, jthread &j_thread) {

	jvmtiError err;			// Get all thread info
	jvmtiThreadInfo info;	// Get thread name: optional (here left out) after finding Thread main
	(void)memset(&info, 0, sizeof(info));

	err = jvmti_env->GetThreadInfo(j_thread, &info);

	if (!checkJvmtiError(jvmti_env, err, NULL)) {
		return NULL;
	}

	// Get thread name
	char *name = new char[strlen(info.name)+1];
	strcpy(name,info.name);

	// Correct memory deallocations
	jni_env->DeleteLocalRef(reinterpret_cast<jobject>(&info));
	return name;
}


/*
 * Invoke java.lang.Thread.getId()
 */
static jclass stateJavaLangThreadClass = NULL;
long getThreadId(JNIEnv *jni_env, jthread& j_thread) {

	if (stateJavaLangThreadClass == NULL) {
		// Correct caching of global reference - store local reference and call NewGlobalRef
		jclass localJavaLangThreadClass = jni_env->FindClass("java/lang/Thread");
		if (!jni_env->ExceptionCheck()) {
			stateJavaLangThreadClass = reinterpret_cast<jclass>(jni_env->NewGlobalRef(localJavaLangThreadClass));
			assert(!jni_env->ExceptionCheck());
		}
	}

	// An id is not a reference - no need for global reference creation
	static jmethodID stateGetIdMethodId = jni_env->GetMethodID(stateJavaLangThreadClass, "getId", "()J");
	return (long) jni_env->CallLongMethod(j_thread, stateGetIdMethodId);
}



/*
 * Invoke java.lang.Object.hashCode()
 */
static jclass stateJavaLangObjectClass = NULL;
int getObjectHashCode(JNIEnv *jni_env, jobject& object) {

	if (stateJavaLangThreadClass == NULL) {
		// Correct caching of global reference - store local reference and call NewGlobalRef
		jclass localJavaLangObjectClass = jni_env->FindClass("java/lang/Object");
		if (!jni_env->ExceptionCheck()) {
			stateJavaLangObjectClass = reinterpret_cast<jclass>(jni_env->NewGlobalRef(localJavaLangObjectClass));
			assert(!jni_env->ExceptionCheck());
		}
	}

	static jmethodID stateHashCodeMethodId = jni_env->GetMethodID(stateJavaLangObjectClass, "hashCode", "()I");

	// Get current thread by invoking Thread.currentThread()
	int hashCode = (int) jni_env->CallIntMethod(object, stateHashCodeMethodId);
	if (jni_env->ExceptionCheck()) {
		fprintf(stderr, "Could not get Object hashCode\n");fflush(stderr);
		return 0;
	}

	return hashCode;

}

/*
 * Used to profile the additional overhead of the VEX instruments
 */
void profileInstrumentationCodes(JNIEnv* jni_env) {
	jclass javaEventNotifier;

	// the following two JNI calls will be made only once for the entire execution
	javaEventNotifier = jni_env->FindClass("virtualtime/EventNotifier");
	if (jni_env->ExceptionCheck()) {
		fprintf(stderr, "Could not find virtualtime/EventNotifier\n");fflush(stderr);
		return;
	}

	jmethodID profileICId = jni_env->GetStaticMethodID(javaEventNotifier, "profileInstrumentationCode", "()V");
	if (jni_env->ExceptionCheck()) {
		fprintf(stderr, "could not find method id for profileInstrumentationCode\n");fflush(stderr);
		return;
	}

	// Get current thread by invoking Thread.currentThread()
	jni_env->CallStaticVoidMethod(javaEventNotifier, profileICId);
	if (jni_env->ExceptionCheck()) {
		fprintf(stderr, "shouldThreadBeProfiled could not call CallStaticVoidMethod method for EventNotifier.profileInstrumentationCode()\n");fflush(stderr);
		return;
	}

}


/*
 * Check that the value of the timeout does not exceed the upper limit (LONG_MAX)
 */
void checkThatTimeoutIsInsideLimits(jlong &timeout) {
	if (timeout >= (LONG_MAX/1000000)) {
		timeout = LONG_MAX;
	} else {
		timeout *= 1000000;
	}
}


/*
 * Method to add some delay before the beginning of the simulation to allow gdb connection to the process
 */
void addOneTimeArtificialDelayToAllowGdbToConnect() {
	static bool debuggingLatch = true;
	if (debuggingLatch) {
		cout << "Starting GDB delay (to give enough time to attach gdb to process)" << endl;
		double temp = 1.0;
		for(int i =0 ; i<250000000; i++) {
			temp += i/(i+1);
		}
		debuggingLatch = false;
		cout << "Ending GDB delay (" << temp << ")" << endl;
	}
}
