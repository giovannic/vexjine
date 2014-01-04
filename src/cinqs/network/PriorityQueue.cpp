#include "network/PriorityQueue.h"

#include "Check.h"
#include "FIFOQueue.h"

#include "network/Customer.h"
//
// Customer priorities must be 0, 1, .., nqueues-1
// Priority 0 is the highest priority
//
PriorityQueue::PriorityQueue( int n ) {
	nqueues = n ;
	buildPriorityQueue() ;
}

//
// By default each individual queue is FIFO...
//
Queue *PriorityQueue::buildOneQueue() {
	return new FIFOQueue() ;
}

void PriorityQueue::buildPriorityQueue() {
	qs = new Queue *[ nqueues ] ;
	for ( int i = 0 ; i < nqueues ; i++ ) {
		qs[i] = buildOneQueue() ;
	}
}

PriorityQueue::~PriorityQueue() {
	for ( int i = 0 ; i < nqueues ; i++ ) {
		delete qs[i];
	}
	delete qs;
}

//
//
bool PriorityQueue::canAccept( Customer *c ) {
	return qs[ c->getPriority() ]->canAccept( c ) ;
}

//
//

void PriorityQueue::insertIntoQueue( Customer *e ) {
	int priority = e->getPriority() ;
	qs[ priority ]->enqueue( e ) ;
}

void PriorityQueue::insertAtHeadOfQueue( Customer *e ) {
	int priority = e->getPriority() ;
	qs[ priority ]->enqueueAtHead( e ) ;
}

Customer *PriorityQueue::headOfQueue() {
	for ( int i = 0 ; i < nqueues ; i++ ) {
		if ( qs[ i ]->queueLength() > 0 ) {
			return qs[ i ]->head() ;
		}
	}
	Check::check( false, "Priority queue - all queues empty during head\n(This cannot happen!)" ) ;
	return NULL ;
}

Customer *PriorityQueue::removeFromQueue() {
	for ( int i = 0 ; i < nqueues ; i++ ) {
		if ( qs[ i ]->queueLength() > 0 ) {
			return qs[ i ]->dequeue() ;
		}
	}
	Check::check( false, "Priority queue - all queues empty during remove\n(This cannot happen!)" ) ;
	return NULL ;
}

