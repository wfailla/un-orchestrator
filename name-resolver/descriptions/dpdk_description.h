#ifndef DPDK_DESCRIPTION_H_
#define DPDK_DESCRIPTION_H_ 1

#include "../description.h"

class DPDKDescription : public Description {

private:
	std::string cores;
	std::string location;

public:
	DPDKDescription(nf_t type, std::string uri, std::string cores, std::string location);

	json_spirit::Object toJSON();
};


#endif //DPDK_DESCRIPTION_H_
