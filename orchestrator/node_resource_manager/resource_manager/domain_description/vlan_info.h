#ifndef NODE_ORCHESTRATOR_ETHERNET_VLAN_INFO_H
#define NODE_ORCHESTRATOR_ETHERNET_VLAN_INFO_H

#include <string>
#include <list>
#include <json_spirit/json_spirit.h>
#include <json_spirit/value.h>
#include <json_spirit/writer.h>

#include "../keywords.h"
using namespace std;
using namespace json_spirit;

namespace domainInformations
{

    class VlanInfo
    {
    public:
        VlanInfo();

        void setVlanRange(int start, int end);
        void addUsedVlan(int number);
        void addFreeVlan(string vlanString);
        /**
        *	@brief: set the portType
        */
        void setPortType(string type);
		Object toJSON();

	private:
        /**
        *	@brief: type of interface (e.g. "TRUNK")
        */
        string interfaceMode;
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
