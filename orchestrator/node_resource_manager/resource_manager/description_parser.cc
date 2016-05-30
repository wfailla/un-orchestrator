#include "description_parser.h"

bool DescriptionParser::parseDescription(std::string description, domainInformations::DomainDescription *domainDescription)
{
    Value initialValue;
    read(description, initialValue);
    try
    {
        Object rootObj = initialValue.getObject();

        Object::const_iterator rootObjIterator = rootObj.find(DOMAIN_INFORMATIONS);

        if(rootObjIterator==rootObj.end())
        {
            logger(ORCH_WARNING, MODULE_NAME, __FILE__, __LINE__, "Invalid Json, expected key: %s",DOMAIN_INFORMATIONS);
            return false;
        }

        Object domain_info;
        try
        {
            domain_info = (rootObjIterator->second).getObject();
        } catch(exception& e)
        {
            logger(ORCH_WARNING, MODULE_NAME, __FILE__, __LINE__, "The content does not respect the JSON syntax: \"%s\" should be an Object", DOMAIN_INFORMATIONS);
            return false;
        }
        for(Object::const_iterator domain_info_iter = domain_info.begin(); domain_info_iter != domain_info.end(); domain_info_iter++)
        {
            const string &domain_info_key = domain_info_iter->first;
            const Value &domain_info_value = domain_info_iter->second;

            if (domain_info_key == NAME) {
                domainDescription->setName(domain_info_value.getString());
            }
            else if (domain_info_key == TYPE) {
                domainDescription->setType(domain_info_value.getString());
            }
            else if (domain_info_key == MANAGEMENT_ADDRESS) {
                domainDescription->setManagementAddress(domain_info_value.getString());
            }
            else if (domain_info_key == NETWORK_MANAGER_INFO) {
                domainInformations::NetworkManager *networkManager = new domainInformations::NetworkManager();
                domainDescription->setNetworkManager(networkManager);

                Object network_manager_info;
                try {
                    network_manager_info = domain_info_value.getObject();
                } catch (exception &e) {
                    logger(ORCH_WARNING, MODULE_NAME, __FILE__, __LINE__, "The content does not respect the JSON syntax: \"%s\" should be an Object", NETWORK_MANAGER_INFO);
                    return false;
                }
                for (Object::const_iterator network_manager_info_iter = network_manager_info.begin(); network_manager_info_iter != network_manager_info.end(); network_manager_info_iter++) {
                    const string &network_manager_info_key = network_manager_info_iter->first;
                    const Value &network_manager_info_value = network_manager_info_iter->second;

                    if (network_manager_info_key == OPENCONFIG_IF_INTERFACES) {
                        Object openconfig_if_interfaces;
                        try {
                            openconfig_if_interfaces = network_manager_info_value.getObject();
                        } catch (exception &e) {
                            logger(ORCH_WARNING, MODULE_NAME, __FILE__, __LINE__, "The content does not respect the JSON syntax: \"%s\" should be an Object", OPENCONFIG_IF_INTERFACES);
                            return false;
                        }
                        for (Object::const_iterator openconfig_if_interfaces_iter = openconfig_if_interfaces.begin(); openconfig_if_interfaces_iter != openconfig_if_interfaces.end(); openconfig_if_interfaces_iter++) {
                            const string &openconfig_if_interfaces_key = openconfig_if_interfaces_iter->first;
                            const Value &openconfig_if_interfaces_value = openconfig_if_interfaces_iter->second;

                            if (openconfig_if_interfaces_key == OPENCONFIG_IF_INTERFACE) {
                                try {
                                    openconfig_if_interfaces_value.getArray();
                                } catch (exception &e) {
                                    logger(ORCH_WARNING, MODULE_NAME, __FILE__, __LINE__, "The content does not respect the JSON syntax: \"%s\" should be an Array", OPENCONFIG_IF_INTERFACE);
                                    return false;
                                }
                                const Array& openconfig_if_interface_array = openconfig_if_interfaces_value.getArray();
                                for( unsigned int i = 0; i < openconfig_if_interface_array.size(); i++ )
                                {
                                    Object openconfig_if_interface;
                                    try {
                                        openconfig_if_interface = openconfig_if_interface_array[i].getObject();
                                    } catch (exception &e) {
                                        logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "The content does not respect the JSON syntax: element of array \"%s\" should be an Objects", OPENCONFIG_IF_INTERFACE);
                                        return false;
                                    }
                                    domainInformations::Interface interface;
                                    parseInterface(openconfig_if_interface,interface);
                                    networkManager->addInterface(interface);
                                }
                            }
                        }
                    }
                }
            }
        }

    }
    catch(exception& e)
    {
        logger(ORCH_WARNING, MODULE_NAME, __FILE__, __LINE__, "The content does not respect the JSON syntax: %s",e.what());
        return false;
    }

    return true;
}

