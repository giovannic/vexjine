/*
 * AgentTester.h
 *
 *  Created on: 12 Oct 2010
 *      Author: root
 */

#ifndef AGENTTESTER_H_
#define AGENTTESTER_H_

#include "Tester.h"

class AgentTester: public Tester {
public:
	AgentTester();
	virtual ~AgentTester();

	bool test();
	string testName();

	static void methodEntry(int methodId);
	static void methodExit(int methodId);

	static void afterMethodIoEntry(int methodId, int invocationPointHashValue);
	static void beforeMethodIoExit(int methodId);

	static void onThreadEnd(VexThreadState * state);
	static void onThreadSpawn(VexThreadState * state);

	static void onThreadWaitingStart(VexThreadState * state, long timeout);
	static void onThreadWaitingEnd(VexThreadState * state);
	static void onThreadContendedEnter(VexThreadState * state, pthread_mutex_t *mutex);
	static void onThreadContendedEntered(VexThreadState * state);

	static void onThreadWaitingStart(long timeout);
	static void onThreadWaitingEnd();

	static void profileOnlySelected();
	static bool interruptOnVirtualTimeout(VexThreadState *state, pthread_mutex_t *mutex, pthread_cond_t *cond, long timeout);
	static bool notifyAllTimedWaitingThreads(VexThreadState *state, pthread_mutex_t *mutex);
	static void onThreadInteractionPointEncounter(VexThreadState *state);

private:
	static bool onlySelectedProfiling;
};

#endif /* AGENTTESTER_H_ */
