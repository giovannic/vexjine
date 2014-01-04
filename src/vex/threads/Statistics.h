/*
 * Statistics.h
 *
 *  Created on: 21 Apr 2011
 *      Author: root
 */

#ifndef STATISTICS_H_
#define STATISTICS_H_

#include "DebugUtil.h"
#include <iostream>
#include <cstring>

#define POSSIBLE_THREADSTATES 25

#if GDB_USAGE == 1
#define LOG_LAST_VEX_METHOD(state) \
state->setLastVexMethodInvoked(__FUNCTION__);

#define LOG_LAST_VEX_METHOD_TO_CURRENT_STATE \
VexThreadState *state = VexThreadState::getCurrentThreadState(); \
if (state != NULL) state->setLastVexMethodInvoked(__FUNCTION__);
#else
#define LOG_LAST_VEX_METHOD(state) ;
#define LOG_LAST_VEX_METHOD_TO_CURRENT_STATE ;
#endif

typedef unsigned int StateTransitionMatrix[POSSIBLE_THREADSTATES][POSSIBLE_THREADSTATES];

// Time monitoring position codes
enum {
	AFTER_ENTERING_METHOD = 1,
	BEFORE_EXITING_METHOD = 2,
	IN_SIGNAL_HANDLER = 3,
	UPDATING_INTERNAL = 4,
	BEFORE_SUSPENDING = 5,
	JUMPING_TO_CURRENT_TIME = 6
};

class Statistics {
public:
	Statistics();
	virtual ~Statistics();
	void logTransition(const short &currentState, const short &nextState) {
		#if COUNT_STATE_TRANSITIONS == 1
		++stateTransitions[currentState][nextState];
		#endif
	}

	void logTransition(const short &currentState, const short &nextState, long long durationOfLastState) {
		totalTimePerState[currentState] += durationOfLastState;
		logTransition(currentState, nextState);
	}
	inline short const &getMonitorPosition() {
		return monitorPosition;
	}
	inline void setMonitorPosition(const short &_newPosition) {
		monitorPosition = _newPosition;
	}
	inline void clearMonitorPosition() {
		monitorPosition = 0;
	}
	inline unsigned long getInvocationPoints() {
		return invocationPoints;
	};
	inline void setInvocationPoints(const unsigned long &points) {
		invocationPoints = points;
	};
	inline void addInvocationPoints() {
		++invocationPoints;
	};
	inline long long *getTotalTimePerState() {
		return &totalTimePerState[0];
	};
	inline StateTransitionMatrix *getStateTransitionsMatrix() {
		return &stateTransitions;
	};
	inline void setLastVexMethodInvoked(const char *_lastMethod) {
		strcpy(forelastVEXMethodInvoked, lastVEXMethodInvoked);
		strcpy(lastVEXMethodInvoked, _lastMethod);
	};

	void clearTransitionCounters();
//protected:
	short monitorPosition;	// denotes the position from which the thread is making a time update call
	long long totalTimePerState[POSSIBLE_THREADSTATES];
	unsigned long invocationPoints;
	StateTransitionMatrix stateTransitions;

	char forelastVEXMethodInvoked[64];
	char lastVEXMethodInvoked[64];

};

#endif /* STATISTICS_H_ */
