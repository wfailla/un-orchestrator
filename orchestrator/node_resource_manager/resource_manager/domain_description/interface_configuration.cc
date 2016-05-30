#include "interface_configuration.h"

namespace domainInformations
{
    InterfaceConfiguration::InterfaceConfiguration()
    {

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
    }

    void InterfaceConfiguration::setDescription(string description)
    {
        this->description = description;
    }

    void InterfaceConfiguration::setEnabled(bool enabled)
    {
        this->enabled = enabled;
    }
}
