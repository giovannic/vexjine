#ifndef NETWORK_ORDEREDQUEUE_H
#define NETWORK_ORDEREDQUEUE_H

#include "Queue.h"
#include "OrderedList.h"

typedef Customer *Object;

class OrderedCustomerList : public OrderedList {
public:
	bool before( Object x, Object y );
};

class OrderedQueue : public Queue {

public:
	OrderedQueue() : Queue() {q = new OrderedCustomerList();};
	OrderedQueue( int cap ) : Queue(cap) {q = new OrderedCustomerList();};

	~OrderedQueue();
	bool before( Object x, Object y ) ;

protected:
	void insertIntoQueue( Customer *e ) ;
	void insertAtHeadOfQueue( Customer *e ) ;
	Customer *headOfQueue() ;
	Customer *removeFromQueue() ;

private:
	OrderedCustomerList *q; ;

};

#endif
