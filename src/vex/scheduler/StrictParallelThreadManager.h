#ifndef STRICTTHREADMANAGER_H_
#define STRICTTHREADMANAGER_H_

#include "ThreadManager.h"

class StrictParallelThreadManager : public ThreadManager {

public:
	StrictParallelThreadManager(unsigned int _id, VirtualTimelineController *timelines, ThreadQueue *runnableThreads, IoSimulator *ioSim, ThreadRegistry *tReg, ObjectRegistry *oReg) : ThreadManager(_id, timelines, runnableThreads, ioSim, tReg, oReg) , synchronizationWaits(0), normalContinuations(0), blockedByErt(0) {} ;
	~StrictParallelThreadManager();

	std::string getStats();
	void resumeThread(VexThreadState *state);

protected:
	void suspendRunningResumeNext();
	void afterModelSimulation();
	bool noThreadsToSchedule();

//	void onIntentionToKeepOnRunning(VexThreadState *state);
	bool _suspendThread(VexThreadState *state);

private:
	bool isThreadTooFarAheadInVirtualTime(VexThreadState *state);

	long synchronizationWaits;
	long normalContinuations;
	long blockedByErt;
};

#endif /*STRICTTHREADMANAGER_H_*/
