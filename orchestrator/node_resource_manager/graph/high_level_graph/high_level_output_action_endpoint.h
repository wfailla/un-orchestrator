#ifndef HIGH_LEVEL_ACTION_ENDPOINT_H_
#define HIGH_LEVEL_ACTION_ENDPOINT_H_ 1

#include "high_level_output_action.h"
#include "../../../utils/logger.h"

#include <iostream>
#include <sstream>

using namespace std;

namespace highlevel
{

class ActionEndPoint : public Action
{
private:

	/**
	*	@brief: endpoint identifier
	*/
	unsigned int endpoint;

	/**
	*	@brief: the name of the endpoint (e.g., endpoint:00000001)
	*/
	string input_endpoint;

public:

	ActionEndPoint(unsigned int endpoint, string input_endpoint);
	string getInfo();
	unsigned int getPort();
	string getInputEndpoint();
	string toString();

	bool operator==(const ActionEndPoint &other) const;

	void print();
	Object toJSON();
};

}

#endif //HIGH_LEVEL_ACTION_ENDPOINT_H_
