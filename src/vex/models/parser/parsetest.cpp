/*
   Test program for TinyXML.
 */


#include "tinyxml.h"
#include "JmtNetworkParser.h"
#include "cinqs.h"


class ExtractedSim : public Sim {

public:
	ExtractedSim() {};

	void setNetwork(Network *_network) {
		network = _network;
	}

	bool stop() {
		return network->getCustomerCompletions() == 10000;
	}

private:
	Network *network;

};


int main(int argc, char **argv) {
	const char *modelFileName;
	if (argc != 2) {
		modelFileName = "tinyxml/samples/super1.jsimg";
	} else {
		modelFileName = argv[1];
	}

	ExtractedSim *extractedSim = new ExtractedSim();
	Network *network =  JmtNetworkParser::parseNetwork(modelFileName, true, true);
//	extractedSim->setNetwork(network);
//	extractedSim->simulate() ;
//	network->logResults() ;
//	network->displayResults() ;
	delete network;

	return 0;
}
