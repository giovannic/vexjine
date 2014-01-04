#ifndef NETWORK_RANDOMQUEUE_H
#define NETWORK_RANDOMQUEUE_H

#include "Queue.h"

class RandomQueue : public Queue {

public:
	RandomQueue() : Queue() {q = new List( "Random Queue" ) ;} ;
	RandomQueue( int cap ) : Queue(cap) {q= new List( "Random Queue" ) ;};
	virtual ~RandomQueue();

protected:
	void insertIntoQueue( Customer *e ) ;
	void insertAtHeadOfQueue( Customer *e ) ;
	Customer *headOfQueue() ;
	Customer *removeFromQueue() ;

private:
	List *q;

};

#endif
