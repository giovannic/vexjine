#ifndef TOOLS_SYSTEMMEASURE_H
#define TOOLS_SYSTEMMEASURE_H

#include "Measure.h"

class SystemMeasure : public Measure {

public:
	SystemMeasure() ;
	SystemMeasure( int m ) : Measure(m) {
		lastChange = 0.0;
		current = 0.0;
	};
	~SystemMeasure();
	void add( double x ) ;
	double mean() ;
	double variance() ;
	double currentValue() ;
	double timeLastChanged() ;

private:
	double lastChange;
	double current;

};

#endif
