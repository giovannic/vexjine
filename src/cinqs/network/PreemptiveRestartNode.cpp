#include "network/PreemptiveRestartNode.h"

#include <string>

//
// In the restart policy the remaining service time is the same
// as the original service demand...
//
double PreemptiveRestartNode::remainingServiceTime( Customer *c ) {
	return c->getServiceDemand() ;
}

string PreemptiveRestartNode::getNodeType() {
	return "PreemptiveRestartNode";
}
