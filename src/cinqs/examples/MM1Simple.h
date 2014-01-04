#ifndef EXAMPLES_MM1SIMPLE_H
#define EXAMPLES_MM1SIMPLE_H

#include "Sim
Event
Event.h"

class MM1Simple : public Sim {
  class Arrival : public Event {
  class Departure : public Event {

public:
	Arrival( double t ) ;
	void invoke() ;
	Departure( double t ) ;
	void invoke() ;
	void resetMeasures() ;
	bool stop() ;
	MM1Simple() ;

};

#endif
