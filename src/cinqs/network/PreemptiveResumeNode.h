#ifndef NETWORK_PREEMPTIVERESUMENODE_H
#define NETWORK_PREEMPTIVERESUMENODE_H

#include "QueueingNode.h"
#include "LIFOQueue.h"

class PreemptiveResumeNode : public QueueingNode {

public:
	PreemptiveResumeNode( Network *_network, const std::string &s, Delay *d ) : QueueingNode(_network, s, d, 1, new LIFOQueue()) {};
	PreemptiveResumeNode( Network *_network, const std::string &s, Delay *d, Queue *q ) : QueueingNode(_network, s, d, 1, q) {
		Check::check( q->isInfinite(), "Node '" + name + "' must have an infinite-capacity queue" ) ;};
	//
	// Note: all queues (FIFO, LIFO, Random...) behave the same
	// way here, with the exception of a priority queue.
	// All preemptive nodes must have a single server and
	// infinite queue capacity. Because of the single server assumption
	// the lastEndServiceEvent defined in the InfiniteServer superclass
	// refers to the the last and only outstanding service completion
	// event.  This is the one to deschedule on preemption.
	//
	double remainingServiceTime( Customer *c );
	virtual std::string getNodeType();

protected:
	void accept( Customer *c ) ;

};

#endif
