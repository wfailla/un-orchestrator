#include "capability.h"

Capability::Capability(std::string name, std::string path) : name(name), path(path) {}

Capability::~Capability() {}

std::string Capability::getName(){
	return name;
}

std::string Capability::getPath(){
	return path;
}
