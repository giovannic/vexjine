/*
 * JVMProfilingInvalidationEnforcer.cpp for Java applications executed on the VTF
 * Used to communicate to the upper-level that a class must be reloaded to remove
 * the profiling instruments from a method.
 *
 *  Created on: Jun 01, 2010
 *      Author: Nick
 *
 */

#include "JVMTIUtil.h"
#include "JVMProfilingInvalidationEnforcer.h"
#include "VTF.h"

#include <string.h>
#include <cassert>
#include <iostream>

using namespace std;


void JVMProfilingInvalidationEnforcer::check_out_of_memory(jobject aClass) {
	if (aClass == NULL) {
		cerr << "Out of memory exception thrown during class initialization of JVMProfilingInvalidationEnforcer" << endl;
		assert(false);
	}
}

void JVMProfilingInvalidationEnforcer::check_error(bool condition, const char *problemDescription) {
	if (condition) {
    	cerr << "Error in initializing JVMProfilingInvalidationEnforcer: Could not find " << problemDescription << endl;
    	assert(false);
	}
}


void JVMProfilingInvalidationEnforcer::createGlobalReferenceForClass(JNIEnv* jni_env, const char *className, jclass &jClass) {
	jclass localRefCls = jni_env->FindClass(className);
	check_error(localRefCls == NULL, className);
	jClass = reinterpret_cast<jclass>(jni_env->NewGlobalRef(localRefCls));
    check_out_of_memory(jClass);
}


void JVMProfilingInvalidationEnforcer::cacheStaticMethodId(JNIEnv* jni_env, jclass &jClass, const char *methodName, const char *methodDesc, jmethodID &jMethodId) {
	jMethodId = jni_env->GetStaticMethodID(jClass, methodName, methodDesc);
    check_error(jni_env->ExceptionCheck(), methodName);
}

void JVMProfilingInvalidationEnforcer::cacheMethodId(JNIEnv* jni_env, jclass &jClass, const char *methodName, const char *methodDesc, jmethodID &jMethodId) {
	jMethodId = jni_env->GetMethodID(jClass, methodName, methodDesc);
    check_error(jni_env->ExceptionCheck(), methodName);
}

// Create global references for all the jclass/jint objects involved in the invalidation and thread checking
JVMProfilingInvalidationEnforcer::JVMProfilingInvalidationEnforcer(JNIEnv* jni_env) {

	createGlobalReferenceForClass(jni_env, "java/lang/Thread", javaLangThreadClass);
    cacheStaticMethodId(jni_env, javaLangThreadClass, "currentThread", "()Ljava/lang/Thread;", currentThreadMethodId);
    cacheMethodId(jni_env, javaLangThreadClass, "getVtfProfiled", "()Z", getVtfProfiledMethodId);
	createGlobalReferenceForClass(jni_env, "virtualtime/ClassTransformer", javaClassTransformerClass);
    cacheStaticMethodId(jni_env, javaClassTransformerClass, "invalidateMethodProfiling", "(Ljava/lang/String;II)V", invalidateMethodMethodId);
    cacheStaticMethodId(jni_env, javaClassTransformerClass, "adaptRecursiveMethod", "(Ljava/lang/String;II)V", adaptRecursiveMethodId);

}

bool JVMProfilingInvalidationEnforcer::printingFoundRecursiveCalls;
void JVMProfilingInvalidationEnforcer::setProfilingInvalidationPrintouts(const bool &_printingFoundRecursiveCalls) {
	printingFoundRecursiveCalls = _printingFoundRecursiveCalls;
}


/*
 * This method contacts JVM to reload and adapt (remove instruments) for the method with id methodId
 */
void JVMProfilingInvalidationEnforcer::invalidateProfilingOf(JNIEnv *jni_env, const int & methodId) {
	MethodData *methodData = VEX::eventLogger->getMethodDataOf(methodId);
	if (methodData != NULL && !methodData->isInvalidated()) {

		jstring methodNameConverted = NULL;
		methodNameConverted = jni_env->NewStringUTF(methodData->getName());

		if (methodNameConverted != NULL) {

			jint method0Converted = static_cast<jint>(VEX::eventLogger->getMethod0(methodId));
			jint methodIdConverted = static_cast<jint>(methodId);

			if (printingFoundRecursiveCalls) {
				cout << "Invalidating profiling of: " << methodData->getName() <<" with methodId "<<methodId<<" and father " << method0Converted << endl;
			}

			jni_env->CallStaticVoidMethod(javaClassTransformerClass, invalidateMethodMethodId, methodNameConverted, methodIdConverted, method0Converted);
			if (jni_env->ExceptionCheck()) {
				jni_env->ExceptionDescribe();
				jni_env->ExceptionClear();
				cerr << "JVMProfilingInvalidationEnforcer could not call object method for ClassTransformer.invalidateProfilingOf " << methodData->getName() <<" with methodId "<<methodId<<" and father " << method0Converted << endl;
				return;
			}

			// Now that everything went fine, do not allow anyone else to invalidate the method again
			methodData->setInvalidated();
		} 
	}


}


/*
 * This method contacts JVM to reload and adapt (remove instruments) for the recursive method with id methodId
 * This approach has been enforced, because of strong indications that recursive methods are JIT-compiled
 * differently (tail-loop optimization) if instruments exist or not - removing the instruments seemed to do
 * the trick. The final tests on recursive methods have not validated this though.
 */
void JVMProfilingInvalidationEnforcer::reinstrumentRecursiveMethod(JNIEnv *jni_env, const int & methodId) {
	MethodData *methodData = VEX::eventLogger->getMethodDataOf(methodId);
	if (methodData != NULL && !methodData->isRecursive()) {

		jstring methodNameConverted = NULL;
		methodNameConverted = jni_env->NewStringUTF(methodData->getName());

		if (methodNameConverted != NULL) {

			jint method0Converted = static_cast<jint>(VEX::eventLogger->getMethod0(methodId));
			jint methodIdConverted = static_cast<jint>(methodId);

			if (printingFoundRecursiveCalls) {
				cout << "Recursive call: " << methodData->getName() <<" with methodId " << methodId << " and father " << method0Converted << endl;
			}

			jni_env->CallStaticVoidMethod(javaClassTransformerClass, adaptRecursiveMethodId, methodNameConverted, methodIdConverted, method0Converted);
			if (jni_env->ExceptionCheck()) {
				jni_env->ExceptionDescribe();
				jni_env->ExceptionClear();
				cerr << "JVMProfilingInvalidationEnforcer could not call object method for classTransformer.adaptRecursiveMethodId" << endl;
				return;
			}

			// Now that everything went fine, do not allow anyone else to adapt the recursive method again
			methodData->setRecursive();

		}

	}
}


