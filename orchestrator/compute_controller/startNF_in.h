#ifndef StartNFIn_H_
#define StartNFIn_ 1

#pragma once

#include <string>
#include <map>
#include <inttypes.h>
#include <list>

/**
* @file StartNFIn_in.h
*
* @brief Parameters to be used to start the network function.
*/

using namespace std;

class StartNFIn
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
	*	@brief: mask of the cores to be assigned to the network functon.
	*			0x0 means that no binding has to be done
	*/
	uint64_t coreMask;

protected:
	StartNFIn(uint64_t lsiID, string nf_name, map<unsigned int, string> namesOfPortsOnTheSwitch, uint64_t coreMask = 0x0)
		: lsiID(lsiID), nf_name(nf_name), namesOfPortsOnTheSwitch(namesOfPortsOnTheSwitch), coreMask(coreMask)
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
	
	uint64_t getCoreMask() const
	{
		return coreMask;
	}
};


#endif //StartNFIn_H_
