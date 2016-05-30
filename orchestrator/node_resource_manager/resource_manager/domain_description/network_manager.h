#ifndef NODE_ORCHESTRATOR_NETWORK_MANAGER_H
#define NODE_ORCHESTRATOR_NETWORK_MANAGER_H

#include <list>
#include "interface.h"

using namespace std;

namespace domainInformations
{

    class NetworkManager
    {
    private:
        /**
        *	@brief: list of interfaces
        */
        list<Interface> interfaces;
    public:
        NetworkManager();
        void addInterface(Interface interface);
    };

}


#endif //NODE_ORCHESTRATOR_NETWORK_MANAGER_H
