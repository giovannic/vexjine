/*
 * JmtNetworkParser.h
 *
 *  Created on: 25 May 2011
 *      Author: root
 */

#ifndef JMTNETWORKPARSER_H_
#define JMTNETWORKPARSER_H_

#include "tinyxml/tinyxml.h"
#include "cinqs.h"
#include "JmtParser.h"
#include "VexQueueingNetwork.h"

class JmtNetworkParser {
public:
	JmtNetworkParser();
	virtual ~JmtNetworkParser();

	static VexQueueingNetwork *parseNetwork(const char *filename, const bool &useOriginalCinqsDataStructures, const bool &useSchedulerSimNetwork);
};

#endif /* JMTNETWORKPARSER_H_ */
