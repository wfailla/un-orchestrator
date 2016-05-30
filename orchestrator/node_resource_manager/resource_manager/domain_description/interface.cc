#include "interface.h"

namespace domainInformations
{

    Interface::Interface()
    {
        this->type = type;
    };

    void Interface::setName(string name)
    {
        this->name = name;
    }

    void Interface::setType(string type)
    {
        this->type = type;
    }

    void Interface::setInterfaceConfiguration(InterfaceConfiguration *configuration)
    {
        this->configuration = configuration;
    }

    void Interface::setInterfaceState(InterfaceState *state)
    {
        this->state = state;
    }

    void Interface::addSubinterface(Interface subinterface)
    {
        subinterfaces.push_back(subinterface);
    }

    void Interface::setEthernetInfo(EthernetInfo *ethernetInfo)
    {
        this->ethernetInfo=ethernetInfo;
    }

    void Interface::setInterfaceCapabilities(InterfaceCapabilities *capabilities)
    {
        this->capabilities=capabilities;
    }

    void Interface::addGreInterface(Interface interfaceGre)
    {
        greInterfaces.push_back(interfaceGre);
    }

    void Interface::setInterfaceOptions(InterfaceOptions *options)
    {
        this->options=options;
    }


}
