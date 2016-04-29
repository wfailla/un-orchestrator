#ifndef HIGH_LEVEL_ACTION_NF_H_
#define HIGH_LEVEL_ACTION_NF_H_ 1

#include "high_level_output_action.h"
#include "../../../utils/logger.h"

#include <iostream>
#include <sstream>

using namespace std;

namespace highlevel
{

class ActionNetworkFunction : public Action
{
private:
	/**
	*	@brief: the name of the NF (e.g., firewall)
	*/
	string nfName;

	/**
	*	@brief: the name of the endpoint port (e.g., vnf:00000001:inout:0)
	*/
	string endpointPortName;

	/**
	*	@brief: the port of the NF (e.g., 1)
	*/
	unsigned int port;

public:

	ActionNetworkFunction(string nfName, string endpointPortName, unsigned int port = 1);
	string getInfo();
	unsigned int getPort();
	string toString();

	bool operator==(const ActionNetworkFunction &other) const;

	Object toJSON();
};

}

#endif //HIGH_LEVEL_ACTION_NF_H_
