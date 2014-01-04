/*
 * JmtNodeParser.cpp
 *
 *  Created on: 23 May 2011
 *      Author: root
 */

#include "JmtNodeParser.h"
#include "JmtQueueParser.h"
#include "VexInfiniteServerNode.h"
#include "VexLocalInfiniteServerNode.h"
#include "VexQueueingNode.h"
#include "VexLocalQueueingNode.h"

#include <climits>

JmtNodeParser::JmtNodeParser(TiXmlNode *_baseNode, const bool &_useOriginalCinqsDataStructures) {
	baseNode = _baseNode;
	useOriginalCinqsDataStructures = _useOriginalCinqsDataStructures;
	customerClasses = 1;
	customerClassesIds = NULL;
}


JmtNodeParser::JmtNodeParser(TiXmlNode *_baseNode, const bool &_useOriginalCinqsDataStructures, const int &_customerClasses) {
	baseNode = _baseNode;
	useOriginalCinqsDataStructures = _useOriginalCinqsDataStructures;
	customerClasses = _customerClasses;
	if (_customerClasses > 1) {
		customerClassesIds = new int[_customerClasses];
		for (int i = 0; i<_customerClasses; i++) {
			customerClassesIds[i] = i;
		}
	} else {
		customerClassesIds = NULL;
	}
}


int JmtNodeParser::getServers(TiXmlNode *node) {
	return 1;
}

CinqsNode *JmtNodeParser::nodeFactory(Network *network, TiXmlNode *node, const string &nodeType, const string &nodeName) {
	if (nodeType.compare("Queue") == 0) {
		if (isDelay()) {
			TiXmlNode *node = getLowerLevelNodeWith(baseNode, "section", "className", "Delay");
			if (useOriginalCinqsDataStructures) {
				return new InfiniteServerNode(network, nodeName, getDelay(node));
			} else {
				if (isResourceConsumptionLocal(node)) {
					return new VexLocalInfiniteServerNode(network, nodeName, getDelay(node));
				} else {
//					cout << "PARSING DISTRIBUTION OF " << nodeName << endl;
					return new VexInfiniteServerNode(network, nodeName, getDelay(node));
				}
			}
		} else if (isRoutingStation()) {
//			cout << "routing station " << nodeName << endl;
			JmtQueueParser parseQueue(node);
//			TiXmlNode *node = getLowerLevelNodeWith(baseNode, "section", "className", "Router");
//			return new ResourcePool(network, nodeName, new Exp(100000000000.0), INT_MAX/2, parseQueue.getCinqsQueue());
			// Use a local queue with <1ns avg service time to simulate routing stations: the local queue will not lead to any waiting
			return new VexLocalQueueingNode(network, nodeName, new Delay(new Normal(0.00000000001, 0.0)), INT_MAX/2, parseQueue.getCinqsQueue());
		} else {
//			cout << "queueing node " << nodeName << endl;
			JmtQueueParser parseQueue(node);
			TiXmlNode *node = getLowerLevelNodeWith(baseNode, "section", "className", "Server");
			if (useOriginalCinqsDataStructures) {
				return new QueueingNode(network, nodeName, getDelay(node), getServers(node), parseQueue.getCinqsQueue());
			} else {
				if (isResourceConsumptionLocal(node)) {
					return new VexLocalQueueingNode(network, nodeName, getDelay(node), getServers(node), parseQueue.getCinqsQueue());
				} else {
					return new VexQueueingNode(network, nodeName, getDelay(node), getServers(node), parseQueue.getCinqsQueue());
				}
			}
		}
	} else 	if (nodeType.find("Source") != string::npos) {
		return new Source(network, nodeName, getServiceTimeDistribution(node));

	} else if (nodeType.find("Sink") != string::npos) {
		return new Sink(network, nodeName);

	}

	return NULL;
};

CinqsNode *JmtNodeParser::extractCinqsNodeTo(Network *network) {
	element = baseNode->ToElement();
	string nodeName = element->Attribute("name");

	TiXmlNode *node = baseNode->FirstChild("section");
	element = node->ToElement();

	return nodeFactory(network, node, element->Attribute("className") , nodeName);
}



bool JmtNodeParser::isServer() {
	TiXmlNode *node = getLowerLevelNodeWith(baseNode, "section", "className", "Server");
	if (node != NULL) {
		return true;
	}
	return false;
}

bool JmtNodeParser::isDelay() {

	TiXmlNode *node = getLowerLevelNodeWith(baseNode, "section", "className", "Delay");
	if (node != NULL) {
		return true;
	}
	return false;
}

bool JmtNodeParser::isRoutingStation() {

	TiXmlNode *node = getLowerLevelNodeWith(baseNode, "section", "className", "Router");
	if (node != NULL) {
		return !isServer();	// a routing station has a Router section, but no delay section
	}
	return false;
}


/**
 * The method finds the number of classes and then the delay for each class
 */
Delay *JmtNodeParser::getDelay(TiXmlNode *originalNode) {
	if (customerClasses == 1) {
		return new Delay(getServiceTimeDistribution(originalNode));
	} else {
		TiXmlNode *parameterNode = getLowerLevelNodeWith(originalNode, "parameter", "name", "ServiceStrategy");
		TiXmlNode *subparametersNode = getLowerLevelNodeWith(parameterNode, "subParameter", "name", "ServiceTimeStrategy");
		DistributionSampler **distributionSamplers = new DistributionSampler*[customerClasses];
		int classId = 0;
		while (subparametersNode != NULL) {
			distributionSamplers[classId++] = getDistribution(subparametersNode);
			subparametersNode = subparametersNode->NextSibling("subParameter");
		}

		bool allAreSame = true;
		for (int i = 0; i<customerClasses; i++) {
			for (int j = i+1; j<customerClasses; j++) {

				if(strlen(distributionSamplers[i]->getDistributionSampleType()) != strlen(distributionSamplers[j]->getDistributionSampleType()) || strcmp(distributionSamplers[i]->getDistributionSampleType(), distributionSamplers[j]->getDistributionSampleType()) != 0) {
					allAreSame = false;
					break;
				}
			}
		}

//
		if (allAreSame) {
			DistributionSampler *sampler = distributionSamplers[0];
			for (int i = 1; i<customerClasses; i++) {
				delete distributionSamplers[i];
			}
			delete[] distributionSamplers;
//			cout << "Delay " << sampler->getDistributionSampleType() << " -- " << sampler << endl;
			return new Delay(sampler);
		} else {
//			for (int i = 0; i<customerClasses; i++) {
//					cout << "Class dependent delay for class " << i << " " << distributionSamplers[i]->getDistributionSampleType() << " -- " << distributionSamplers[i] << endl;
//			}
			return new ClassDependentDelay(customerClassesIds, distributionSamplers, customerClasses);
		}

//		return new Exp(1/0.005);
//
//		new ClassDependentDelay()
//		TiXmlNode *parameterNode = getLowerLevelNodeWith(originalNode, "parameter", "name", "ServiceStrategy");

	}

}


JmtNodeParser::~JmtNodeParser() {

}
