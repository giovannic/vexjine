#include "network/Network.h"
#include "Logger.h"
#include "SimulationTime.h"
#include "Check.h"

using namespace std;

const int Network::maxNodes = 1000 ;
const int Network::maxClasses = 100 ;
const int Network::maxPriorities = 100 ;
CinqsNode *Network::nullNode = NULL;


Network::Network() {
	init(maxNodes);
}

Network::Network(const int &totalNodes) {
	init(totalNodes);
}

void Network::init(const int &totalNodes) {
	Logger::initialize();
	nodes = new CinqsNode *[totalNodes] ;
	for (int i = 0; i< totalNodes; i++) {
		nodes[i] = NULL;
	}
	nodeCount = 0 ;
	completions = 0 ;
	responseTime = new CustomerMeasure();
	initialised = true ;
	if (nullNode == NULL) {
		nullNode = new NullNode();
	}
}


Network::~Network() {
	for (int i = 0; i< 1000; i++) {
		if (nodes[i] != NULL) {
			delete nodes[i];
			nodes[i] = NULL;
		}
	}
	delete []nodes;
	nodes = NULL;

	delete responseTime;
	responseTime = NULL;

}

int Network::add( CinqsNode *n ) {
	Check::check( initialised, "This may cause problems with replicated runs.\nSimulation aborting." ) ;
	nodes[ nodeCount++ ] = n ;
	return nodeCount-1 ;
}

void Network::resetMeasures() {
	for ( int i = 0 ; i < nodeCount ; i++ ) {
		nodes[ i ]->resetMeasures() ;
	}
}

void Network::enterNode( Customer *c, CinqsNode *n ) {
	nodes[n->getId()]->enter(c);
}

void Network::registerCompletion( double t ) {
	responseTime->add( t ) ;
}

void Network::increaseCustomerCompletions() {
	++completions;
}

void Network::displayResults() {
	Logger::displayResults() ;
}

void Network::displayResults( double alpha ) {
	Logger::displayResults( alpha ) ;
}

void Network::logResult( const string &id, double result ) {
	Logger::logResult( id, result ) ;
}

void Network::logResults() {
	Logger::logResult( "Completion time", SimulationTime::now() ) ;
	Logger::logResult( "Completed customers", completions ) ;
	Logger::logResult( "Mean time in network", responseTime->mean() ) ;
	Logger::logResult( "Variance of time in network", responseTime->variance() ) ;
	for ( int i = 0 ; i < nodeCount ; i++ ) {
		nodes[ i ]->logResults() ;
	}
}


