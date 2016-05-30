#ifndef NODE_ORCHESTRATOR_INTERFACE_CONFIGURATION_H
#define NODE_ORCHESTRATOR_INTERFACE_CONFIGURATION_H

#include <string>

using namespace std;

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

    };

}

#endif //NODE_ORCHESTRATOR_INTERFACE_CONFIGURATION_H