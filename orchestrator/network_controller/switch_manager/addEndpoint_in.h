#ifndef AddEndpointIn_H_
#define AddEndpointIn_ 1

#pragma once

#include <string>
#include <list>
#include <inttypes.h>

/**
* @file addEndpoint_in.h
*
* @brief Description of endpoint to be created.
*/

using namespace std;

class AddEndpointIn
{

friend class GraphManager;

private:

	/**
	*	@brief: identifier of the lsi to which the endpoint must be connected
	*/
	uint64_t dpid;
	
	/**
	*	@brief: name of the endpoint that must be connected to the lsi
	*/
	string ep_name;
	
	/**
	*	@brief: pair of information related by the endpoint
	*/
	pair<string, string> ep_iface;

protected:
	AddEndpointIn(uint64_t dpid, string ep_name, pair<string, string> ep_iface)
		: dpid(dpid), ep_name(ep_name), ep_iface(ep_iface)
	{
	}
	
public:
	
	uint64_t getDpid()
	{
		return dpid;
	}
	
	string getEPname()
	{
		return ep_name;
	}
	
	pair<string, string> getEPiface()
	{
		return ep_iface;
	}
	
};


#endif //AddEndpointIn_H_
