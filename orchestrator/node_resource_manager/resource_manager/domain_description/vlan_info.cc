#include "vlan_info.h"

namespace domainInformations
{
     VlanInfo::VlanInfo()
    {

    }

    void VlanInfo::addFreeVlan(int number)
    {
        freeVlans.push_back(number);
    }

    void VlanInfo::addFreeVlanRange(string range)
    {
		freeVlansRange.push_back(range);
    }

    void VlanInfo::setPortType(string type)
    {
        interfaceMode=type;
    }

    Object VlanInfo::toJSON()
    {
        /*
          		{
                "openconfig-vlan:config": {
                  "interface-mode": "TRUNK",
                  "trunk-vlans": [
                    "1..4090"
                  ]
                }
              }
        */
        Object vlanConfig, vlanConfigContent;
        vlanConfigContent[IF_MODE] = interfaceMode;
		if(freeVlans.size()!=0 || freeVlansRange.size()!=0)
		{
			Array trunks_vlan_array;
			if(freeVlansRange.size()!=0)
				for(list<string>::iterator iter = freeVlansRange.begin(); iter!=freeVlansRange.end(); iter++)
					trunks_vlan_array.push_back(*iter);
			if(freeVlans.size()!=0)
				for(list<int>::iterator iter = freeVlans.begin(); iter!=freeVlans.end(); iter++)
					trunks_vlan_array.push_back(*iter);
			vlanConfigContent[TRUNK_VLANS]=trunks_vlan_array;
		}
        vlanConfig[VLAN_CONFIG] = vlanConfigContent;
        return vlanConfig;
    }

}