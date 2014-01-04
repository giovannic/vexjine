#ifndef NETWORK_ORDEREDQUEUEENTRY_H
#define NETWORK_ORDEREDQUEUEENTRY_H

#include "Customer.h"
#include "Ordered.h"

class OrderedQueueEntry : public Customer {

public:
	Customer *entry ;
	OrderedQueueEntry( Customer *c, double t ) ;
	bool smallerThan( Customer *e ) ;
	double getTime() {
		return time;
	}
protected:
	double time ;
};

#endif
