#ifndef HIGH_LEVEL_GRAPH_ENPOINT_INTERFACE_OUT_H_
#define HIGH_LEVEL_GRAPH_ENPOINT_INTERFACE_OUT_H_ 1

#include "../../../utils/logger.h"
#include "../../../utils/constants.h"

#include <iostream>
#include <sstream>

#include <string.h>

#include <json_spirit/json_spirit.h>
#include <json_spirit/value.h>
#include <json_spirit/writer.h>

using namespace json_spirit;

using namespace std;

namespace highlevel
{

class EndPointInterfaceOut
{
private:
	/**
	*	@brief: the id of the endpoint port (e.g., 00000003)
	*/
	string id;

	/**
	*	@brief: the name of the endpoint port (e.g., ingress)
	*/
	string name;

	/**
	*	@brief: the id of the port (e.g., 10.0.0.1)
	*/
	string node_id;

	/**
	*	@brief: the switch id of the port (e.g., 1)
	*/
	string sw_id;

	/**
	*	@brief: the interface of the port (e.g., eth0)
	*/
	string interface;

public:

	EndPointInterfaceOut(string id, string name, string node_id, string sw_id, string interface);
	string getId();
	string getName();
	string getNodeId();
	string getSwId();
	string getInterface();

	~EndPointInterfaceOut();

	bool operator==(const EndPointInterfaceOut &other) const;

	void print();
	Object toJSON();
};

}

#endif //HIGH_LEVEL_GRAPH_ENPOINT_INTERFACE_OUT_H_
