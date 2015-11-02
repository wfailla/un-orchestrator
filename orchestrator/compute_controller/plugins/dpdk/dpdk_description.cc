#include "dpdk_description.h"

DPDKDescription::~DPDKDescription(){}

DPDKDescription::DPDKDescription(nf_t type, std::string uri, std::string cores, std::string location) :
		Description(type, uri), cores(cores), location(location) {}

DPDKDescription::DPDKDescription(std::string type, std::string uri, std::string cores, std::string location) :
		Description(type, uri), cores(cores), location(location) {}

std::string DPDKDescription::getCores() {
	return cores;
}

std::string DPDKDescription::getLocation() {
	return location;
}
