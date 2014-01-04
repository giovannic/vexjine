#ifndef NETWORK_NETWORK_H
#define NETWORK_NETWORK_H

#include "NullNode.h"
#include "CustomerMeasure.h"

#include <string>


class Network {

public:
	Network();
	Network(const int &totalNodes);
	~Network();

	void init(const int &totalNodes);

	virtual int add( CinqsNode *n ) ;

	void resetMeasures() ;
	void registerCompletion( double t ) ;
	void increaseCustomerCompletions();
	int getCustomerCompletions() {
		return completions;
	}
	int getNodeCount() {
		return nodeCount;
	}
	CinqsNode **getNodes() {
		return nodes;
	}
	void displayResults() ;
	void displayResults( double alpha ) ;
	void logResult( const std::string &id, double result ) ;
	void logResults() ;
	virtual void enterNode( Customer *c, CinqsNode *n );

	static const int maxNodes;
	static const int maxClasses;
	static const int maxPriorities;

	static CinqsNode *nullNode;

protected:
	CustomerMeasure *responseTime;
	int completions;
	int nodeCount;
	bool initialised;
	CinqsNode **nodes;

};

#endif
