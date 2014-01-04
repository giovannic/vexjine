/*
 * JmtParser.h
 *
 *  Created on: 23 May 2011
 *      Author: root
 */

#ifndef JMTPARSER_H_
#define JMTPARSER_H_

#include "tinyxml/tinyxml.h"
#include "cinqs.h"
#include <string>

class JmtParser {
public:
	JmtParser();
	virtual ~JmtParser();

protected:

	TiXmlNode *baseNode;
	TiXmlElement* element;
	TiXmlNode *getLowerLevelNodeWith(TiXmlNode *originalNode, const string &tagName, const string &elementType, const string &elementValue);
	TiXmlNode *getSameLevelNodeWith(TiXmlNode *originalNode, const string &tagName, const std::string &elementType, const std::string &elementValue);
	DistributionSampler *getDistribution(TiXmlNode *node);
	double getDistributionParameter(TiXmlNode *node, const std::string &parameterName);
	DistributionSampler *getServiceTimeDistribution(TiXmlNode *node);
	bool isResourceConsumptionLocal(TiXmlNode *sectionNode);

};


class NodeInformation {
public:
	NodeInformation(CinqsNode *_node, TiXmlNode *_jmtNode) : node(_node), jmtNode(_jmtNode) {};
	CinqsNode *getNode() {
		return node;
	}

	TiXmlNode *getJmtNode() {
		return jmtNode;
	}
private:
	CinqsNode *node;
	TiXmlNode *jmtNode;
};

#endif /* JMTPARSER_H_ */
