#ifndef NETWORK_PREEMPTIVERESTARTNODE_H
#define NETWORK_PREEMPTIVERESTARTNODE_H

#include "PreemptiveResumeNode.h"
#include <string>
using namespace std;

class PreemptiveRestartNode : public PreemptiveResumeNode {

public:
	PreemptiveRestartNode( Network *_network, const std::string &s, Delay *d ) : PreemptiveResumeNode(_network, s, d) {};
	PreemptiveRestartNode( Network *_network, const std::string &s, Delay *d, Queue *q ) : PreemptiveResumeNode(_network, s, d, q) {};
	double remainingServiceTime( Customer *c );
	virtual std::string getNodeType();
};

#endif
