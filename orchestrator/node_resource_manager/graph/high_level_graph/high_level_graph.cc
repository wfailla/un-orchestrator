#include "high_level_graph.h"

namespace highlevel
{

Graph::Graph(string ID) :
	ID(ID)
{
}

string Graph::getID()
{
	return ID;
}

void Graph::setName(string name)
{
	this->name = name;
}

string Graph::getName()
{
	return name;
}

bool Graph::addEndPointInterface(EndPointInterface endpoint)
{
	for(list<EndPointInterface>::iterator e = endPointsInterface.begin(); e != endPointsInterface.end(); e++)
	{
		if(*e == endpoint)
			return false;
	}

	endPointsInterface.push_back(endpoint);

	return true;
}

list<EndPointInterface> Graph::getEndPointsInterface()
{
	return endPointsInterface;
}

bool Graph::addEndPointInternal(EndPointInternal endpoint)
{
	for(list<EndPointInternal>::iterator e = endPointsInternal.begin(); e != endPointsInternal.end(); e++)
	{
		if(*e == endpoint)
			return false;
	}

	endPointsInternal.push_back(endpoint);

	endpoints[endpoint.getGroup()] = true;

	return endpoints[endpoint.getGroup()];
}

list<EndPointInternal> Graph::getEndPointsInternal()
{
	return endPointsInternal;
}

bool Graph::addEndPointGre(EndPointGre endpoint)
{
	for(list<EndPointGre>::iterator e = endPointsGre.begin(); e != endPointsGre.end(); e++)
	{
		if(*e == endpoint)
			return false;
	}

	endPointsGre.push_back(endpoint);

	return true;
}

list<EndPointGre> Graph::getEndPointsGre()
{
	return endPointsGre;
}

bool Graph::addEndPointVlan(EndPointVlan endpoint)
{
	for(list<EndPointVlan>::iterator e = endPointsVlan.begin(); e != endPointsVlan.end(); e++)
	{
		if(*e == endpoint)
			return false;
	}

	endPointsVlan.push_back(endpoint);

	return true;
}

list<EndPointVlan> Graph::getEndPointsVlan()
{
	return endPointsVlan;
}

bool Graph::addVNF(VNFs vnf)
{
	for(list<VNFs>::iterator v = vnfs.begin(); v != vnfs.end(); v++)
	{
		if(*v == vnf)
			return false;
	}

	vnfs.push_back(vnf);

	return true;
}

list<VNFs> Graph::getVNFs()
{
	return vnfs;
}

set<string> Graph::getPorts()
{
	return ports;
}

map<string, list<unsigned int> > Graph::getNetworkFunctions()
{
	return networkFunctions;
}

map<string, map<unsigned int, port_network_config > > Graph::getNetworkFunctionsConfiguration()
{
	return networkFunctionsConfiguration;
}

#ifdef ENABLE_UNIFY_PORTS_CONFIGURATION
map<string, list<port_mapping_t > > Graph::getNetworkFunctionsControlPorts()
{
	return networkFunctionsControlPorts;
}

map<string, list<string> > Graph::getNetworkFunctionsEnvironmentVariables()
{
	return networkFunctionsEnvironmentVariables;
}
#endif

list<Rule> Graph::getRules()
{
	return rules;
}

bool Graph::addPort(string port)
{
	if(ports.count(port) != 0)
		return false;

	ports.insert(port);

	return true;
}

bool Graph::addNetworkFunction(string nf)
{
	logger(ORCH_DEBUG, MODULE_NAME, __FILE__, __LINE__, "addNetworkFunction(\"%s\")",nf.c_str());

	if(networkFunctions.count(nf) != 0) {
		logger(ORCH_DEBUG, MODULE_NAME, __FILE__, __LINE__, "addNetworkFunction(\"%s\") NF already present!",nf.c_str());
		return false;
	}

	list<unsigned int> ports;
	networkFunctions[nf] = ports;

	return true;
}

bool Graph::addNetworkFunctionPortConfiguration(string nf, map<unsigned int, port_network_config_t > config)
{
	networkFunctionsConfiguration[nf] = config;

	return true;
}

#ifdef ENABLE_UNIFY_PORTS_CONFIGURATION
void Graph::addNetworkFunctionControlPort(string nf, port_mapping_t control)
{
	networkFunctionsControlPorts[nf].push_back(control);
}

void Graph::addNetworkFunctionEnvironmentVariable(string nf, string env_var)
{
	networkFunctionsEnvironmentVariables[nf].push_back(env_var);
}
#endif

bool Graph::updateNetworkFunction(string nf, unsigned int port)
{
	if(networkFunctions.count(nf) == 0)
	{
		logger(ORCH_WARNING, MODULE_NAME, __FILE__, __LINE__, "NF \"%s\" does not exist",nf.c_str());
		return false;
	}
	list<unsigned int> ports = networkFunctions.find(nf)->second;

	ports.push_back(port);

	networkFunctions[nf] = ports;

	return true;
}

bool Graph::addRule(Rule rule)
{
	for(list<Rule>::iterator r = rules.begin(); r != rules.end(); r++)
	{
		if(*r == rule)
			return false;
	}

	rules.push_back(rule);

	return true;
}

bool Graph::ruleExists(string ID)
{
	for(list<Rule>::iterator r = rules.begin(); r != rules.end(); r++)
	{
		if(r->getFlowID() == ID)
			return true;
	}
	return false;
}

Rule Graph::getRuleFromID(string ID)
{
	list<Rule>::iterator r = rules.begin();

	for(; r != rules.end(); r++)
	{
		if(r->getFlowID() == ID)
			return *r;
	}

	assert(0);

	//This cannot happen; it is just for the compiler.
	return *r;
}

RuleRemovedInfo Graph::removeRuleFromID(string ID)
{
	RuleRemovedInfo rri;

	for(list<Rule>::iterator r = rules.begin(); r != rules.end(); r++)
	{
		if(r->getFlowID() == ID)
		{
			Match match = r->getMatch();
			Action *action = r->getAction();

			action_t actionType = action->getType();
			bool matchOnPort = match.matchOnPort();
			bool matchOnNF = match.matchOnNF();
			bool matchOnEPInternal = match.matchOnEndPointInternal();

			if(actionType == ACTION_ON_PORT)
			{
				//Removed an action on a port. It is possible that a vlink must be removed
				rri.port = ((ActionPort*)action)->getInfo();
				rri.isNFport = false;
				rri.isPort = true;
				rri.isEndpointInternal = false;
				rri.isEndpointGre = false;

				rri.ports.push_back(rri.port);
			}
			else if(actionType == ACTION_ON_NETWORK_FUNCTION)
			{
				//Removed an action on a NF. It is possible that a vlink must be removed
				stringstream nf_port;
				nf_port << ((ActionNetworkFunction*)action)->getInfo() << "_" << ((ActionNetworkFunction*)action)->getPort();
				rri.nf_port = nf_port.str();

				//Potentially, the NF is useless in the graph
				rri.nfs.push_back(((ActionNetworkFunction*)action)->getInfo());
				rri.isNFport = true;
				rri.isPort = false;
				rri.isEndpointInternal = false;
				rri.isEndpointGre = false;
			}
			else if(actionType == ACTION_ON_ENDPOINT_GRE)
			{
				//Removed an action on an endpoint gre
				rri.endpointGre = action->toString();

				rri.isNFport = false;
				rri.isPort = false;
				rri.isEndpointInternal = false;
				rri.isEndpointGre = true;
			}
			else if(actionType == ACTION_ON_ENDPOINT_INTERNAL)
			{
				//Removed an action on an endpoint internal
				rri.endpointInternal = action->toString();

				rri.isNFport = false;
				rri.isPort = false;
				rri.isEndpointInternal = true;
				rri.isEndpointGre = false;
			}

			if(matchOnNF)
				//Potentially, the NF is useless in the graph
				rri.nfs.push_back(match.getNF());
			else if(matchOnPort)
				//Potentially, the port is useless in the graph
				rri.ports.push_back(match.getPhysicalPort());
			else
			{
				stringstream ss;
				ss << match.getEndPoint();
				if(matchOnEPInternal)
					rri.endpointInternal = ss.str();
				else
					rri.endpointGre = ss.str();
			}

			//finally, remove the rule!
			rules.erase(r);

			logger(ORCH_DEBUG, MODULE_NAME, __FILE__, __LINE__, "The graph still contains the rules: ");
			for(list<Rule>::iterator print = rules.begin(); print != rules.end(); print++)
				logger(ORCH_DEBUG, MODULE_NAME, __FILE__, __LINE__, "\t%s",print->getFlowID().c_str());

			return rri;
		}//end if(r->getFlowID() == ID)
	}

	assert(0);

	//Just for the compiler
	return rri;
}

int Graph::getNumberOfRules()
{
	return rules.size();
}

void Graph::print()
{
	if(LOGGING_LEVEL <= ORCH_DEBUG_INFO)
	{
		cout << "Graph :" << endl << "{" << endl;
		for(list<Rule>::iterator r = rules.begin(); r != rules.end(); r++)
			r->print();
		cout << "}" << endl;
	}
}

Object Graph::toJSON()
{
	Object forwarding_graph, big_switch;

	Array flow_rules, end_points, vnf;
	for(list<Rule>::iterator r = rules.begin(); r != rules.end();r++)
	{
		flow_rules.push_back(r->toJSON());
	}

	for(list<EndPointInterface>::iterator e = endPointsInterface.begin(); e != endPointsInterface.end();e++)
	{
		end_points.push_back(e->toJSON());
	}

	for(list<EndPointInternal>::iterator e = endPointsInternal.begin(); e != endPointsInternal.end();e++)
	{
		end_points.push_back(e->toJSON());
	}

	for(list<EndPointGre>::iterator e = endPointsGre.begin(); e != endPointsGre.end();e++)
	{
		end_points.push_back(e->toJSON());
	}

	for(list<EndPointVlan>::iterator e = endPointsVlan.begin(); e != endPointsVlan.end();e++)
	{
		end_points.push_back(e->toJSON());
	}

	for(list<VNFs>::iterator v = vnfs.begin(); v != vnfs.end();v++)
	{
		vnf.push_back(v->toJSON());
	}

	forwarding_graph[_ID] = ID;
	forwarding_graph[_NAME] = name;
	if(end_points.size() != 0)
		forwarding_graph[END_POINTS] = end_points;
	if(vnf.size() != 0)
		forwarding_graph[VNFS] = vnf;
	big_switch[FLOW_RULES] = flow_rules;

	forwarding_graph[BIG_SWITCH] = big_switch;

	return forwarding_graph;
}

bool Graph::stillExistNF(string nf)
{
	if(networkFunctions.count(nf) == 0)
		return false;

	for(list<Rule>::iterator r = rules.begin(); r != rules.end(); r++)
	{
		Match match = r->getMatch();
		Action *action = r->getAction();

		action_t actionType = action->getType();
		bool matchOnNF = match.matchOnNF();

		if(matchOnNF)
		{
			if(match.getNF() == nf)
				//The NF still exists into the graph
				return true;
		}

		if(actionType == ACTION_ON_NETWORK_FUNCTION)
		{
			if(((ActionNetworkFunction*)action)->getInfo() == nf)
				//The NF still exist into the graph
				return true;
		}
	}

	networkFunctions.erase(nf);

	list<VNFs>::iterator vnf = vnfs.begin();
	for(; vnf != vnfs.end(); vnf++)
	{
		if(nf == vnf->getName())
		{
			vnfs.erase(vnf);
			break;
		}
	}

	assert(vnf != vnfs.end());

	return false;
}

bool Graph::stillExistEndpoint(string endpoint)
{
	if(endpoints.count(endpoint) == 0)
		return false;
	return true;

	endpoints.erase(endpoint);
	return false;
}

bool Graph::stillExistEndpointGre(string endpoint)
{
	for(list<highlevel::EndPointGre>::iterator e = endPointsGre.begin(); e != endPointsGre.end(); e++)
	{
		if(e->getId().compare(endpoint) == 0)
			return true;
	}

	return false;
}

bool Graph::stillExistPort(string port)
{
	if(ports.count(port) == 0)
		return false;

	for(list<Rule>::iterator r = rules.begin(); r != rules.end(); r++)
	{
		Match match = r->getMatch();
		Action *action = r->getAction();

		action_t actionType = action->getType();
		bool matchOnPort = match.matchOnPort();

		if(matchOnPort)
		{
			if(match.getPhysicalPort() == port)
				//The port still exists into the graph
				return true;
		}

		if(actionType == ACTION_ON_PORT)
		{
			if(((ActionPort*)action)->getInfo() == port)
				//The port still exist into the graph
				return true;
		}
	}

	ports.erase(port);

	return false;
}

bool Graph::isDefinedHere(string endpoint)
{
	return endpoints[endpoint];
}

string Graph::getEndpointInvolved(string flowID)
{
	highlevel::Rule r = getRuleFromID(flowID);
	highlevel::Match m = r.getMatch();
	highlevel::Action *a = r.getAction();

	if(a->getType() == highlevel::ACTION_ON_ENDPOINT_INTERNAL)
		return a->toString();

	if(m.matchOnEndPointInternal())
	{
		stringstream ss;
		ss << m.getInputEndpoint();
		return ss.str();
	}

	return "";
}

list<highlevel::Rule> Graph::calculateNewRules(Graph *other)
{
	list<highlevel::Rule> new_rules;

	list<highlevel::Rule> other_rules = other->getRules();
	//Iterater over the other rules, and save those that are not part of the current graph
	for(list<highlevel::Rule>::iterator ors = other_rules.begin(); ors != other_rules.end(); ors++)
	{
		list<highlevel::Rule>::iterator r = rules.begin();
		for(; r != rules.end(); r++)
		{
			if((*ors) == (*r))
				//the rule is part of the current graph
				break;
		}
		if(r == rules.end())
			//If we are here, it means that we found a rule that is not part in the current graph
			new_rules.push_back(*ors);
	}
	return new_rules;
}

Graph *Graph::calculateDiff(Graph *other, string graphID)
{

	logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Generating graph diff...");

	Graph *diff = new Graph(graphID);
	
	set<string> new_vnfs_name;

	// a) Add the new rules to "diff"
	
	//Extract the new rules to be instantiated
	list<highlevel::Rule> newrules = this->calculateNewRules(other); //FIXME: put this function directly here?
	//XXX: a similar procedur can be defined to extract the rules to be removed

	for(list<highlevel::Rule>::iterator rule = newrules.begin(); rule != newrules.end(); rule++)
		diff->addRule(*rule);
	
	logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "New rules required: ");
	diff->print();

