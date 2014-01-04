#ifndef NETWORK_DEBUG_H
#define NETWORK_DEBUG_H

#include "Node.h"
#include "Customer.h"

#include <sstream>

class Debug {

public:
	static void setDebugOn() ;
	static void setDebugOff() ;
	static void dumpState(Network *network) ;
	static void traceMove(Customer *c, CinqsNode *to ) ;
	static void trace( const std::string &s ) ;
	static bool debug;

};

#endif
