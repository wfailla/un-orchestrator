#ifndef AddNFportsOut_H_
#define AddNFportsOut_ 1

#pragma once

#include <string>
#include <list>

/**
* @file addNFports_out.h
*
* @brief Description of network function ports destroyed.
*/

using namespace std;

class AddNFportsOut
{

friend class GraphManager;

private:

	/**
	*	@brief: name of the network functions whose ports have been connected to the lsi
	*/
	string nf_name;

	/**
	*	@brief: map of ports name, identifier within the lsi
	*/
	map<string, unsigned int> ports;
	
	/**
	*	@brief: list of ports on the vSwitch that are associated with the network function
	*/
	list<string> ports_name_on_switch;
	
protected:

	string getNFname()
	{
		return nf_name;
	}

	map<string, unsigned int> getPorts()
	{
		return ports;
	}
	
	list<string> getPortsNameOnSwitch()
	{
		return ports_name_on_switch;
	}

public:
	AddNFportsOut(string nf_name,map<string, unsigned int> ports, list<string> ports_name_on_switch) 
		: nf_name(nf_name), ports(ports.begin(),ports.end()), ports_name_on_switch(ports_name_on_switch.begin(),ports_name_on_switch.end())
	{
	}
	
};


#endif //AddNFportsOut_H_
