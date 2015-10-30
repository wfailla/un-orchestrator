#include "dpdk_description.h"

DPDKDescription::DPDKDescription(nf_t type, string uri, string cores, string location)
	: Description(type, uri), cores(cores), location(location){}

Object DPDKDescription::toJSON(){
	Object descr;

	descr["uri"]  = uri;
	descr["type"]  = "dpdk";
	descr["cores"] = cores;
	descr["location"] = location;

	return descr;
}
