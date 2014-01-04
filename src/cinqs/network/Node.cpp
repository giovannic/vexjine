#include "network/Node.h"

#include "Logger.h"
#include "Debug.h"
#include "Link.h"

#include <iostream>
using namespace std;

CinqsNode::CinqsNode(Network *_network) {
	link = Link::earth ;
	arrivals = 0 ;
	name = "Base Node" ;
	network = _network;
	if (_network != NULL) {
		id = network->add( this ) ;
	}

}

CinqsNode::CinqsNode(Network *_network, const string &s ) {
	name = s ;
	link = Link::earth ;
	arrivals = 0 ;
	network = _network;
	if (_network != NULL) {
		id = network->add( this ) ;
	}
}

CinqsNode::~CinqsNode() {
	if (link != Link::earth) {
		delete link;
	}
}
string CinqsNode::toString() {
	return name ;
}

string CinqsNode::getName() {
	return name ;
}

int CinqsNode::getId() {
	return id ;
}

void CinqsNode::setLink( Link *r ) {
	link = r ;
	link->setOwner( this ) ;
}

string CinqsNode::getNodeType() {
	return "Generic Node";
}

void CinqsNode::resetMeasures() {
}

void CinqsNode::logResults() {
	Logger::logResult( name + " arrivals", arrivals ) ;
}

void CinqsNode::displayResults() {
	cout <<  name << ":" << endl ;
	cout <<  "  Number of arrivals: " <<  arrivals << endl;
}

// ----------------------------------------------------------------------

//
// Customers enter from the outside, are then processed by accept
// and then forwarded to the next node by forward.
// These methods can variously be overridden to effect different
// behaviour.
//
void CinqsNode::enter( Customer *c ) {
//	Debug::trace( c->toString() + " entering " + toString()) ;
	arrivals++ ;
	c->setLocation( this ) ;
	accept( c ) ;
}

void CinqsNode::accept( Customer *c ) {
	forward( c ) ;
}

void CinqsNode::forward( Customer *c ) {
	link->move( c ) ;
}

