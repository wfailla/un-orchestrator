#ifndef NODE_ORCHESTRATOR_INTERFACE_H
#define NODE_ORCHESTRATOR_INTERFACE_H

#include <string>
#include <list>

#include "interface_configuration.h"
#include "interface_state.h"
#include "interface_capabilities.h"
#include "interface_options.h"
#include "ethernet_info.h"

using namespace std;


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
        InterfaceConfiguration *configuration;
        /**
        *	@brief: state of the interface
        */
        InterfaceState *state;
        /**
        *	@brief: state of the interface
        */
        InterfaceCapabilities *capabilities;
        /**
        *	@brief: interface options
        */
        InterfaceOptions *options;
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
        EthernetInfo *ethernetInfo;

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
    };

}

#endif //NODE_ORCHESTRATOR_INTERFACE_H
