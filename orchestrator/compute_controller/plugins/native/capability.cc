#include "capability.h"

Capability::Capability(std::string name, std::string path, captype_t type)
	: name(name), path(path), type(type) {}

Capability::~Capability() {}

std::string Capability::getName(){
	return name;
}

std::string Capability::getPath(){
	return path;
}
