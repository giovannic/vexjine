#ifndef NETWORK_NULLNODE_H
#define NETWORK_NULLNODE_H

#include "Node.h"

class NullNode : public CinqsNode {

public:
	NullNode() : CinqsNode( NULL, "Null node" ) {};

	void enter( Customer *c ) ;
	void logResults() ;
	void displayResults() ;

};

#endif
