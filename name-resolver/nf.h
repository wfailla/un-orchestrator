#ifndef NF_H_
#define NF_H_ 1

#include <string>
#include <assert.h>
#include <list>

#include <json_spirit/json_spirit.h>
#include <json_spirit/value.h>
#include <json_spirit/writer.h>

#include "constants.h"

#include "description.h"
#include "descriptions/native_description.h"
#include "descriptions/dpdk_description.h"

using namespace std;
using namespace json_spirit;

class NF
{
private:
	/**
	*	@brief: name of the NF
	*/
	string name;
	
	/**
	*	@brief: number of ports of the NF
	*/
	int nports;
	
	/**
	*	@brief: text describing the network function
	*/
	string description;
		
	/**
	*	@brief: list of possible implementations for the network function
	*/
	list<Description*> descriptions;
	
public:
	NF(string name, int nports, string description);
	void addImplementation(Description *description);
	
	string getName();
	
	Object toJSON();
};

#endif //NF_H_
