#include "network_manager.h"

namespace domainInformations
{

    NetworkManager::NetworkManager()
    {

    };

    void NetworkManager::addInterface(Interface interface)
    {
        interfaces.push_back(interface);
    }

    Object NetworkManager::toJSON()
    {
        Object openconfig_if_interfaces, openconfig_if_interface;
        Array interface_array;
        for(list<Interface>::iterator iter = interfaces.begin() ; iter != interfaces.end() ; iter++)
            interface_array.push_back(iter->toJSON());
		openconfig_if_interface[OPENCONFIG_IF_INTERFACE] = interface_array;
        openconfig_if_interfaces[OPENCONFIG_IF_INTERFACES] = openconfig_if_interface;
		return openconfig_if_interfaces;
    }

}