	// b) Add the new NFs to "diff"

	//Retrieve the NFs already existing in the graph
	highlevel::Graph::t_nfs_ports_list nfs = this->getNetworkFunctions();

	//Retrieve the NFs required by the update (this part is related to the network functions ports)
	highlevel::Graph::t_nfs_ports_list new_nfs = other->getNetworkFunctions();
	for(highlevel::Graph::t_nfs_ports_list::iterator it = new_nfs.begin(); it != new_nfs.end(); it++)
	{
		if(nfs.count(it->first) == 0)
		{
			//The udpdate requires a NF that was not part of the graph
			diff->addNetworkFunction(it->first);
#ifndef UNIFY_NFFG
			//XXX The number of ports of a VNF does not depend on the flows described in the NFFG
			list<unsigned int> ports = it->second;
			for(list<unsigned int>::iterator p = ports.begin(); p != ports.end(); p++)
				diff->updateNetworkFunction(it->first, *p);
#endif
		}
#ifndef UNIFY_NFFG
		//XXX The number of ports of a VNF does not depend on the flows described in the NFFG
		else
		{
			//The NF is already part of the graph, but the update
			//must not contain new ports for the NF
			list<unsigned int> new_ports = it->second;
			list<unsigned int> ports = nfs.find(it->first)->second;
			for(list<unsigned int>::iterator np = new_ports.begin(); np != new_ports.end(); np++)
			{
				list<unsigned int>::iterator p = ports.begin();
				for(; p != ports.end(); p++)
				{
					if(*np == *p)
						break;
				}
				if(p == ports.end())
				{
					logger(ORCH_WARNING, MODULE_NAME, __FILE__, __LINE__, "A new port '%d' is required for NF '%s'", *np, it->first.c_str());
					return NULL;
				}
			}
		}
#endif
	}

