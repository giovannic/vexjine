#include "tools/Event.h"

int Event::nextId = 0 ;

Event::Event( double time ) {
	_invokeTime = time ;
	id = nextId++ ;
}

double Event::invokeTime() {
	return _invokeTime ;
}

Event::~Event() {

}
