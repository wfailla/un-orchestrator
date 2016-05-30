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
        if(type==TRUNK)
            interfaceMode=TRUNK_MODE;
        else
            interfaceMode=ACCESS_MODE;
    }

    void VlanInfo::addFreeVlan(string vlanString)
    {
        //TODO: write this function
    }

}