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

	Object EthernetInfo::toJSON()
	{
		Object ethernetInfo,mac_address_obj;
		Array neighbor_array;
		if(macAddress!="")
		{
			mac_address_obj[MAC_ADDRESS]=macAddress;
			ethernetInfo[ETHERNET_CONFIG]=mac_address_obj;
		}
		if(vlan!=NULL)
			ethernetInfo[OPENCONFIG_VLAN]=vlan->toJSON();
		if(neighbors.size()!=0)
		{
			for(list<NeighborInfo>::iterator iter = neighbors.begin(); iter != neighbors.end(); iter++)
				neighbor_array.push_back(iter->toJSON());
			ethernetInfo[NEIGHBOR]=neighbor_array;
		}
		return ethernetInfo;

	}


}
