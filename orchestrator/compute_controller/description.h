#ifndef DESCRIPTION_H_
#define DESCRIPTION_H_ 1

#include "nf_type.h"
#include "nfs_manager.h"

#pragma once

#include "../utils/logger.h"
#include "../utils/constants.h"

#include <string>
#include <vector>
#include <assert.h>

using namespace std;

class NFsManager;

enum PortType {
	INVALID_PORT = -1,
	UNDEFINED_PORT = 0,
	//Ports used for virtual machines
	USVHOST_PORT,			//user space vhost port
	IVSHMEM_PORT,			//ivshmem port
	VHOST_PORT,				//(in kernel) vhost port
	//Ports used fro Docker containers 
	VETH_PORT,				//veth pair port
	//Ports used for DPDK processes executed in the host
	DPDKR_PORT				//dpdkr port
};

PortType portTypeFromString(const std::string& s);
std::string portTypeToString(PortType t);

struct nf_port_info
{
	string port_name;
	PortType port_type;
};
bool operator==(const nf_port_info& lhs, const nf_port_info& rhs);

class Description
{
friend NFsManager;

public:

private:
	nf_t type;
	string uri;
	//The next attribute are meningful only for DPDK VNFs
	//FIXME: this is bad.. The same description should be valid for all the NFs. Then, it is up to the proper
	//plugin to decide wheter an information has to be used or not.
	string cores;
	string location;
	std::map<unsigned int, PortType> port_types;
	
public:
	Description(nf_t type, string uri, string cores, string location, std::map<unsigned int, PortType>& port_types);
	Description(string type, string uri, string cores, string location, std::map<unsigned int, PortType>& port_types);
	
	string getURI() const;
	string getLocation() const;
	nf_t getType() const;
	const std::map<unsigned int, PortType>& getPortTypes() const { return port_types; }
	PortType getPortType(unsigned int port_id) const;
	
protected:
	string getCores() const;
};

#endif //DESCRIPTION_H_
