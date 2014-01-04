#include "network/Debug.h"

#include "Network.h"
#include "Sim.h"

#include <iostream>
using namespace std;

bool Debug::debug = false;

void Debug::setDebugOn() {
	debug = true ;
}

void Debug::setDebugOff() {
	debug = false ;
}

void Debug::dumpState(Network *network) {
	if ( debug ) {
		cout <<  "-----------------" << endl ;
		CinqsNode **nodes = network->getNodes();
		for ( int i = 0 ; i < network->getNodeCount() ; i++ ) {
			cout <<  (nodes[ i ])->getName() << endl ;
		}
		cout <<  "-----------------" << endl ;
	}
}

void Debug::traceMove( Customer *c, CinqsNode *to ) {
	if ( debug ) {
		cout <<  "Time: " << Sim::now() << ", " << c->toString() << " moving from " << c->getLocation()->getName() << " to " << to->getName() << endl ;
	}
}

void Debug::trace( const string &s ) {
	if ( debug ) {
		cout <<  s  << endl;
	}
}
