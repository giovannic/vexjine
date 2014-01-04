#include "network/OrderedQueueEntry.h"


/**
 * Some queues may be built using other queues (e.g. a priority
 * queue).  It is therefore convenient for queue entries to be
 * customers themselves. An outer enqueue can then pass its
 * argument directly to the inner queue, and so on.
 * To simplify the definition of ordered queues, it is also
 * useful for a default ordering to be based on time.  For example, 
 * in a PS queue each queue entry will be paired up with a finish time.
 * It is then important to order the queue entries by finish time.
 */
OrderedQueueEntry::OrderedQueueEntry( Customer *c, double t ) {
	time = t ;
	entry = c ;
}

bool OrderedQueueEntry::smallerThan( Customer *e ) {
	return this->time <= ((OrderedQueueEntry *)e)->time ;
}
