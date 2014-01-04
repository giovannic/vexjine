#ifndef NETWORK_NODE_H
#define NETWORK_NODE_H

#include "Customer.h"
#include <string>


class Network;
class Link;

class CinqsNode {

public:
	CinqsNode(Network *network) ;
	CinqsNode(Network *network, const std::string &s ) ;

	virtual ~CinqsNode();
	std::string toString() ;
	std::string getName() ;
	int getId() ;
	void setLink( Link *r ) ;

	virtual std::string getNodeType();
	virtual void resetMeasures() ;
	virtual void logResults() ;
	virtual void displayResults() ;
	virtual void enter( Customer *c ) ;
	virtual void forward( Customer *c ) ;

	Network *getNetwork() {
		return network;
	}
protected:
	virtual void accept( Customer *c ) ;

	Network *network;	// the queueing network that this node belongs to
	int id;
	std::string name ;
	Link *link;
	int arrivals ;
};

#endif
