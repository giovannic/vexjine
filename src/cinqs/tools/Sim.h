#ifndef TOOLS_SIM_H
#define TOOLS_SIM_H

#include "Event.h"
#include "Diary.h"

class Sim {

public:
	virtual bool stop() = 0;
	Sim() ;
	static double now() ;
	static void progress(double t) ;
	static void schedule( Event *e ) ;
	static void deschedule( Event *e ) ;
	void simulate() ;
	void simulate( double t ) ;
	void go() ;
	void resetMeasures() ;
	//EndOfWarmUp( double t ) ;
	void invoke() ;

	static Diary *diary ;
	static double *vtime ;
	static void initialize();
};


class EndOfWarmUp : public Event {
public:
	EndOfWarmUp( double t, Sim *sim ) : Event(t) {
		this->sim = sim;
	}
	void invoke() {
		sim->resetMeasures() ;
	}
private:
	Sim *sim;

};


#endif
