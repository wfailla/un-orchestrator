#include "interface_options.h"

namespace domainInformations
{
    InterfaceOptions::InterfaceOptions()
    {

    }

    void InterfaceOptions::setGreLocalIP(string localIP)
    {
        this->localIP=localIP;
    }

    void InterfaceOptions::setGreRemoteIP(string remoteIP)
    {
        this->remoteIP=remoteIP;
    }

    void InterfaceOptions::setGreKey(string key)
    {
        this->key=key;
    }

}
