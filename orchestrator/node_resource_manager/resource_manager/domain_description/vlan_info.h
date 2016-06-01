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

        void addFreeVlan(int number);
		void addFreeVlanRange(string range);
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
        *	@brief: list of free vlan numbers expressed as range (e.g "20..30")
        */
		list<string> freeVlansRange;
        /**
        *	@brief: list of free vlan numbers
        */
        list<int> freeVlans;

    };

}


#endif //NODE_ORCHESTRATOR_ETHERNET_VLAN_INFO_H
