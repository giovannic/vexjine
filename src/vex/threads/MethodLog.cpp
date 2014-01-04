/*
 * MethodLog.cpp
 *
 *  Created on: 21 Apr 2011
 *      Author: root
 */

#include "MethodLog.h"

using namespace std;

MethodLog::MethodLog() {
	currentMethodInfo = NULL;	// used for accounting of lost time
	exitingMethodInfo = new MethodCallInfo();

	methodStack = NULL;	// stack of the methods invoked by the thread

	nextMethodInfoToUse = 0;
	methodInfoStorage = new vector<MethodCallInfo *>;	// we never (unless in destructor) delete these MethodCallInfos, just update them
}

MethodLog::~MethodLog() {
	if (methodInfoStorage != NULL) {
		vector<MethodCallInfo*>::iterator iter;
		for (iter = methodInfoStorage->begin(); iter != methodInfoStorage->end(); iter++) {
			if ((*iter) != NULL) {
				delete (*iter);			// this cannot be NULL
				(*iter) = NULL;
			}
		}

		delete methodInfoStorage;	// none of them can be NULL
		methodInfoStorage = NULL;
	}
	if (exitingMethodInfo!= NULL) {
		delete exitingMethodInfo;
		exitingMethodInfo = NULL;
	}
}


/*
 * Push a new method entry event into the stack where all profiled methods entered by the thread are stored
 */
void MethodLog::pushMethodEntryEventIntoThreadMethodStack(MethodCallInfo *callinfo) {
	stack<MethodCallInfo*>* threadMethodStack = getThreadMethodStack();
	threadMethodStack -> push(callinfo);
	currentMethodInfo = callinfo;
}

/*
 * Returning the id of the current method
 */
int MethodLog::getCurrentMethodId() {
	if (currentMethodInfo != NULL) {
		return currentMethodInfo->getMethodId();
	} else {
		return 0;
	}
}

/*
 * Return the stack where all profiled methods entered by the thread are stored
 */
stack<MethodCallInfo*>* MethodLog::getThreadMethodStack() {
	if (methodStack == NULL) {
		methodStack = new stack<MethodCallInfo*>;
		if (methodStack == NULL) {
			return NULL;
		}
	}
	return methodStack;
}
