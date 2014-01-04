/*
 * Statistics.cpp
 *
 *  Created on: 21 Apr 2011
 *      Author: root
 */

#include "Statistics.h"

Statistics::Statistics() {

	invocationPoints = 0;
	monitorPosition = 0;

	strcpy(forelastVEXMethodInvoked, "not run yet");
	strcpy(lastVEXMethodInvoked, "not run yet");

	clearTransitionCounters();

}

Statistics::~Statistics() {
	 
}


void Statistics::clearTransitionCounters() {
	// All these used for the state transition graphs
	for (int i =0; i<POSSIBLE_THREADSTATES; i++) {
		for (int j =0; j<POSSIBLE_THREADSTATES; j++) {
			stateTransitions[i][j] = 0;
		}
		totalTimePerState[i] = 0;
	}
}
