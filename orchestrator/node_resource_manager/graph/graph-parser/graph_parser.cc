#include "graph_parser.h"

bool GraphParser::parseGraph(Value value, highlevel::Graph &graph, bool newGraph, GraphManager *gm)
{
	//for each NF, contains the set of ports it requires
	map<string,set<unsigned int> > nfs_ports_found;
	//for each NF, contains the id
	map<string, string> nfs_id;
	//for each endpoint (interface), contains the id
	map<string, string> iface_id;
	//for each endpoint (interface-out), contains the id
	map<string, string> iface_out_id;
	//for each endpoint (gre), contains the id
	map<string, string> gre_id;
	//for each endpoint (vlan), contains the pair vlan id, interface
	map<string, pair<string, string> > vlan_id; //XXX: currently, this information is ignored

	/**
	*	The graph is defined according to this schema:
	*		https://github.com/netgroup-polito/nffg-library/blob/master/schema.json
	*/
	try
	{
		Object obj = value.getObject();
		vector<Object> gre_array(256);
		Object big_switch, ep_gre;
	  	bool foundFlowGraph = false;

		//Iterates on the json received
		for(Object::const_iterator i = obj.begin(); i != obj.end(); ++i )
		{
	 		const string& name  = i->first;
			const Value&  value = i->second;

			//Identify the forwarding graph
			if(name == FORWARDING_GRAPH)
			{
	    		foundFlowGraph = true;

			bool foundEP = false;
	    		vector<string> id_gre (256);

				Object forwarding_graph;
				try
				{
		  			forwarding_graph = value.getObject();
				} catch(exception& e)
				{
					logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "The content does not respect the JSON syntax: \"%s\" should be an Object", FORWARDING_GRAPH);
					return false;
				}
				for(Object::const_iterator fg = forwarding_graph.begin(); fg != forwarding_graph.end(); fg++)
		    	{
					bool e_if = false, e_vlan = false;
#if 0
					bool e_if_out = false
#endif

					string id, v_id, node, iface, e_name, node_id, sw_id, interface;

	        		const string& fg_name  = fg->first;
		       		const Value&  fg_value = fg->second;

					if(fg_name == _ID)
	         			logger(ORCH_DEBUG, MODULE_NAME, __FILE__, __LINE__, "\"%s\"->\"%s\": \"%s\"",FORWARDING_GRAPH,_ID,fg_value.getString().c_str());
					else if(fg_name == _NAME)
					{
	         			logger(ORCH_DEBUG, MODULE_NAME, __FILE__, __LINE__, "\"%s\"->\"%s\": \"%s\"",FORWARDING_GRAPH,_NAME,fg_value.getString().c_str());

						//set name of the graph
						graph.setName(fg_value.getString());
					}
					else if(fg_name == F_DESCR)
					{
		         			logger(ORCH_DEBUG, MODULE_NAME, __FILE__, __LINE__, "\"%s\"->\"%s\": \"%s\"",FORWARDING_GRAPH,F_DESCR,fg_value.getString().c_str());

						//XXX: currently, this information is ignored
					}
					//Identify the VNFs
					else if(fg_name == VNFS)
					{
						try
						{
							try
							{
								fg_value.getArray();
							} catch(exception& e)
							{
								logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "The content does not respect the JSON syntax: \"%s\" should be an Array", VNFS);
								return false;
							}

							const Array& vnfs_array = fg_value.getArray();

							//XXX We may have no VNFs in the following cases:
							//*	graph with only physical ports
							//*	update of a graph that only adds new flows
							//However, when there are no VNFs, we provide a warning

					    	//Itearate on the VNFs
					    	for( unsigned int vnf = 0; vnf < vnfs_array.size(); ++vnf )
							{
								try
								{
									vnfs_array[vnf].getObject();
								} catch(exception& e)
								{
									logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "The content does not respect the JSON syntax: \"%s\" element should be an Object", VNFS);
									return false;
								}

								/**
								*	According to https://github.com/netgroup-polito/nffg-library/blob/master/schema.json , a VNF can contain:
								*		- id
								*		- name	(mandatory)
								*		- vnf_template
								*		- domain
								*		- ports
								*			- id
								*			- name
								*			- mac
								*			- unify-ip
								*		- unify-control
								*		- groups
								*/

								Object network_function = vnfs_array[vnf].getObject();

								bool foundName = false;

								string id, name, vnf_template, port_id, port_name;
								list<string> groups;
#ifdef ENABLE_UNIFY_PORTS_CONFIGURATION
								int vnf_tcp_port, host_tcp_port;
								//list of pair element "host TCP port" and "VNF TCP port" related by the VNF
								list<pair<string, string> > controlPorts;
								//list of environment variables in the form "variable=value"
								list<string> environmentVariables;
#endif
								//list of four element port id, port name, mac address and ip address related by the VNF
								list<vector<string> > portS;

								//Parse the network function
								for(Object::const_iterator nf = network_function.begin(); nf != network_function.end(); nf++)
								{
									const string& nf_name  = nf->first;
									const Value&  nf_value = nf->second;

									if(nf_name == _NAME)
									{
										logger(ORCH_DEBUG, MODULE_NAME, __FILE__, __LINE__, "\"%s\"->\"%s\": \"%s\"",VNFS,_NAME,nf_value.getString().c_str());
										foundName = true;
										if(!graph.addNetworkFunction(nf_value.getString()))
										{
											logger(ORCH_WARNING, MODULE_NAME, __FILE__, __LINE__, "Two VNFs with the same name \"%s\" in \"%s\"",nf_value.getString().c_str(),VNFS);
											return false;
										}

										name = nf_value.getString();

										nfs_id[id] = nf_value.getString();
										logger(ORCH_DEBUG, MODULE_NAME, __FILE__, __LINE__, "\"%s\"->\"%s\"",id.c_str(), nfs_id[id].c_str());
									}
									else if(nf_name == VNF_TEMPLATE)
									{
										logger(ORCH_DEBUG, MODULE_NAME, __FILE__, __LINE__, "\"%s\"->\"%s\": \"%s\"",VNFS,VNF_TEMPLATE,nf_value.getString().c_str());
										vnf_template = nf_value.getString();
										logger(ORCH_WARNING, MODULE_NAME, __FILE__, __LINE__, "Key \"%s\" found. It is ignored in the current implementation of the %s",VNF_TEMPLATE,MODULE_NAME);
									}
									else if(nf_name == _ID)
									{
										logger(ORCH_DEBUG, MODULE_NAME, __FILE__, __LINE__, "\"%s\"->\"%s\": \"%s\"",VNFS,_ID,nf_value.getString().c_str());

										//store value of VNF id
										id.assign(nf_value.getString().c_str());
									}
									else if(nf_name == UNIFY_CONTROL)
									{
#ifndef ENABLE_UNIFY_PORTS_CONFIGURATION
										logger(ORCH_WARNING, MODULE_NAME, __FILE__, __LINE__, "Key \"%s\" is ignored in this configuration of the %s!",UNIFY_CONTROL,MODULE_NAME);
										continue;
#else
										try{
											nf_value.getArray();
										} catch(exception& e)
										{
											logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "The content does not respect the JSON syntax: \"%s\" should be an Array", UNIFY_CONTROL);
											return false;
										}

										const Array& control_array = nf_value.getArray();

										//Itearate on the control ports
										for( unsigned int ctrl = 0; ctrl < control_array.size(); ++ctrl )
										{
											try{
												control_array[ctrl].getObject();
											} catch(exception& e)
											{
												logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "The content does not respect the JSON syntax: element of \"%s\" should be an Object", UNIFY_CONTROL);
												return false;
											}

											//This is a VNF control port, with an host TCP port and a vnf VNF port
											Object control = control_array[ctrl].getObject();

											port_mapping_t port_mapping;

											vector<string> port_descr(4);

											//Parse the control port
											for(Object::const_iterator c = control.begin(); c != control.end(); c++)
											{
												const string& c_name  = c->first;
												const Value&  c_value = c->second;

												if(c_name == HOST_PORT)
												{
													logger(ORCH_DEBUG, MODULE_NAME, __FILE__, __LINE__, "\"%s\"->\"%s\": \"%d\"",UNIFY_CONTROL,HOST_PORT,c_value.getInt());

													host_tcp_port = c_value.getInt();
												}
												else if(c_name == VNF_PORT)
												{
													logger(ORCH_DEBUG, MODULE_NAME, __FILE__, __LINE__, "\"%s\"->\"%s\": \"%d\"",UNIFY_CONTROL,VNF_PORT,c_value.getInt());

													vnf_tcp_port = c_value.getInt();
												}
											}

											stringstream ss, sss;
											ss << host_tcp_port;

											sss << vnf_tcp_port;

											port_mapping.host_port = ss.str();
											port_mapping.guest_port = sss.str();

											//Add VNF control port description
											graph.addNetworkFunctionControlPort(name, port_mapping);
											controlPorts.push_back(make_pair(ss.str(), sss.str()));
										}//end iteration on the control ports
#endif
									}
									else if(nf_name == UNIFY_ENV_VARIABLES)
									{
#ifndef ENABLE_UNIFY_PORTS_CONFIGURATION
										logger(ORCH_WARNING, MODULE_NAME, __FILE__, __LINE__, "Key \"%s\" is ignored in this configuration of the %s!",UNIFY_ENV_VARIABLES,MODULE_NAME);
										continue;
#else

										try{
											nf_value.getArray();
										} catch(exception& e)
										{
											logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "The content does not respect the JSON syntax: \"%s\" should be an Array", UNIFY_CONTROL);
											return false;
										}

										const Array& env_variables_array = nf_value.getArray();

										//Itearate on the environment variables
										for(unsigned int env_var = 0; env_var < env_variables_array.size(); ++env_var)
										{
											try{
												env_variables_array[env_var].getObject();
											} catch(exception& e)
											{
												logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "The content does not respect the JSON syntax: element of \"%s\" should be an Object", UNIFY_CONTROL);
												return false;
											}

											//This is an envirnment variable
											Object env_variable = env_variables_array[env_var].getObject();

											stringstream theEnvVar;

											//Parse the environment variable
											for(Object::const_iterator ev = env_variable.begin(); ev != env_variable.end(); ev++)
											{
												const string& ev_name  = ev->first;
												const Value&  ev_value = ev->second;

												if(ev_name == VARIABLE)
												{
													logger(ORCH_DEBUG, MODULE_NAME, __FILE__, __LINE__, "\"%s\"->\"%s\": \"%s\"",UNIFY_ENV_VARIABLES,VARIABLE,(ev_value.getString()).c_str());
													theEnvVar << ev_value.getString();
												}
												else
												{
													logger(ORCH_DEBUG, MODULE_NAME, __FILE__, __LINE__, "Invalid key \"%s\" in an element of \"%s\"",ev_name.c_str(),UNIFY_ENV_VARIABLES);
													return false;
												}
											}

											//Add an environment variable for the VNF
											graph.addNetworkFunctionEnvironmentVariable(name, theEnvVar.str());
											environmentVariables.push_back(theEnvVar.str());
										}//end iteration on the environment variables

#endif
									}
									else if(nf_name == VNF_PORTS)
									{
										try{
											nf_value.getArray();
										} catch(exception& e)
										{
											logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "The content does not respect the JSON syntax: \"%s\" should be an Array", VNF_PORTS);
											return false;
										}

										const Array& ports_array = nf_value.getArray();

										map<unsigned int, port_network_config_t > vnf_port_config;

										//Itearate on the ports
										for( unsigned int ports = 0; ports < ports_array.size(); ++ports )
										{
											try{
												ports_array[ports].getObject();
											} catch(exception& e)
											{
												logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "The content does not respect the JSON syntax: \"%s\" element should be an Object", VNF_PORTS);
												return false;
											}

											//This is a VNF port, with an ID and a name
											Object port = ports_array[ports].getObject();

											vector<string> port_descr(4);

											port_network_config_t port_config;

											//Parse the port
											for(Object::const_iterator p = port.begin(); p != port.end(); p++)
											{
												const string& p_name  = p->first;
												const Value&  p_value = p->second;

												if(p_name == _ID)
												{
													logger(ORCH_DEBUG, MODULE_NAME, __FILE__, __LINE__, "\"%s\"->\"%s\": \"%s\"",VNF_PORTS,_ID,p_value.getString().c_str());

													port_id = p_value.getString();

													port_descr[0] = port_id;
												}
												else if(p_name == _NAME)
												{
													logger(ORCH_DEBUG, MODULE_NAME, __FILE__, __LINE__, "\"%s\"->\"%s\": \"%s\"",VNF_PORTS,_NAME,p_value.getString().c_str());

													port_name = p_value.getString();

													port_descr[1] = port_name;
												}
												else if(p_name == PORT_MAC)
												{
													logger(ORCH_DEBUG, MODULE_NAME, __FILE__, __LINE__, "\"%s\"->\"%s\": \"%s\"",VNF_PORTS,PORT_MAC,p_value.getString().c_str());

													port_config.mac_address = p_value.getString();
													port_descr[2] = port_config.mac_address;
												}
												else if(p_name == PORT_IP)
												{
#ifndef ENABLE_UNIFY_PORTS_CONFIGURATION
													logger(ORCH_WARNING, MODULE_NAME, __FILE__, __LINE__, "Key \"%s\" is ignored in this configuration of the %s!",PORT_IP,MODULE_NAME);
													continue;
#else
													logger(ORCH_DEBUG, MODULE_NAME, __FILE__, __LINE__, "\"%s\"->\"%s\": \"%s\"",VNF_PORTS,PORT_IP,p_value.getString().c_str());

													port_config.ip_address = p_value.getString();
													port_descr[3] = port_config.ip_address;
#endif
												}
												else
												{
													logger(ORCH_DEBUG, MODULE_NAME, __FILE__, __LINE__, "Invalid key \"%s\" in a VNF of \"%s\"",p_name.c_str(),VNF_PORTS);
													return false;
												}
											}

											//Each VNF port has its own configuration if provided
											vnf_port_config[ports+1] = port_config;

											//Add NF ports descriptions
											if(!graph.addNetworkFunctionPortConfiguration(name, vnf_port_config))
											{
												logger(ORCH_WARNING, MODULE_NAME, __FILE__, __LINE__, "Two VNFs with the same name \"%s\" in \"%s\"",nf_value.getString().c_str(),VNFS);
												return false;
											}

											portS.push_back(port_descr);
										}
									}
									else if(nf_name == VNF_GROUPS)
									{

										try
										{
											nf_value.getArray();
										} catch(exception& e)
										{
											logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "The content does not respect the JSON syntax: \"%s\" element should be an Object", VNFS);
											return false;
										}
										const Array& myGroups_Array = nf_value.getArray();
										for(unsigned int i; i<myGroups_Array.size();i++)
										{
											logger(ORCH_DEBUG, MODULE_NAME, __FILE__, __LINE__, "\"%s\"->\"%s\": \"%s\"",VNFS,VNF_GROUPS,myGroups_Array[i].getString().c_str());
											string group = myGroups_Array[i].getString();
											groups.push_back(group);
										}
										logger(ORCH_WARNING, MODULE_NAME, __FILE__, __LINE__, "Key \"%s\" found. It is ignored in the current implementation of the %s",VNF_GROUPS,MODULE_NAME);
									}
									else
									{
										logger(ORCH_DEBUG, MODULE_NAME, __FILE__, __LINE__, "Invalid key \"%s\" in a VNF of \"%s\"",nf_name.c_str(),VNFS);
										return false;
									}
								}
								if(!foundName)
								{
									logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Key \"%s\" not found in an element of \"%s\"",_NAME,VNFS);
									return false;
								}

