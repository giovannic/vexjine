/*
 * VexSchedSimCustomer.h
 *
 *  Created on: 28 Jun 2011
 *      Author: root
 */

#ifndef VEXSCHEDSIMCUSTOMER_H_
#define VEXSCHEDSIMCUSTOMER_H_

#include "ThreadState.h"
#include "ThreadManager.h"
#include "Customer.h"
#include "Node.h"

class VexSchedSimCustomer : public Customer {
public:
	VexSchedSimCustomer() : Customer(), state(NULL) {};
	VexSchedSimCustomer(int type) : Customer(type), state(NULL) {};
	VexSchedSimCustomer(int type, int priority) : Customer(type, priority), state(NULL) {};

	VexSchedSimCustomer(VexThreadState *_state) : Customer(), state(_state) {
		manager = _state->getThreadCurrentlyControllingManagerPtr();
	};
	VexSchedSimCustomer(VexThreadState *_state, int type) : Customer(type), state(_state) {
		manager = _state->getThreadCurrentlyControllingManagerPtr();
	};
	VexSchedSimCustomer(VexThreadState *_state, int type, int priority) : Customer(type, priority), state(_state) {
		manager = _state->getThreadCurrentlyControllingManagerPtr();
	};

	void setThreadState(VexThreadState *_state) {
		manager = _state->getThreadCurrentlyControllingManagerPtr();
		state = _state;
	};

	void setLocation( CinqsNode *n ) ;

	const char *getThreadName();
	bool service(const long &time);
	bool blockAtQueue(pthread_mutex_t *q);
	void resume(const long long &resumeTime);
	bool waitRemote(const long &);

	long long getEstimatedRealTime();
	void setEstimatedRealTime(const long long &estimatedRealTime);
	void setFinished();
	void clearFinishedFlag();
private:
	ThreadManager **manager;
	VexThreadState *state;

};
#endif /* VEXSCHEDSIMCUSTOMER_H_ */
