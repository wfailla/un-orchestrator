#ifndef NODE_ORCHESTRATOR_INTERFACE_H
#define NODE_ORCHESTRATOR_INTERFACE_H

#include <string>
#include <list>
#include <json_spirit/json_spirit.h>
#include <json_spirit/value.h>
#include <json_spirit/writer.h>

#include "interface_configuration.h"
#include "interface_state.h"
#include "interface_capabilities.h"
#include "interface_options.h"
#include "ethernet_info.h"
#include "../keywords.h"

using namespace std;
using namespace json_spirit;

namespace domainInformations
{

    class Interface
    {
    private:
        /**
        *	@brief: interface name (e.g. "eth0")
        */
        string name;
        /**
        *	@brief: interface type (e.g. "core")
        */
        string type;
        /**
        *	@brief: configuration of the interface
        */
        InterfaceConfiguration *configuration = NULL;
        /**
        *	@brief: state of the interface
        */
        InterfaceState *state = NULL;
        /**
        *	@brief: state of the interface
        */
        InterfaceCapabilities *capabilities = NULL;
        /**
        *	@brief: interface options
        */
        InterfaceOptions *options = NULL;
        /**
        *	@brief: list of subinterfaces
        */
        list<Interface> subinterfaces;
        /**
        *	@brief: list of gre interfaces
        */
        list<Interface> greInterfaces;
        /**
        *	@brief: ethernet informations
        */
        EthernetInfo *ethernetInfo = NULL;

    public:

        Interface();
        /**
        *	@brief: set the name of the interface
        */
        void setName(string name);
        /**
        *	@brief: set the type of the interface
        */
        void setType(string type);
        /**
        *	@brief: set the configuration of the interface
        */
        void setInterfaceConfiguration(InterfaceConfiguration *configuration);
        /**
        *	@brief: set the state of the interface
        */
        void setInterfaceState(InterfaceState *state);
        /**
        *	@brief: set the capabilities of the interface
        */
        void setInterfaceCapabilities(InterfaceCapabilities *capabilities);
        /**
        *	@brief: set interface options
        */
        void setInterfaceOptions(InterfaceOptions *options);
        /**
        *	@brief: set the ethernet informations of the interface
        */
        void setEthernetInfo(EthernetInfo *ethernetInfo);
        /**
        *	@brief: add a subinterface to this interface
        */
        void addSubinterface(Interface subinterface);
        /**
        *	@brief: add a gre interface to this interface
        */
        void addGreInterface(Interface interfaceGre);

		Object toJSON();

	};

}

#endif //NODE_ORCHESTRATOR_INTERFACE_H
