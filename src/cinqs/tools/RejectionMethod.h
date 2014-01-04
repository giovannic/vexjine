#ifndef TOOLS_REJECTIONMETHOD_H
#define TOOLS_REJECTIONMETHOD_H

#include "Uniform.h"

class RejectionMethod {

public:
	RejectionMethod( double a, double b, double m ) ;
	double next() ;
	virtual double density( double x ) = 0;
private:
	double a, b, m ;

};

#endif
