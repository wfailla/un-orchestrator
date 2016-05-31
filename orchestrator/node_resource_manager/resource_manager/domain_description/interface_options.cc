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

	Object InterfaceOptions::toJSON()
	{
		Object options;
		if(localIP!="")
			options[OPT_LOCAL_IP]=localIP;
		if(remoteIP!="")
			options[OPT_REMOTE_IP]=remoteIP;
		if(key!="")
			options[OPT_KEY]=key;
		return options;
	}

}
