#ifndef NETWORK_SINK_H
#define NETWORK_SINK_H

#include "Node.h"

class Sink : public CinqsNode {

public:
	Sink( Network *_network ) : CinqsNode (_network, "Sink") {};
	Sink( Network *_network, const std::string &name ) : CinqsNode (_network, name) {};

	virtual std::string getNodeType();
protected:
	void accept( Customer *c ) ;

};

#endif
