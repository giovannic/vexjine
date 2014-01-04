#include "network/FIFOQueue.h"


FIFOQueue::~FIFOQueue() {
	delete q;
}

void FIFOQueue::insertIntoQueue( Customer *e ) {
	q->insertAtBack( e ) ;
}

void FIFOQueue::insertAtHeadOfQueue( Customer *e ) {
	q->insertAtFront( e ) ;
}

Customer *FIFOQueue::headOfQueue() {
	return q->first() ;
}

Customer *FIFOQueue::removeFromQueue() {
	return q->removeFromFront() ;
}

