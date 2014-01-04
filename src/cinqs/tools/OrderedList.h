#ifndef TOOLS_ORDEREDLIST_H
#define TOOLS_ORDEREDLIST_H

#include "List.h"
#include "Event.h"

#include <string>


class OrderedList : public List {

public:
	virtual bool before( Object x, Object y ) = 0;
	OrderedList() : List() {};
	OrderedList( const std::string &name ) : List(name) {};

	virtual void insertInOrder( Object o ) ;
	virtual void insertInOrder( Event *e ) ;

};

#endif
