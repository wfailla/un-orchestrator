#ifndef NODE_ORCHESTRATOR_DOMAIN_INFORMATIONS_H
#define NODE_ORCHESTRATOR_DOMAIN_INFORMATIONS_H

#include "network_manager.h"
#include <string>

using namespace std;

namespace domainInformations
{

    class DomainDescription
    {
    private:
        /**
        *	@brief: domain name (e.g. "universal node")
        */
        string name;
        /**
        *	@brief: domain type (e.g. "un")
        */
        string type;
        /**
        *	@brief: management address of the domain (e.g. "10.0.0.1:8080")
        */
        string managementAddress;
        /**
        *	@brief: network manager of the domain
        */
        NetworkManager *networkManager;

    public:

        DomainDescription();

        /**
        *	@brief: set the name of the domain
        */
        void setName(string name);

        /**
        *	@brief: set the type of the domain
        */
        void setType(string type);
        /**
        *	@brief: set the network manager of the domain
        */
        void setNetworkManager(NetworkManager *networkManager);
        /**
        *	@brief: set the management address
        */
        void setManagementAddress(string managementAddress);
    };

}

#endif //NODE_ORCHESTRATOR_DOMAIN_INFORMATIONS_H
