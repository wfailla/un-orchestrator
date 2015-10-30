#ifndef NATIVE_DESCRIPTION_H_
#define NATIVE_DESCRIPTION_H_ 1

#include "../description.h"

class NativeDescription : public Description {

private:
	std::string location;
	std::string dependencies;

public:
	NativeDescription(nf_t type, std::string uri, std::string dependencies, std::string location);

	json_spirit::Object toJSON();
};


#endif //NATIVE_DESCRIPTION_H_
