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
}
