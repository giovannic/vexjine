#include "network/BoxedQueue.h"

#include "Logger.h"
/**
 * @param q The queue to be boxed
 */

void BoxedQueue::accept( Customer *c ) {
	queue->enqueue( c ) ;
}

void BoxedQueue::dequeue() {
	forward( queue->dequeue() ) ;
}

/**
 * Generic measures - copied from Queue, as there is no multiple
 * inheritance!
 */
double BoxedQueue::meanQueueLength() {
	return queue->meanQueueLength() ;
}

double BoxedQueue::varQueueLength() {
	return queue->varQueueLength() ;
}

double BoxedQueue::meanTimeInQueue() {
	return queue->meanTimeInQueue() ;
}

double BoxedQueue::varTimeInQueue() {
	return queue->varTimeInQueue() ;
}

void BoxedQueue::resetMeasures() {
	queue->resetMeasures() ;
}

void BoxedQueue::logResults() {
	Logger::logResult( name + ", Mean number of customers in queue",
			meanQueueLength() ) ;
	Logger::logResult( name + ", Variance of number of customers in queue",
			varQueueLength() ) ;
	Logger::logResult( name + ", Mean time in queue",
			meanTimeInQueue() ) ;
	Logger::logResult( name + ", Variance of time in queue",
			varTimeInQueue() ) ;
}

