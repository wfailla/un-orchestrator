#include "interface_capabilities.h"

namespace domainInformations
{

    InterfaceCapabilities::InterfaceCapabilities()
    {
		isGreSupportDefined=false;
    };

    void InterfaceCapabilities::setGreSupport(bool greSupport)
    {
        this->greSupport=greSupport;
		isGreSupportDefined=true;
    }

    Object InterfaceCapabilities::toJSON()
    {
        Object capabilities;
		if(isGreSupportDefined)
			capabilities[GRE] = greSupport;
        return capabilities;
    }

}
