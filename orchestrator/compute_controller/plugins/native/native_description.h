#ifndef NATIVE_DESCRIPTION_H_
#define NATIVE_DESCRIPTION_H_ 1

#include "../../description.h"
#include <list>

class NativeDescription : public Description {
private:
	std::list<std::string> *requirements;
public:
	std::list<std::string>* getRequirements();
	NativeDescription(nf_t type, std::string uri, std::string cores, std::string location, std::list<std::string>* requirements);
	NativeDescription(std::string type, std::string uri, std::string cores, std::string location, std::list<std::string>* requirements);
	~NativeDescription();
};

#endif //NATIVE_DESCRIPTION_H_
