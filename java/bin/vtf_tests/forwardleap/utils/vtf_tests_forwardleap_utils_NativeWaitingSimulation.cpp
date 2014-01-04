#include "vtf_tests_forwardleap_utils_NativeWaitingSimulation.h"
#include <unistd.h>
#include <iostream>
#include <pthread.h>

using namespace std;

void myWait(int totalSeconds) {

	pthread_mutex_t mutex;
	pthread_cond_t cond;
	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&cond, NULL);

	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	ts.tv_sec += totalSeconds;

	pthread_mutex_lock(&mutex);
	pthread_cond_timedwait(&cond, &mutex, &ts);
	pthread_mutex_unlock(&mutex);

	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&cond);
	
}

JNIEXPORT void JNICALL Java_vtf_1tests_forwardleap_utils_NativeWaitingSimulation_waitOnceForLong(JNIEnv *, jclass) {
	myWait(6);
}

/*
 * Class:     vtf_tests_forwardleap_utils_NativeWaitingSimulation
 * Method:    waitShort
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_vtf_1tests_forwardleap_utils_NativeWaitingSimulation_waitShort(JNIEnv *, jclass) {
	myWait(1);
}