	// c) Add the new NFs to "diff"
	//FIXME: why do we have two different representations for the network functions in a graph?

	//Retrieve the network functions already instantiated (and convert them in a set)
	list<highlevel::VNFs> vnfs_already_there = this->getVNFs();
	//Retrieve the network functions required by the update, and their configuration
	list<highlevel::VNFs> new_vnfs_required = other->getVNFs();
	for(list<highlevel::VNFs>::iterator it = new_vnfs_required.begin(); it != new_vnfs_required.end(); it++)
	{
		//Check if this VNF is already in the graph
		bool alreadyThere = false;
		for(list<highlevel::VNFs>::iterator there = vnfs_already_there.begin(); there != vnfs_already_there.end(); there++)
		{
			if((*there) == (*it))
			{
				alreadyThere = true;
				break;
			}
		}
		if(!alreadyThere)
		{
			logger(ORCH_INFO, MODULE_NAME, __FILE__, __LINE__, "A new VNF is required - ID: '%s' - name: '%s'", (it->getId()).c_str(),(it->getName()).c_str());
			diff->addVNF(*it);
			new_vnfs_name.insert(it->getName());
		}
	}
	
	// d) Add the new physical ports

	//Retrieve the ports already existing in the graph
	set<string> ports = this->getPorts();
	//Retrieve the ports required by the update
	set<string> new_ports = other->getPorts();
	for(set<string>::iterator it = new_ports.begin(); it != new_ports.end(); it++)
	{
		if(ports.count(*it) == 0)
		{
			//The update requires a physical port that was not part of the graph
			logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Port %s is added to the graph",(*it).c_str());
			diff->addPort(*it);
		}
		else
			logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Port %s is already in the graph",(*it).c_str());
	}

