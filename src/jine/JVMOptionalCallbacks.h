/*
 * JVMOptionalCallbacks: Used to enable a number of JVMTI callbacks for various debugging reasons.
 * Not used in the core framework
 *
 *  Created on: 12 Mar 2011
 *      Author: root
 */
#include "virtualtime_EventNotifier.h"
#include "JVMTIUtil.h"

void JNICALL callbackMethodEntry(jvmtiEnv *jvmti_env, JNIEnv* jni_env, jthread thread, jmethodID method);

void JNICALL callbackMethodExit(jvmtiEnv *jvmti_env, JNIEnv* jni_env, jthread thread, jmethodID method, jboolean was_popped_by_exception, jvalue return_value);

void JNICALL
callbackNativeMethodBind(jvmtiEnv *jvmti_env, JNIEnv* jni_env, jthread thread, jmethodID method, void* address, void** new_address_ptr);

void JNICALL
callbackCFLHook(jvmtiEnv *jvmti_env, JNIEnv* jni_env, jclass class_being_redefined, jobject loader, const char* name, jobject protection_domain, jint class_data_len, const unsigned char* class_data, jint* new_class_data_len, unsigned char** new_class_data);


void JNICALL
callbackCompiledMethodLoad(jvmtiEnv *jvmti_env, jmethodID method, jint code_size, const void* code_addr, jint map_length, const jvmtiAddrLocationMap* map, const void* compile_info);
