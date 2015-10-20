#include "native_description.h"

NativeDescription::~NativeDescription(){
	delete requirements;
}

NativeDescription::NativeDescription(nf_t type, std::string uri, std::string cores, std::string location, std::list<std::string>* requirements)
		: Description(type, uri, cores, location), requirements(requirements){}



NativeDescription::NativeDescription(std::string type, std::string uri, std::string cores, std::string location, std::list<std::string>* requirements)
		: Description(type, uri, cores, location), requirements(requirements){}

std::list<std::string>* NativeDescription::getRequirements(){
	return requirements;
}
