/*
 * VexLocalInfiniteServerNode.h
 *
 *  Created on: 28 Jun 2011
 *      Author: root
 */

#ifndef VEXLOCALINFINITESERVERNODE_H_
#define VEXLOCALINFINITESERVERNODE_H_

#include "VexInfiniteServerNode.h"

class VexLocalInfiniteServerNode : public VexInfiniteServerNode {

public:
	VexLocalInfiniteServerNode(Network *_network, const std::string &s, Delay *d) : VexInfiniteServerNode(_network, s, d) {};

	std::string getNodeType();
	~VexLocalInfiniteServerNode();

protected:
	void accept( Customer *c ) ;
};

#endif /* VEXLOCALINFINITESERVERNODE_H_ */
