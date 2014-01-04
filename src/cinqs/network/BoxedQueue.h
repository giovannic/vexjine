#ifndef NETWORK_BOXEDQUEUE_H
#define NETWORK_BOXEDQUEUE_H

#include "Node.h"
#include "Queue.h"

class BoxedQueue : public CinqsNode {

public:
	BoxedQueue( Network *_network, Queue *q ) : CinqsNode(_network, "Boxed Queue"), queue(q) {};
	BoxedQueue( Network *_network, const std::string &name, Queue *q ) : CinqsNode(_network, name), queue(q) {};

	void dequeue() ;
	double meanQueueLength() ;
	double varQueueLength() ;
	double meanTimeInQueue() ;
	double varTimeInQueue() ;


	void resetMeasures() ;
	void logResults() ;

protected:
	void accept( Customer *c ) ;

	/**
	 * A boxed queue is a node containing just a queue.  As it's a passive
	 * object a method ({@link dequeue}) is needed to dequeue
	 * and forward customers explicitly.
	 */
	Queue *queue ;

};

#endif
