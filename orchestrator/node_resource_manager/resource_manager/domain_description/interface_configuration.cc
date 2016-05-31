#include "interface_configuration.h"

namespace domainInformations
{
    InterfaceConfiguration::InterfaceConfiguration()
    {
		isEnabledDefined = false;
		isUnnumberedDefined = false;
    }

    void InterfaceConfiguration::setName(string name)
    {
        this->name = name;
    }

    void InterfaceConfiguration::setType(string type)
    {
        this->type = type;
    }

    void InterfaceConfiguration::setUnnumbered(bool unnumbered)
    {
        this->unnumbered = unnumbered;
		isUnnumberedDefined=true;
    }

    void InterfaceConfiguration::setDescription(string description)
    {
        this->description = description;
    }

    void InterfaceConfiguration::setEnabled(bool enabled)
    {
        this->enabled = enabled;
		isEnabledDefined=true;
    }

	Object InterfaceConfiguration::toJSON()
	{
		Object configuration;
		if(isUnnumberedDefined)
			configuration[UNNUMBERED]=unnumbered;
		if(name!="")
			configuration[NAME]=name;
		if(type!="")
			configuration[TYPE]=type;
		if(description!="")
			configuration[DESCRIPTION]=description;
		if(isEnabledDefined)
			configuration[ENABLED]=enabled;
		return configuration;

	}

}
