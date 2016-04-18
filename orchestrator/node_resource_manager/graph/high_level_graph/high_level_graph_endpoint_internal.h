#ifndef HIGH_LEVEL_GRAPH_ENPOINT_INTERNAL_H_
#define HIGH_LEVEL_GRAPH_ENPOINT_INTERNAL_H_ 1

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

class EndPointInternal
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

public:

	EndPointInternal(string id, string name);
	string getId();
	string getName();

	~EndPointInternal();

	bool operator==(const EndPointInternal &other) const;

	void print();
	Object toJSON();
};

}

#endif //HIGH_LEVEL_GRAPH_ENPOINT_INTERNAL_H_
