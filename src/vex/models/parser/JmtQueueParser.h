/*
 * JmtQueueParser.h
 *
 *  Created on: 23 May 2011
 *      Author: root
 */

#ifndef JMTQUEUEPARSER_H_
#define JMTQUEUEPARSER_H_

#include "tinyxml/tinyxml.h"
#include "cinqs.h"
#include "JmtParser.h"

#include <string>

class JmtQueueParser : public JmtParser {
public:
	JmtQueueParser(TiXmlNode *_baseNode);

	Queue *getCinqsQueue();

	virtual ~JmtQueueParser();

protected:
	int getSize();

};

#endif /* JMTQUEUEPARSER_H_ */
