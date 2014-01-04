/*
 * JmtNetworkParser.cpp
 *
 *  Created on: 25 May 2011
 *      Author: root
 */

#include "JmtNetworkParser.h"

#include <iostream>
#include <map>
using namespace std;

#include "JmtNodeParser.h"
#include "JmtProbabilitiesParser.h"
#include "VexQueueingNetwork.h"
#include "VexSchedSimQueueingNetwork.h"

JmtNetworkParser::JmtNetworkParser() {


}

VexQueueingNetwork *JmtNetworkParser::parseNetwork(const char *modelFileName, const bool &useOriginalCinqsDataStructures, const bool &useSchedulerSimNetwork) {

	TiXmlDocument doc( modelFileName );
	bool loadOkay = doc.LoadFile();

	if ( !loadOkay ) {
		printf( "Could not load performance model file '%s'. Error='%s'. Exiting.\n", modelFileName, doc.ErrorDesc() );
		return NULL;
	}

	TiXmlNode* node = 0;

	VexQueueingNetwork *network;
	if (useSchedulerSimNetwork) {
		network = new VexSchedSimQueueingNetwork();
	} else {
		network = new VexQueueingNetwork();
	}

	node = doc.FirstChild( "archive" );
	if (node == NULL) {
		cerr << "Error: Performance model file '" << modelFileName << "' is not in the correct format" << endl;
		return NULL;
	}
	node = node->FirstChild("sim");
	if (node == NULL) {
		cerr << "Error: Performance model file '" << modelFileName << "' is not in the correct format" << endl;
		return NULL;
	}

	TiXmlNode *classNode = node;

	int classes = 0;
	classNode = classNode->FirstChild("userClass");
	while (classNode != NULL) {
		++classes;
		classNode = classNode->NextSibling("userClass");
	}

	TiXmlNode *simNode = node;
	node = node->FirstChild("node");
	map<string, NodeInformation> nodeNamesToNodes;

	do {
		JmtNodeParser parser(node, useOriginalCinqsDataStructures, classes);
		CinqsNode *newNode = parser.extractCinqsNodeTo(network);
		nodeNamesToNodes.insert(make_pair(newNode->getName(), NodeInformation(newNode, node)));
	} while ((node = node->NextSibling("node")) != 0);


	map<string, NodeInformation>::iterator mapIt = nodeNamesToNodes.begin();
	map<string, NodeInformation>::iterator targetNodeFinder;
	while (mapIt != nodeNamesToNodes.end()) {
		CinqsNode *n = ((NodeInformation)mapIt ->second).getNode();

//		cout << n->getName() << ": " << n->getNodeType() << endl;
		string nodeName = n->getName();

		CinqsNode *targetNode = NULL;
		node = simNode->FirstChild("connection");
		size_t targetNodesTotal = 0;
		while (node != NULL) {
			TiXmlElement *element = node->ToElement();

			if (nodeName.compare(element->Attribute("source")) == 0) {
				const char *target = element->Attribute("target");
				targetNodeFinder = nodeNamesToNodes.find(target);
				targetNode = targetNodeFinder->second.getNode();
				++targetNodesTotal;
			}
			node = node->NextSibling("connection");
		}

		if (targetNodesTotal == 1) {
			n -> setLink(new Link(targetNode));

		} else if (targetNodesTotal != 0){

			TiXmlNode *jmtNode = ((NodeInformation)mapIt->second).getJmtNode();
			JmtProbabilitiesParser probabilitiesParser(jmtNode, &nodeNamesToNodes, targetNodesTotal);

			n -> setLink(probabilitiesParser.parseProbabilisticBranch());
		}
		++mapIt;
	}

	return network;
}


JmtNetworkParser::~JmtNetworkParser() {

}
