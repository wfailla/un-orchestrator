#include "native_description.h"

NativeDescription::~NativeDescription(){
	delete requirements;
}

NativeDescription::NativeDescription(nf_t type, std::string uri, std::string location, std::list<std::string>* requirements)
		: Description(type, uri), location(location), requirements(requirements){}



NativeDescription::NativeDescription(std::string type, std::string uri, std::string location, std::list<std::string>* requirements)
		: Description(type, uri), location(location), requirements(requirements){}

std::list<std::string>* NativeDescription::getRequirements(){
	return requirements;
}

std::string NativeDescription::getLocation() {
	return location;
}
