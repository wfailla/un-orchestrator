#ifndef HIGH_LEVEL_ACTION_ENDPOIN_INTERNAL_H_
#define HIGH_LEVEL_ACTION_ENDPOINT_INTERNAL_H_ 1

#include "high_level_output_action.h"
#include "../../../utils/logger.h"

#include <iostream>
#include <sstream>
#include <string.h>

using namespace std;

namespace highlevel
{

class ActionEndPointInternal : public Action
{
private:
	/**
	*	@brief: ID of the graph defining this endpoint
	*/
	string graphID;
	
	/**
	*	@brief: endpoint identifier
	*/
	unsigned int endpoint;
	
	/**
	*	@brief: the name of the endpoint (e.g., endpoint:00000001)
	*/
	string input_endpoint;

public:

	ActionEndPointInternal(string graphID, unsigned int endpoint, string input_endpoint);
	string getInfo();
	unsigned int getPort();
	string getInputEndpoint();
	string toString();
	
	bool operator==(const ActionEndPointInternal &other) const;
	
	void print();
	Object toJSON();
};

}

#endif //HIGH_LEVEL_ACTION_ENDPOINT_INTERNAL_H_
