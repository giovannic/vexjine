#ifndef NETWORK_QUEUE_H
#define NETWORK_QUEUE_H

#include <cstdlib>
class CustomerMeasure;
class SystemMeasure;


class Customer;

class Queue {

public:
	Queue() ;
	Queue( int cap ) ;
	int getCapacity() ;
	bool isInfinite() ;
	bool isEmpty() ;
	bool canAccept( Customer *c ) ;
	int queueLength() ;
	void enqueue( Customer *c ) ;
	void enqueueAtHead( Customer *c ) ;
	Customer *head() ;
	Customer *dequeue() ;

	double meanQueueLength() ;
	double varQueueLength() ;
	double meanTimeInQueue() ;
	double varTimeInQueue() ;
	void resetMeasures() ;

	virtual ~Queue();
protected:
	int pop;

	virtual void insertIntoQueue( Customer *o ) = 0;
	virtual void insertAtHeadOfQueue( Customer *o ) = 0;
	virtual Customer *headOfQueue() = 0;
	virtual Customer *removeFromQueue() = 0;

private:
	CustomerMeasure *queueingTime;
	SystemMeasure *popMeasure;
	int capacity ;

};

#endif
