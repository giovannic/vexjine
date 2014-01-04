#ifndef TOOLS_DIARY_H
#define TOOLS_DIARY_H

#include "OrderedList.h"
#include "DiaryInterface.h"

class Diary : public OrderedList, DiaryInterface {

public:
	bool before( Object x, Object y ) ;
	void insertInOrder( Event *e ) ;
	void remove( Event *e ) ;
	Event *removeFromFront() ;
};

#endif
