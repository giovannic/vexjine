#ifndef NETWORK_LIFOQUEUE_H
#define NETWORK_LIFOQUEUE_H

#include "Queue.h"
class List;

class LIFOQueue : public Queue {

public:
	LIFOQueue() : Queue() {q = new List( "LIFO Queue" ) ;};
	LIFOQueue( int cap ) : Queue(cap) {q = new List( "LIFO Queue" ) ;};

	~LIFOQueue();
protected:
	void insertIntoQueue( Customer *e ) ;
	void insertAtHeadOfQueue( Customer *e ) ;
	Customer *headOfQueue() ;
	Customer *removeFromQueue() ;

private:
	List *q ;

};

#endif
