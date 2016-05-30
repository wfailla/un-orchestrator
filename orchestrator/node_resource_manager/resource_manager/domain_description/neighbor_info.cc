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

}