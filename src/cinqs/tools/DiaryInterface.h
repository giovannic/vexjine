#ifndef TOOLS_DIARYINTERFACE_H
#define TOOLS_DIARYINTERFACE_H

#include "Event.h"

class DiaryInterface {
public:
	virtual void insertInOrder( Event *e ) = 0;
	virtual void remove( Event *e ) = 0;
	virtual Event *removeFromFront() = 0;
};

#endif
