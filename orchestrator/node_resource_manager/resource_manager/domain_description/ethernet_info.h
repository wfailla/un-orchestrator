#ifndef NODE_ORCHESTRATOR_ETHERNET_INFO_H
#define NODE_ORCHESTRATOR_ETHERNET_INFO_H

#include <string>
#include <list>
#include <json_spirit/json_spirit.h>
#include <json_spirit/value.h>
#include <json_spirit/writer.h>
#include "vlan_info.h"
#include "neighbor_info.h"
#include "../keywords.h"

using namespace std;
using namespace json_spirit;

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
        VlanInfo *vlan = NULL;
        /**
        *	@brief: neighbors information
        */
        list<NeighborInfo> neighbors;

    public:

        EthernetInfo();
        void setMacAddress(string macAddress);
        void setVlanInfo(VlanInfo *vlan);
        void addNeighborInfo(NeighborInfo neighbor);
		Object toJSON();

	};

}

#endif //NODE_ORCHESTRATOR_ETHERNET_INFO_H
