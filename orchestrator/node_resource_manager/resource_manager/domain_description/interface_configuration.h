#ifndef NODE_ORCHESTRATOR_INTERFACE_CONFIGURATION_H
#define NODE_ORCHESTRATOR_INTERFACE_CONFIGURATION_H

#include <string>
#include <json_spirit/json_spirit.h>
#include <json_spirit/value.h>
#include <json_spirit/writer.h>
#include "../keywords.h"

using namespace std;
using namespace json_spirit;

namespace domainInformations
{

    class InterfaceConfiguration
    {
    private:
        /**
        *	@brief: interface name (e.g. "eth1")
        */
        string name;
        /**
        *	@brief: interface type (e.g. "ethernetCsmacd")
        */
        string type;
        /**
        *	@brief:
        */
        bool unnumbered;
        /**
        *	@brief: interface description (e.g. "something")
        */
        string description;
        /**
        *	@brief: describe if the interface in enabled
        */
        bool enabled;
		/**
        *	@brief: describe if the enabled variable makes sense (to decide to print or not in toJSON function)
        */
		bool isEnabledDefined ;
		/**
        *	@brief: describe if the enabled variable makes sense (to decide to print or not in toJSON function)
        */
		bool isUnnumberedDefined ;
    public:

        InterfaceConfiguration();
        /**
        *	@brief: set the name of the interface
        */
        void setName(string name);
        /**
        *	@brief: set the type of the interface
        */
        void setType(string type);
        /**
        *	@brief:
        */
        void setUnnumbered(bool unnumbered);
        /**
        *	@brief: set the description of the interface
        */
        void setDescription(string description);
        /**
        *	@brief: set if the interface is enabled
        */
        void setEnabled(bool enabled);
		Object toJSON();

    };

}

#endif //NODE_ORCHESTRATOR_INTERFACE_CONFIGURATION_H