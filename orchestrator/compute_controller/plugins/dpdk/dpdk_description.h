#ifndef DPDK_DESCRIPTION_H_
#define DPDK_DESCRIPTION_H_ 1

#include "../../description.h"

class DPDKDescription : public Description {
private:
	std::string cores;
	std::string location;
public:
	std::string getLocation();
	std::string getCores();
	DPDKDescription(nf_t type, std::string uri, std::string cores, std::string location);
	DPDKDescription(std::string type, std::string uri, std::string cores, std::string location);
	~DPDKDescription();
};

#endif //DPDK_DESCRIPTION_H_
