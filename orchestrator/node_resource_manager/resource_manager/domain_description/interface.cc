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

	Object Interface::toJSON()
	{
		Object interface, openconfig_if_subinterfaces, openconfig_if_subinterface;
		Array subinterface_array, gre_if_array;
		if(name!="")
			interface[NAME]=name;
		if(type!="")
			interface[IF_TYPE]=type;
		if(configuration!=NULL)
			interface[IF_CONFIG]=configuration->toJSON();
		if(state!=NULL)
			interface[IF_STATE]=state->toJSON();
		if(capabilities!=NULL)
			interface[IF_CAPABILITIES]=capabilities->toJSON();
		if(options!=NULL)
			interface[OPTIONS]=options->toJSON();
		if(ethernetInfo!=NULL)
			interface[OPENCONFIG_IF_ETHERNET]=ethernetInfo->toJSON();
		if(subinterfaces.size()!=0)
		{
			for(list<Interface>::iterator iter = subinterfaces.begin(); iter != subinterfaces.end(); iter++)
				subinterface_array.push_back(iter->toJSON());
			openconfig_if_subinterface[OPENCONFIG_IF_SUBINTERFACE] = subinterface_array;
			interface[OPENCONFIG_IF_SUBINTERFACES] = openconfig_if_subinterface;
		}
		if(greInterfaces.size()!=0)
		{
			for(list<Interface>::iterator iter = greInterfaces.begin(); iter != greInterfaces.end(); iter++)
				gre_if_array.push_back(iter->toJSON());
			interface[IF_GRE] = gre_if_array;
		}
		return interface;
	}


}
