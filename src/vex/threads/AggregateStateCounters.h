/*
 * AggregateStateCounters.h
 *
 *  Created on: 20 Apr 2011
 *      Author: root
 */

#ifndef AGGREGATESTATECOUNTERS_H_
#define AGGREGATESTATECOUNTERS_H_

#include "ThreadState.h"

class AggregateStateCounters {
public:
	AggregateStateCounters();
	virtual ~AggregateStateCounters();

	void addTransitionsOf(VexThreadState *state);
	void printTransitionMapAndGraph(const char *filename);
	void show();
	void clear();


	unsigned long getTotalInvocationPoints() {
		return totalInvocationPoints;
	};

protected:
	unsigned long totalInvocationPoints;
	StateTransitionMatrix aggregateStateTransitions;
	long long aggregateTotalTimePerState[POSSIBLE_THREADSTATES];
	pthread_mutex_t stateTransitionMutex;
};

#endif /* AGGREGATESTATECOUNTERS_H_ */
