#ifndef NODE_ORCHESTRATOR_ETHERNET_INFO_H
#define NODE_ORCHESTRATOR_ETHERNET_INFO_H

#include <string>
#include <list>

#include "vlan_info.h"
#include "neighbor_info.h"

using namespace std;

namespace domainInformations
{

    class EthernetInfo
    {
    private:
        /**
        *	@brief: mac-address of the ethernet interface
        */
        string macAddress;
        /**
        *	@brief: vlan information
        */
        VlanInfo *vlan;
        /**
        *	@brief: neighbors information
        */
        list<NeighborInfo> neighbors;

    public:

        EthernetInfo();
        void setMacAddress(string macAddress);
        void setVlanInfo(VlanInfo *vlan);
        void addNeighborInfo(NeighborInfo neighbor);
    };

}

#endif //NODE_ORCHESTRATOR_ETHERNET_INFO_H
