#ifndef NETWORK_PROCESSORSHARINGNODE_H
#define NETWORK_PROCESSORSHARINGNODE_H

#include "ResourcePool.h"
#include "Event.h"
#include "OrderedQueue.h"

class ProcessorSharingNode : public ResourcePool {

public:
	ProcessorSharingNode( Network *_network, const std::string &s, Delay *d ) : ResourcePool( _network, s, d, 1, new OrderedQueue()) {vtime = 0.0;};
	//EndOfService( double t ) ;
	void releaseResource() ;

	void serviceNextCustomer();
	void invoke();
protected:
	void accept( Customer *c ) ;
	Event *nextEndServiceEvent ;
	double vtime;
	double timeOfLastSchedule ;


};

class EndOfService : public Event {
public:
	EndOfService( double t , ProcessorSharingNode *processorSharingNode) : Event(t) {this->processorSharingNode = processorSharingNode;};

	void invoke() {
		processorSharingNode->invoke();
//		vtime += ( Sim::now() - timeOfLastSchedule ) / queue.queueLength() ;
//		OrderedQueueEntry *e = queue->dequeue() ;
//		forward( e->entry ) ;
//		if ( queue->queueLength() == 0 ) {
//			resources->release() ;
//		} else {
//			serviceNextCustomer() ;
//		}
	}

private:
	ProcessorSharingNode *processorSharingNode;
};

#endif
