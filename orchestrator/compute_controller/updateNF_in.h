#ifndef UpdateNFIn_H_
#define UpdateNFIn_ 1

#pragma once

#include <string>
#include <map>
#include <inttypes.h>
#include <list>

#include "../node_resource_manager/graph/high_level_graph/nf_port_configuration.h"

/**
* @file UpdateNFIn_in.h
*
* @brief Parameters to be used to update the network function.
*/

using namespace std;

class UpdateNFIn
{

friend class ComputeController;

private:

	/**
	*	@brief: identifier of the LSI to which the VNF is connected
	*/
	uint64_t lsiID;

	/**
	*	@brief: name of the network function
	*/
	string nf_name;

	/**
	*	@brief: mapping of port_id to name of port on the vSwitch for ports associated with the network function
	*/
	map<unsigned int, string> namesOfPortsOnTheSwitch;

	/**
	*	@brief: mapping of port_id to configuration (mac_address, ip_address) associated with the network function
	*/
	map<unsigned int, port_network_config_t > portsConfiguration;

	/**
        *       @brief: list of the ports that needs to be added
        */
	list<unsigned int> newPortsToAdd;

protected:
	UpdateNFIn(uint64_t lsiID, string nf_name, map<unsigned int, string> namesOfPortsOnTheSwitch, map<unsigned int, port_network_config_t > portsConfiguration, list<unsigned int> newPortsToAdd)
			: lsiID(lsiID), nf_name(nf_name), namesOfPortsOnTheSwitch(namesOfPortsOnTheSwitch), portsConfiguration(portsConfiguration), newPortsToAdd(newPortsToAdd)
	{
	}

public:

	uint64_t getLsiID() const
	{
		return lsiID;
	}

	string getNfName() const
	{
		return nf_name;
	}

	const map<unsigned int, string>& getNamesOfPortsOnTheSwitch() const
	{
		return namesOfPortsOnTheSwitch;
	}

	const map<unsigned int, port_network_config_t >& getPortsConfiguration() const
	{
		return portsConfiguration;
	}

	const list<unsigned int>& getNewPortsToAdd() const
	{
		return newPortsToAdd;
	}
};


#endif //UpdateNFIn_H_
