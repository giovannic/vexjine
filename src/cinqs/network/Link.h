#ifndef NETWORK_LINK_H
#define NETWORK_LINK_H

#include "Network.h"

class Earth;

class Link {

public:
	static Earth *earth;
	Link() ;
	Link( CinqsNode *n ) ;
	void setOwner( CinqsNode *n ) ;
	CinqsNode *getOwner() ;
	virtual void move( Customer *c ) ;
	virtual ~Link();

protected:
	void send( Customer *c, CinqsNode *n ) ;

	Network *network;
	CinqsNode *n ;
	CinqsNode *owner ;
};

class Earth : public Link {
public:
	void move( Customer *c ) {
		send( c, Network::nullNode ) ;
	}
};



#endif
