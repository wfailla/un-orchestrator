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
	*	@brief: parameters related to the endpoint [key, local_ip, remote_ip, interface]
	*/
	vector<string> ep_param;
	
	/*
	*	@brief: value that indicates if endpoint is safe (true) or unsafe (false) 
	*/
	bool is_safe;

protected:
	AddEndpointIn(uint64_t dpid, string ep_name, vector<string> ep_param, bool is_safe = false)
		: dpid(dpid), ep_name(ep_name), ep_param(ep_param), is_safe(is_safe)
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
	
	vector<string> getEPparam()
	{
		return ep_param;
	}
	
	bool isSafe()
	{
		return is_safe;
	}
	
};


#endif //AddEndpointIn_H_