	// e) Add the new endpoints - This part is quite complex as it considers all the types of endpoints

	//Retrieve the interface endpoints already existing in the graph
	list<highlevel::EndPointInterface> endpointsInterface = this->getEndPointsInterface();
	//Retrieve the interface endpoints required by the update
	list<highlevel::EndPointInterface> new_endpoints_interface = other->getEndPointsInterface();
	for(list<highlevel::EndPointInterface>::iterator nei = new_endpoints_interface.begin(); nei != new_endpoints_interface.end(); nei++)
	{
		bool found = false;
		string it = nei->getInterface();
		
		for(list<highlevel::EndPointInterface>::iterator mitt = endpointsInterface.begin(); mitt != endpointsInterface.end(); mitt++)
		{
			if(mitt->getInterface().compare(it) == 0)
			{
				found = true;
				break;
			}
		}

		if(!found)
		{
			diff->addEndPointInterface(*nei);
			logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Interface endpoint %s is added the to graph",it.c_str());
		}
		else
			logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Interface endpoint %s is already in the graph",it.c_str());
	}

	//Retrieve the gre endpoints already existing in the graph
	list<highlevel::EndPointGre> endpointsGre = this->getEndPointsGre();
	//Retrieve the gre endpoints required by the update
	list<highlevel::EndPointGre> new_endpoints_gre = other->getEndPointsGre();
	for(list<highlevel::EndPointGre>::iterator neg = new_endpoints_gre.begin(); neg != new_endpoints_gre.end(); neg++)
	{
		bool found = false;
		string it = neg->getLocalIp();
		string it1 = neg->getRemoteIp();
		string it2 = neg->getGreKey();

		for(list<highlevel::EndPointGre>::iterator mitt = endpointsGre.begin(); mitt != endpointsGre.end(); mitt++)
		{
			//TODO: use the == on the EndPointGre
			if(mitt->getLocalIp().compare(it) == 0 && mitt->getRemoteIp().compare(it1) == 0 && mitt->getGreKey().compare(it2) == 0)
			{
				found = true;
				break;
			}
		}

		if(!found)
		{
			diff->addEndPointGre(*neg);
			logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "GRE endpoint %s is added to the graph",neg->getId().c_str());
		}
		else
			logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "GRE endpoint %s is already in the graph",neg->getId().c_str());
	}

	//Retrieve the internal endpoints already existing in the graph, in form of strings
	list<highlevel::EndPointInternal> endpointsInternal = this->getEndPointsInternal();
	//Retrieve the internal endpoints required by the update
	list<highlevel::EndPointInternal> new_endpointsInternal = other->getEndPointsInternal();
	for(list<highlevel::EndPointInternal>::iterator it = new_endpointsInternal.begin(); it != new_endpointsInternal.end(); it++)
	{
		if(endpoints.count(it->getGroup()) == 0)
		{
			string tmp_ep = it->getGroup();

			//The internal endpoint is not part of the graph
			diff->addEndPointInternal(*it);
			logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Internal endpoint %s is added to the graph",it->getGroup().c_str());
		}
		else
			logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Internal endpoint %s is already in the graph",it->getGroup().c_str());
	}

	//Retrieve the internal endpoints already existing in the graph
	list<highlevel::EndPointInternal> endpoint_internal = this->getEndPointsInternal();
	//Retrieve the internal endpoints required by the update
	list<highlevel::EndPointInternal> new_endpoints_internal = other->getEndPointsInternal();
	for(list<highlevel::EndPointInternal>::iterator mit = new_endpoints_internal.begin(); mit != new_endpoints_internal.end(); mit++)
	{
		bool found = false;
		string it = mit->getId();

		for(list<highlevel::EndPointInternal>::iterator mitt = endpoint_internal.begin(); mitt != endpoint_internal.end(); mitt++)
		{
			if(mitt->getId().compare(it) == 0)
			{
				found = true;
				break;
			}
		}

		if(!found)
		{
			logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Internal endpoint %s is added to the graph",it.c_str());
			diff->addEndPointInternal(*mit);
		}
		else
			logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Internal endpoint %s is already in the graph",it.c_str());
	}

	//Retrieve the vlan endpoints already existing in the graph
	list<highlevel::EndPointVlan> endpointsVlan = this->getEndPointsVlan();
	//Retrieve the vlan endpoints required by the update
	list<highlevel::EndPointVlan> new_endpoints_vlan = other->getEndPointsVlan();
	for(list<highlevel::EndPointVlan>::iterator mit = new_endpoints_vlan.begin(); mit != new_endpoints_vlan.end(); mit++)
	{
		bool found = false;
		string it = mit->getInterface();
		string it1 = mit->getVlanId();

		for(list<highlevel::EndPointVlan>::iterator mitt = endpointsVlan.begin(); mitt != endpointsVlan.end(); mitt++)
		{
			if(mitt->getInterface().compare(it) == 0 && mitt->getVlanId().compare(it1) == 0)
			{
				found = true;
				break;
			}
		}

		if(!found)
		{
			diff->addEndPointVlan(*mit);
			logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Vlan endpoint %s is added to the graph",mit->getId().c_str());
		}
		else
			logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Vlan endpoint %s is already in the graph",mit->getId().c_str());
	}

	// f) Handle the configuration of the VNF
	
	//Insert only the configuration related to the network functions inserted in "diff"
	//t_nfs_configuration is  the following
	//	< string nf, map<unsigned int, port_network_config_t > >
	t_nfs_configuration other_networkFunctionsConfiguration = other->getNetworkFunctionsConfiguration();
	for(t_nfs_configuration::iterator config = other_networkFunctionsConfiguration.begin(); config != other_networkFunctionsConfiguration.end(); config++)
	{
		if(new_vnfs_name.count(config->first) != 0)
		{
			//This is a new VNF, then we have to add its configuration to "diff"
			diff->addNetworkFunctionPortConfiguration(config->first,config->second);
		}
	}

