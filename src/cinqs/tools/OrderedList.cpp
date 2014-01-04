#include "tools/OrderedList.h"

//TODO clean memory
void OrderedList::insertInOrder( Object o ) {
	ListIterator *it = this->getIterator() ;
	while ( it->canAdvance() && before( it->getValue(), o ) ) {
		it->advance() ;
	}
	it->add( o ) ;
	delete it;
}


void OrderedList::insertInOrder( Event *e ) {
	ListIterator *it = this->getIterator() ;
	while ( it->canAdvance() && before( it->getValue(), (Object)e ) ) {
		it->advance() ;
	}
	it->add( (Object) e ) ;
	delete it;
}


