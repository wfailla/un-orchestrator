#include "vlan_info.h"

namespace domainInformations
{
     VlanInfo::VlanInfo()
    {

    }

    void VlanInfo::addUsedVlan(int number)
    {
        usedVlans.push_back(number);
    }

    void VlanInfo::setVlanRange(int start, int end)
    {
        lowerVlanNumber=start;
        upperVlanNumber=end;
    }

    void VlanInfo::setPortType(string type)
    {
        interfaceMode=type;
    }

    void VlanInfo::addFreeVlan(string vlanString)
    {
        //TODO: write this function
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
        vlanConfig[VLAN_CONFIG] = vlanConfigContent;
        return vlanConfig;
    }

}