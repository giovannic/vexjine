/*
 * JVMOptionalCallbacks: Used to enable a number of JVMTI callbacks for various debugging reasons.
 * Not used in the core framework
 *
 *  Created on: 12 Mar 2011
 *      Author: root
 */
#include "virtualtime_EventNotifier.h"
#include "VTF.h"
#include "JVMTIUtil.h"

using namespace VEX;

extern jvmtiEnv *globalJvmti;	// declared in JVMTIUtil.cpp
/*************************************************************************
 *
 * OPTIONAL JVMTI-ENABLED CALLBACKS
 * reporting_methods
 * changing_native_methods
 * tracking_gc
 * tracking_class_hooks
 *
 ************************************************************************/
#if REPORT_ALL_METHODS > 0
long long baseRT;
long long baseVT;
#include <stack>
using namespace std;
stack<long long*> vtimesStack;
stack<long long*> rtimesStack;

FILE *logFile = stdout;
void JNICALL callbackMethodEntry(jvmtiEnv *jvmti_env, JNIEnv* jni_env, jthread thread, jmethodID method) {

	/*
	 //	ThreadState *state;
	 //	jvmtiError err = globalJvmti->GetThreadLocalStorage(NULL, (void**) &state);
	 //	char threadName[32];
	 //	if (state != NULL) {
	 //		strcpy(threadName, state->getName());
	 //	} else {
	 //		strcpy(threadName, "UFO");
	 //	}
	 */
	char *methodName;
	char *sign;
	char *class_sig;
	jvmti_env->GetMethodName(method, &methodName, &sign, NULL);
	jclass klass;
	jvmti_env->GetMethodDeclaringClass(method, &klass);
	jvmti_env->GetClassSignature(klass, &class_sig, NULL);

	/*
	 if (strcmp(methodName, "write")!=0 && strcmp(methodName, "open")!=0) {
	 return;
	 }
	 */
	long long *startingCallbackTime = new long long[1]; // used to account for as lost time
	if (startingCallbackTime == NULL) {
		return;
	}
	//	(*startingCallbackTime) = Time::getVirtualTime();

	long long *startingCallbackRTime = new long long; // used to account for as lost time
	if (startingCallbackRTime == NULL) {
		return;
	}
	//	(*startingCallbackRTime) = Time::getRealTime();

	vtimesStack.push(startingCallbackTime);
	rtimesStack.push(startingCallbackRTime);
	//	fprintf(logFile, "JVMTI\t%s entered method\t%s -> %s %s\t%lld\t%lld\t\t\t\t", threadName, class_sig, methodName, sign, (*startingCallbackTime) *1000, (*startingCallbackRTime) *1000);

/*
	fprintf(logFile, "JVMTI entered method\t%s -> %s %s\t%lld\t%lld\t\t\t\t", class_sig, methodName, sign, (*startingCallbackTime), (*startingCallbackRTime));

	int size = vtimesStack.size();
	for (int i=0;i<size;i++) {
		fprintf(logFile, "\t");

	}
	fprintf(logFile, "{\n");
*/
	fprintf(logFile, "entered\t%s -> %s %s\n", class_sig, methodName, sign);
}




void JNICALL callbackMethodExit(jvmtiEnv *jvmti_env, JNIEnv* jni_env, jthread thread, jmethodID method, jboolean was_popped_by_exception, jvalue return_value) {

	/*
	 //
	 //	ThreadState *state;
	 //	jvmtiError err = globalJvmti->GetThreadLocalStorage(NULL, (void**) &state);
	 //	char threadName[32];
	 //	if (state != NULL) {
	 //		strcpy(threadName, state->getName());
	 //	} else {
	 //		strcpy(threadName, "UFO");
	 //	}
	 */
	char *methodName;
	char *sign;
	char *class_sig;
	jvmti_env->GetMethodName(method, &methodName, &sign, NULL);
	jclass klass;
	jvmti_env->GetMethodDeclaringClass(method, &klass);
	jvmti_env->GetClassSignature(klass, &class_sig, NULL);

	/*
	 if (strcmp(methodName, "write")!=0 && strcmp(methodName, "open")!=0) {
	 return;
	 }
	 */

	long long *startingCallbackTime = new long long[1];
	long long *startingCallbackRTime = new long long[1];
	if (startingCallbackTime == NULL || startingCallbackRTime == NULL) {
		return;
	}

	int size = vtimesStack.size();
	if (size > 0) {
		//		startingCallbackTime = vtimesStack.top();
		//		startingCallbackRTime = rtimesStack.top();

		//	long long vct = Time::getVirtualTime();
		//	long long rct = Time::getRealTime();

		//	(*startingCallbackTime) = (vct)-(*startingCallbackTime);
		//	(*startingCallbackRTime) = (rct)-(*startingCallbackRTime);

		//fprintf(logFile, "JVMTI\t%s exited method\t%s -> %s %s\t%lld\t%lld\t%lld\t%lld\t\t", threadName, class_sig, methodName, sign, vct*1000, rct*1000, (*startingCallbackTime) *1000, (*startingCallbackRTime) *1000);
/*
		fprintf(logFile, "JVMTI exited method\t%s -> %s %s\t%lld\t%lld\t\t\t\t", class_sig, methodName, sign, (*startingCallbackTime), (*startingCallbackRTime));

		for (int i=0;i<size;i++) {
			fprintf(logFile, "\t");

		}
		fprintf(logFile, "}\n");
*/
	fprintf(logFile, "exited\t%s -> %s %s\n", class_sig, methodName, sign);
		vtimesStack.pop();
		rtimesStack.pop();

		delete startingCallbackTime;
		delete startingCallbackRTime;
	}
}
#endif

