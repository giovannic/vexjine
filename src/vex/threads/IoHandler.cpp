/*
 * IoHandler.cpp
 *
 *  Created on: 21 Apr 2011
 *      Author: root
 */

#include "IoHandler.h"

IoHandler::IoHandler() {
	stackDepth = 0;				// account stack depth of instrumented methods
	ioInvocationPointHashValue = 0;
	timesIoPredictionWasAddedToEstimatedTime = 1;
	lastIoPrediction = 0;
	stackTraceHash = 0;
	ioFinishedBeforeLogging = false;
	ignoringIo = false;
	recognizingIoAsCached = false;
}

IoHandler::~IoHandler() {

}
