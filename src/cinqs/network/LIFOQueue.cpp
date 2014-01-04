#include "List.h"
#include "network/LIFOQueue.h"



LIFOQueue::~LIFOQueue() {
	delete q;
}

void LIFOQueue::insertIntoQueue( Customer *e ) {
	q->insertAtFront( e ) ;
}

void LIFOQueue::insertAtHeadOfQueue( Customer *e ) {
	q->insertAtFront( e ) ;
}

Customer *LIFOQueue::headOfQueue() {
	return q->first() ;
}

Customer *LIFOQueue::removeFromQueue() {
	return q->removeFromFront() ;
}