#if CHANGE_NATIVE_METHODS > 0
void JNICALL
callbackNativeMethodBind(jvmtiEnv *jvmti_env, JNIEnv* jni_env, jthread thread, jmethodID method, void* address, void** new_address_ptr) {

	char *methodName;
	char *sign;
	char *class_sig;
	jvmti_env->GetMethodName(method, &methodName, &sign, NULL);
	jclass klass;
	jvmti_env->GetMethodDeclaringClass(method, &klass);
	jvmti_env->GetClassSignature(klass, &class_sig, NULL);
	/*
	 if (strcmp(methodName, "write")!=0 && strcmp(methodName, "open")!=0) {
	 return;
	 }
	 */

	long long vct = Time::getVirtualTime();
	long long rct = Time::getRealTime();

	fprintf(logFile, "JVMTI\tBINDING NATIVE METHOD\t%s -> %s %s\t%lld\t%lld\t\t\n", class_sig, methodName, sign, vct, rct);

}
#endif	


#if TRIGGER_CLASS_HOOKS > 0
void JNICALL
callbackCFLHook(jvmtiEnv *jvmti_env, JNIEnv* jni_env, jclass class_being_redefined, jobject loader, const char* name, jobject protection_domain, jint class_data_len, const unsigned char* class_data, jint* new_class_data_len, unsigned char** new_class_data) {

	fprintf(stderr,"CFLHook for %s with class data %s\n", name, class_data);
	fflush(stderr);
	enterCriticalSection(jvmti); {

		// Safety check, if VM is dead, skip this.
		if ( !gdata->vmDead ) {
			const char * classname;

			// If you have no classname, dig it out of the class file.
			if ( name == NULL ) {
				classname = java_crw_demo_classname(class_data,
						class_data_len, NULL);
			} else {
				classname = strdup(name);
			}

			// Assume you won't change the class file at first.
			*new_class_data_len = 0;
			*new_class_data = NULL;

			// Be careful that you don't track the tracker class.
			if (strcmp(classname, STRING(HEAP_TRACKER_class))!=0) {
				jint cnum;
				int systemClass;
				unsigned char *newImage;
				long newLength;

				// Processed class counter
				cnum = gdata->ccount++;

				// Tell java_crw_demo if this is an early class.
				systemClass = 0;
				if ( !gdata->vmStarted ) {
					systemClass = 1;
				}

				// Use java_crw_demo to create a new class file.
				newClassData = NULL;
				newLength = 0;
				java_crw_demo(cnum, classname, class_data,
						class_data_len, systemClass,
						STRING(HEAP_TRACKER_class),
						"L" STRING(HEAP_TRACKER_class) ";",
						NULL, NULL, NULL, NULL,
						STRING(HEAP_TRACKER_newobj),
						"(Ljava/lang/Object;)V",
						STRING(HEAP_TRACKER_newarr),
						"(Ljava/lang/Object;)V",
						&newClassData, &newLength, NULL, NULL);

				// If it did something, make a JVM TI copy.
				if ( newLength > 0 ) {
					unsigned char *jvmti_space;
					jvmti_space = (unsigned char *)
							allocate(jvmti, (jint)newLength);
					(void)memcpy(jvmti_space,
							newClassData, newLength);
					*new_class_data_len = (jint)newLength;
					*new_class_data = jvmti_space;
				}

				// Free any malloc space created.
				if ( newClassData != NULL ) {
					(void)free((void*)newClassData);
				}
			}

			// Free the classname (malloc space too).
			(void)free((void*)classname);
		}
	}exitCriticalSection(jvmti);
}

}
#endif

#if TRACK_JIT > 0
void JNICALL
callbackCompiledMethodLoad(jvmtiEnv *jvmti_env, jmethodID method, jint code_size, const void* code_addr, jint map_length, const jvmtiAddrLocationMap* map, const void* compile_info) {

	char *methodName;
	char *sign;
	jvmti_env->GetMethodName(method, &methodName, &sign, NULL);

	if (agentDebug > 0) {
		fprintf(logFile,"JIT: compiled method %s %s\n", methodName, sign);

	}

}

#endif


