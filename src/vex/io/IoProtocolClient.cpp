/*
 * IoProtocolClient.cpp
 *
 *  Created on: 13 Oct 2010
 *      Author: root
 */

#include "IoProtocolClient.h"

/***
 * I/O serial: only one thread runs at each time - the manager gets frozen at the time
 **/
IoProtocolSerialClient::IoProtocolSerialClient() {

}

void IoProtocolSerialClient::onStart(VexThreadState *state) {

}

void IoProtocolSerialClient::onEnd(VexThreadState *state, const long long &actualIoDuration) {
	((ThreadManagerClient *)state->getThreadCurrentlyControllingManager())->sendSchedulerRequest(REQUEST_PROGRESS_TIME_BY, state, actualIoDuration-state->getLastRealTime());
}



/***
 * I/O strict: 	threads are put on the top of the runnable threads. When they resume they execute their I/O, while allowing other threads
 * 				to resume. The real time of the I/O is used for the simulation time progress. No prediction
 **/
IoProtocolStrictClient::IoProtocolStrictClient() {

}

void IoProtocolStrictClient::onStart(VexThreadState *state) {

}

void IoProtocolStrictClient::onEnd(VexThreadState *state, const long long &actualIoDuration) {


}


/***
 * I/O lax: threads execute the I/O immediately while allowing otherare put on the top of the runnable threads. When they resume they execute their I/O, while allowing other threads
 * 			to resume. The real time of the I/O is used for the simulation time progress. No prediction
 **/
IoProtocolLaxClient::IoProtocolLaxClient() {
}

void IoProtocolLaxClient::onStart(VexThreadState *state) {

}

void IoProtocolLaxClient::onEnd(VexThreadState *state, const long long &actualIoDuration) {

}


/***
 * I/O normal: a thread may only run if its execution is before the expected predicted end of a previous I/O
 */
IoProtocolNormalClient::IoProtocolNormalClient() {
}

void IoProtocolNormalClient::onStart(VexThreadState *state) {

}


void IoProtocolNormalClient::onEnd(VexThreadState *state, const long long &actualIoDuration) {
	((ThreadManagerClient *)state->getThreadCurrentlyControllingManager())->sendSchedulerRequest(REQUEST_SET_GLOBAL_TIME_TO_THREADTIME, state);


}

