/*
 * JmtParser.cpp
 *
 *  Created on: 23 May 2011
 *      Author: root
 */

#include "JmtParser.h"

JmtParser::JmtParser() {
	

}

JmtParser::~JmtParser() {

}

TiXmlNode *JmtParser::getLowerLevelNodeWith(TiXmlNode *originalNode, const string &tagName, const string &elementType, const string &elementValue) {
	if (originalNode == NULL) {
		return NULL;
	}
	TiXmlNode *node = originalNode->FirstChild(tagName.c_str());
	return getSameLevelNodeWith(node, tagName, elementType, elementValue);
}

TiXmlNode *JmtParser::getSameLevelNodeWith(TiXmlNode *originalNode, const string &tagName, const string &elementType, const string &elementValue) {
	if (originalNode == NULL) {
		return NULL;
	}
	TiXmlNode *node = originalNode;
	while (node != NULL) {
		element = node -> ToElement();
		const char *elementValueForType = element->Attribute(elementType.c_str());
		if (elementValueForType != NULL && elementValue.compare(elementValueForType) == 0) {
			return node;
		}
		node = node -> NextSibling(tagName.c_str());
	}

	return NULL;
}


double JmtParser::getDistributionParameter(TiXmlNode *node, const string &parameterName) {
	TiXmlElement* element = 0;

	node = node->FirstChild("subParameter");
	element = node->ToElement();
	while (parameterName.compare(element->Attribute("name")) != 0) {
//		cout << "itan " << parameterName << " " << element->Attribute("name") << endl;
		node = node->NextSibling("subParameter");
		if (node == 0) {
			cerr << "Parameter not found!" << endl;
			return 0;
		}
		element = node->ToElement();
	}

	node = node->FirstChild("value");
	element = node->ToElement();

//cout << atof(element->GetText()) << endl;

	return atof(element->GetText());

}


DistributionSampler *JmtParser::getDistribution(TiXmlNode *node) {

	node = node->FirstChild("subParameter");
	if (node == NULL) {
		return NULL;
	}
	TiXmlElement* element = 0;
	element = node->ToElement();
	string distributionType = element->Attribute("name");

	string distrParLiteral = "distrPar";
	do {
		node = node->NextSibling("subParameter");
		element = node->ToElement();
//		cout << "found " << element->Attribute("name") << endl;
	} while (distrParLiteral.compare(element->Attribute("name")) != 0);

	if (distributionType == "Exponential") {
		return new Exp(getDistributionParameter(node, "lambda"));

	} else if (distributionType == "Normal") {
		return new Normal(getDistributionParameter(node, "mean"), getDistributionParameter(node, "standardDeviation"));

	}
	//TODO: FIX: the other distributions

	return NULL;
}



DistributionSampler *JmtParser::getServiceTimeDistribution(TiXmlNode *originalNode) {
	TiXmlNode *parameterNode = getLowerLevelNodeWith(originalNode, "parameter", "name", "ServiceStrategy");
	TiXmlNode *subparametersNode = getLowerLevelNodeWith(parameterNode, "subParameter", "name", "ServiceTimeStrategy");
	if (subparametersNode != NULL) {
		return getDistribution(subparametersNode);
	}
	return new Exp(1/0.005);
}


bool JmtParser::isResourceConsumptionLocal(TiXmlNode *sectionNode) {
	TiXmlNode *parameterNode = getLowerLevelNodeWith(sectionNode, "parameter", "name", "ServiceStrategy");
	TiXmlElement *element = parameterNode -> ToElement();
	if (element != NULL) {
		const char *resourceConsumption = element->Attribute("resourceConsumption");
		if (resourceConsumption != NULL && strcmp(resourceConsumption, "Local")==0) {
			return true;
		}
	}
	return false;
}
