/**
 * 
 * Comments by Nick, 31 Oct 2008
 * 
 * This is the basic Agent class. First the Agent_OnLoad() is called and instantiates the 
 * global ThreadManager *manager.
 * 
 * Then the callbackVMInit method is called which starts the scheduler thread executing 
 * the threadManagerWorker function, which runs managers->start()
 *  
 * 
 */ 

#include <jvmti.h>
#include <pthread.h>
#include <queue>
#include <map>
#include <list>

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <syscall.h>
#include <string.h>
#include <sys/time.h>

#include "virtualtime_EventNotifiervtonly.h"
//#include "JVMTIUtil.h"

#define USING_PERFCTR 1
#if USING_PERFCTR == 0
#include <papi.h>
#else
extern "C" {
#include "perfdirect.h"
}
#endif
/* -------------------------------------------------------------------- */

/* Some constant maximum sizes */


#define DEBUG_AGENT 0


static jvmtiEnv *jvmti = NULL;
struct timeval program_start;
 

JNIEXPORT jlong JNICALL Java_virtualtime_EventNotifiervtonly__1getThreadVirtualTime(JNIEnv *, jclass) {

#if USING_PERFCTR == 0
	return PAPI_get_virt_nsec();//managers->getCurrentThreadVirtualtime(jvmti, jni_env, (jthread) thread);
#else	
	return PERFCTR_getVT();
#endif
}


JNIEXPORT jlong JNICALL Java_virtualtime_EventNotifiervtonly__1getPapiRealTime
  (JNIEnv *, jclass) {
#if USING_PERFCTR == 0
	return PAPI_get_real_nsec();
#else
	return PERFCTR_getRT();
#endif	

}


JNIEXPORT jlong JNICALL Java_virtualtime_EventNotifiervtonly__1getJvmtiVirtualTime
  (JNIEnv *, jclass, jobject thread) {
	jlong vtime;
	jvmti->GetThreadCpuTime((jthread)thread, &vtime);
	return vtime;
	//return  PAPI_get_virt_usec() * 1000;//managers->getCurrentThreadVirtualtime(jvmti, jni_env, (jthread) thread);

}


JNIEXPORT jlong JNICALL Java_virtualtime_EventNotifiervtonly__1getJvmtiRealTime
  (JNIEnv *, jclass) {


	jlong rtime;
	jvmti->GetTime(&rtime);
	return rtime;


}



JNIEXPORT jlong JNICALL Java_virtualtime_EventNotifiervtonly__1getElapsedTime
  (JNIEnv *, jclass) {
  	
  	struct timeval current_time;
  	gettimeofday(&current_time, NULL);
  	
	if (program_start.tv_usec > current_time.tv_usec) {  current_time.tv_usec += 1000000;  current_time.tv_sec--; }
	jlong result = (current_time.tv_usec - program_start.tv_usec) + (current_time.tv_sec - program_start.tv_sec) * 1000000;
	result *= 1000;    	
/*
	jvmtiTimerInfo timerInfo;
	jvmti->GetCurrentThreadCpuTimerInfo(&timerInfo);
	fprintf(stderr, "%d\n", timerInfo.kind); fflush(stderr);
*/	return result;

}


unsigned long gettid() {
	return (unsigned long) syscall(SYS_gettid);
}

void initTimers() {
#if USING_PERFCTR == 0
	int retval;
	fprintf(stderr, "1\n");

	if ((retval = PAPI_library_init(PAPI_VER_CURRENT)) != PAPI_VER_CURRENT) {
		printf("PAPI Library initialization error! \n");
		exit(1);
	}

	fprintf(stderr, "2\n");
	if ((retval = PAPI_set_debug(PAPI_VERB_ESTOP)) != PAPI_OK) {
		printf("PAPI set debug error! \n");
		exit(1);
	}
	fprintf(stderr, "3\n");
	if (PAPI_thread_init(pthread_self) != PAPI_OK) {
		printf("PAPI thread Library initialisation error! \n");
		exit(1);
	}
	fprintf(stderr, "4\n");
	PAPI_thread_init(gettid);
	PAPI_register_thread();
#else
	PERFCTR_initialize();
	PERFCTR_register_thread();
#endif

	gettimeofday(&program_start, NULL);
}


JNIEXPORT void JNICALL Java_virtualtime_EventNotifiervtonly_init(JNIEnv *, jclass) {
	initTimers();
	fprintf(stderr, "Usage of CPU time enabled!\n");
}


/**
 * Called when the agent is first loaded...
 * */
JNIEXPORT jint JNICALL Agent_OnLoad(JavaVM *jvm, char *options, void *reserved)

{
	
	if (DEBUG_AGENT > 1) {
		fprintf(stderr, "Agent_OnLoad\n");
		fflush(stderr);	
	}

	jvmtiError error;

	jint res;

	jvmtiEventCallbacks callbacks;

	jvmtiCapabilities capaTemp;
 
 jvmtiCapabilities capa;

	
	res = jvm->GetEnv((void **) &jvmti, JVMTI_VERSION_1_0);

	if (res != JNI_OK || jvmti == NULL) {
		printf("ERROR: Unable to access JVMTI Version 1 (0x%x),"

			" is your J2SE a 1.5 or newer version?"

			" JNIEnv's GetEnv() returned %d\n",

		JVMTI_VERSION_1, res);

	}


	
	(void) memset(&capaTemp, 1, sizeof(jvmtiCapabilities));
	
	// Relinguish all
	error = jvmti->GetCapabilities(&capaTemp);
//	fprintf(stderr, "capaTemp.can_suspend %d\n",capaTemp.can_suspend);
//		fflush(stderr);
	error = jvmti->RelinquishCapabilities(&capaTemp);

	// Add	
	(void) memset(&capa, 0, sizeof(jvmtiCapabilities));
	capa.can_signal_thread = 1;
	capa.can_get_owned_monitor_info = 1;
	capa.can_generate_monitor_events = 1;
	capa.can_get_thread_cpu_time = 1;
	capa.can_get_current_thread_cpu_time = 1;
	capa.can_suspend = 1;
	capa.can_tag_objects = 1;
	capa.can_set_native_method_prefix = 1;


	capa.can_get_current_thread_cpu_time = 1;
	
	error = jvmti->AddCapabilities(&capa);

	(void) memset(&callbacks, 0, sizeof(callbacks));
	error = jvmti->SetNativeMethodPrefix("__vtf_native_prefix_");

	initTimers();
//fprintf(stderr, "2.0 Finished VT Light Framework initialization\n");	fflush(stderr);
	return JNI_OK;

}

/* Agent_OnUnload: This is called immediately before the shared library is

 *   unloaded. This is the last code executed.

 */

JNIEXPORT void JNICALL

Agent_OnUnload(JavaVM *vm)

{

	/* Make sure all malloc/calloc/strdup space is freed */

}

