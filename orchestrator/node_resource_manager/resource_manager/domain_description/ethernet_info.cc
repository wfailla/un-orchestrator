#include "ethernet_info.h"

namespace domainInformations
{
    EthernetInfo::EthernetInfo()
    {

    }

    void EthernetInfo::addNeighborInfo(NeighborInfo neighbor)
    {
        neighbors.push_back(neighbor);
    }

    void EthernetInfo::setMacAddress(string macAddress)
    {
        this->macAddress = macAddress;
    }

    void EthernetInfo::setVlanInfo(VlanInfo *vlan)
    {
       this->vlan=vlan;
    }

}
