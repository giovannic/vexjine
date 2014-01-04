#include "List.h"
#include "network/RandomQueue.h"
#include "network/Customer.h"

//
// Random queue.  Customers are added at a random position
// in the queue.  Customers are removed from the head of the queue.
// Note: this is the same as random removal with insertion at the tail.
//
RandomQueue::~RandomQueue() {
	delete q;
}


void RandomQueue::insertIntoQueue( Customer *e ) {
	//GeneralIterator *it = q->getIterator() ;
	ListIterator *it = q->getIterator() ;
	int index = (int)( ((double)rand()/(double)RAND_MAX) * ( pop + 1 ) ) ;
	for ( int i = 0 ; i < index ; i++ ) {
		it->advance() ;
	}
	it->add( e ) ;
	delete it;
}

void RandomQueue::insertAtHeadOfQueue( Customer *e ) {
	q->insertAtFront( e ) ;
}

Customer *RandomQueue::headOfQueue() {
	return q->first() ;
}

Customer *RandomQueue::removeFromQueue() {
	return q->removeFromFront() ;
}

