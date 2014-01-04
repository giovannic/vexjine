#include "cinqs.h"
#include "Supermarket.h"

Supermarket::Supermarket(Network *_network) {
	network = _network;

	Source *entrance = new Source(network, "Entrance", new Exp( 1.0/0.08 ) ) ;

	InfiniteServerNode *shopping = new InfiniteServerNode(network,"Shopping", new Delay(new Exp( 1.0/20.0 )));

	QueueingNode *cashier = new QueueingNode(network, "Cashiers", new Delay( new Exp( 1.0/3.5 )) , 1 ) ;
	QueueingNode *machines = new QueueingNode(network, "Auto check-in", new Delay( new Exp( 1.0/1.5 )) , 1 ) ;

	Sink *sink          = new Sink(network, "Sink" ) ;

	double *routingProbs = new double[2];
	routingProbs[0]=0.1;
	routingProbs[1]=0.9;

	CinqsNode **nodes = new CinqsNode *[2];
	nodes[0] = cashier;
	nodes[1] = machines;

	ProbabilisticBranch *checkOut = new ProbabilisticBranch( routingProbs, nodes ) ;

	entrance->setLink(new Link( shopping ) ) ;
	shopping->setLink(checkOut) ;
	cashier->setLink( new Link( sink ) ) ;
	machines->setLink( new Link( sink ) ) ;

	simulate() ;
	network->logResults() ;
	network->displayResults() ;
	delete network;
}


bool Supermarket::stop() {
//	if (Network::completions % 1000 == 0	) {
//		cout << Network::completions << endl;
//	}
	return network->getCustomerCompletions() == 10000 ;
}

int main() {
	Network *network = new Network();
	Supermarket *c = new Supermarket(network);

	delete c;
	return 0;
}
