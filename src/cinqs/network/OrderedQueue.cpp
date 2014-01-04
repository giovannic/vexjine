#include "network/OrderedQueue.h"
#include "network/Customer.h"

OrderedQueue::~OrderedQueue() {
	delete q;
}

void OrderedQueue::insertIntoQueue( Customer *e ) {
	q->insertInOrder( e ) ;
}

void OrderedQueue::insertAtHeadOfQueue( Customer *e ) {
	q->insertAtFront( e ) ;
}

Customer *OrderedQueue::headOfQueue() {
	return q->first() ;
}

Customer *OrderedQueue::removeFromQueue() {
	return q->removeFromFront() ;
}

// 
// The items inserted into the queue above comprise a customer
// and an insertion time.  The customer must implement the
// Ordered interface. The before method in OrderedList is
// given two queue entries.  It extracts the customer fields and
// then applies the ordering (smallerThan).  The ordering can be
// based on any attribute.
//
bool OrderedCustomerList::before( Object x, Object y ) {
	//Ordered x1 = (Ordered)((Customer)x) ;
	return ((Customer *)x)->smallerThan( y ) ;
}


