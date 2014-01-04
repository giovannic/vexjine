/*
 * LooseParallelThreadManager.h
 *
 *  Created on: 1 Sep 2011
 *      Author: root
 */

#ifndef LOOSEPARALLELTHREADMANAGER_H_
#define LOOSEPARALLELTHREADMANAGER_H_

#include "ThreadManager.h"
class LooseParallelThreadManager : public ThreadManager {
public:
	LooseParallelThreadManager(unsigned int _id, VirtualTimelineController *_globalTimer, ThreadQueue *runnableThreads, IoSimulator *ioSim, ThreadRegistry *tReg, ObjectRegistry *oReg) : ThreadManager(_id, _globalTimer, runnableThreads, ioSim, tReg, oReg) {};

	void setTimedWaitingThread(VexThreadState *state);
	void setRunningThread(VexThreadState *state);
	void setSuspended(VexThreadState *state);
	void setSuspendedAndPushToRunnables(VexThreadState *state);
	void notifySchedulerForVirtualizedTime(VexThreadState *state, const float &scalingFactor);

protected:
	void locklessUpdateTimeBy(const long long &timeDiff, VexThreadState *state);

	void onIntentionToKeepOnRunning(VexThreadState *state);
	void onIntentionToBeDisregarded(VexThreadState *state);

	void afterModelSimulation();
};

#endif /* LOOSEPARALLELTHREADMANAGER_H_ */
