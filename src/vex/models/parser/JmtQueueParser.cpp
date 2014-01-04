/*
 * JmtQueueParser.cpp
 *
 *  Created on: 23 May 2011
 *      Author: root
 */

#include "JmtQueueParser.h"


JmtQueueParser::JmtQueueParser(TiXmlNode *_baseNode) {
	baseNode = _baseNode;
	baseNode = getSameLevelNodeWith(baseNode, "section", "className", "Queue");
}


JmtQueueParser::~JmtQueueParser() {

}


Queue *JmtQueueParser::getCinqsQueue() {
	if (baseNode != NULL) {
		int queueSize = getSize();
		if (getSameLevelNodeWith(baseNode, "parameter", "name", "FCFSstrategy") != NULL) {
			if (queueSize > 0) {
				return new FIFOQueue(queueSize);
			} else {
				return new FIFOQueue();
			}
		}
	}
	return new FIFOQueue();
}


int JmtQueueParser::getSize() {
	TiXmlNode *sizeNode = getSameLevelNodeWith(baseNode, "parameter", "name", "size");
	if (sizeNode != NULL) {
		TiXmlNode *node = sizeNode->FirstChild("value");
		if (node != NULL) {
			element = node->ToElement();
			return atoi(element->GetText());
		} else {
			cerr << "could not find value tag under size" << endl;
			return -1;
		}
	}

	return -1;
}



