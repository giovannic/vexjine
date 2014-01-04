/*
 * JmtProbabilitiesParser.cpp
 *
 *  Created on: 24 May 2011
 *      Author: root
 */

#include "JmtProbabilitiesParser.h"

using namespace std;

JmtProbabilitiesParser::JmtProbabilitiesParser(TiXmlNode *_baseNode, std::map<std::string, NodeInformation> *_allNodes, const size_t &_totalTargets) {
	baseNode = _baseNode;
	allNodes = _allNodes;
	totalTargets = _totalTargets;

	nodes = new CinqsNode *[totalTargets];
	probs = new double[totalTargets];

}

ProbabilisticBranch *JmtProbabilitiesParser::parseProbabilisticBranch() {
	TiXmlNode *node = getLowerLevelNodeWith(baseNode, "section", "className" , "Router");
	node = getLowerLevelNodeWith(node, "parameter", "name" , "RoutingStrategy");
	node = getLowerLevelNodeWith(node, "subParameter", "name" , "Probabilities");
	node = getLowerLevelNodeWith(node, "subParameter", "name" , "EmpiricalEntryArray");

	if (node == NULL) {
		return NULL;
	}
	node = node->FirstChild("subParameter");
	int count = 0;
	while (node != NULL) {
		if (getTargetNodeWithProbability(node, count)) {
			++count;
		}
		node = node->NextSibling("subParameter");
	}

//	cout << "Multi-link to ";
//	for (int i = 0 ; i< count ;i++) {
//		cout << nodes[i]->getName() << " (" << probs[i] << ") and ";
//	}
//	cout << endl;
	return new ProbabilisticBranch(probs, nodes);
}


CinqsNode **JmtProbabilitiesParser::getNodes() {
	return nodes;
}

double *JmtProbabilitiesParser::getProbabilities() {
	return probs;
}

bool JmtProbabilitiesParser::getTargetNodeWithProbability(TiXmlNode *node, const int &count) {
	TiXmlNode *nameNode = getLowerLevelNodeWith(node, "subParameter", "name" , "stationName");
	if (nameNode != NULL) {
		nameNode = nameNode ->  FirstChild("value");
		TiXmlElement *element = nameNode->ToElement();
		map<string, NodeInformation>::iterator nodesIt = allNodes->find(element->GetText());
		nodes[count] = (nodesIt->second).getNode();


		TiXmlNode *probNode = getLowerLevelNodeWith(node, "subParameter", "name" , "probability");
		if (probNode != NULL) {
			probNode = probNode ->  FirstChild("value");
			element = probNode->ToElement();
			probs[count] = atof(element->GetText());
			return true;
		}
	}
	return false;
}

JmtProbabilitiesParser::~JmtProbabilitiesParser() {

}