bool DescriptionParser::parseInterface(Object &openconfig_if_interface,domainInformations::Interface& interface)
{
    for (Object::const_iterator openconfig_if_interface_iter = openconfig_if_interface.begin(); openconfig_if_interface_iter != openconfig_if_interface.end(); openconfig_if_interface_iter++)
    {
        const string &openconfig_if_interface_key = openconfig_if_interface_iter->first;
        const Value &openconfig_if_interface_value = openconfig_if_interface_iter->second;

        if (openconfig_if_interface_key == NAME)
        {
            interface.setName(openconfig_if_interface_value.getString());
        }
        else if (openconfig_if_interface_key == IF_TYPE)
        {
            interface.setType(openconfig_if_interface_value.getString());
        }
        else if (openconfig_if_interface_key == IF_CONFIG)
        {
            Object if_config;
            try
            {
                if_config = openconfig_if_interface_value.getObject();
            } catch(exception& e)
            {
                logger(ORCH_WARNING, MODULE_NAME, __FILE__, __LINE__, "The content does not respect the JSON syntax: \"%s\" should be an Object", IF_CONFIG);
                return false;
            }
            domainInformations::InterfaceConfiguration *interfaceConfiguration = new domainInformations::InterfaceConfiguration();
            for(Object::const_iterator if_config_iter = if_config.begin(); if_config_iter != if_config.end(); if_config_iter++)
            {
                const string &if_config_key = if_config_iter->first;
                const Value &if_config_value = if_config_iter->second;

                if (if_config_key == NAME)
                {
                    interfaceConfiguration->setName(if_config_value.getString());
                }
                else if (if_config_key == UNNUMBERED)
                {
                    interfaceConfiguration->setUnnumbered(if_config_value.getBool());
                }
                else if (if_config_key == DESCRIPTION)
                {
                    interfaceConfiguration->setDescription(if_config_value.getString());
                }
                else if (if_config_key == ENABLED)
                {
                    interfaceConfiguration->setEnabled(if_config_value.getBool());
                }
            }
            interface.setInterfaceConfiguration(interfaceConfiguration);
        }
        else if (openconfig_if_interface_key == IF_STATE)
        {
            Object if_state;
            try
            {
                if_state = openconfig_if_interface_value.getObject();
            } catch(exception& e)
            {
                logger(ORCH_WARNING, MODULE_NAME, __FILE__, __LINE__, "The content does not respect the JSON syntax: \"%s\" should be an Object", IF_STATE);
                return false;
            }
            domainInformations::InterfaceState *interfaceState = new domainInformations::InterfaceState();
            for(Object::const_iterator if_state_iter = if_state.begin(); if_state_iter != if_state.end(); if_state_iter++)
            {
                const string &if_state_key = if_state_iter->first;
                const Value &if_state_value = if_state_iter->second;

                if (if_state_key == ADMIN_STATUS)
                {
                    interfaceState->setAdminStatus(if_state_value.getString());
                }
                else if (if_state_key == OPER_STATUS)
                {
                    interfaceState->setOperStatus(if_state_value.getString());
                }
            }
            interface.setInterfaceState(interfaceState);
        }
        else if (openconfig_if_interface_key == OPENCONFIG_IF_SUBINTERFACES)
        {
			try {
				openconfig_if_interface_value.getArray();
			} catch (exception &e) {
				logger(ORCH_WARNING, MODULE_NAME, __FILE__, __LINE__, "The content does not respect the JSON syntax: \"%s\" should be an Array", OPENCONFIG_IF_SUBINTERFACES);
				return false;
			}
			const Array& if_subinterface_array = openconfig_if_interface_value.getArray();
			for( unsigned int i = 0; i < if_subinterface_array.size(); i++ )
			{
				Object if_subinterface;
				try {
					if_subinterface = if_subinterface_array[i].getObject();
				} catch (exception &e) {
					logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "The content does not respect the JSON syntax: element of array \"%s\" should be an Objects", OPENCONFIG_IF_SUBINTERFACES);
					return false;
				}
				domainInformations::Interface subinterface;
				parseInterface(if_subinterface,subinterface);
				interface.addSubinterface(subinterface);
			}
        }
        else if (openconfig_if_interface_key == OPENCONFIG_IF_ETHERNET)
        {
            Object if_ethernet;
            try
            {
                if_ethernet = openconfig_if_interface_value.getObject();
            } catch(exception& e)
            {
                logger(ORCH_WARNING, MODULE_NAME, __FILE__, __LINE__, "The content does not respect the JSON syntax: \"%s\" should be an Object", OPENCONFIG_IF_ETHERNET);
                return false;
            }
            domainInformations::EthernetInfo *ethernetInfo = new domainInformations::EthernetInfo();
            for(Object::const_iterator if_ethernet_iter = if_ethernet.begin(); if_ethernet_iter != if_ethernet.end(); if_ethernet_iter++)
            {
                const string &if_ethernet_key = if_ethernet_iter->first;
                const Value &if_ethernet_value = if_ethernet_iter->second;

                if (if_ethernet_key == ETHERNET_CONFIG)
                {
                    Object ethernet_config;
                    try
                    {
                        ethernet_config = if_ethernet_value.getObject();
                    } catch(exception& e)
                    {
                        logger(ORCH_WARNING, MODULE_NAME, __FILE__, __LINE__, "The content does not respect the JSON syntax: \"%s\" should be an Object", ETHERNET_CONFIG);
                        return false;
                    }
                    for(Object::const_iterator ethernet_config_iter = ethernet_config.begin(); ethernet_config_iter != ethernet_config.end(); ethernet_config_iter++)
                    {
                        const string &ethernet_config_key = ethernet_config_iter->first;
                        const Value &ethernet_config_value = ethernet_config_iter->second;

                        if (ethernet_config_key == ETHERNET_CONFIG)
                        {
                            ethernetInfo->setMacAddress(ethernet_config_value.getString());
                        }
                    }
                }
                else if (if_ethernet_key == OPENCONFIG_VLAN)
                {
					Object vlan;
					try
					{
						vlan = if_ethernet_value.getObject();
					} catch(exception& e)
					{
						logger(ORCH_WARNING, MODULE_NAME, __FILE__, __LINE__, "The content does not respect the JSON syntax: \"%s\" should be an Object", OPENCONFIG_VLAN);
						return false;
					}
					domainInformations::VlanInfo *vlanInfo = new domainInformations::VlanInfo();
					for(Object::const_iterator vlan_iter = vlan.begin(); vlan_iter != vlan.end(); vlan_iter++)
					{
						const string &vlan_key = vlan_iter->first;
						const Value &vlan_value = vlan_iter->second;

						if (vlan_key == VLAN_CONFIG)
						{
							Object vlan_config;
							try
							{
								vlan_config = vlan_value.getObject();
							} catch(exception& e)
							{
								logger(ORCH_WARNING, MODULE_NAME, __FILE__, __LINE__, "The content does not respect the JSON syntax: \"%s\" should be an Object", VLAN_CONFIG);
								return false;
							}
							for(Object::const_iterator vlan_config_iter = vlan_config.begin(); vlan_config_iter != vlan_config.end(); vlan_config_iter++)
							{
								const string &vlan_config_key = vlan_config_iter->first;
								const Value &vlan_config_value = vlan_config_iter->second;

								if (vlan_config_key == IF_MODE)
								{
									vlanInfo->setPortType(vlan_config_value.getString());
								}
								else if (vlan_config_key == TRUNK_VLANS)
								{
									try {
										vlan_config_value.getArray();
									} catch (exception &e) {
										logger(ORCH_WARNING, MODULE_NAME, __FILE__, __LINE__, "The content does not respect the JSON syntax: \"%s\" should be an Array", TRUNK_VLANS);
										return false;
									}
									const Array& free_vlan_array = vlan_config_value.getArray();
									for( unsigned int i = 0; i < free_vlan_array.size(); i++ )
									{
										string free_vlan;
										try {
											free_vlan = free_vlan_array[i].getString();
										} catch (exception &e) {
											logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "The content does not respect the JSON syntax: element of array \"%s\" should be a String", TRUNK_VLANS);
											return false;
										}
										vlanInfo->addFreeVlan(free_vlan);
									}
								}
							}
						}
					}
					ethernetInfo->setVlanInfo(vlanInfo);
                }
                else if (if_ethernet_key == NEIGHBOR)
                {
                    try {
                        if_ethernet_value.getArray();
                    } catch (exception &e) {
                        logger(ORCH_WARNING, MODULE_NAME, __FILE__, __LINE__, "The content does not respect the JSON syntax: \"%s\" should be an Array", NEIGHBOR);
                        return false;
                    }
                    const Array& neighbor_array = if_ethernet_value.getArray();
                    for( unsigned int i = 0; i < neighbor_array.size(); i++ )
                    {
                        Object neighbor;
                        try {
                            neighbor = neighbor_array[i].getObject();
                        } catch (exception &e) {
                            logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "The content does not respect the JSON syntax: element of array \"%s\" should be an Objects", NEIGHBOR);
                            return false;
                        }
						domainInformations::NeighborInfo neighborInfo;
						for (Object::const_iterator neighbor_iter = neighbor.begin(); neighbor_iter != neighbor.end(); neighbor_iter++)
						{
							const string &neighbor_key = neighbor_iter->first;
							const Value &neighbor_value = neighbor_iter->second;

							if (neighbor_key == NEIGHBOR_NAME)
							{
								neighborInfo.setDomainName(neighbor_value.getString());
							}
							else if (neighbor_key == NEIGHBOR_INTERFACE)
							{
								neighborInfo.setRemoteInterface(neighbor_value.getString());
							}
						}
						ethernetInfo->addNeighborInfo(neighborInfo);
                    }
                }
            }
            interface.setEthernetInfo(ethernetInfo);
        }
        else if (openconfig_if_interface_key == IF_CAPABILITIES)
        {
            Object if_capabilities;
            try
            {
                if_capabilities = openconfig_if_interface_value.getObject();
            } catch(exception& e)
            {
                logger(ORCH_WARNING, MODULE_NAME, __FILE__, __LINE__, "The content does not respect the JSON syntax: \"%s\" should be an Object", IF_CAPABILITIES);
                return false;
            }
            domainInformations::InterfaceCapabilities *interfaceCapabilities = new domainInformations::InterfaceCapabilities();
            for(Object::const_iterator if_capabilities_iter = if_capabilities.begin(); if_capabilities_iter != if_capabilities.end(); if_capabilities_iter++)
            {
                const string &if_capabilities_key = if_capabilities_iter->first;
                const Value &if_capabilities_value = if_capabilities_iter->second;

                if (if_capabilities_key == GRE)
                {
                    interfaceCapabilities->setGreSupport(if_capabilities_value.getBool());
                }
            }
            interface.setInterfaceCapabilities(interfaceCapabilities);
        }
        else if (openconfig_if_interface_key == IF_GRE)
        {
			try {
				openconfig_if_interface_value.getArray();
			} catch (exception &e) {
				logger(ORCH_WARNING, MODULE_NAME, __FILE__, __LINE__, "The content does not respect the JSON syntax: \"%s\" should be an Array", IF_GRE);
				return false;
			}
			const Array& if_gre_array = openconfig_if_interface_value.getArray();
			for( unsigned int i = 0; i < if_gre_array.size(); i++ )
			{
				Object if_gre;
				try {
					if_gre = if_gre_array[i].getObject();
				} catch (exception &e) {
					logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "The content does not respect the JSON syntax: element of array \"%s\" should be an Objects", IF_GRE);
					return false;
				}
				domainInformations::Interface interfaceGre;
				parseInterface(if_gre,interfaceGre);
				interface.addGreInterface(interfaceGre);
			}
        }
        else if (openconfig_if_interface_key == OPTIONS)
        {
			Object if_options;
			try
			{
				if_options = openconfig_if_interface_value.getObject();
			} catch(exception& e)
			{
				logger(ORCH_WARNING, MODULE_NAME, __FILE__, __LINE__, "The content does not respect the JSON syntax: \"%s\" should be an Object", OPTIONS);
				return false;
			}
			domainInformations::InterfaceOptions *interfaceOptions = new domainInformations::InterfaceOptions();
			for(Object::const_iterator if_options_iter = if_options.begin(); if_options_iter != if_options.end(); if_options_iter++)
			{
				const string &if_options_key = if_options_iter->first;
				const Value &if_options_value = if_options_iter->second;

				if (if_options_key == OPT_LOCAL_IP)
				{
					interfaceOptions->setGreLocalIP(if_options_value.getString());
				}
				else if (if_options_key == OPT_REMOTE_IP)
				{
					interfaceOptions->setGreRemoteIP(if_options_value.getString());
				}
				else if (if_options_key == OPT_KEY)
				{
					interfaceOptions->setGreKey(if_options_value.getString());
				}
			}
			interface.setInterfaceOptions(interfaceOptions);
        }
    }
    return true;
}