#ifndef AddNFportsIn_H_
#define AddNFportsIn_ 1

#pragma once

#include <string>
#include <list>
#include <inttypes.h>

#include "../../compute_controller/description.h"

/**
* @file addNFports_in.h
*
* @brief Description of network function ports to be created.
*/

using namespace std;

class AddNFportsIn
{

friend class GraphManager;

private:

	/**
	*	@brief: identifier of the lsi to which the network function ports must be connected
	*/
	uint64_t dpid;
	
	/**
	*	@brief: name of the network functions whose ports must be connected to the lsi
	*/
	string nf_name;
	
	/**
	*	@brief: type of the network function to be connected to the lsi
	*/
	nf_t type;
	
	/**
	*	@brief: list of ports of the network function to be attached to the lsi
	*/
	list<struct nf_port_info> nf_ports;

protected:
	AddNFportsIn(uint64_t dpid, string nf_name, nf_t type, list<struct nf_port_info> nf_ports)
		: dpid(dpid), nf_name(nf_name), type(type), nf_ports(nf_ports)
	{
	}
	
public:
	
	uint64_t getDpid()
	{
		return dpid;
	}
	
	string getNFname()
	{
		return nf_name;
	}
	
	nf_t getNFtype()
	{
		return type;
	}
	
	list<struct nf_port_info> getNetworkFunctionsPorts()
	{
		return nf_ports;
	}
	
};


#endif //AddNFportsIn_H_
