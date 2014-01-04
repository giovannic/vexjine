#ifndef EXAMPLES_CPUSIM_H
#define EXAMPLES_CPUSIM_H

#include "Sim.h"

class Supermarket : public Sim {

public:
	Supermarket(Network *network) ;
	bool stop() ;

private:
	Network *network;
};

#endif
