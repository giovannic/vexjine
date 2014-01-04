/*
 * AggregateStateCounters.cpp
 *
 *  Created on: 20 Apr 2011
 *      Author: root
 */

#include "AggregateStateCounters.h"

AggregateStateCounters::AggregateStateCounters() {
	pthread_mutex_init(&stateTransitionMutex, NULL);
	for (int i =0; i<POSSIBLE_THREADSTATES; i++) {
		for (int j =0; j<POSSIBLE_THREADSTATES; j++) {
			aggregateStateTransitions[i][j] = 0;
		}
		aggregateTotalTimePerState[i] = 0;
	}

	totalInvocationPoints = 0;

}

AggregateStateCounters::~AggregateStateCounters() {
	pthread_mutex_destroy(&stateTransitionMutex);
}


void AggregateStateCounters::addTransitionsOf(VexThreadState *state) {
	long long *totalTimePerState = state->getTotalTimePerState();
	StateTransitionMatrix *stateTransitionsMatrix = state->getStateTransitionsMatrix();
	pthread_mutex_lock(&stateTransitionMutex);
	for (int i =0; i<POSSIBLE_THREADSTATES; i++) {
		for (int j =0; j<POSSIBLE_THREADSTATES; j++) {
			aggregateStateTransitions[i][j] += (*stateTransitionsMatrix)[i][j];
		}
		aggregateTotalTimePerState[i] += totalTimePerState[i];
	}

	totalInvocationPoints += state->getInvocationPoints();
	pthread_mutex_unlock(&stateTransitionMutex);
}


void AggregateStateCounters::printTransitionMapAndGraph(const char *filename) {

	ofstream csvOutput;
	ofstream dotOutput;

	char threadFilename[strlen(filename)+6];
	sprintf(threadFilename, "%s.csv", filename);
	csvOutput.open(threadFilename, ios::out);

	sprintf(threadFilename, "%s.dot", filename);
	dotOutput.open(threadFilename, ios::out);

	dotOutput << "digraph state_transition_graph {" << endl;

	int totalOccurencesPerState[POSSIBLE_THREADSTATES];
	// Print as csv file
	pthread_mutex_lock(&stateTransitionMutex);
	long long totalTimeInAllStates = 1;
	for (int i =0; i<POSSIBLE_THREADSTATES; i++) {
		totalTimeInAllStates += aggregateTotalTimePerState[i];
		totalOccurencesPerState[i] = 0;
		for (int j=0; j<POSSIBLE_THREADSTATES; j++) {
			totalOccurencesPerState[i] += aggregateStateTransitions[j][i];
		}
	}

	csvOutput << "State,Occurences,Time,Percent" << endl;
	for (int i =0; i<POSSIBLE_THREADSTATES; i++) {
		VexThreadStates::Code threadState = static_cast<VexThreadStates::Code>(i);
		csvOutput << VexThreadState::getStateName(threadState) << "," << totalOccurencesPerState[i] << "," << aggregateTotalTimePerState[i] << "," << (100*aggregateTotalTimePerState[i]/totalTimeInAllStates) << endl;
	}
	csvOutput << endl << endl;

	csvOutput << "From,To,Times" << endl;
	for (int i =0; i<POSSIBLE_THREADSTATES; i++) {
		VexThreadStates::Code threadStateFrom = static_cast<VexThreadStates::Code>(i);
		for (int j =0; j<POSSIBLE_THREADSTATES; j++) {
			if (aggregateStateTransitions[i][j] != 0) {
				VexThreadStates::Code threadStateTo = static_cast<VexThreadStates::Code>(j);
				csvOutput << VexThreadState::getStateName(threadStateFrom) << "," << VexThreadState::getStateName(threadStateTo) << "," << aggregateStateTransitions[i][j] << endl;
				//dotOutput << "\t" << ThreadState::getStateName(i) << "_" << (100*aggregateTotalTimePerState[i]/totalTimeInAllStates) << " -> " << ThreadState::getStateName(j) << "_" << (100*aggregateTotalTimePerState[j]/totalTimeInAllStates) << " [label=\"" << aggregateStateTransitions[i][j] << "\"];" << endl;
				dotOutput << "\t" << VexThreadState::getStateName(threadStateFrom) << " -> " << VexThreadState::getStateName(threadStateTo) << " [label=\"" << aggregateStateTransitions[i][j] << "\"];" << endl;
			}
		}
	}
	pthread_mutex_unlock(&stateTransitionMutex);

	dotOutput << "}" << endl;
	csvOutput.close();
	dotOutput.close();

	// Create ps file
	char psFilename[strlen(filename)+6];
	sprintf(psFilename, "-o%s.ps" , filename);

	char dotExecutable[] = {"dot"};
	char outputFlag[] = {"-Tps"};

	char * const arguments[] = {dotExecutable, threadFilename, psFilename, outputFlag, NULL};
	if (fork() == 0) {
		execv("/usr/bin/dot", arguments);
	}

}


void AggregateStateCounters::show() {
	pthread_mutex_init(&stateTransitionMutex, NULL);
	for (int i =0; i<POSSIBLE_THREADSTATES; i++) {
		VexThreadStates::Code threadStateFrom = static_cast<VexThreadStates::Code>(i);
		for (int j =0; j<POSSIBLE_THREADSTATES; j++) {
			if (aggregateStateTransitions[i][j] != 0) {
				VexThreadStates::Code threadStateTo = static_cast<VexThreadStates::Code>(j);
				cout << VexThreadState::getStateName(threadStateFrom) << " to " << VexThreadState::getStateName(threadStateTo) << " = " << aggregateStateTransitions[i][j] << endl;
			}
		}
	}
}

void AggregateStateCounters::clear() {
	pthread_mutex_lock(&stateTransitionMutex);
	for (int i =0; i<POSSIBLE_THREADSTATES; i++) {
		for (int j =0; j<POSSIBLE_THREADSTATES; j++) {
			aggregateStateTransitions[i][j] = 0;
		}
		aggregateTotalTimePerState[i] = 0;
	}
	pthread_mutex_unlock(&stateTransitionMutex);
}

