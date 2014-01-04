#include "network/Queue.h"

#include <climits>
#include "Check.h"
#include "CustomerMeasure.h"
#include "SystemMeasure.h"
#include "List.h"
#include "Sim.h"
#include "network/Customer.h"

Queue::Queue() {
	capacity = INT_MAX;
	pop = 0;
	queueingTime = new CustomerMeasure() ;
	popMeasure = new SystemMeasure() ;
}

Queue::Queue( int cap ) {
	capacity = cap ;
	pop = 0 ;
	queueingTime = new CustomerMeasure() ;
	popMeasure = new SystemMeasure() ;
}

Queue::~Queue() {
	delete queueingTime;
	delete popMeasure;
}

int Queue::getCapacity() {
	return capacity ;
}

bool Queue::isInfinite() {
	return ( capacity == INT_MAX ) ;
}

bool Queue::isEmpty() {
	return ( pop == 0 ) ;
}

bool Queue::canAccept( Customer *c ) {
	return pop < capacity ;
}

int Queue::queueLength() {
	return pop ;
}

void Queue::enqueue( Customer *c ) {
	Check::check( canAccept( c ), "Attempt to add to a full queue" ) ;
	c->setQueueInsertionTime( Sim::now() ) ;
	insertIntoQueue( c ) ;
	pop++ ;
	popMeasure->add( (float)pop ) ;
}

//
// The check isn't necessary as this can only be called after preemption
// i.e. after an arrival; the arrival will have checked the queue
// for spare capacity
//
void Queue::enqueueAtHead( Customer *c ) {
	Check::check( canAccept( c ), "Attempt to add to a full queue" ) ;
	c->setQueueInsertionTime( Sim::now() ) ;
	insertAtHeadOfQueue( c ) ;
	pop++ ;
	popMeasure->add( (float)pop ) ;
}

Customer *Queue::head() {
	Check::check( pop > 0, "Attempt to take the head of an empty queue" ) ;
	Customer *c = headOfQueue() ;
	return c ;
}

Customer *Queue::dequeue() {
	Check::check( pop > 0, "Attempt to dequeue an empty queue!" ) ;
	Customer *c = removeFromQueue() ;
	pop-- ;
	popMeasure->add( (float)pop ) ;
	queueingTime->add( Sim::now() - c->getQueueInsertionTime() ) ;
	return c ;
}

/**
 * These abstract methods allow different queueing disciplines
 * to be supported - see the various subclasses
 */






/**
 * Generic measures
 */

double Queue::meanQueueLength() {
	return popMeasure->mean() ;
}

double Queue::varQueueLength() {
	return popMeasure->variance() ;
}

double Queue::meanTimeInQueue() {
	return queueingTime->mean() ;
}

double Queue::varTimeInQueue() {
	return queueingTime->variance() ;
}

void Queue::resetMeasures() {
	queueingTime->resetMeasures() ;
	popMeasure->resetMeasures() ;
}

