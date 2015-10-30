#include "./native_description.h"

NativeDescription::NativeDescription(nf_t type, std::string uri, std::string dependencies, std::string location) :
	Description(type, uri), location(location), dependencies(dependencies) {}

Object NativeDescription::toJSON() {
	Object descr;

	descr["uri"]  = uri;
	descr["type"]  = "native";
	descr["dependencies"] = dependencies;
	descr["location"] = location;

	return descr;
}
