#include "domain_description.h"

namespace domainInformations
{

    DomainDescription::DomainDescription()
    {

    };

    void DomainDescription::setName(string name)
    {
        this->name = name;
    };

    void DomainDescription::setType(string type)
    {
        this->type = type;
    };

    void DomainDescription::setNetworkManager(NetworkManager *networkManager)
    {
        this->networkManager = networkManager;
    };

    void DomainDescription::setManagementAddress(string managementAddress)
    {
        this->managementAddress = managementAddress;
    };

}
