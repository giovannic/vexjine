#ifndef NETWORK_PRIORITYQUEUE_H
#define NETWORK_PRIORITYQUEUE_H

#include "Queue.h"

class PriorityQueue : public Queue {

public:
	PriorityQueue( int n ) ;
	bool canAccept( Customer *c ) ;
	virtual ~PriorityQueue();

protected:
	Queue *buildOneQueue() ;
	void buildPriorityQueue();

	void insertIntoQueue( Customer *e ) ;
	void insertAtHeadOfQueue( Customer *e ) ;
	Customer *headOfQueue() ;
	Customer *removeFromQueue() ;



	int nqueues ;
	Queue **qs ;
};

#endif
