#include "neighbor_info.h"

namespace domainInformations
{
    NeighborInfo::NeighborInfo()
    {

    }

    void NeighborInfo::setDomainName(string name)
    {
        domainName=name;
    }

    void NeighborInfo::setRemoteInterface(string remoteInterface)
    {
        this->remoteInterface = remoteInterface;
    }

    void NeighborInfo::setType(string domainType)
    {
        this->domainType=domainType;
    }

	Object NeighborInfo::toJSON()
	{
		Object neighborInfo;
		if(domainName!="")
			neighborInfo[NEIGHBOR_NAME]=domainName;
		if(domainType!="")
			neighborInfo[NEIGHBOR_TYPE]=domainType;
		if(remoteInterface!="")
			neighborInfo[NEIGHBOR_INTERFACE]=remoteInterface;
		return neighborInfo;
	}

}