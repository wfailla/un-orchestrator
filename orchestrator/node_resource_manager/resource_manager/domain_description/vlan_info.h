#ifndef NODE_ORCHESTRATOR_ETHERNET_VLAN_INFO_H
#define NODE_ORCHESTRATOR_ETHERNET_VLAN_INFO_H

#include <string>
#include <list>
#include "../keywords.h"

using namespace std;

namespace domainInformations
{

    class VlanInfo
    {
    public:
        typedef enum {TRUNK_MODE,ACCESS_MODE} portType;

        VlanInfo();

        void setVlanRange(int start, int end);
        void addUsedVlan(int number);
        void addFreeVlan(string vlanString);
        /**
        *	@brief: set the portType
        */
        void setPortType(string type);
    private:
        /**
        *	@brief: type of port
        */
        portType interfaceMode;
        /**
        *	@brief: vlan lower bound
        */
        int lowerVlanNumber;
        /**
        *	@brief: vlan upper bound
        */
        int upperVlanNumber;
        /**
        *	@brief: list of used vlan numbers
        */
        list<int> usedVlans;
    };

}


#endif //NODE_ORCHESTRATOR_ETHERNET_VLAN_INFO_H
