/*
 * MethodLog.h
 *
 *  Created on: 21 Apr 2011
 *      Author: root
 */

#ifndef METHODLOG_H_
#define METHODLOG_H_

#include "MethodCallInfo.h"
#include "Timers.h"

#include <stack>

class MethodLog {
public:
	MethodLog();
	virtual ~MethodLog();

	inline MethodCallInfo *getCurrentMethodInfo() {
		return currentMethodInfo;
	};
	inline void setCurrentMethodInfo(MethodCallInfo *_newMethod) {
		currentMethodInfo = _newMethod;
	};
	inline MethodCallInfo *getExitingMethodInfo() {
		return exitingMethodInfo;
	};
	void decreaseNextMethodInfoToUse() {
		--nextMethodInfoToUse;
	}

	int getCurrentMethodId();
	std::stack<MethodCallInfo*>* getThreadMethodStack();
	void pushMethodEntryEventIntoThreadMethodStack(MethodCallInfo *callinfo);

	unsigned int const & getNextMethodInfoToUse() {
		return nextMethodInfoToUse;
	}
	std::vector<MethodCallInfo *> *getMethodInfoStorage() {
		return methodInfoStorage;
	}
	void increaseNextMethodInfoToUse() {
		++nextMethodInfoToUse;
	}
protected:

//	inline void setExitingMethodInfo(MethodCallInfo *_newMethod) {
//		exitingMethodInfo = _newMethod;
//	};


private:
	MethodCallInfo *currentMethodInfo;	// used to keep accounting of information regarding virtual accelerators
	MethodCallInfo *exitingMethodInfo;	// used to keep accounting of information regarding virtual accelerators
	std::vector<MethodCallInfo *> *methodInfoStorage;
	unsigned int nextMethodInfoToUse;
	std::stack<MethodCallInfo*>* methodStack;

	Timers *timers;
};

#endif /* METHODLOG_H_ */
