/*
 * JmtProbabilitiesParser.h
 *
 *  Created on: 24 May 2011
 *      Author: root
 */

#ifndef JMTPROBABILITIESPARSER_H_
#define JMTPROBABILITIESPARSER_H_

#include "tinyxml/tinyxml.h"
#include "cinqs.h"
#include "JmtParser.h"

#include <map>
#include <string>

class JmtProbabilitiesParser : public JmtParser {
public:
	JmtProbabilitiesParser(TiXmlNode *_baseNode, std::map<std::string, NodeInformation> *_allNodes, const size_t &_totalTargets);
	ProbabilisticBranch *parseProbabilisticBranch();

	virtual ~JmtProbabilitiesParser();
	double *getProbabilities();
	CinqsNode **getNodes();

private:
	bool getTargetNodeWithProbability(TiXmlNode *node, const int &count);

	std::map<std::string, NodeInformation> *allNodes;
	CinqsNode **nodes;
	size_t totalTargets;
	double *probs;
};

#endif /* JMTPROBABILITIESPARSER_H_ */
