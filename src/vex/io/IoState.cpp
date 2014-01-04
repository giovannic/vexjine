/*
 * IoState.cpp
 *
 *  Created on: 24 Aug 2011
 *      Author: root
 */

#include "IoState.h"


#include <sstream>
IoState::IoState() {
	threadsInIo = 0;
	traceHash 	= 0;
	ioOperation = 0;
}

IoState::~IoState() {

}

void IoState::getIoPointFileSuffix(std::stringstream &str) {
	if (threadsInIo != 0) {
		str << "_" << (threadsInIo+1) << "thr";
	}
	if (traceHash != 0) {
		str << "_" << traceHash << "trc";
	} else {
		if (ioOperation != 0) {
			str << "_" << ioOperation << "iop";
		}
	}

}

