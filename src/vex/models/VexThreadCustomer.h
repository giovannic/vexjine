/*
 * VexThreadCustomer.h
 *
 *  Created on: 26 May 2011
 *      Author: root
 */

#ifndef VEXTHREADCUSTOMER_H_
#define VEXTHREADCUSTOMER_H_

#include "ThreadState.h"
#include "ThreadManager.h"
#include "Customer.h"
#include "Node.h"

class VexThreadCustomer : public Customer {
public:
	VexThreadCustomer() : Customer(), state(NULL), servicingResult(0) {
		pthread_cond_init(&customerQueueingCond, NULL);
	};
	VexThreadCustomer(int type) : Customer(type), state(NULL) , servicingResult(0){
		pthread_cond_init(&customerQueueingCond, NULL);
	};
	VexThreadCustomer(int type, int priority) : Customer(type, priority), state(NULL) , servicingResult(0){
		pthread_cond_init(&customerQueueingCond, NULL);
	};

	VexThreadCustomer(VexThreadState *_state) : Customer(), state(_state) , servicingResult(0) {
		manager = _state->getThreadCurrentlyControllingManagerPtr();
		pthread_cond_init(&customerQueueingCond, NULL);
	};
	VexThreadCustomer(VexThreadState *_state, int type) : Customer(type), state(_state) , servicingResult(0) {
		manager = _state->getThreadCurrentlyControllingManagerPtr();
		pthread_cond_init(&customerQueueingCond, NULL);
	};
	VexThreadCustomer(VexThreadState *_state, int type, int priority) : Customer(type, priority), state(_state) , servicingResult(0) {
		manager = _state->getThreadCurrentlyControllingManagerPtr();
		pthread_cond_init(&customerQueueingCond, NULL);
	};

	void setThreadState(VexThreadState *_state) {
		manager = _state->getThreadCurrentlyControllingManagerPtr();
		state = _state;
	};

	static void calculateIterationsPerNs();

	void lock();
	void unlock();

	const char *getThreadName();

	bool service(const long &time);
	bool blockAtQueue(pthread_mutex_t *);
	void resume(const long long &resumeTime);
	bool waitRemote(const long &);

	long long getEstimatedRealTime();
	void setEstimatedRealTime(const long long &estimatedRealTime);

	double getServicingResult() {
		return servicingResult;
	}
	void setFinished();
	void clearFinishedFlag();

	virtual ~VexThreadCustomer();
private:
	ThreadManager **manager;
	VexThreadState *state;

	pthread_cond_t customerQueueingCond;

	double servicingResult;
	void simulateExecutionOnCpuFor(const long &time);

	static void testServiceSimulationError(VexThreadCustomer *c);
	static double iterationsPerNs;
};

#endif /* VEXTHREADCUSTOMER_H_ */
