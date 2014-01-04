/*
 * JmtNodeParser.h
 *
 *  Created on: 23 May 2011
 *      Author: root
 */

#ifndef JMTNODEPARSER_H_
#define JMTNODEPARSER_H_

#include "tinyxml/tinyxml.h"
#include "cinqs.h"
#include "JmtParser.h"

class JmtNodeParser : public JmtParser {
public:
	JmtNodeParser(TiXmlNode *_baseNode, const bool &_useOriginalCinqsDataStructures);
	JmtNodeParser(TiXmlNode *_baseNode, const bool &_useOriginalCinqsDataStructures, const int &_customerClasses);
	CinqsNode *extractCinqsNodeTo(Network *network);
	virtual ~JmtNodeParser();

protected:
	bool isServer();
	bool isDelay();
	bool isRoutingStation();
	Delay *getDelay(TiXmlNode *originalNode);

	int getServers(TiXmlNode *node);
	CinqsNode *nodeFactory(Network *network, TiXmlNode *node, const string &nodeType, const string &nodeName);

	bool useOriginalCinqsDataStructures;
	int customerClasses;
	int *customerClassesIds;
};

#endif /* JMTNODEPARSER_H_ */