#ifdef ENABLE_UNIFY_PORTS_CONFIGURATION
	map<string, list<port_mapping_t> > other_NetworkFunctionsControlPorts = other->getNetworkFunctionsControlPorts();
	for(map<string, list<port_mapping_t> >::iterator control = other_NetworkFunctionsControlPorts.begin(); control != other_NetworkFunctionsControlPorts.end(); control++)
	{
		if(new_vnfs_name.count(control->first) != 0)
		{
			//This is a new VNF, then we have to its control information to "diff"
			list<port_mapping_t> control_ports = control->second;
			for(list<port_mapping_t>::iterator cp = control_ports.begin(); cp != control_ports.end(); cp++)
				diff->addNetworkFunctionControlPort(control->first,*cp);
		}
	}
	
	map<string, list<string> > other_NetworkFunctionsEnvironmentVariables = other->getNetworkFunctionsEnvironmentVariables();
	for(map<string, list<string> >::iterator envvar = other_NetworkFunctionsEnvironmentVariables.begin(); envvar != other_NetworkFunctionsEnvironmentVariables.end(); envvar++)
	{
		if(new_vnfs_name.count(envvar->first) != 0)
		{
			//This is a new VNF, then we have to its control information to "diff"
			list<string> environment_variables = envvar->second;
			for(list<string>::iterator ev = environment_variables.begin(); ev != environment_variables.end(); ev++)
				diff->addNetworkFunctionEnvironmentVariable(envvar->first,*ev);
		}
	}
#endif

	return diff;
}

bool Graph::endpointIsUsedInMatch(string endpoint)
{
	for(list<Rule>::iterator r = rules.begin(); r != rules.end(); r++)
	{
		Match match = r->getMatch();
		if(match.matchOnEndPointInternal())
		{
			stringstream ss;
			ss << match.getGraphID() << ":" << match.getEndPoint();
			if(ss.str() == endpoint)
				//The endpoint is used in a match
				return true;
		}
	}
	return false;
}

bool Graph::endpointIsUsedInAction(string endpoint)
{
	for(list<Rule>::iterator r = rules.begin(); r != rules.end(); r++)
	{
		Action *action = r->getAction();
		action_t actionType = action->getType();

		if(actionType == ACTION_ON_ENDPOINT_INTERNAL)
		{
			if(action->toString() == endpoint)
				//The port is used in an action
				return true;
		}
	}
	return false;
}

}