#ifdef ENABLE_UNIFY_PORTS_CONFIGURATION
								highlevel::VNFs vnfs(id, name, groups, vnf_template, portS, controlPorts,environmentVariables);
#else
								highlevel::VNFs vnfs(id, name, groups, vnf_template, portS);
#endif
								graph.addVNF(vnfs);
								portS.clear();
#ifdef ENABLE_UNIFY_PORTS_CONFIGURATION
								controlPorts.clear();
								environmentVariables.clear();
#endif
							}
						}
						catch(exception& e)
						{
							logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "The \"%s\" element does not respect the JSON syntax: \"%s\"", VNFS, e.what());
							return false;
						}
			    	}
					//Identify the end-points
					else if(fg_name == END_POINTS)
				    {
						try
						{
							try
							{
								fg_value.getArray();
							} catch(exception& e)
							{
								logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "The content does not respect the JSON syntax: \"%s\" should be an Array", END_POINTS);
								return false;
							}

							/**
							*	According to https://github.com/netgroup-polito/nffg-library/blob/master/schema.json , an endpoint can contain:
							*	- id
							*	- name
							*	- type
							*		- internal
							*		- interface
							*		- interface-out		-> NOT SUPPORTED
							*		- gre-tunnel
							*		- vlan
							*	- Other information that depend on the type
							*/

					    	const Array& end_points_array = fg_value.getArray();

							foundEP = true;	//this variable is valid only for the current iteration

							logger(ORCH_DEBUG, MODULE_NAME, __FILE__, __LINE__, "\"%s\"",END_POINTS);

				    		//Iterate on the end-points
				    		for( unsigned int ep = 0; ep < end_points_array.size(); ++ep )
							{
								try{
									end_points_array[ep].getObject();
								} catch(exception& e)
								{
									logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "The content does not respect the JSON syntax: \"%s\" element should be an Object", END_POINTS);
									return false;
								}

								//This is a endpoints, with a name, a type, and an interface
								Object end_points = end_points_array[ep].getObject();

								//Iterate on the elements of an endpoint
								for(Object::const_iterator aep = end_points.begin(); aep != end_points.end(); aep++)
								{
									const string& ep_name  = aep->first;
									const Value&  ep_value = aep->second;

									if(ep_name == _ID)
									{
										logger(ORCH_DEBUG, MODULE_NAME, __FILE__, __LINE__, "\"%s\"->\"%s\": \"%s\"",END_POINTS,_ID,ep_value.getString().c_str());
										id = ep_value.getString();
									}
									else if(ep_name == _NAME)
									{
										logger(ORCH_DEBUG, MODULE_NAME, __FILE__, __LINE__, "\"%s\"->\"%s\": \"%s\"",END_POINTS,_NAME,ep_value.getString().c_str());
										e_name = ep_value.getString();
									}
									else if(ep_name == EP_TYPE)
									{
										logger(ORCH_DEBUG, MODULE_NAME, __FILE__, __LINE__, "\"%s\"->\"%s\": \"%s\"",END_POINTS,EP_TYPE,ep_value.getString().c_str());
										string type = ep_value.getString();
									}
									else if(ep_name == EP_REM)
									{
										logger(ORCH_DEBUG, MODULE_NAME, __FILE__, __LINE__, "\"%s\"->\"%s\": \"%s\"",END_POINTS,EP_REM,ep_value.getString().c_str());
										logger(ORCH_WARNING, MODULE_NAME, __FILE__, __LINE__, "Element \"%s\" is ignored by the current implementation of the %s", EP_REM,MODULE_NAME);
										//XXX: currently, this information is ignored
									}
									else if(ep_name == EP_PR)
									{
										logger(ORCH_DEBUG, MODULE_NAME, __FILE__, __LINE__, "\"%s\"->\"%s\": \"%s\"",END_POINTS,EP_PR,ep_value.getString().c_str());
										logger(ORCH_WARNING, MODULE_NAME, __FILE__, __LINE__, "Element \"%s\" is ignored by the current implementation of the %s", EP_PR,MODULE_NAME);
										//XXX: currently, this information is ignored
									}
									//identify interface end-points
									else if(ep_name == IFACE)
									{
										try
										{
											ep_value.getObject();
										} catch(exception& e)
										{
											logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "The content does not respect the JSON syntax: \"%s\" should be an Object", IFACE);
											return false;
										}

										Object ep_iface = ep_value.getObject();
										e_if = true;

										for(Object::const_iterator epi = ep_iface.begin(); epi != ep_iface.end(); epi++)
										{
											const string& epi_name  = epi->first;
											const Value&  epi_value = epi->second;

											if(epi_name == NODE_ID)
											{
												logger(ORCH_DEBUG, MODULE_NAME, __FILE__, __LINE__, "\"%s\"->\"%s\": \"%s\"",IFACE,NODE_ID,epi_value.getString().c_str());
												logger(ORCH_WARNING, MODULE_NAME, __FILE__, __LINE__, "Element \"%s\" is ignored by the current implementation of the %s", EP_PR,NODE_ID);
												node_id = epi_value.getString();
											}
											else if(epi_name == SW_ID)
											{
												logger(ORCH_DEBUG, MODULE_NAME, __FILE__, __LINE__, "\"%s\"->\"%s\": \"%s\"",IFACE,SW_ID,epi_value.getString().c_str());
												logger(ORCH_WARNING, MODULE_NAME, __FILE__, __LINE__, "Element \"%s\" is ignored by the current implementation of the %s", EP_PR,SW_ID);
												sw_id = epi_value.getString();
											}
											else if(epi_name == IFACE)
											{
												logger(ORCH_DEBUG, MODULE_NAME, __FILE__, __LINE__, "\"%s\"->\"%s\": \"%s\"",IFACE,IFACE,epi_value.getString().c_str());

												interface = epi_value.getString();
												iface_id[id] = epi_value.getString();
												logger(ORCH_DEBUG, MODULE_NAME, __FILE__, __LINE__, "\"%s\"->\"%s\"",id.c_str(), iface_id[id].c_str());
											}
										}
									}
									else if(ep_name == EP_INTERNAL)
									{
										logger(ORCH_WARNING, MODULE_NAME, __FILE__, __LINE__, "Element \"%s\" is ignored by the current implementation of the %s. This type of end-point is not supported!", EP_INTERNAL,MODULE_NAME);
									}
									//identify interface-out end-points
									else if(ep_name == EP_IFACE_OUT)
									{
										logger(ORCH_WARNING, MODULE_NAME, __FILE__, __LINE__, "Element \"%s\" is ignored by the current implementation of the %s. This type of end-point is not supported!", EP_IFACE_OUT,MODULE_NAME);
									}
									//identify vlan end-points
									else if(ep_name == VLAN)
									{
										try{
											ep_value.getObject();
										} catch(exception& e)
										{
											logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "The content does not respect the JSON syntax: \"%s\" should be an Object", VLAN);
											return false;
										}

										Object ep_vlan = ep_value.getObject();

										e_vlan = true;

										for(Object::const_iterator epi = ep_vlan.begin(); epi != ep_vlan.end(); epi++)
										{
											const string& epi_name  = epi->first;
											const Value&  epi_value = epi->second;

											if(epi_name == V_ID)
											{
												logger(ORCH_DEBUG, MODULE_NAME, __FILE__, __LINE__, "\"%s\"->\"%s\": \"%s\"",VLAN,VLAN_ID,epi_value.getString().c_str());
												v_id = epi_value.getString();
											}
											else if(epi_name == IFACE)
											{
												logger(ORCH_DEBUG, MODULE_NAME, __FILE__, __LINE__, "\"%s\"->\"%s\": \"%s\"",VLAN,IFACE,epi_value.getString().c_str());
												interface = epi_value.getString();
											}
											else if(epi_name == SW_ID)
											{
												logger(ORCH_DEBUG, MODULE_NAME, __FILE__, __LINE__, "\"%s\"->\"%s\": \"%s\"",VLAN,SW_ID,epi_value.getString().c_str());
												sw_id = epi_value.getString();
											}
											else if(epi_name == NODE_ID)
											{
												logger(ORCH_DEBUG, MODULE_NAME, __FILE__, __LINE__, "\"%s\"->\"%s\": \"%s\"",VLAN,NODE_ID,epi_value.getString().c_str());
												node_id = epi_value.getString();
											}
										}

										vlan_id[id] = make_pair(v_id, interface);
										logger(ORCH_DEBUG, MODULE_NAME, __FILE__, __LINE__, "\"%s\"->\"%s\":\"%s\"",id.c_str(),vlan_id[id].first.c_str(),vlan_id[id].second.c_str());
									}
									else if(ep_name == EP_GRE)
									{
										try
										{
											ep_value.getObject();
										} catch(exception& e)
										{
											logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "The content does not respect the JSON syntax: \"%s\" should be an Object", EP_GRE);
											return false;
										}

										//In order to get e-d-point ID (it wil be parsed later)
										Object::const_iterator aep2 = aep;
										aep2++;
										for(; aep2 != end_points.end(); aep2++)
										{
											const string& ep2_name  = aep2->first;
											const Value&  ep2_value = aep2->second;
											if(ep2_name == _ID)
											{
												id = ep2_value.getString();
												break;
											}
										}


										try
										{
											vector<string> gre_param (5);

											string local_ip, remote_ip, interface, ttl, gre_key;
											bool safe = false;

											ep_gre=ep_value.getObject();

											for(Object::const_iterator epi = ep_gre.begin(); epi != ep_gre.end(); epi++)
											{
												const string& epi_name  = epi->first;
												const Value&  epi_value = epi->second;

												if(epi_name == LOCAL_IP)
												{
													logger(ORCH_DEBUG, MODULE_NAME, __FILE__, __LINE__, "\"%s\"->\"%s\": \"%s\"",EP_GRE,LOCAL_IP,epi_value.getString().c_str());

													local_ip = epi_value.getString();

													gre_param[1] = epi_value.getString();

												}
												else if(epi_name == REMOTE_IP)
												{
													logger(ORCH_DEBUG, MODULE_NAME, __FILE__, __LINE__, "\"%s\"->\"%s\": \"%s\"",EP_GRE,REMOTE_IP,epi_value.getString().c_str());

													remote_ip = epi_value.getString();

													gre_param[2] = epi_value.getString();

												}
												else if(epi_name == IFACE)
												{
													logger(ORCH_DEBUG, MODULE_NAME, __FILE__, __LINE__, "\"%s\"->\"%s\": \"%s\"",EP_GRE,IFACE,epi_value.getString().c_str());

													interface = epi_value.getString();

													gre_param[3] = interface;

													logger(ORCH_DEBUG, MODULE_NAME, __FILE__, __LINE__, "\"%s\"->\"%s\"",id.c_str(), interface.c_str());
												}
												else if(epi_name == TTL)
												{
													logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "\"%s\"->\"%s\": \"%s\"",EP_GRE,TTL,epi_value.getString().c_str());

													ttl = epi_value.getString();
												}
												else if(epi_name == GRE_KEY)
												{
													logger(ORCH_DEBUG, MODULE_NAME, __FILE__, __LINE__, "\"%s\"->\"%s\": \"%s\"",EP_GRE,GRE_KEY,epi_value.getString().c_str());

													gre_key = epi_value.getString();

													gre_param[0] = epi_value.getString();

												}
												else if(epi_name == SAFE)
												{
													logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "\"%s\"->\"%s\": \"%d\"",EP_GRE,SAFE,epi_value.getBool());

													safe = epi_value.getBool();
												}
											}

											if(safe)
												gre_param[4] = string("true");
											else
												gre_param[4] = string("false");

											gre_id[id] = interface;

											//Add gre-tunnel end-points
											highlevel::EndPointGre ep_gre(id, e_name, local_ip, remote_ip, interface, gre_key, ttl, safe);

											graph.addEndPointGre(ep_gre);

											graph.addEndPoint(id,gre_param);
										}
										catch(exception& e)
										{
											logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "The \"%s\" element does not respect the JSON syntax: \"%s\"", EP_GRE, e.what());
											return false;
										}
									}
									else
										logger(ORCH_DEBUG, MODULE_NAME, __FILE__, __LINE__, "\"%s\"->\"%s\": \"%s\"",ACTIONS,END_POINTS,ep_value.getString().c_str());
								}//End of iteration on the elements of an endpoint

								//add interface end-points
								if(e_if)
								{
									//FIXME: are we sure that "interface" has been specified?
									//FIXME: node_id and sw_id should be ignored
									highlevel::EndPointInterface ep_if(id, e_name, node_id, sw_id, interface);
									graph.addEndPointInterface(ep_if);
									e_if = false;
								}
								//add vlan end-points
								else if(e_vlan)
								{
									//FIXME: are we sure that "interface" and "v_id" have been specified?
									//FIXME: node_id and sw_id should be ignored
									highlevel::EndPointVlan ep_vlan(id, e_name, v_id, node_id, sw_id, interface);
									graph.addEndPointVlan(ep_vlan);
									e_vlan = false;
								}
							}//End iteration on the endpoints
						}
						catch(exception& e)
						{
							logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "The \"%s\" element does not respect the JSON syntax: \"%s\"", END_POINTS, e.what());
							return false;
						}
					}//End if(fg_name == END_POINTS)
					//Identify the big-switch
					else if(fg_name == BIG_SWITCH)
					{
						try{
							fg_value.getObject();
						} catch(exception& e)
						{
							logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "The content does not respect the JSON syntax: \"%s\" should be an Object", BIG_SWITCH);
							return false;
						}

						big_switch = fg_value.getObject();
						//The content of the "big-switch" element will be parsed later.
						//In fact it requires that the "end-points" element and the "VNFs" element will be found
					}
					else
					{
						logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Invalid key \"%s\" in \"%s\"",fg_name.c_str(),FORWARDING_GRAPH);
						return false;
					}

					//Since we found the element "end-points", we can now parse the content of "big-switch"
					if(foundEP)
					{
						logger(ORCH_DEBUG, MODULE_NAME, __FILE__, __LINE__, "\"%s\"",BIG_SWITCH);
						foundEP = false;

		    		}//end if(foundEP)
				}// End iteration on the elements of "forwarding-graph"

				/*******************************************/
				// Iterate on the element of the big-switch
				bool foundFlowRules = false;
				for(Object::const_iterator bs = big_switch.begin(); bs != big_switch.end(); bs++)
				{
					const string& bs_name  = bs->first;
					const Value&  bs_value = bs->second;

					if (bs_name == FLOW_RULES)
					{
						foundFlowRules = true;

						try
						{
							try{
								bs_value.getArray();
							} catch(exception& e)
							{
								logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "The content does not respect the JSON syntax: \"%s\" should be an Array", FLOW_RULES);
								return false;
							}

							const Array& flow_rules_array = bs_value.getArray();

#ifndef UNIFY_NFFG
							//FIXME: put the flowrules optional also in case of "standard| nffg?
							if(flow_rules_array.size() == 0)
							{
								logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Key \"%s\" without rules",FLOW_RULES);
								return false;
							}
#endif
							//Itearate on the flow rules
							for( unsigned int fr = 0; fr < flow_rules_array.size(); ++fr )
							{
								//This is a rule, with a match, an action, and an ID
								Object flow_rule = flow_rules_array[fr].getObject();
								highlevel::Action *action = NULL;
								list<GenericAction*> genericActions;
								highlevel::Match match;
								string ruleID;
								uint64_t priority = 0;

								bool foundAction = false;
								bool foundMatch = false;
								bool foundID = false;

								//Parse the rule
								for(Object::const_iterator afr = flow_rule.begin(); afr != flow_rule.end(); afr++)
								{
									const string& fr_name  = afr->first;
									const Value&  fr_value = afr->second;
									if(fr_name == _ID)
									{
										foundID = true;
										ruleID = fr_value.getString();
									}
									else if(fr_name == F_DESCR)
									{
										logger(ORCH_DEBUG, MODULE_NAME, __FILE__, __LINE__, "\"%s\"->\"%s\": \"%s\"",FLOW_RULES,F_DESCR,fr_value.getString().c_str());

										//XXX: currently, this information is ignored
									}
									else if(fr_name == PRIORITY)
									{
										logger(ORCH_DEBUG, MODULE_NAME, __FILE__, __LINE__, "\"%s\"->\"%s\": \"%d\"",FLOW_RULES,PRIORITY,fr_value.getInt());

										priority = fr_value.getInt();
									}
									else if(fr_name == MATCH)
									{
										try{
											foundMatch = true;
											if(!MatchParser::parseMatch(fr_value.getObject(),match,(*action),nfs_ports_found,nfs_id,iface_id,iface_out_id,vlan_id,gre_id,graph))
											{
												return false;
											}
										} catch(exception& e)
										{
											logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "The content does not respect the JSON syntax: \"%s\"", MATCH);
											return false;
										}
									}
									else if(fr_name == ACTIONS)
									{
										try
										{
											try
											{
												fr_value.getArray();
											} catch(exception& e)
											{
												logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "The content does not respect the JSON syntax: \"%s\" should be an Array", ACTIONS);
												return false;
											}

											const Array& actions_array = fr_value.getArray();

											enum port_type { VNF_PORT_TYPE, EP_PORT_TYPE };

											//One and only one output_to_port is allowed
											bool foundOneOutputToPort = false;

											//Itearate on all the actions specified for this flowrule
											for( unsigned int ac = 0; ac < actions_array.size(); ++ac )
											{
												foundAction = true;
												try{
													actions_array[ac].getObject();
												} catch(exception& e)
												{
													logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "The content does not respect the JSON syntax: \"%s\" element should be an Object", ACTIONS);
													return false;
												}

												//A specific action of the array can have a single keyword inside
												Object theAction = actions_array[ac].getObject();
												assert(theAction.size() == 1);
												if(theAction.size() != 1)
												{
													logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Too many keywords in an element of \"%s\"",ACTIONS);
													return false;
												}

												for(Object::const_iterator a = theAction.begin(); a != theAction.end(); a++)
												{
													const string& a_name  = a->first;
													const Value&  a_value = a->second;

													if(a_name == OUTPUT)
													{
														//The action is "output_to_port"

														string port_in_name = a_value.getString();
														string realName;
														const char *port_in_name_tmp = port_in_name.c_str();
														char vnf_name_tmp[BUFFER_SIZE];

														//Check the name of port
														char delimiter[] = ":";
													 	char * pnt;

														port_type p_type = VNF_PORT_TYPE;

														char tmp[BUFFER_SIZE];
														strcpy(tmp,(char *)port_in_name_tmp);
														pnt=strtok(tmp, delimiter);
														int i = 0;

														//The "output_to_port" action can refer to:
														//	- an endpoint
														//	- the port of a VNF
														while( pnt!= NULL )
														{
															switch(i)
															{
																case 0:
																	//VNFs port type
																	if(strcmp(pnt,VNF) == 0)
																	{
																		p_type = VNF_PORT_TYPE;
																	}
																	//end-points port type
																	else if (strcmp(pnt,ENDPOINT) == 0)
																	{
																		p_type = EP_PORT_TYPE;
																	}
																	break;
																case 1:
																	if(p_type == VNF_PORT_TYPE)
																	{
																		strcpy(vnf_name_tmp,nfs_id[pnt].c_str());
																		strcat(vnf_name_tmp, ":");
																	}
																	break;
																case 3:
																	if(p_type == VNF_PORT_TYPE)
																	{
																		strcat(vnf_name_tmp,pnt);
																	}
															}

															pnt = strtok( NULL, delimiter );
															i++;
														}

														if(foundOneOutputToPort)
														{
															logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Only one between keys \"%s\", \"%s\" and \"%s\" are allowed in \"%s\"",PORT_IN,VNF,ENDPOINT,ACTIONS);
															return false;
														}
														foundOneOutputToPort = true;

														if(p_type == VNF_PORT_TYPE)
														{
															//This is an output action referred to a VNF port

															//convert char *vnf_name_tmp to string vnf_name
															string vnf_name(vnf_name_tmp, strlen(vnf_name_tmp));

															logger(ORCH_DEBUG, MODULE_NAME, __FILE__, __LINE__, "\"%s\"->\"%s\": \"%s\"",ACTIONS,VNF,vnf_name.c_str());

															string name = MatchParser::nfName(vnf_name);
															char *tmp_vnf_name = new char[BUFFER_SIZE];
															strcpy(tmp_vnf_name, (char *)vnf_name.c_str());
															unsigned int port = MatchParser::nfPort(string(tmp_vnf_name));
															bool is_port = MatchParser::nfIsPort(string(tmp_vnf_name));

															if(name == "" || !is_port)
															{
																logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Network function \"%s\" is not valid. It must be in the form \"name:port\"",vnf_name.c_str());
																return false;
															}

															/*nf port starts from 0*/
															port++;

															action = new highlevel::ActionNetworkFunction(name, string(port_in_name_tmp), port);

															set<unsigned int> ports_found;
															if(nfs_ports_found.count(name) != 0)
																ports_found = nfs_ports_found[name];
															ports_found.insert(port);
															nfs_ports_found[name] = ports_found;
														}
														//end-points port type
														else if(p_type == EP_PORT_TYPE)
														{
															//This is an output action referred to an endpoint

															bool iface_found = false, vlan_found = false, gre_found=false;

															char *s_a_value = new char[BUFFER_SIZE];
															strcpy(s_a_value, (char *)a_value.getString().c_str());
															string eP = MatchParser::epName(a_value.getString());
															if(eP != "")
															{
																map<string,string>::iterator it = iface_id.find(eP);
																map<string,string>::iterator it1 = iface_out_id.find(eP);
																map<string,pair<string,string> >::iterator it2 = vlan_id.find(eP);
																map<string,string>::iterator it3 = gre_id.find(eP);
																if(it != iface_id.end())
																{
																	//physical port
																	realName.assign(iface_id[eP]);
																	iface_found = true;
																}
																else if(it1 != iface_out_id.end())
																{
																	//physical port
																	realName.assign(iface_out_id[eP]);
																	iface_found = true;
																}
																else if(it2 != vlan_id.end())
																{
																	//vlan
																	vlan_found = true;
																}
																else if(it3 != gre_id.end())
																{
																	//gre
																	gre_found = true;
																}
															}
															//physical endpoint
															if(iface_found)
															{
																	action = new highlevel::ActionPort(realName, string(s_a_value));
																	graph.addPort(realName);
															}
															//vlan endpoint
															else if(vlan_found)
															{
																vlan_action_t actionType;
																unsigned int vlanID = 0;

																actionType = ACTION_ENDPOINT_VLAN;

																sscanf(vlan_id[eP].first.c_str(),"%u",&vlanID);

																/*add "output_port" action*/
																action = new highlevel::ActionPort(vlan_id[eP].second, string(s_a_value));
																graph.addPort(vlan_id[eP].second);

																/*add "push_vlan" action*/
																GenericAction *ga = new VlanAction(actionType,string(s_a_value),vlanID);
																action->addGenericAction(ga);
															}
															//gre-tunnel endpoint
															else if(gre_found)
															{
																unsigned int endPoint = MatchParser::epPort(string(s_a_value));
																if(endPoint == 0)
																{
																	logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Graph end point \"%s\" is not valid. It must be in the form \"graphID:endpoint\"",value.getString().c_str());
																	return false;
																}
																action = new highlevel::ActionEndPoint(endPoint, string(s_a_value));
															}
														}
													}//End action == output_to_port
													else if(a_name == SET_VLAN_ID)
													{
														logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "\"%s\"->\"%s\": \"%s\"",ACTIONS,SET_VLAN_ID,a_value.getString().c_str());

														//XXX: currently, this information is ignored
													}
													else if(a_name == SET_VLAN_PRIORITY)
													{
														logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "\"%s\"->\"%s\": \"%s\"",ACTIONS,SET_VLAN_PRIORITY,a_value.getString().c_str());

														//XXX: currently, this information is ignored
													}
													else if(a_name == VLAN_PUSH)
													{
														//The action is "push_vlan"

														vlan_action_t actionType;
														unsigned int vlanID = 0;

														actionType = ACTION_VLAN_PUSH;

														string strVlanID = a_value.getString();
														vlanID = strtol (strVlanID.c_str(),NULL,0);

														GenericAction *ga = new VlanAction(actionType,string(""),vlanID);
														genericActions.push_back(ga);

													}//end if(a_name == VLAN_PUSH)
													else if(a_name == VLAN_POP)
													{
														//A vlan pop action is required
														vlan_action_t actionType;
														unsigned int vlanID = 0;

														bool is_vlan_pop = a_value.getBool();
														if(is_vlan_pop)
														{
															actionType = ACTION_VLAN_POP;

															//Finally, we are sure that the command is correct!
															GenericAction *ga = new VlanAction(actionType,string(""),vlanID);
															genericActions.push_back(ga);
														}
													}//end if(a_name == VLAN_POP)
													else if(a_name == SET_ETH_SRC_ADDR)
													{
														logger(ORCH_WARNING, MODULE_NAME, __FILE__, __LINE__, "\"%s\" \"%s\" not available",ACTIONS,SET_ETH_SRC_ADDR);

														logger(ORCH_DEBUG, MODULE_NAME, __FILE__, __LINE__, "\"%s\"->\"%s\": \"%s\"",ACTIONS,SET_ETH_SRC_ADDR,a_value.getString().c_str());

														//XXX: currently, this information is ignored
													}
													else if(a_name == SET_ETH_DST_ADDR)
													{
														logger(ORCH_WARNING, MODULE_NAME, __FILE__, __LINE__, "\"%s\" \"%s\" not available",ACTIONS,SET_ETH_DST_ADDR);

														logger(ORCH_DEBUG, MODULE_NAME, __FILE__, __LINE__, "\"%s\"->\"%s\": \"%s\"",ACTIONS,SET_ETH_DST_ADDR,a_value.getString().c_str());

														//XXX: currently, this information is ignored
													}
													else if(a_name == SET_IP_SRC_ADDR)
													{
														logger(ORCH_WARNING, MODULE_NAME, __FILE__, __LINE__, "\"%s\" \"%s\" not available",ACTIONS,SET_IP_SRC_ADDR);

														logger(ORCH_DEBUG, MODULE_NAME, __FILE__, __LINE__, "\"%s\"->\"%s\": \"%s\"",ACTIONS,SET_IP_SRC_ADDR,a_value.getString().c_str());

														//XXX: currently, this information is ignored
													}
													else if(a_name == SET_IP_DST_ADDR)
													{
														logger(ORCH_WARNING, MODULE_NAME, __FILE__, __LINE__, "\"%s\" \"%s\" not available",ACTIONS,SET_IP_DST_ADDR);

														logger(ORCH_DEBUG, MODULE_NAME, __FILE__, __LINE__, "\"%s\"->\"%s\": \"%s\"",ACTIONS,SET_IP_DST_ADDR,a_value.getString().c_str());

														//XXX: currently, this information is ignored
													}
													else if(a_name == SET_IP_TOS)
													{
														logger(ORCH_WARNING, MODULE_NAME, __FILE__, __LINE__, "\"%s\" \"%s\" not available",ACTIONS,SET_IP_TOS);

														logger(ORCH_DEBUG, MODULE_NAME, __FILE__, __LINE__, "\"%s\"->\"%s\": \"%s\"",ACTIONS,SET_IP_TOS,a_value.getString().c_str());

														//XXX: currently, this information is ignored
													}
													else if(a_name == SET_L4_SRC_PORT)
													{
														logger(ORCH_WARNING, MODULE_NAME, __FILE__, __LINE__, "\"%s\" \"%s\" not available",ACTIONS,SET_L4_SRC_PORT);

														logger(ORCH_DEBUG, MODULE_NAME, __FILE__, __LINE__, "\"%s\"->\"%s\": \"%s\"",ACTIONS,SET_L4_SRC_PORT,a_value.getString().c_str());

														//XXX: currently, this information is ignored
													}
													else if(a_name == SET_L4_DST_PORT)
													{
														logger(ORCH_WARNING, MODULE_NAME, __FILE__, __LINE__, "\"%s\" \"%s\" not available",ACTIONS,SET_L4_DST_PORT);

														logger(ORCH_DEBUG, MODULE_NAME, __FILE__, __LINE__, "\"%s\"->\"%s\": \"%s\"",ACTIONS,SET_L4_DST_PORT,a_value.getString().c_str());

														//XXX: currently, this information is ignored
													}
													else if(a_name == OUT_TO_QUEUE)
													{
														logger(ORCH_WARNING, MODULE_NAME, __FILE__, __LINE__, "\"%s\" \"%s\" not available",ACTIONS,OUT_TO_QUEUE);

														logger(ORCH_DEBUG, MODULE_NAME, __FILE__, __LINE__, "\"%s\"->\"%s\": \"%s\"",ACTIONS,OUT_TO_QUEUE,a_value.getString().c_str());

														//XXX: currently, this information is ignored
													}
													else if(a_name == DROP)
													{
														logger(ORCH_WARNING, MODULE_NAME, __FILE__, __LINE__, "\"%s\" \"%s\" not available",ACTIONS,DROP);

														logger(ORCH_DEBUG, MODULE_NAME, __FILE__, __LINE__, "\"%s\"->\"%s\": \"%s\"",ACTIONS,DROP,a_value.getString().c_str());

														//XXX: currently, this information is ignored
													}
													else if(a_name == OUTPUT_TO_CTRL)
													{
														logger(ORCH_WARNING, MODULE_NAME, __FILE__, __LINE__, "\"%s\" \"%s\" not available",ACTIONS,OUTPUT_TO_CTRL);

														logger(ORCH_DEBUG, MODULE_NAME, __FILE__, __LINE__, "\"%s\"->\"%s\": \"%s\"",ACTIONS,OUTPUT_TO_CTRL,a_value.getString().c_str());

														//XXX: currently, this information is ignored
													}
													else
													{
														logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Invalid key \"%s\" in \"%s\"",a_name.c_str(),ACTIONS);
														return false;
													}
												}//end iteration on the keywords of an action element (remember that a single keywork is allowed in each element)


											}//Here terminates the loop on the array actions
											if(!foundOneOutputToPort)
											{
												//"output_to_port" is a mandatory action
												logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Key \"%s\" not found in \"%s\"",OUTPUT,ACTIONS);
												return false;
											}
											assert(action != NULL);
											for(list<GenericAction*>::iterator ga = genericActions.begin(); ga != genericActions.end(); ga++)
												action->addGenericAction(*ga);
										}//end of try
										catch(exception& e)
										{
											logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "The \"%s\" element does not respect the JSON syntax: \"%s\"", ACTIONS, e.what());
											return false;
										}
									}//end if(fr_name == ACTION)
									else
									{
										logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Invalid key \"%s\" in a rule of \"%s\"",name.c_str(),FLOW_RULES);
										return false;
									}
								}

								if(!foundAction || !foundMatch || !foundID)
								{
									logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Key \"%s\", or key \"%s\", or key \"%s\", or all of them not found in an elmenet of \"%s\"",_ID,MATCH,ACTIONS,FLOW_RULES);
									return false;
								}

								highlevel::Rule rule(match,action,ruleID,priority);

								if(!graph.addRule(rule))
								{
									logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "The graph has at least two rules with the same ID: %s",ruleID.c_str());
									return false;
								}

							}//for( unsigned int fr = 0; fr < flow_rules_array.size(); ++fr )

							bool same_priority = false;
							list<highlevel::Rule> rules = graph.getRules();
							for(list<highlevel::Rule>::iterator r = rules.begin(); r != rules.end(); r++)
							{
								list<highlevel::Rule>::iterator next_rule = r;
								next_rule++;
								if(next_rule != rules.end())
								{
									uint64_t priority = (*r).getPriority();
									for(list<highlevel::Rule>::iterator r1 = next_rule; r1 != rules.end(); r1++)
									{
										if((*r1).getPriority() == priority)
											same_priority = true;
									}
								}
							}

							if(same_priority)
							{
								logger(ORCH_WARNING, MODULE_NAME, __FILE__, __LINE__, "One or more flow rule with the same priority...");
								logger(ORCH_WARNING, MODULE_NAME, __FILE__, __LINE__, "Note that, if they match the same port, they may cause a conflict on the vSwitch!");
							}
						}
						catch(exception& e)
						{
							logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "The \"%s\" element does not respect the JSON syntax: \"%s\"", FLOW_RULES, e.what());
							return false;
						}
					}// end  if (fg_name == FLOW_RULES)
					else
					{
						logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Invalid key: %s",bs_name.c_str());
						return false;
					}
				}//End iteration on the elements inside "big-switch"

				if(!foundFlowRules)
				{
					logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Key \"%s\" not found in \"%s\"",FLOW_RULES,FORWARDING_GRAPH);
					return false;
				}


			}//End if(name == FORWARDING_GRAPH)
			else
			{
				logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Invalid key: %s",name.c_str());
				return false;
			}
		}
		if(!foundFlowGraph)
		{
			logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Key \"%s\" not found",FORWARDING_GRAPH);
			return false;
		}
	}catch(exception& e)
	{
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "The content does not respect the JSON syntax: %s",e.what());
		return false;
	}

	//FIXME The number of ports is provided by the name resolver, and should not depend on the flows inserted. In fact,
	//it should be possible to start VNFs without setting flows related to such a function!
    for(map<string,set<unsigned int> >::iterator it = nfs_ports_found.begin(); it != nfs_ports_found.end(); it++)
	{
		set<unsigned int> ports = it->second;
		assert(ports.size() != 0);

		for(set<unsigned int>::iterator p = ports.begin(); p != ports.end(); p++)
		{
			if(!graph.updateNetworkFunction(it->first,*p))
			{
				if(newGraph)
					return false;
				else
				{
					//The message does not describe the current NF into the section "VNFs". However, this NF could be
					//already part of the graph, and in this case the match/action using it is valid. On the contrary,
					//if the NF is no longer part of the graph, there is an error, and the graph cannot be updated.
					if(gm->graphContainsNF(graph.getID(),it->first))
					{
						graph.addNetworkFunction(it->first);
						graph.updateNetworkFunction(it->first,*p);
					}
					else
						return false;
				}
			}
		}

		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "NF \"%s\" requires ports:",it->first.c_str());
		for(set<unsigned int>::iterator p = ports.begin(); p != ports.end(); p++)
			logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "\t%d",*p);
	}

	return true;
}

