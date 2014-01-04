#include "tools/Diary.h"
#include <iostream>
bool Diary::before( Object x, Object y ) {
	return ((Event *)x)->invokeTime() <= ((Event *)y)->invokeTime();
	//return ( ((Event)x).invokeTime <= ((Event)y).invokeTime ) ;
}

void Diary::insertInOrder( Event *e ) {
	OrderedList::insertInOrder( e ) ;
}

void Diary::remove( Event *e ) {

	OrderedList::remove( e ) ;
}

Event *Diary::removeFromFront() {
	Event *e = (Event *)OrderedList::removeFromFront() ;
	return e ;
}
