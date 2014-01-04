/*
 * MethodOperations.cpp
 *
 *  Created on: 12 Oct 2010
 *      Author: root
 */

#include "MethodOperations.h"

MethodOperations::MethodOperations() {
	
}


MethodOperations::~MethodOperations() {
	 
}

Calculations::Calculations(float _duration) {
	duration = _duration;
	temp = 0;
}

Calculations::~Calculations() {

}
IoOperation::IoOperation(float _duration, int _methodId, int _invocationPointHashValue) {
	duration = _duration;
	methodId = _methodId;
	invocationPointHashValue = _invocationPointHashValue;
}

IoOperation::~IoOperation() {

}

WaitOperation::WaitOperation(float _duration) {
	duration = _duration;
}

WaitOperation::~WaitOperation() {

}
