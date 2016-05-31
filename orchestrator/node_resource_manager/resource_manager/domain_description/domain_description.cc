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

	Object DomainDescription::toJSON()
	{
		Object domainInfo, domainInfoContent;
		if(name!="")
			domainInfoContent[NAME]=name;
		if(type!="")
			domainInfoContent[TYPE]=type;
		if(managementAddress!="")
			domainInfoContent[MANAGEMENT_ADDRESS]=managementAddress;
		if(networkManager!=NULL)
			domainInfoContent[NETWORK_MANAGER_INFO]=networkManager->toJSON();
		domainInfo[DOMAIN_INFORMATIONS] = domainInfoContent;
		return domainInfo;
	}

}
