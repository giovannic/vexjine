/*
 * IoProtocolClient.h
 *
 *  Created on: 13 Oct 2010
 *      Author: root
 */

#ifndef IOPROTOCOLCLIENT_H_
#define IOPROTOCOLCLIENT_H_

#include "IoProtocol.h"
#include "distributed/ThreadManagerClient.h"


class IoProtocolSerialClient: public IoProtocol {
public:
	IoProtocolSerialClient();
	void onStart(VexThreadState *state);
	void onEnd(VexThreadState *state, const long long &actualIoDuration);
};


class IoProtocolStrictClient: public IoProtocol {
public:
	IoProtocolStrictClient();
	void onStart(VexThreadState *state);
	void onEnd(VexThreadState *state, const long long &actualIoDuration);
};

class IoProtocolLaxClient: public IoProtocol {
public:
	IoProtocolLaxClient();
	void onStart(VexThreadState *state);
	void onEnd(VexThreadState *state, const long long &actualIoDuration);
};

class IoProtocolNormalClient : public IoProtocol {
public:
	IoProtocolNormalClient();
	void onStart(VexThreadState *state);
	void onEnd(VexThreadState *state, const long long &actualIoDuration);
};
#endif /* IOPROTOCOLCLIENT_H_ */
