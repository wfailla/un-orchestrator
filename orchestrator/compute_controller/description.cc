#include "description.h"

bool operator==(const nf_port_info& lhs, const nf_port_info& rhs)
{
    return (lhs.port_name.compare(rhs.port_name) == 0) && (lhs.port_type == rhs.port_type);
}

Description::Description(nf_t type, string uri, string cores, string location, std::map<unsigned int, PortType>& port_types) :
	type(type), uri(uri), cores(cores), location(location), port_types(port_types)
{
}

Description::Description(string type, string uri, string cores, string location, std::map<unsigned int, PortType>& port_types) :
	 uri(uri), cores(cores), location(location), port_types(port_types)
{

	if(type == "dpdk")
	{
		this->type = DPDK;
		return;
	}
#ifdef ENABLE_DOCKER
	else if(type == "docker")
	{
		this->type = DOCKER;
		return;
	}
#endif	
#ifdef ENABLE_KVM
	else if(type == "kvm")
	{
		this->type = KVM;
		return;
	}
#endif	

	//[+] Add here other implementations for the execution environment

	assert(0);
	return;
}

nf_t Description::getType() const
{
	return type;
}

string Description::getURI() const
{
	return uri;
}

string Description::getCores() const
{
	return cores;
}

string Description::getLocation() const
{
	assert(type == DPDK
#ifdef ENABLE_KVM
	|| type == KVM
#endif
	);

	return location;
}

PortType Description::getPortType(unsigned int port_id)
{
	std::map<unsigned int, PortType>::iterator it = port_types.find(port_id);
	if (it != port_types.end()) {
		return it->second;
	}
	return UNDEFINED_PORT;  // TODO: Should we make this INVALID_PORT to notify an error? Question is also: do we make the port specification in the NF description mandatory?
}

PortType portTypeFromString(const std::string& s)
{
	if (s.compare("ivshmem") == 0)
		return IVSHMEM_PORT;
	else if (s.compare("usvhost") == 0)
		return USVHOST_PORT;
	else if (s.compare("vhost") == 0)
		return VHOST_PORT;

	return UNDEFINED_PORT;
}

std::string portTypeToString(PortType t)
{
	switch (t) {
	case IVSHMEM_PORT:
		return "ivshmem";
		break;
	case USVHOST_PORT:
		return "usvhost";
		break;
	case VHOST_PORT:
		return "vhost";
		break;
	case UNDEFINED_PORT:
		return "undefined";
		break;
	default:
		break;
	}
	return "UNKNOWN";
}
