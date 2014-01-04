#ifndef NETWORK_FIFOQUEUE_H
#define NETWORK_FIFOQUEUE_H

#include "Queue.h"
#include "List.h"

class FIFOQueue : public Queue {

public:
	FIFOQueue() : Queue() {q = new List( "FIFO Queue" );};
	FIFOQueue( int cap ) : Queue(cap) {q = new List( "FIFO Queue" );};
	~FIFOQueue();

protected:
	void insertIntoQueue( Customer *e ) ;
	void insertAtHeadOfQueue( Customer *e ) ;
	Customer *headOfQueue() ;
	Customer *removeFromQueue() ;

private:
	List *q;

};

#endif
