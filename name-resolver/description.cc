#include "description.h"

Description::Description(nf_t type, string uri) :
	type(type), uri(uri)
{
}

Object Description::toJSON()
{
	Object descr;

	descr["uri"]  = uri;
	descr["type"]  = (type == DPDK)? "dpdk" : ((type == DOCKER)? "docker" : ((type == KVM) ? "kvm" : "native"));

	return descr;
}
