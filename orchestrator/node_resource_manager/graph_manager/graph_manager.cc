#include "graph_manager.h"

pthread_mutex_t GraphManager::graph_manager_mutex;
uint32_t GraphManager::nextControllerPort = FIRTS_OF_CONTROLLER_PORT;

void GraphManager::mutexInit()
{
	pthread_mutex_init(&graph_manager_mutex, NULL);
}

GraphManager::GraphManager(int core_mask,set<string> physical_ports,string un_address,bool orchestrator_in_band,string un_interface,string ipsec_certificate) :
	un_address(un_address), orchestrator_in_band(orchestrator_in_band), un_interface(un_interface), ipsec_certificate(ipsec_certificate), switchManager()
{
	//Parse the file containing the description of the physical ports to be managed by the node orchestrator
	set<CheckPhysicalPortsIn> phyPortsRequired;
	for(set<string>::iterator pp = physical_ports.begin(); pp != physical_ports.end(); pp++)
	{
		CheckPhysicalPortsIn cppi(*pp);
		phyPortsRequired.insert(cppi);
	}

	set<string> phyPorts;//maps the name into the side
	for(set<CheckPhysicalPortsIn>::iterator pp = phyPortsRequired.begin(); pp != phyPortsRequired.end(); pp++)
		phyPorts.insert(pp->getPortName());

	//Create the openflow controller for the LSI-0

	pthread_mutex_lock(&graph_manager_mutex);
	uint32_t controllerPort = nextControllerPort;
	nextControllerPort++;
	pthread_mutex_unlock(&graph_manager_mutex);

	ostringstream strControllerPort;
	strControllerPort << controllerPort;

	//Create the LSI-0 with all the physical ports required
	logger(ORCH_INFO, MODULE_NAME, __FILE__, __LINE__, "Checking the available physical interfaces...");
	try
	{
		//Check the available physical ports
		switchManager.checkPhysicalInterfaces(phyPortsRequired);
	} catch (...)
	{
		throw GraphManagerException();
	}

	logger(ORCH_INFO, MODULE_NAME, __FILE__, __LINE__, "\t%d physical interfaces under the control of the un-orchestrator:",phyPorts.size());
	for(set<string>::iterator p = phyPorts.begin(); p != phyPorts.end(); p++)
		logger(ORCH_INFO, MODULE_NAME, __FILE__, __LINE__, "\t\t%s",(*p).c_str());

	//Create the openflow controller for the lsi-0
	logger(ORCH_INFO, MODULE_NAME, __FILE__, __LINE__, "Creating the openflow controller for LSI-0...");

	//lowlevel::Graph graph;

	rofl::openflow::cofhello_elem_versionbitmap versionbitmap;
	switch(OFP_VERSION)
	{
		case OFP_10:
			logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "\tUsing Openflow 1.0");
			versionbitmap.add_ofp_version(rofl::openflow10::OFP_VERSION);
			break;
		case OFP_12:
			logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "\tUsing Openflow 1.2");
			versionbitmap.add_ofp_version(rofl::openflow12::OFP_VERSION);
			break;
		case OFP_13:
			logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "\tUsing Openflow 1.3");
			versionbitmap.add_ofp_version(rofl::openflow13::OFP_VERSION);
			break;
	}

	Controller *controller = new Controller(versionbitmap,graph,strControllerPort.str());
	controller->start();

	logger(ORCH_INFO, MODULE_NAME, __FILE__, __LINE__, "Creating the LSI-0...");

	//The three following structures are empty. No NF and no virtual link is attached.
	map<string, list<unsigned int> > dummy_network_functions;
	map<string, map<unsigned int, PortType> > dummy_nfs_ports_type;
	list<highlevel::EndPointGre> dummy_endpoints;
	vector<VLink> dummy_virtual_links;
	map<string,nf_t>  nf_types;

	unsigned int i = 0;

	LSI *lsi = new LSI(string(OF_CONTROLLER_ADDRESS), strControllerPort.str(), phyPorts, dummy_network_functions,
	dummy_endpoints,dummy_virtual_links,dummy_nfs_ports_type);

	try
	{
		//Create a new LSI, which is the LSI-0 of the node
		map<string, vector<string> > endpoints;
		if(lsi->getEndpointsPorts().size() != 0)
		{
			for(list<highlevel::EndPointGre>::iterator e = lsi->getEndpointsPorts().begin(); e != lsi->getEndpointsPorts().end(); e++)
			{
				endpoints[e->getId()][0] = e->getGreKey();
				endpoints[e->getId()][1] = e->getLocalIp();
				endpoints[e->getId()][2] = e->getRemoteIp();
				endpoints[e->getId()][3] = un_interface;
			}
		}

		map<string,list<nf_port_info> > netFunctionsPortsInfo;
		CreateLsiIn cli(string(OF_CONTROLLER_ADDRESS),strControllerPort.str(),lsi->getPhysicalPortsName(),nf_types,netFunctionsPortsInfo,endpoints,lsi->getVirtualLinksRemoteLSI(), this->un_address, this->ipsec_certificate);

		CreateLsiOut *clo = switchManager.createLsi(cli);

		lsi->setDpid(clo->getDpid());
		map<string,unsigned int> physicalPorts = clo->getPhysicalPorts();
		//TODO check that the physical ports returned are the same provided to the switch manager
		for(map<string,unsigned int>::iterator it = physicalPorts.begin(); it != physicalPorts.end(); it++)
		{
			if(!lsi->setPhysicalPortID(it->first,it->second))
			{
				logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "An unknown physical interface \"%s\" has been attached to the lsi-0",it->first.c_str());
				delete(clo);
				throw GraphManagerException();
			}
		}

		map<string,map<string, unsigned int> > nfsports = clo->getNetworkFunctionsPorts();
		if(!nfsports.empty())
		{
			logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "Non required NFs ports have been attached to the lsi-0");
			delete(clo);
			throw GraphManagerException();
		}

		map<string, unsigned int> epsports = clo->getEndpointsPorts();
		if(!epsports.empty())
		{
			logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "Non required endpoints ports have been attached to the lsi-0");
			delete(clo);
			throw GraphManagerException();
		}

		list<pair<unsigned int, unsigned int> > vl = clo->getVirtualLinks();
		if(!vl.empty())
		{
			logger(ORCH_WARNING, MODULE_NAME, __FILE__, __LINE__, "Non required connections have been created between the lsi-0 and other(?) lsis");
			delete(clo);
			throw GraphManagerException();
		}

		delete(clo);
	} catch (SwitchManagerException e)
	{
		logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "%s",e.what());
		throw GraphManagerException();
	}

	dpid0 = lsi->getDpid();
	map<string,unsigned int> lsi_ports = lsi->getPhysicalPorts();

	logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "LSI ID: %d",dpid0);
	logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Physical ports:",lsi_ports.size());
	for(map<string,unsigned int>::iterator p = lsi_ports.begin(); p != lsi_ports.end(); p++)
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "\t%s -> %d",(p->first).c_str(),p->second);

	graphInfoLSI0.setLSI(lsi);
	graphInfoLSI0.setController(controller);

	logger(ORCH_INFO, MODULE_NAME, __FILE__, __LINE__, "LSI-0 and its controller are created");

	ComputeController::setCoreMask(core_mask);

	//if control is in band install the default rules on LSI-0 otherwise skip this code
	if(orchestrator_in_band && !un_interface.empty() && !un_address.empty())
	{
		//remove first " character
		un_interface.erase(0,1);
		//remove last " character
		un_interface.erase(un_interface.size()-1,1);

		//Install the default rules on LSI-0
		lowlevel::Match lsi0Match, lsi0Match0, lsi0Match1, lsi0Match2;
		if(lsi_ports.count((char *)un_interface.c_str()) == 0)
		{
			logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "Control interface does not exist in a list of available plysical ports.");
			throw GraphManagerException();
		}

		map<string,unsigned int>::iterator translation = lsi_ports.find((char *)un_interface.c_str());

		/* It is necessary to intercept incoming arp requests with IP source equal to un_interface?

		lsi0Match.setArpSpa((char *)un_address.c_str());
		lsi0Match.setEthType(2054 & 0xFFFF);
		lsi0Match.setInputPort(translation->second);


		//Create the rule and add it to the graph
		//The rule ID is created as follows DEFAULT-GRAPH_ID
		newRuleID << DEFAULT_GRAPH << "_" << i;
		lowlevel::Rule lsi0Rule(lsi0Match,lsi0Action,newRuleID.str(),HIGH_PRIORITY);
		graphLSI0lowLevel.addRule(lsi0Rule);

		i++;
		*/

		lsi0Match0.setArpTpa((char *)un_address.c_str());
		lsi0Match0.setEthType(2054 & 0xFFFF);
		lsi0Match0.setInputPort(translation->second);

		lsi0Match1.setIpv4Dst((char *)un_address.c_str());
		lsi0Match1.setEthType(2048 & 0xFFFF);
		lsi0Match1.setInputPort(translation->second);

		lowlevel::Action lsi0Action(true);

		//Create the rule and add it to the graph
		//The rule ID is created as follows DEFAULT-GRAPH_ID
		stringstream newRuleID;
		newRuleID << DEFAULT_GRAPH << "_" << i;
		lowlevel::Rule lsi0Rule0(lsi0Match0,lsi0Action,newRuleID.str(),HIGH_PRIORITY);
		graphLSI0lowLevel.addRule(lsi0Rule0);

		i++;

		//Create the rule and add it to the graph
		//The rule ID is created as follows DEFAULT-GRAPH_ID
		newRuleID.str("");
		newRuleID << DEFAULT_GRAPH << "_" << i;
		lowlevel::Rule lsi0Rule1(lsi0Match1,lsi0Action,newRuleID.str(),HIGH_PRIORITY);
		graphLSI0lowLevel.addRule(lsi0Rule1);

		i++;

		lowlevel::Match lsi0Match3(true);

		lowlevel::Action lsi0Action1(translation->second);

		//Create the rule and add it to the graph
		//The rule ID is created as follows DEFAULT-GRAPH_ID
		newRuleID.str("");
		newRuleID << DEFAULT_GRAPH << "_" << i;
		lowlevel::Rule lsi0Rule3(lsi0Match3,lsi0Action1,newRuleID.str(),HIGH_PRIORITY);
		graphLSI0lowLevel.addRule(lsi0Rule3);

		graphLSI0lowLevel.print();

		//Insert new rules into the LSI-0
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Adding the new rules to the LSI-0");
		controller->installNewRules(graphLSI0lowLevel.getRules());

		printInfo(graphLSI0lowLevel,graphInfoLSI0.getLSI());
	}

}

GraphManager::~GraphManager()
{
	//Deleting tenants LSIs
	for(map<string,GraphInfo>::iterator lsi = tenantLSIs.begin(); lsi != tenantLSIs.end();)
	{
		map<string,GraphInfo>::iterator tmp = lsi;
		lsi++;
		try
		{
			deleteGraph(tmp->first, true);
		}catch(...)
		{
			assert(0);
			/*nothing to do, since the node orchestrator is terminating*/
		}
	}

	//Deleting LSI-0
	logger(ORCH_INFO, MODULE_NAME, __FILE__, __LINE__, "Deleting the graph for the LSI-0...");
	LSI *lsi0 = graphInfoLSI0.getLSI();

	try
	{
		switchManager.destroyLsi(lsi0->getDpid());
	} catch (SwitchManagerException e)
	{
		logger(ORCH_WARNING, MODULE_NAME, __FILE__, __LINE__, "%s",e.what());
		//we don't throw any exception here, since the graph manager is terminating
	}

	Controller *controller = graphInfoLSI0.getController();
	delete(controller);
	controller = NULL;
}

bool GraphManager::graphExists(string graphID)
{
	if(tenantLSIs.count(graphID) == 0)
		return false;

	return true;
}

bool GraphManager::graphContainsNF(string graphID,string nf)
{
	if(!graphExists(graphID))
		return false;

	GraphInfo graphInfo = (tenantLSIs.find(graphID))->second;
	highlevel::Graph *graph = graphInfo.getGraph();

	return graph->stillExistNF(nf);
}

bool GraphManager::flowExists(string graphID, string flowID)
{
	assert(tenantLSIs.count(graphID) != 0);

	GraphInfo graphInfo = (tenantLSIs.find(graphID))->second;
	highlevel::Graph *graph = graphInfo.getGraph();

	if(!graph->ruleExists(flowID))
		return false;

	return true;
}

Object GraphManager::toJSON(string graphID)
{
	if(tenantLSIs.count(graphID) == 0)
	{
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "The graph \"%s\" does not exist",graphID.c_str());
		assert(0);
		throw GraphManagerException();
	}
	highlevel::Graph *graph = (tenantLSIs.find(graphID))->second.getGraph();
	assert(graph != NULL);

	Object flow_graph;

	try
	{
		flow_graph[FORWARDING_GRAPH] = graph->toJSON();
	}catch(...)
	{
		assert(0);
		throw GraphManagerException();
	}

	return flow_graph;
}

bool GraphManager::deleteGraph(string graphID, bool shutdown)
{
	if(tenantLSIs.count(graphID) == 0)
	{
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "The graph \"%s\" does not exist",graphID.c_str());
		return false;
	}

	/**
	*	@outline:
	*
	*		0) check if the graph can be removed
	*		1) remove the rules from the LSI0
	*		2) stop the NFs
	*		3) delete the LSI, the virtual links and the
	*			ports related to NFs
	*		4) delete the endpoints defined by the graph
	*/

	logger(ORCH_INFO, MODULE_NAME, __FILE__, __LINE__, "Deleting graph '%s'...",graphID.c_str());

	LSI *tenantLSI = (tenantLSIs.find(graphID))->second.getLSI();
	highlevel::Graph *highLevelGraph = (tenantLSIs.find(graphID))->second.getGraph();

	/**
	*		0) check if the graph can be removed
	*/
	if(!shutdown)
	{
		list<highlevel::EndPointInternal> endpointsInternal = highLevelGraph->getEndPointsInternal();
		for(list<highlevel::EndPointInternal>::iterator ep = endpointsInternal.begin(); ep != endpointsInternal.end(); ep++)
		{
			if(highLevelGraph->isDefinedHere(ep->getGroup()))
			{
				if(availableEndPoints.find(ep->getGroup())->second !=0)
				{
					logger(ORCH_INFO, MODULE_NAME, __FILE__, __LINE__, "The graph cannot be deleted. It defines the endpoint \"%s\" that is used %d times in other graphs; first remove the rules in those graphs.",ep->getGroup().c_str(),availableEndPoints.find(ep->getGroup())->second);
					return false;
				}
			}
		}
	}

	/**
	*		1) remove the rules from the LSI-0
	*/
	logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "1) Remove the rules from the LSI-0");

	lowlevel::Graph graphLSI0 = GraphTranslator::lowerGraphToLSI0(highLevelGraph,tenantLSI,graphInfoLSI0.getLSI(),endPointsDefinedInMatches,endPointsDefinedInActions,availableEndPoints,un_interface,orchestrator_in_band,false);
	graphLSI0lowLevel.removeRules(graphLSI0.getRules());

	//Remove rules from the LSI-0
	graphInfoLSI0.getController()->removeRules(graphLSI0.getRules());

	/**
	*		2) stop the NFs
	*/
	ComputeController *computeController = (tenantLSIs.find(graphID))->second.getComputeController();
#ifdef RUN_NFS
	logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "2) Stop the NFs");
	computeController->stopAll();
#else
	logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "2) Flag RUN_NFS disabled. No NF to be stopped");
#endif

	/**
	*		3) delete the LSI, the virtual links, the
	*			ports related to NFs and the ports related to GRE tunnel
	*/
	logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "3) Delete the LSI, the vlinks, and the ports used by NFs");

	try
	{
		switchManager.destroyLsi(tenantLSI->getDpid());
	} catch (SwitchManagerException e)
	{
		logger(ORCH_WARNING, MODULE_NAME, __FILE__, __LINE__, "%s",e.what());
		throw GraphManagerException();
	}

	/**
	*		4) delete the endpoints defined by the graph
	*/
	if(!shutdown)
	{
		list<highlevel::EndPointInternal> endpointsInternal = highLevelGraph->getEndPointsInternal();
		for(list<highlevel::EndPointInternal>::iterator ep = endpointsInternal.begin(); ep != endpointsInternal.end();)
		{
			if(highLevelGraph->isDefinedHere(ep->getGroup()))
			{
				assert(availableEndPoints.find(ep->getGroup())->second ==0);
				assert(endPointsDefinedInMatches.count(ep->getGroup()) != 0 || endPointsDefinedInActions.count(ep->getGroup()) != 0);

				list<highlevel::EndPointInternal>::iterator tmp = ep;
				ep++;

				availableEndPoints.erase(tmp->getGroup());
				if(endPointsDefinedInActions.count(tmp->getGroup()) != 0)
					endPointsDefinedInActions.erase(tmp->getGroup());
				if(endPointsDefinedInMatches.count(tmp->getGroup()) != 0)
					endPointsDefinedInMatches.erase(tmp->getGroup());

				logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "The endpoint \"%s\" is no longer available",tmp->getGroup().c_str());
			}
			else
				ep++;
		}
	}

	tenantLSIs.erase(tenantLSIs.find(highLevelGraph->getID()));

	delete(highLevelGraph);
	delete(tenantLSI);
	delete(computeController);

	highLevelGraph = NULL;
	tenantLSI = NULL;
	computeController = NULL;

	logger(ORCH_INFO, MODULE_NAME, __FILE__, __LINE__, "Tenant LSI (ID: %s) and its controller have been destroyed!",graphID.c_str());
	printInfo(graphLSI0lowLevel,graphInfoLSI0.getLSI());

	return true;
}

bool GraphManager::deleteFlow(string graphID, string flowID)
{
	if(tenantLSIs.count(graphID) == 0)
	{
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "The graph \"%s\" does not exist",graphID.c_str());
		assert(0);
		return false;
	}

	GraphInfo graphInfo = (tenantLSIs.find(graphID))->second;
	highlevel::Graph *graph = graphInfo.getGraph();

	if(!graph->ruleExists(flowID))
	{
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "The flow \"%s\" does not exist in graph \"%s\"",flowID.c_str(),graphID.c_str());
		assert(0);
		return false;
	}

	//if the graph has only this flow, remove the entire graph
	if(graph->getNumberOfRules() == 1)
	{
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "The graph \"%s\" has only one flow. Then the entire graph will be removed",graphID.c_str());
		return deleteGraph(graphID);
	}

	/**
	*	The flow can be removed only if does not define an endpoint used by some other graph
	*/
#if 0
	if(!canDeleteFlow(graph,flowID))
		return false;
#endif
	string endpointInvolved = graph->getEndpointInvolved(flowID);
	bool definedHere = false;
	if(endpointInvolved != "")
		definedHere = graph->isDefinedHere(endpointInvolved);

	logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Removing the flow from the LSI-0 graph");
	Controller *lsi0Controller = graphInfoLSI0.getController();
	stringstream lsi0FlowID;
	lsi0FlowID << graph->getID() << "_" << flowID;
	lsi0Controller->removeRuleFromID(lsi0FlowID.str());
	graphLSI0lowLevel.removeRuleFromID(lsi0FlowID.str());

	logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Removing the flow from the tenant-LSI graph");
	Controller *tenantController = graphInfo.getController();
	tenantController->removeRuleFromID(flowID);

	logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Removing the flow from the high level graph");
	RuleRemovedInfo rri = graph->removeRuleFromID(flowID);

	ComputeController *nfs_manager = graphInfo.getComputeController();
	LSI *lsi = graphInfo.getLSI();

	removeUselessPorts_NFs_Endpoints_VirtualLinks(rri,nfs_manager,graph,lsi);

	if(endpointInvolved != "")
	{
		if(definedHere)
		{
			availableEndPoints.erase(endpointInvolved);
			if(endPointsDefinedInActions.count(endpointInvolved) != 0)
				endPointsDefinedInActions.erase(endpointInvolved);
			if(endPointsDefinedInMatches.count(endpointInvolved) != 0)
				endPointsDefinedInMatches.erase(endpointInvolved);
		}
		else
		{
			availableEndPoints[endpointInvolved]--;
			logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "endpoint \"%s\" still used %d times",endpointInvolved.c_str(), availableEndPoints[endpointInvolved]);
		}
	}

	printInfo(graphLSI0lowLevel,graphInfoLSI0.getLSI());

	return true;
}

bool GraphManager::checkGraphValidity(highlevel::Graph *graph, ComputeController *computeController)
{
	set<string> phyPorts = graph->getPorts();
	list<highlevel::EndPointInternal> endPointsInternal = graph->getEndPointsInternal();
	list<highlevel::EndPointGre> endPointsGre = graph->getEndPointsGre();
	list<highlevel::EndPointVlan> endPointsVlan = graph->getEndPointsVlan();

	string graphID = graph->getID();

	logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "The command requires %d new physical ports",phyPorts.size());

	LSI *lsi0 = graphInfoLSI0.getLSI();
	map<string,unsigned int> physicalPorts = lsi0->getPhysicalPorts();

	for(set<string>::iterator p = phyPorts.begin(); p != phyPorts.end(); p++)
	{
		if((physicalPorts.count(*p)) == 0)
		{
			logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Physical port \"%s\" does not exist",(*p).c_str());
			return false;
		}
	}

	logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "The command requires %d internal endpoints (i.e., logical ports to be used to connect two graphs together)",endPointsInternal.size());

	for(list<highlevel::EndPointInternal>::iterator graphEP = endPointsInternal.begin(); graphEP != endPointsInternal.end(); graphEP++)
	{
		if(!graph->isDefinedHere(graphEP->getGroup()))
		{
			//since this internal endpoint is defined into another graph, that internal endpoint must already exist
			if(availableEndPoints.count(graphEP->getGroup()) == 0)
			{
				logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Internal endpoint \"%s\" is not defined by the current graph, and it does not exist yet",graphEP->getGroup().c_str());
				return false;
			}

			if(graph->endpointIsUsedInMatch(graphEP->getGroup()))
			{
				//Another graph must have been defined it in an action
				if(endPointsDefinedInActions.count(graphEP->getGroup()) == 0)
				{
					logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Internal endpoint \"%s\" is used in a match of the current graph, but it was not defined in an action of another graph",graphEP->getGroup().c_str());
					return false;
				}
			}
			if(graph->endpointIsUsedInAction(graphEP->getGroup()))
			{
				//Another graph must have been defined it in a match
				if(endPointsDefinedInMatches.count(graphEP->getGroup()) == 0)
				{
					logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Internal endpoint \"%s\" is used in an action of the current graph, but it was not defined in a match of another graph",graphEP->getGroup().c_str());
					return false;
				}
			}
		}
	}

	logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "The command requires %d gre endpoints (i.e., gre ports to be used to connect two nodes together)",endPointsGre.size());

	logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "The command requires %d vlan endpoints",endPointsVlan.size());

	map<string,list<unsigned int> > network_functions = graph->getNetworkFunctionsPorts();
	logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "The command requires to retrieve %d new NFs",network_functions.size());
	for(highlevel::Graph::t_nfs_ports_list::iterator nf = network_functions.begin(); nf != network_functions.end(); nf++)
	{
		nf_manager_ret_t retVal = computeController->retrieveDescription(nf->first);

		if(retVal == NFManager_NO_NF)
		{
			logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "NF \"%s\" cannot be retrieved",nf->first.c_str());
			return false;
		}
		else if(retVal == NFManager_SERVER_ERROR)
		{
			throw GraphManagerException();
		}
	}

	return true;
}

void *startNF(void *arguments)
{
    to_thread_t *args = (to_thread_t *)arguments;
    assert(args->computeController != NULL);

    if(!args->computeController->startNF(args->nf_name, args->namesOfPortsOnTheSwitch, args->portsConfiguration
#ifdef ENABLE_UNIFY_PORTS_CONFIGURATION
	    , args->controlConfiguration, args->environmentVariables
#endif
	))
    	return (void*) 0;
    else
    	return (void*) 1;
}


bool GraphManager::newGraph(highlevel::Graph *graph)
{
	logger(ORCH_INFO, MODULE_NAME, __FILE__, __LINE__, "Creating a new graph '%s'...",graph->getID().c_str());

	assert(tenantLSIs.count(graph->getID()) == 0);

	/**
	*	@outline:
	*
	*		0) check the validity of the graph
	*		1) create the Openflow controller for the tenant LSI
	*		2) select an implementation for each NF of the graph
	*		3) create the LSI, with the proper ports
	*		4) start the NFs
	*		5) download the rules in LSI-0 and tenant-LSI
	*/

	/**
	*	0) Check the validity of the graph
	*/
	logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "0) Check the validity of the graph");

	ComputeController *computeController = new ComputeController();

	if(!checkGraphValidity(graph,computeController))
	{
		//This is an error in the request
		delete(computeController);
		computeController = NULL;
		return false;
	}

	/**
	*	1) Create the Openflow controller for the tenant LSI
	*/
	logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "1) Create the Openflow controller for the tenant LSI");

	pthread_mutex_lock(&graph_manager_mutex);
	uint32_t controllerPort = nextControllerPort;
	nextControllerPort++;
	pthread_mutex_unlock(&graph_manager_mutex);

	ostringstream strControllerPort;
	strControllerPort << controllerPort;

	rofl::openflow::cofhello_elem_versionbitmap versionbitmap;
	switch(OFP_VERSION)
	{
		case OFP_10:
			logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "\tUsing Openflow 1.0");
			versionbitmap.add_ofp_version(rofl::openflow10::OFP_VERSION);
			break;
		case OFP_12:
			logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "\tUsing Openflow 1.2");
			versionbitmap.add_ofp_version(rofl::openflow12::OFP_VERSION);
			break;
		case OFP_13:
			logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "\tUsing Openflow 1.3");
			versionbitmap.add_ofp_version(rofl::openflow13::OFP_VERSION);
			break;
	}

	lowlevel::Graph graphTmp ;
	Controller *controller = new Controller(versionbitmap,graphTmp,strControllerPort.str());
	controller->start();

	/**
	*	2) Select an implementation for each network function of the graph
	*/
	logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "2) Select an implementation for each NF of the graph");
	if(!computeController->selectImplementation())
	{
		//This is an internal error
		delete(computeController);
		delete(controller);
		computeController = NULL;
		controller = NULL;
		throw GraphManagerException();
	}

	/**
	*	3) Create the LSI
	*/
	logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "3) Create the LSI");

	set<string> phyPorts = graph->getPorts();

	map<string, list<unsigned int> > network_functions = graph->getNetworkFunctionsPorts();
	map<string, map<unsigned int, port_network_config > > network_functions_ports_configuration = graph->getNetworkFunctionsConfiguration();
#ifdef ENABLE_UNIFY_PORTS_CONFIGURATION
	map<string, list<port_mapping_t> > network_functions_control_configuration = graph->getNetworkFunctionsControlPorts();
	map<string, list<string> > network_functions_environment_variables = graph->getNetworkFunctionsEnvironmentVariables();
#endif
	list<highlevel::EndPointInternal> endpointsInternal = graph->getEndPointsInternal();
	list<highlevel::EndPointGre> endpointsGre = graph->getEndPointsGre();

	vector<set<string> > vlVector = identifyVirtualLinksRequired(graph);
	set<string> vlNFs = vlVector[0];
	set<string> vlPhyPorts = vlVector[1];
	set<string> vlEndPointsGre = vlVector[2];
	set<string> vlEndPointsInternal = vlVector[3];
	set<string> NFsFromEndPoint = vlVector[4];
	set<string> GREsFromEndPoint = vlVector[5];

	/**
	*	A virtual link can be used in two direction, hence it can be shared between a NF port and a physical port.
	*	In principle a virtual link could also be shared between a NF port and an endpoint but, for simplicity, we
	*	use separated virtual links in case of endpoint.
	*/
	//FIXME: this is completely wrong! It does not work with a simple graph as the following:
	// pri: 100 - eth0 & ip.src==1.1.1.1 -> VNF
	// pri: 10  - eth0 & ip.src==1.1.1.2 -> gre
	// In fact, according to the rules below, this would require a single virtual link. Instead, two vlinks are required toward the tenant-lsi! One
	// that brings traffic to the VNF, and one that bring traffic into the tunnel!
	unsigned int numberOfVLrequiredBeforeEndPoints = /*(vlNFs.size() > vlPhyPorts.size())? vlNFs.size() : vlPhyPorts.size();*/(vlNFs.size() > ((vlPhyPorts.size() > vlEndPointsGre.size()) ? vlPhyPorts.size():vlEndPointsGre.size())) ? vlNFs.size():((vlPhyPorts.size() > vlEndPointsGre.size()) ? vlPhyPorts.size():vlEndPointsGre.size());

	unsigned int numberOfVLrequired = numberOfVLrequiredBeforeEndPoints + vlEndPointsInternal.size()/* + vlEndPointsGre.size()*/;

	logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "%d virtual links are required to connect the new LSI with LSI-0",numberOfVLrequired);

	vector<VLink> virtual_links;
	for(unsigned int i = 0; i < numberOfVLrequired; i++)
		virtual_links.push_back(VLink(dpid0));

	//The tenant-LSI is not connected to physical ports, but just the LSI-0
	//through virtual links, and to network functions through virtual ports
	set<string> dummyPhyPorts;

	map<string, nf_t>  nf_types;
	map<string, map<unsigned int, PortType> > nfs_ports_type;  // nf_name -> map( port_id -> port_type )
	for(highlevel::Graph::t_nfs_ports_list::iterator nf_it = network_functions.begin(); nf_it != network_functions.end(); nf_it++) {
		const string& nf_name = nf_it->first;
		list<unsigned int>& nf_ports = nf_it->second;

		nf_types[nf_name] = computeController->getNFType(nf_name);

		//Gather VNF ports types
		const Description* descr = computeController->getNFSelectedImplementation(nf_name);
		map<unsigned int, PortType> nf_ports_type = descr->getPortTypes();  // Port types as specified by the retrieved and selected NF implementation

		if (nf_ports_type.size() != nf_ports.size())
			logger(ORCH_WARNING, MODULE_NAME, __FILE__, __LINE__, "Number of ports from (%d) graph does not match number of ports from NF description (%d) for \"%s\"",nf_ports.size(),nf_ports_type.size(), nf_name.c_str());

		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "NF \"%s\" selected implementation (type %d) defines type for %d ports", nf_name.c_str(), nf_types[nf_name], nf_ports_type.size());
		// Fill in incomplete port type specifications (unless we make it mandatory input from name-resolver)
		for (list<unsigned int>::iterator p_it = nf_ports.begin(); p_it != nf_ports.end(); p_it++) {
			map<unsigned int, PortType>::iterator pt_it = nf_ports_type.find(*p_it);
			if (pt_it == nf_ports_type.end()) {
				logger(ORCH_WARNING, MODULE_NAME, __FILE__, __LINE__, "\tNF Port \"%s\":%d has no type defined in NF description", nf_name.c_str(), (*p_it));
				logger(ORCH_WARNING, MODULE_NAME, __FILE__, __LINE__, "\tThe ports ID used in the graph must correspond to those specified in the name resolver...");
				//This is an error of the client, which specified a wrong NF-FG (wrong ports towards a VNF)
				delete(computeController);
				delete(controller);
				computeController = NULL;
				controller = NULL;
				return false;
			}
			logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "\tNF Port \"%s\":%d is of type '%s'", nf_name.c_str(), (*p_it), portTypeToString(pt_it->second).c_str());
		}
		nfs_ports_type[nf_name] = nf_ports_type;
	}

	//Prepare the structure representing the new tenant-LSI
	LSI *lsi = new LSI(string(OF_CONTROLLER_ADDRESS), strControllerPort.str(), dummyPhyPorts, network_functions,
		endpointsGre,virtual_links,nfs_ports_type);

	CreateLsiOut *clo = NULL;
	try
	{
		//Create a new tenant-LSI
		map<string, vector<string> > endpoints;
		list<highlevel::EndPointGre> endpoints_gre = lsi->getEndpointsPorts();
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "%d GRE endpoints must be created",endpoints_gre.size());
		if(endpoints_gre.size() != 0)
		{
			vector<string> v_ep(5);
			string iface;
			for(list<highlevel::EndPointGre>::iterator e = endpoints_gre.begin(); e != endpoints_gre.end(); e++)
			{
				iface.assign(e->getId());
				v_ep[0].assign(e->getGreKey());
				v_ep[1].assign(e->getLocalIp());
				v_ep[2].assign(e->getRemoteIp());
				v_ep[3].assign(un_interface);
				if(e->isSafe())
					v_ep[4].assign("true");
				else
					v_ep[4].assign("false");

				endpoints[iface] = v_ep;
				
				logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "\tkey: %s - local IP: %s - remote IP: %s - iface: %s",(e->getGreKey()).c_str(),(e->getLocalIp()).c_str(),(e->getRemoteIp()).c_str(),iface.c_str());
			}

		}

		assert(endpoints.size() == endpoints_gre.size());

		map<string,list<string> > netFunctionsPortsName;
		for(map<string, list<unsigned int> >::iterator nf = network_functions.begin(); nf != network_functions.end(); nf++)
		{
			netFunctionsPortsName[nf->first] = lsi->getNetworkFunctionsPortNames(nf->first);
		}
		CreateLsiIn cli(string(OF_CONTROLLER_ADDRESS),strControllerPort.str(), lsi->getPhysicalPortsName(), nf_types, lsi->getNetworkFunctionsPortsInfo(), endpoints, lsi->getVirtualLinksRemoteLSI(), string(OF_CONTROLLER_ADDRESS), this->ipsec_certificate);

		clo = switchManager.createLsi(cli);

		lsi->setDpid(clo->getDpid());

		map<string,unsigned int> physicalPorts = clo->getPhysicalPorts();
		if(physicalPorts.size() > 1)
		{
			logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "Non required physical ports have been attached to the tenant-lsi");
			delete(clo);
			throw GraphManagerException();
		}

		map<string,map<string, unsigned int> > nfsports = clo->getNetworkFunctionsPorts();
		//TODO: check if the number of vnfs and ports is the same required
		for(map<string,map<string, unsigned int> >::iterator nfp = nfsports.begin(); nfp != nfsports.end(); nfp++)
		{
			if(!lsi->setNfSwitchPortsID(nfp->first,nfp->second))
			{
				logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "A non-required network function port related to the network function \"%s\" has been attached to the tenant-lsi",nfp->first.c_str());
				delete(clo);
				throw GraphManagerException();
			}
		}

		map<string,unsigned int > epsports = clo->getEndpointsPorts();
		for(map<string,unsigned int >::iterator ep = epsports.begin(); ep != epsports.end(); ep++)
		{
			if(!lsi->setEndpointPortID(ep->first,ep->second))
			{
				logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "A non-required endpoint port \"%s\" has been attached to the tenant-lsi",ep->first.c_str());
				delete(clo);
				throw GraphManagerException();
			}
		}

		map<string,list<string> > networkFunctionsPortsNameOnSwitch = clo->getNetworkFunctionsPortsNameOnSwitch();

		for(map<string,list<string> >::iterator nfpnos = networkFunctionsPortsNameOnSwitch.begin(); nfpnos != networkFunctionsPortsNameOnSwitch.end(); nfpnos++)
			lsi->setNetworkFunctionsPortsNameOnSwitch(nfpnos->first, nfpnos->second);

		list<pair<unsigned int, unsigned int> > vl = clo->getVirtualLinks();
		//TODO: check if the number of vlinks is the same required
		unsigned int currentTranslation = 0;
		for(list<pair<unsigned int, unsigned int> >::iterator it = vl.begin(); it != vl.end(); it++)
		{
			lsi->setVLinkIDs(currentTranslation,it->first,it->second);
			currentTranslation++;
		}
		delete(clo);

	} catch (SwitchManagerException e)
	{
		logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "%s",e.what());
		switchManager.destroyLsi(lsi->getDpid());
		if(clo != NULL)
			delete(clo);
		delete(graph);
		delete(lsi);
		delete(computeController);
		delete(controller);
		graph = NULL;
		lsi = NULL;
		computeController = NULL;
		controller = NULL;
		throw GraphManagerException();
	}

	uint64_t dpid = lsi->getDpid();

	map<string,unsigned int> lsi_ports = lsi->getPhysicalPorts();
	set<string> nfs = lsi->getNetworkFunctionsName();
	list<highlevel::EndPointGre > eps = lsi->getEndpointsPorts();
	vector<VLink> vls = lsi->getVirtualLinks();

	logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "LSI ID: %d",dpid);
#if 0
	logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Ports (%d):",lsi_ports.size());
	for(map<string,unsigned int>::iterator p = lsi_ports.begin(); p != lsi_ports.end(); p++)
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "\t%s -> %d",(p->first).c_str(),p->second);
#endif
	logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Network functions (%d):",nfs.size());
	for(set<string>::iterator it = nfs.begin(); it != nfs.end(); it++)
	{
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "\t\tNF %s:",it->c_str());

#ifdef ENABLE_UNIFY_PORTS_CONFIGURATION
		if(network_functions_control_configuration.count(*it) != 0)
		{
			list<port_mapping_t > nfs_control_configuration = network_functions_control_configuration[*it];

			logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "\t\t\tControl interfaces (%d):",nfs_control_configuration.size());
			for(list<port_mapping_t >::iterator n = nfs_control_configuration.begin(); n != nfs_control_configuration.end(); n++)
			{
				logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "\t\t\t\tHost TCP port -> %s",(n->host_port).c_str());
				logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "\t\t\t\tVNF TCP port -> %s",(n->guest_port).c_str());
			}
		}

		if(network_functions_environment_variables.count(*it) != 0)
		{
			list<string> nfs_environment_variables = network_functions_environment_variables[*it];

			logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "\t\t\tEnvironment variables (%d):",nfs_environment_variables.size());
			for(list<string>::iterator ev = nfs_environment_variables.begin(); ev != nfs_environment_variables.end(); ev++)
			{
				logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "\t\t\t\t%s",ev->c_str());
			}
		}
#endif

		map<string,unsigned int> nfs_ports = lsi->getNetworkFunctionsPorts(*it);

		map<unsigned int, port_network_config > nfs_ports_configuration = network_functions_ports_configuration[*it];

		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "\t\t\tPorts (%d):",nfs_ports.size());
		map<unsigned int, port_network_config >::iterator nd = nfs_ports_configuration.begin();
		for(map<string,unsigned int>::iterator n = nfs_ports.begin(); n != nfs_ports.end(); n++, nd++)
		{
			logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "\t\t\t\t%s -> %d",(n->first).c_str(),nd->first);
			if(!(nd->second.mac_address).empty())
				logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "\t\t\t\tMac address -> %s",(nd->second.mac_address).c_str());
#ifdef ENABLE_UNIFY_PORTS_CONFIGURATION
			if(!(nd->second.ip_address).empty())
				logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "\t\t\t\tIp address -> %s",(nd->second.ip_address).c_str());
#endif
		}
	}

	logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "GRE endpoints (%d):",eps.size());
	for(list<highlevel::EndPointGre>::iterator it = eps.begin(); it != eps.end(); it++){
		int id = 0;
		sscanf(it->getId().c_str(), "%d", &id);
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "\t\tID %d:", id);
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "\t\t\t\tKey: %s", it->getGreKey().c_str());
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "\t\t\t\tLocal ip: %s", it->getLocalIp().c_str());
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "\t\t\t\tRemote_ip: %s", it->getRemoteIp().c_str());
	}

	logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Virtual links (%u): ",vls.size());

	for(vector<VLink>::iterator v = vls.begin(); v != vls.end(); v++)
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "\t\t(ID: %x) %x:%d -> %x:%d",v->getID(),dpid,v->getLocalID(),v->getRemoteDpid(),v->getRemoteID());

	//associate the vlinks to the NFs ports
	logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "NF port is virtual link ID:");
	map<string, uint64_t> nfs_vlinks;
	if(vls.size() != 0)
	{
		vector<VLink>::iterator vl1 = vls.begin();
		for(set<string>::iterator nf = vlNFs.begin(); nf != vlNFs.end(); nf++, vl1++)
		{
			nfs_vlinks[*nf] = vl1->getID();
			logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "\t\t%s -> %x",(*nf).c_str(),vl1->getID());

			if(NFsFromEndPoint.count(*nf) != 0)
			{
				//since this rule has a graph endpoint in the match that is defined in this
				//graph, we save the port of the vlink of the NF in LSI-0, so that other graphs can use this endpoint
				string ep = findEndPointTowardsNF(graph,*nf);
				logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "The internal endpoint \"%s\" is defined in a match of the current Graph. Other graphs can use it expressing an action on the port %d of the LSI-0",ep.c_str(),vl1->getRemoteID());
				endPointsDefinedInMatches[ep] = vl1->getRemoteID();

				//This endpoint is currently not used in any other graph, since it is defined in the current graph
				availableEndPoints[ep] = 0;
			}
		}

		lsi->setNFsVLinks(nfs_vlinks);
		//associate the vlinks to the physical ports
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Physical port is virtual link ID:");
		map<string, uint64_t> ports_vlinks;
		vector<VLink>::iterator vl2 = vls.begin();
		for(set<string>::iterator p = vlPhyPorts.begin(); p != vlPhyPorts.end(); p++, vl2++)
		{
			ports_vlinks[*p] = vl2->getID();
			logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "\t\t%s -> %x",(*p).c_str(),vl2->getID());
		}
		lsi->setPortsVLinks(ports_vlinks);

		//associate the vlinks to the GRE endpoints
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Gre endpoint is virtual link ID:");
		map<string, uint64_t> endpoints_vlinks;
		vector<VLink>::iterator vl3 = vls.begin();

		for(set<string>::iterator ep = vlEndPointsGre.begin(); ep != vlEndPointsGre.end(); ep++, vl3++)
		{
			endpoints_vlinks[*ep] = vl3->getID();
			logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "\t\t%s -> %x",(*ep).c_str(),vl3->getID());

			if(GREsFromEndPoint.count(*ep) != 0)
			{
				//since this rule has an internal endpoint in the match that is defined in this
				//graph, we save the port of the vlink of the GRE in LSI-0, so that other graphs can use this endpoint
				string epp = findEndPointTowardsGRE(graph,*ep);
				logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "The internal endpoint \"%s\" is defined in a match of the current Graph. Other graphs can use it expressing an action on the port %d of the LSI-0",epp.c_str(),vl3->getRemoteID());
				endPointsDefinedInMatches[epp] = vl3->getRemoteID();

				//This endpoint is currently not used in any other graph, since it is defined in the current graph
				availableEndPoints[epp] = 0;
			}
		}
		lsi->setEndPointsGreVLinks(endpoints_vlinks);

		//associate the vlinks to the graph endpoints
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Internal endpoint is virtual link ID:");
		map<string, uint64_t> endpoints_internal_vlinks;
		vector<VLink>::iterator vl4 = vls.begin();

		unsigned int aux = 0;
		while(aux < numberOfVLrequiredBeforeEndPoints/* + vlEndPointsGre.size()*/)
		{
			//The first vlinks are only used for NFs, gre endpoints and physical ports
			//TODO: this could be optimized, although it is not easy (and useful)
			aux++;
			vl4++;
		}

		for(set<string>::iterator ep = vlEndPointsInternal.begin(); ep != vlEndPointsInternal.end(); ep++, vl4++)
		{
			endpoints_internal_vlinks[*ep] = vl4->getID();

			logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "\t\t%s -> %x",(*ep).c_str(),vl4->getID());
			if(/*graph->isDefinedHere(*ep)*/endPointsDefinedInActions.count(*ep) == 0)
			{
				//since this endpoint is in an action (hence it requires a virtual link), and it is defined in this
				//graph, we save the port of the vlink in LSI-0, so that other graphs can use this endpoint

				logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "The internal endpoint \"%s\" is defined in an action of the current Graph. Other graph can use it expressing a match on the port %d of the LSI-0",(*ep).c_str(),vl4->getRemoteID());
				endPointsDefinedInActions[*ep] = vl4->getRemoteID();

				//This endpoint is currently not used in any other graph, since it is defined in the current graph
				availableEndPoints[*ep] = 0;
			}
		}
		lsi->setEndPointsVLinks(endpoints_internal_vlinks);
	}

	/**
	*	4) Start the network functions
	*/
#ifdef RUN_NFS
	logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "4) start the network functions");

	computeController->setLsiID(dpid);
	#ifndef STARTVNF_SINGLE_THREAD
	pthread_t some_thread[network_functions.size()];
	#endif
	to_thread_t thr[network_functions.size()];
	int i = 0;


	for(highlevel::Graph::t_nfs_ports_list::iterator nf = network_functions.begin(); nf != network_functions.end(); nf++)
	{
		thr[i].nf_name = nf->first;
		thr[i].computeController = computeController;
		thr[i].namesOfPortsOnTheSwitch = lsi->getNetworkFunctionsPortsNameOnSwitchMap(nf->first);
		thr[i].portsConfiguration = network_functions_ports_configuration[nf->first];
#ifdef ENABLE_UNIFY_PORTS_CONFIGURATION
		thr[i].controlConfiguration = network_functions_control_configuration[nf->first];
		thr[i].environmentVariables = network_functions_environment_variables[nf->first];
#endif

#ifdef STARTVNF_SINGLE_THREAD
		startNF((void *) &thr[i]);
#else
		if (pthread_create(&some_thread[i], NULL, &startNF, (void *)&thr[i]) != 0)
		{
			assert(0);
			logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "An error occurred while creating a new thread");
			throw GraphManagerException();
		}
#endif
		i++;
	}

	bool ok = true;
#ifndef STARTVNF_SINGLE_THREAD
	for(int j = 0; j < i;j++)
	{
		void *returnValue;
		pthread_join(some_thread[j], &returnValue);
		int *c = (int*)returnValue;

		if(c == 0)
			ok = false;
	}
#endif

	if(!ok)
	{
		for(highlevel::Graph::t_nfs_ports_list::iterator nf = network_functions.begin(); nf != network_functions.end(); nf++)
			computeController->stopNF(nf->first);

		switchManager.destroyLsi(lsi->getDpid());

		delete(graph);
		delete(lsi);
		delete(computeController);
		delete(controller);

		graph = NULL;;
		lsi = NULL;
		computeController = NULL;
		controller = NULL;

		throw GraphManagerException();
	}

#else
	logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "3) Flag RUN_NFS disabled. NFs will not start");
#endif

	/**
	*	5) Create the rules and download them in LSI-0 and tenant-LSI
	*/
	logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "5) Create the rules and download them in LSI-0 and tenant-LSI");
	try
	{
		//creates the rules for LSI-0 and for the tenant-LSI
		lowlevel::Graph graphLSI0 = GraphTranslator::lowerGraphToLSI0(graph,lsi,graphInfoLSI0.getLSI(),endPointsDefinedInMatches,endPointsDefinedInActions,availableEndPoints,un_interface,orchestrator_in_band);
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "New graph for LSI-0:");
		graphLSI0.print();

		graphLSI0lowLevel.addRules(graphLSI0.getRules());

		lowlevel::Graph graphTenant =  GraphTranslator::lowerGraphToTenantLSI(graph,lsi,graphInfoLSI0.getLSI());
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Graph for tenant LSI:");
		graphTenant.print();

		controller->installNewRules(graphTenant.getRules());

		GraphInfo graphInfoTenantLSI;
		graphInfoTenantLSI.setGraph(graph);
		graphInfoTenantLSI.setComputeController(computeController);
		graphInfoTenantLSI.setLSI(lsi);
		graphInfoTenantLSI.setController(controller);

		//Save the graph information
		tenantLSIs[graph->getID()] = graphInfoTenantLSI;

		//Insert new rules into the LSI-0
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Adding the new rules to the LSI-0");
		(graphInfoLSI0.getController())->installNewRules(graphLSI0.getRules());

		logger(ORCH_INFO, MODULE_NAME, __FILE__, __LINE__, "Tenant LSI (ID: %s) and its controller are created",graph->getID().c_str());

		printInfo(graphLSI0lowLevel,graphInfoLSI0.getLSI());

	} catch (SwitchManagerException e)
	{
#ifdef RUN_NFS
		for(highlevel::Graph::t_nfs_ports_list::iterator nf = network_functions.begin(); nf != network_functions.end(); nf++)
			computeController->stopNF(nf->first);
#endif

		switchManager.destroyLsi(lsi->getDpid());

		if(tenantLSIs.count(graph->getID()) != 0)
			tenantLSIs.erase(tenantLSIs.find(graph->getID()));

		delete(graph);
		delete(lsi);
		delete(computeController);
		delete(controller);

		graph = NULL;
		lsi = NULL;
		computeController = NULL;
		controller = NULL;

		logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "%s",e.what());
		throw GraphManagerException();
	}

	return true;
}

bool GraphManager::updateGraph(string graphID, highlevel::Graph *newGraph)
{
	/**
	*	Limitations:
	*
	*	- only used to add new parts to the graph, and not to remove parts
	*		- new VNFs
	*		- new endpoints (interface, GRE, vlan, internal)
	*	- it is not possible to update a VNF, then the following parameters cannot be updated:
	*		- number of ports
	*		- ports configuration
	*		- environment variables
	*		- control connection
	**/

	logger(ORCH_INFO, MODULE_NAME, __FILE__, __LINE__, "Updating the graph '%s'...",graphID.c_str());

	assert(tenantLSIs.count(graphID) != 0);

	//Retrieve the information already stored for the graph (i.e., retrieve the as it is
	//currently implemented, without the update)
	GraphInfo graphInfo = (tenantLSIs.find(graphID))->second;
	ComputeController *computeController = graphInfo.getComputeController();
	highlevel::Graph *graph = graphInfo.getGraph();
	LSI *lsi = graphInfo.getLSI();
	Controller *tenantController = graphInfo.getController();

	uint64_t dpid = lsi->getDpid();

	/**
	*	Outline:
	*
	*	0) calculate the diff with respect to the graph already deployed (and check its validity)
	*	1) update the high level graph
	*	2) select an implementation for the new NFs
	*	3) update the lsi (in case of new ports/NFs/gre endpoints/internal endpoints/vlan endpoints are required)
	*	4) start the new NFs
	*	5) download the new rules in LSI-0 and tenant-LSI
	*/

	/**
	*	The three following variables will be used in the following and that contain
	*	an high level graph:
	*		* graph -> the original graph to be updated
	*		* newGraph -> graph containing the update
	*		* diff -> graph that will contain the parts in newGraph that are not part of graph
	*/

	/**
	*	0) Calculate the diff ad check the validity of the update
	*/
	logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "0) Calculate the new pieces of the graph");
	highlevel::Graph *diff = graph->calculateDiff(newGraph, graphID);
	
	
	Object json_diff = diff->toJSON();
	stringstream ssj;
	write_formatted(json_diff, ssj );
 	string sssj = ssj.str();
 	logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "The diff graph is %s",sssj.c_str());

	if(!checkGraphValidity(diff,computeController))
	{
		//This is an error in the request
		delete(diff);
		diff = NULL;
		return false;
	}
	//The required graph update is valid

	/**
	*	1) Update the high level graph
	*/
	logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "1) Update the high level graph");

	graph->addGraphToGraph(diff);

	graph->print();

	/**
	*	2) Select an implementation for the new NFs
	*/
	logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "2) Select an implementation for the new NFs");
	if(!computeController->selectImplementation())
	{
		//This is an internal error
		delete(computeController);
		computeController = NULL;
		throw GraphManagerException();
	}

	/**
	*	3) Update the lsi (in case of new ports/NFs/gre endpoints/internal endpoints are required)
	*/
	logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "3) update the lsi (in case of new ports/NFs/gre endpoints/internal endpoints are required)");

	set<string> phyPorts = diff->getPorts();

	map<string, list<unsigned int> > network_functions = diff->getNetworkFunctionsPorts();
//	map<string, vector<string> > tmp_endpoints = diff->getEndPoints();//#ADDED
	list<highlevel::EndPointGre> tmp_endpoints = diff->getEndPointsGre();//#ADDED

	//Since the NFs cannot specify new ports, new virtual links can be required only by the new NFs and the physical ports

	vector<set<string> > vlVector = identifyVirtualLinksRequired(diff,lsi);
	set<string> vlNFs = vlVector[0];
	set<string> vlPhyPorts = vlVector[1];
	set<string> vlEndPointsGre = vlVector[2];
	set<string> vlEndPointsInternal = vlVector[3];
	set<string> NFsFromEndPoint = vlVector[4];
	set<string> GREsFromEndPoint = vlVector[5];

	//TODO: check if a virtual link is already available and can be used (because it is currently used only in one direction)
	unsigned int numberOfVLrequiredBeforeEndPoints = (vlNFs.size() > vlPhyPorts.size())? vlNFs.size() : vlPhyPorts.size(); //(vlNFs.size() > ((vlPhyPorts.size() > vlEndPointsGre.size()) ? vlPhyPorts.size():vlEndPointsGre.size())) ? vlNFs.size():((vlPhyPorts.size() > vlEndPointsGre.size()) ? vlPhyPorts.size():vlEndPointsGre.size());
	unsigned int numberOfVLrequired = numberOfVLrequiredBeforeEndPoints + vlEndPointsInternal.size()  + vlEndPointsGre.size();

	logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "%d virtual links are required to connect the new part of the LSI with LSI-0",numberOfVLrequired);

	set<string>::iterator nf = vlNFs.begin();
	set<string>::iterator p = vlPhyPorts.begin();
	for(; nf != vlNFs.end() || p != vlPhyPorts.end() ;)
	{
		//FIXME: here I am referring to a vlink through its position. It would be really better to use its ID
		AddVirtualLinkOut *avlo = NULL;
		try
		{
			logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, " Adding vlink required to physical port/VNF port");
			VLink newLink(dpid0);
			int vlinkPosition = lsi->addVlink(newLink);

			AddVirtualLinkIn avli(dpid,dpid0);
			avlo = switchManager.addVirtualLink(avli);

			assert(avlo != NULL);
			lsi->setVLinkIDs(vlinkPosition,avlo->getIdA(),avlo->getIdB());

			delete(avlo);

			uint64_t vlinkID = newLink.getID();

			VLink vlink = lsi->getVirtualLink(vlinkID); //FIXME: vlink is the same of newLink
			logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Virtual link: (ID: %x) %x:%d -> %x:%d",vlink.getID(),dpid,vlink.getLocalID(),vlink.getRemoteDpid(),vlink.getRemoteID());
			assert(vlinkID == vlink.getID());

			if(nf != vlNFs.end())
			{
				lsi->addNFvlink(*nf,vlinkID);
				logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "NF '%s' uses the vlink '%x'",(*nf).c_str(),vlink.getID());
				nf++;
			}
			if(p != vlPhyPorts.end())
			{
				lsi->addPortvlink(*p,vlinkID);
				logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Physical port '%s' uses the vlink '%x'",(*p).c_str(),vlink.getID());
				p++;
			}
		}catch(SwitchManagerException e)
		{
			logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "%s",e.what());
			if(avlo != NULL)
				delete(avlo);
			delete(diff);
			diff = NULL;
			throw GraphManagerException();
		}
	}

	for(set<string>::iterator ep = vlEndPointsGre.begin(); ep != vlEndPointsGre.end(); ep++)
	{
		//FIXME: here I am referring to a vlink through its position. It would be really better to use its ID
		AddVirtualLinkOut *avlo = NULL;
		try
		{
			logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, " Adding vlink required to GRE tunnel");
		
			VLink newLink(dpid0);
			int vlinkPosition = lsi->addVlink(newLink);

			AddVirtualLinkIn avli(dpid,dpid0);
			avlo = switchManager.addVirtualLink(avli);

			lsi->setVLinkIDs(vlinkPosition,avlo->getIdA(),avlo->getIdB());

			delete(avlo);

			uint64_t vlinkID = newLink.getID();

			VLink vlink = lsi->getVirtualLink(vlinkID);
			logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Virtual link: (ID: %x) %x:%d -> %x:%d",vlink.getID(),dpid,vlink.getLocalID(),vlink.getRemoteDpid(),vlink.getRemoteID());
			assert(vlinkID == vlink.getID());

			lsi->addEndpointGrevlink(*ep,vlinkID);
			logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Gre endpoint '%s' uses the vlink '%x'",(*ep).c_str(),vlink.getID());
		}catch(SwitchManagerException e)
		{
			logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "%s",e.what());
			if(avlo != NULL)
				delete(avlo);
			delete(diff);
			diff = NULL;
			throw GraphManagerException();
		}
	}

	for(set<string>::iterator ep = vlEndPointsInternal.begin(); ep != vlEndPointsInternal.end(); ep++)
	{
		//FIXME: here I am referring to a vlink through its position. It would be really better to use its ID
		AddVirtualLinkOut *avlo = NULL;
		try
		{
			VLink newLink(dpid0);
			int vlinkPosition = lsi->addVlink(newLink);

			AddVirtualLinkIn avli(dpid,dpid0);
			avlo = switchManager.addVirtualLink(avli);

			lsi->setVLinkIDs(vlinkPosition,avlo->getIdA(),avlo->getIdB());

			delete(avlo);

			uint64_t vlinkID = newLink.getID();

			VLink vlink = lsi->getVirtualLink(vlinkID);
			logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Virtual link: (ID: %x) %x:%d -> %x:%d",vlink.getID(),dpid,vlink.getLocalID(),vlink.getRemoteDpid(),vlink.getRemoteID());

			lsi->addEndpointvlink(*ep,vlinkID);
			logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Internal endpoint '%s' uses the vlink '%x'",(*ep).c_str(),vlink.getID());

			if(graph->isDefinedHere(*ep))
			{
				//since this endpoint is in an action (hence it requires a virtual link), and it is defined in this
				//graph, we save the port of the vlink in LSI-0, so that other graphs can use this endpoint
				//Since we are considering this endpoint now, it means that is defined for the first time in this update
				//of the graph.

				logger(ORCH_INFO, MODULE_NAME, __FILE__, __LINE__, "The internal endpoint \"%s\" is defined in an action of the current Graph. Other graph can use it expressing a match on the port %d of the LSI-0",(*ep).c_str(),vlink.getRemoteID());
				endPointsDefinedInActions[*ep] = vlink.getRemoteID();

				//This endpoint is currently not used in any other graph, since it is defined in the current graph
				availableEndPoints[*ep] = 0;
			}
		}catch(SwitchManagerException e)
		{
			logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "%s",e.what());
			if(avlo != NULL)
				delete(avlo);
			delete(diff);
			diff = NULL;
			throw GraphManagerException();
		}

	}

	for(set<string>::iterator nf = NFsFromEndPoint.begin(); nf != NFsFromEndPoint.end(); nf++)
	{
		//XXX: this works because I'm assuming that a graph cannot use twice the same endpoint in matches, if this
		//endpoint is defined by the graph itself.

		//since this rule has a graph endpoint in the match that is defined in this
		//graph, we save the port of the vlink of the NF in LSI-0, so that other graphs can use this endpoint
		string ep = findEndPointTowardsNF(diff,*nf);

		map<string, uint64_t> nfs_vlinks = lsi->getNFsVlinks();
		assert(nfs_vlinks.count(*nf) != 0);

		VLink vlink = lsi->getVirtualLink(nfs_vlinks.find(*nf)->second);

		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "The internal endpoint \"%s\" is defined in a match of the current Graph. Other graphs can use it expressing an action on the port %d of the LSI-0",ep.c_str(),vlink.getRemoteID());
		endPointsDefinedInMatches[ep] = vlink.getRemoteID();

		//This endpoint is currently not used in any other graph, since it is defined in the current graph
		availableEndPoints[ep] = 0;
	}

	//Itarate on all the new network functions
	//TODO: when the hotplug will be introduced, here we will also iterate on the network functions to be updated
	for(highlevel::Graph::t_nfs_ports_list::iterator nf = network_functions.begin(); nf != network_functions.end(); nf++)
	{
		AddNFportsOut *anpo = NULL;
		try
		{
			//TODO: for the hotplug, modify lsi->addNF as suggested into the function itself.
			lsi->addNF(nf->first, nf->second, computeController->getNFSelectedImplementation(nf->first)->getPortTypes());

			map<string, list<struct nf_port_info> >pi_map = lsi->getNetworkFunctionsPortsInfo();//for each network function, retrieve a list of "port name, port type"
			map<string, list<struct nf_port_info> >::iterator pi_it = pi_map.find(nf->first); //select the info related to the network function currently considered
			//TODO: when the hotplug will be introduced, pi_it->second will also contain the old ports of the VNF. Then a further skimming will be required
			assert(pi_it != pi_map.end());
			AddNFportsIn anpi(dpid, nf->first, computeController->getNFType(nf->first), pi_it->second); //prepare the input for the switch manager

			//We add, with a single call, all the ports of a single network function
			anpo = switchManager.addNFPorts(anpi);

			//anpo->getPorts() returns the map "ports name, identifier within the lsi"
			if(!lsi->setNfSwitchPortsID(anpo->getNFname(), anpo->getPorts()))
			{
				logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "A non-required network function port related to the network function \"%s\" has been attached to the tenant-lsi",nf->first.c_str());
				lsi->removeNF(nf->first);
				delete(anpo);
				throw GraphManagerException();
			}

			//FIXME: useful? Probably no!
			//TODO: not sure that this works in case of hotplug. In fact anpo->getPortsNameOnSwitch() returns a list of ports name on the switch, but it does not 
			//map such names with ports names calculated before (or with the port id). 
			//I think that it should be done something similar to anpo->getPorts(), which maps the identifier on the switch to the port name.
			lsi->setNetworkFunctionsPortsNameOnSwitch(anpo->getNFname(),anpo->getPortsNameOnSwitch());

			delete(anpo);
		}catch(SwitchManagerException e)
		{
			logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "%s",e.what());
			lsi->removeNF(nf->first);
			if(anpo != NULL)
				delete(anpo);
			delete(diff);
			diff = NULL;
			throw GraphManagerException();
		}
	}

	for(list<highlevel::EndPointGre>::iterator ep = tmp_endpoints.begin(); ep != tmp_endpoints.end(); ep++)
	{
#ifdef VSWITCH_IMPLEMENTATION_OVSDB
		//fill the vector related to the endpoint params [gre key, local-ip, remote-ip, interface, isSafe]
		vector<string> ep_param(5);
		ep_param[0] = ep->getLocalIp();
		ep_param[1] = ep->getGreKey();
		ep_param[2] = ep->getRemoteIp();
		ep_param[3] = un_interface;
		if(ep->isSafe())
			ep_param[4] = "true";
		else
			ep_param[4] = "false";

		AddEndpointOut *aepo = NULL;
		try
		{
			lsi->addEndpoint(*ep);
			AddEndpointIn aepi(dpid,ep->getId(),ep_param);

			aepo = switchManager.addEndpoint(aepi);

			if(!lsi->setEndpointPortID(aepo->getEPname(), aepo->getEPid()))
			{
				logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "A non-required gre endpoint \"%s\" has been attached to the tenant-lsi",ep->getId().c_str());
				lsi->removeEndpoint(ep->getId());
				delete(aepo);
				throw GraphManagerException();
			}

			delete(aepo);
		}catch(SwitchManagerException e)
		{
			logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "%s",e.what());
			lsi->removeEndpoint(ep->getId());
			if(aepo != NULL)
				delete(aepo);
			delete(diff);
			diff = NULL;
			throw GraphManagerException();
		}
#else
		logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "GRE tunnel unavailable");
		throw GraphManagerException();
#endif
	}

	/**
	*	4) Start the new NFs
	*/
	//TODO: in case of hotplug, the function is already running then a different function on the compute controller should be called.
#ifdef RUN_NFS
	logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "4) start the new NFs");

	computeController->setLsiID(dpid);

	for(highlevel::Graph::t_nfs_ports_list::iterator nf = network_functions.begin(); nf != network_functions.end(); nf++)
	{
		map<unsigned int, string> nfPortIdToNameOnSwitch = lsi->getNetworkFunctionsPortsNameOnSwitchMap(nf->first); //Returns the map <port ID, port name on switch>
		//TODO: the following information should be retrieved through the highlevel graph
		map<string, map<unsigned int, port_network_config > > new_nfs_ports_configuration = diff->getNetworkFunctionsConfiguration();
		map<unsigned int, port_network_config_t > nfs_ports_configuration = new_nfs_ports_configuration[nf->first];
#ifdef ENABLE_UNIFY_PORTS_CONFIGURATION
		map<string, list<port_mapping_t> > new_nfs_control_ports = diff->getNetworkFunctionsControlPorts();
		list<port_mapping_t > nfs_control_configuration = new_nfs_control_ports[nf->first];
		map<string, list<string> > new_nfs_env_variables = diff->getNetworkFunctionsEnvironmentVariables();
		list<string> environment_variables_tmp = new_nfs_env_variables[nf->first];
#endif
		//TODO: for the hotplug, we may extend the computeController with a call that says if a VNF is already running or not.
		//If not, startNF should then be called; if yes, o new function must be called
		if(!computeController->startNF(nf->first, nfPortIdToNameOnSwitch, nfs_ports_configuration
#ifdef ENABLE_UNIFY_PORTS_CONFIGURATION
			, nfs_control_configuration, environment_variables_tmp
#endif
		))
		{
			//TODO: no idea on what I have to do at this point
			assert(0);
			delete(diff);
			diff = NULL;
			throw GraphManagerException();
		}
	}
#else
	logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "4) Flag RUN_NFS disabled. New NFs will not start");
#endif

	/**
	*	5) Create the new rules and download them in LSI-0 and tenant-LSI
	*/
	logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "5) Create the new rules and download them in LSI-0 and tenant-LSI");

	try
	{
		//creates the new rules for LSI-0 and for the tenant-LSI

		lowlevel::Graph graphLSI0 = GraphTranslator::lowerGraphToLSI0(diff,lsi,graphInfoLSI0.getLSI(),endPointsDefinedInMatches,endPointsDefinedInActions,availableEndPoints,un_interface,orchestrator_in_band);

		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "New piece of graph for LSI-0:");
		graphLSI0.print();
		graphLSI0lowLevel.addRules(graphLSI0.getRules());

		lowlevel::Graph graphTenant =  GraphTranslator::lowerGraphToTenantLSI(diff,lsi,graphInfoLSI0.getLSI());
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "New piece of graph for tenant LSI:");
		graphTenant.print();

		//Insert new rules into the LSI-0
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Adding the new rules to the LSI-0");
		(graphInfoLSI0.getController())->installNewRules(graphLSI0.getRules());

		//Insert new rules into the tenant-LSI
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Adding the new rules to the tenant-LSI");
		tenantController->installNewRules(graphTenant.getRules());

		printInfo(graphLSI0lowLevel,graphInfoLSI0.getLSI());

	} catch (SwitchManagerException e)
	{
		//TODO: no idea on what I have to do at this point
		assert(0);
		delete(diff);
		diff = NULL;
		throw GraphManagerException();

		logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "%s",e.what());
		throw GraphManagerException();
	}

	//The new flows have been added to the graph!

	delete(diff);
	diff = NULL;
	return true;
}

vector<set<string> > GraphManager::identifyVirtualLinksRequired(highlevel::Graph *graph)
{
	set<string> NFs;
	set<string> phyPorts;
	set<string> endPointsGre;
	set<string> endPointsInternal;

	set<string> NFsFromEndPoint;
	set<string> GREsFromEndPoint;

	list<highlevel::Rule> rules = graph->getRules();
	for(list<highlevel::Rule>::iterator rule = rules.begin(); rule != rules.end(); rule++)
	{
		highlevel::Action *action = rule->getAction();
		highlevel::Match match = rule->getMatch();
		if(action->getType() == highlevel::ACTION_ON_NETWORK_FUNCTION)
		{
			if(match.matchOnPort() || /*FIXME: match gre -> action NFs requires a virtual link? match.matchOnEndPointGre() ||*/ match.matchOnEndPointInternal())
			{
				highlevel::ActionNetworkFunction *action_nf = (highlevel::ActionNetworkFunction*)action;
				stringstream ss;
				ss << action->getInfo() << "_" << action_nf->getPort();
				NFs.insert(ss.str());

				if(match.matchOnEndPointInternal())
				{
					stringstream ssm;
					ssm << match.getEndPoint();
					if(/*graph->isDefinedHere(ssm.str())*/endPointsDefinedInMatches.count(ssm.str()) == 0)
						NFsFromEndPoint.insert(ss.str());
				}
			}
		}
		else if(action->getType() == highlevel::ACTION_ON_ENDPOINT_GRE)
		{
			if(match.matchOnPort() || /*FIXME: match NFs -> action gre requires a virtual link? match.matchOnNF() ||*/ match.matchOnEndPointInternal())
			{
				highlevel::ActionEndPointGre *action_ep = (highlevel::ActionEndPointGre*)action;
				endPointsGre.insert(action_ep->toString());

				if(match.matchOnEndPointInternal())
				{
					stringstream ssm;
					ssm << match.getEndPoint();
					if(/*graph->isDefinedHere(ssm.str())*/endPointsDefinedInMatches.count(ssm.str()) == 0)
						GREsFromEndPoint.insert(action_ep->toString());
				}
			}
		}
		else if(action->getType() == highlevel::ACTION_ON_PORT)
		{
			if(match.matchOnNF() || match.matchOnEndPointGre())
				phyPorts.insert(action->getInfo());
		}
		else if(action->getType() == highlevel::ACTION_ON_ENDPOINT_INTERNAL)
		{
			assert(match.matchOnNF() || match.matchOnEndPointGre());

			if(!match.matchOnPort())
			{
				highlevel::ActionEndPointInternal *action_ep = (highlevel::ActionEndPointInternal*)action;

				endPointsInternal.insert(action_ep->toString());
			}
		}
	}

	if(NFs.size() != 0)
	{
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Network functions input ports requiring a virtual link:");
		for(set<string>::iterator nf = NFs.begin(); nf != NFs.end(); nf++)
			logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "\t%s",(*nf).c_str());
	}
	if(phyPorts.size() != 0)
	{
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Physical ports requiring a virtual link:");
		for(set<string>::iterator p = phyPorts.begin(); p != phyPorts.end(); p++)
			logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "\t%s",(*p).c_str());
	}
	if(endPointsGre.size() != 0)
	{
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Gre endpoints requiring a virtual link:");
		for(set<string>::iterator e = endPointsGre.begin(); e != endPointsGre.end(); e++)
			logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "\t%s",(*e).c_str());
	}
	if(endPointsInternal.size() != 0)
	{
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Internal endpoints requiring a virtual link:");
		for(set<string>::iterator e = endPointsInternal.begin(); e != endPointsInternal.end(); e++)
			logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "\t%s",(*e).c_str());
	}
	if(NFsFromEndPoint.size() != 0)
	{
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "NFs reached from an internal endpoint defined in this graph:");
		for(set<string>::iterator nfe = NFsFromEndPoint.begin(); nfe != NFsFromEndPoint.end(); nfe++)
			logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "\t%s",(*nfe).c_str());
	}
	if(GREsFromEndPoint.size() != 0)
	{
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Gre endpoints reached from an internal endpoint defined in this graph:");
		for(set<string>::iterator ep = GREsFromEndPoint.begin(); ep != GREsFromEndPoint.end(); ep++)
			logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "\t%s",(*ep).c_str());
	}

	vector<set<string> > retval;
	vector<set<string> >::iterator rv;

	rv = retval.end();
	retval.insert(rv,NFs);
	rv = retval.end();
	retval.insert(rv,phyPorts);
	rv = retval.end();
	retval.insert(rv,endPointsGre);
	rv = retval.end();
	retval.insert(rv,endPointsInternal);
	rv = retval.end();
	retval.insert(rv,NFsFromEndPoint);
	rv = retval.end();
	retval.insert(rv,GREsFromEndPoint);

	return retval;
}

vector<set<string> > GraphManager::identifyVirtualLinksRequired(highlevel::Graph *newPiece, LSI *lsi)
{
	set<string> NFs;
	set<string> phyPorts;
	set<string> endPointsGre;
	set<string> endPointsInternal;

	set<string> NFsFromEndPoint;
	set<string> GREsFromEndPoint;

	list<highlevel::Rule> rules = newPiece->getRules();
	for(list<highlevel::Rule>::iterator rule = rules.begin(); rule != rules.end(); rule++)
	{
		highlevel::Action *action = rule->getAction();
		highlevel::Match match = rule->getMatch();
		if(action->getType() == highlevel::ACTION_ON_NETWORK_FUNCTION)
		{
			if(match.matchOnPort() || match.matchOnEndPointInternal())
			{
				highlevel::ActionNetworkFunction *action_nf = (highlevel::ActionNetworkFunction*)action;

				//Check if a vlink is required for this NF port
				map<string, uint64_t> nfs_vlinks = lsi->getNFsVlinks();
				stringstream ss;
				ss << action->getInfo() << "_" << action_nf->getPort();
				if(nfs_vlinks.count(ss.str()) == 0)
					NFs.insert(ss.str());
			}
		}
		else if(action->getType() == highlevel::ACTION_ON_PORT)
		{
			//check if a vlink is required for this physical port
			map<string, uint64_t> ports_vlinks = lsi->getPortsVlinks();
			highlevel::ActionPort *action_port = (highlevel::ActionPort*)action;
			if(ports_vlinks.count(action_port->getInfo()) == 0)
				phyPorts.insert(action_port->getInfo());
		}
		else if(action->getType() == highlevel::ACTION_ON_ENDPOINT_GRE)
		{
			if(match.matchOnPort() || match.matchOnEndPointInternal())
			{
				//check if a vlink is required for this endpoint
				map<string, uint64_t> endpoints_vlinks = lsi->getEndPointsGreVlinks();
				highlevel::ActionEndPointGre *action_ep = (highlevel::ActionEndPointGre*)action;
				if(endpoints_vlinks.count(action_ep->toString()) == 0)
					endPointsGre.insert(action_ep->toString());
			}
		}
		else if(action->getType() == highlevel::ACTION_ON_ENDPOINT_INTERNAL)
		{
			assert(match.matchOnNF() || match.matchOnEndPointGre());

			//check if a vlink is required for this endpoint
			map<string, uint64_t> endpoints_vlinks = lsi->getEndPointsVlinks();
			highlevel::ActionEndPointInternal *action_ep = (highlevel::ActionEndPointInternal*)action;
			if(endpoints_vlinks.count(action_ep->toString()) == 0)
				endPointsInternal.insert(action_ep->toString());
		}
	}

	logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Network functions input ports requiring a virtual link:");
	for(set<string>::iterator nf = NFs.begin(); nf != NFs.end(); nf++)
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "\t%s",(*nf).c_str());
	logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Physical ports requiring a virtual link:");
	for(set<string>::iterator p = phyPorts.begin(); p != phyPorts.end(); p++)
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "\t%s",(*p).c_str());
	logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Gre endpoints requiring a virtual link:");
	for(set<string>::iterator e = endPointsGre.begin(); e != endPointsGre.end(); e++)
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "\t%s",(*e).c_str());
	logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Internal endpoints requiring a virtual link:");
	for(set<string>::iterator e = endPointsInternal.begin(); e != endPointsInternal.end(); e++)
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "\t%s",(*e).c_str());
	logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "NFs reached from an internal endpoint defined in this graph:");
	for(set<string>::iterator nfe = NFsFromEndPoint.begin(); nfe != NFsFromEndPoint.end(); nfe++)
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "\t%s",(*nfe).c_str());
	logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Gre endpoints reached from an internal endpoint defined in this graph:");
	for(set<string>::iterator ep = GREsFromEndPoint.begin(); ep != GREsFromEndPoint.end(); ep++)
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "\t%s",(*ep).c_str());

	//prepare the return value
	vector<set<string> > retval;
	vector<set<string> >::iterator rv;
	rv = retval.end();
	retval.insert(rv,NFs);
	rv = retval.end();
	retval.insert(rv,phyPorts);
	rv = retval.end();
	retval.insert(rv,endPointsGre);
	rv = retval.end();
	retval.insert(rv,endPointsInternal);
	rv = retval.end();
	retval.insert(rv,NFsFromEndPoint);
	rv = retval.end();
	retval.insert(rv,GREsFromEndPoint);

	return retval;
}

void GraphManager::removeUselessPorts_NFs_Endpoints_VirtualLinks(RuleRemovedInfo rri, ComputeController *computeController,highlevel::Graph *graph, LSI * lsi)
{
	/*
	*	Check if ports, NFs, end point and virtual links used by the rule removed are still useful.
	*	Note that the rule has already been removed from the high level graph
	*/
	map<string, uint64_t> nfs_vlinks = lsi->getNFsVlinks();
	map<string, uint64_t> ports_vlinks = lsi->getPortsVlinks();
#ifndef UNIFY_NFFG
	map<string, uint64_t> endpoints_vlinks = lsi->getEndPointsVlinks();
#endif
	map<string, uint64_t> endpoints_gre_vlinks = lsi->getEndPointsGreVlinks();

	list<highlevel::Rule> rules = graph->getRules();

	logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Checking if ports, network functions, endpoints and virtual links can be removed...");
	if(rri.isNFport)
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Check if the vlink associated with the NF port '%s' must be removed (if this vlink exists)",rri.nf_port.c_str());

	if(rri.isNFport && nfs_vlinks.count(rri.nf_port) != 0)
	{
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "The NF port '%s' is associated with a vlink",rri.nf_port.c_str());

		/**
		*	In case NF:port does not appear in other actions, and it is not use for any physical port, then the vlink must be removed
		*/

		bool equal = false;
		for(list<highlevel::Rule>::iterator again = rules.begin(); again != rules.end(); again++)
		{
			highlevel::Action *a = again->getAction();
			if(a->getType() == highlevel::ACTION_ON_NETWORK_FUNCTION)
			{

				stringstream nf_port;
				nf_port << ((highlevel::ActionNetworkFunction*)a)->getInfo() << "_" << ((highlevel::ActionNetworkFunction*)a)->getPort();
				string nf_port_string = nf_port.str();

				if(nf_port_string == rri.nf_port)
				{
					//The action is on the same NF:port of the removed one, hence
					//the vlink must not be removed
					logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "The vlink cannot be removed, since there are other actions expressed on the NF port '%s'",rri.nf_port.c_str());
					equal = true;
					break;
				}
			}

		}//end of again iterator on the rules of the graph
		if(!equal)
		{
			//We just know that the vlink is no longer used for a NF. However, it might be used in the opposite
			//direction, for a port

			logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Virtual link no longer required for NF port: %s",rri.nf_port.c_str());
			uint64_t tobeRemovedID = nfs_vlinks.find(rri.nf_port)->second;

			//The virtual link is no longer associated with the network function port
			lsi->removeNFvlink(rri.nf_port);

			for(map<string, uint64_t>::iterator pvl = ports_vlinks.begin(); pvl != ports_vlinks.end(); pvl++)
			{
				if(pvl->second == tobeRemovedID)
				{
					logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "The virtual link cannot be removed because it is still used by the port: %s",pvl->first.c_str());
					goto next;
				}
			}

			logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "The virtual link must be removed");

			try
			{
				VLink toBeRemoved = lsi->getVirtualLink(tobeRemovedID);
				DestroyVirtualLinkIn dvli(lsi->getDpid(), toBeRemoved.getLocalID(), toBeRemoved.getRemoteDpid(), toBeRemoved.getRemoteID());
				switchManager.destroyVirtualLink(dvli);
				lsi->removeVlink(tobeRemovedID);
			} catch (SwitchManagerException e)
			{
				logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "%s",e.what());
				throw GraphManagerException();
			}
		}
	}

	if(rri.isEndpointGre)
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Check if the vlink associated with the gre endpoint '%s' must be removed (if this vlink exists)",rri.endpointGre.c_str());

	if(rri.isEndpointGre && endpoints_gre_vlinks.count(rri.endpointGre) != 0)
	{
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "The gre endpoint '%s' is associated with a vlink",rri.endpointGre.c_str());

		/**
		*	In case the gre endpoint does not appear in other actions, the vlink must be removed
		*/
		bool equal = false;
		for(list<highlevel::Rule>::iterator again = rules.begin(); again != rules.end(); again++)
		{

			highlevel::Action *a = again->getAction();
			if(a->getType() == highlevel::ACTION_ON_ENDPOINT_GRE)
			{
				if(((highlevel::ActionEndPointGre*)a)->toString() == rri.endpointGre)
				{
					//The action is on the same gre endpoint of the removed one, hence
					//the vlink must not be removed
					equal = true;
					break;
				}
			}

		}//end of again iterator on the rules of the graph
		if(!equal)
		{
			logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Virtual link no longer required for the gre endpoint: %s",rri.endpointGre.c_str());

			uint64_t tobeRemovedID = endpoints_gre_vlinks.find(rri.endpointGre)->second;
			lsi->removeEndPointGrevlink(rri.endpointGre);

			for(map<string, uint64_t>::iterator pvl = ports_vlinks.begin(); pvl != ports_vlinks.end(); pvl++)
			{
				if(pvl->second == tobeRemovedID)
				{
					logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "The virtual link cannot be removed because it is still used by the port: %s",pvl->first.c_str());
					goto next;
				}
			}

			logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "The virtual link must be removed");

			try
			{
				VLink toBeRemoved = lsi->getVirtualLink(tobeRemovedID);
				DestroyVirtualLinkIn dvli(lsi->getDpid(), toBeRemoved.getLocalID(), toBeRemoved.getRemoteDpid(), toBeRemoved.getRemoteID());
				switchManager.destroyVirtualLink(dvli);
				lsi->removeVlink(tobeRemovedID);
			} catch (SwitchManagerException e)
			{
				logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "%s",e.what());
				throw GraphManagerException();
			}
		}
	}

next:

	if(rri.isPort)
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Check of the vlink associated with the port '%s' must be removed (if this vlink exists)",rri.port.c_str());

	if(rri.isPort && ports_vlinks.count(rri.port) != 0)
	{
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "The port '%s' is associated with a vlink",rri.port.c_str());
		/**
		*	In case port does not appear in other actions, the vlink must be removed
		*/
		bool equal = false;
		for(list<highlevel::Rule>::iterator again = rules.begin(); again != rules.end(); again++)
		{
			highlevel::Action *a = again->getAction();
			if(a->getType() == highlevel::ACTION_ON_PORT)
			{
				if(((highlevel::ActionPort*)a)->getInfo() == rri.port)
				{
					//The action are the same, hence no vlink must be removed
					equal = true;
					break;
				}
			}

		}//end of again iterator on the rules of the graph
		if(!equal)
		{
			//We just know that the vlink is no longer used for a port. However, it might used in the opposite
			//direction, for a NF port
			logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Virtual link no longer required for port: %s",rri.port.c_str());
			uint64_t tobeRemovedID = ports_vlinks.find(rri.port)->second;

			lsi->removePortvlink(rri.port);

			for(map<string, uint64_t>::iterator nfvl = nfs_vlinks.begin(); nfvl != nfs_vlinks.end(); nfvl++)
			{
				if(nfvl->second == tobeRemovedID)
				{
					logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "The virtual link cannot be removed because it is still used by the NF port: %s",nfvl->first.c_str());
					goto next2;
				}
			}

			logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "The virtual link must be removed");
			try
			{
				VLink toBeRemoved = lsi->getVirtualLink(tobeRemovedID);
				DestroyVirtualLinkIn dvli(lsi->getDpid(), toBeRemoved.getLocalID(), toBeRemoved.getRemoteDpid(), toBeRemoved.getRemoteID());
				switchManager.destroyVirtualLink(dvli);
				lsi->removeVlink(tobeRemovedID);
			} catch (SwitchManagerException e)
			{
				logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "%s",e.what());
				throw GraphManagerException();
			}
		}
	}

next2:

#ifndef UNIFY_NFFG
	if(rri.isEndpointInternal)
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Check if the vlink associated with the internal endpoint '%s' must be removed (if this vlink exists)",rri.endpointInternal.c_str());

	if(rri.isEndpointInternal && endpoints_vlinks.count(rri.endpointInternal) != 0)
	{
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "The internal endpoint '%s' is associated with a vlink",rri.endpointInternal.c_str());

		/**
		*	In case the internal endpoint does not appear in other actions, the vlink must be removed
		*/
		bool equal = false;
		for(list<highlevel::Rule>::iterator again = rules.begin(); again != rules.end(); again++)
		{

			highlevel::Action *a = again->getAction();
			if(a->getType() == highlevel::ACTION_ON_ENDPOINT_INTERNAL)
			{
				if(((highlevel::ActionEndPointInternal*)a)->toString() == rri.endpointInternal)
				{
					//The action is on the same endpoint of the removed one, hence
					//the vlink must not be removed
					equal = true;
					break;
				}
			}

		}//end of again iterator on the rules of the graph
		if(!equal)
		{
			logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Virtual link no longer required for the internal endpoint: %s",rri.endpointInternal.c_str());

			uint64_t tobeRemovedID = endpoints_vlinks.find(rri.endpointInternal)->second;
			lsi->removeEndPointvlink(rri.endpointInternal);
			logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "The virtual link must be removed");

			try
			{
				VLink toBeRemoved = lsi->getVirtualLink(tobeRemovedID);
				DestroyVirtualLinkIn dvli(lsi->getDpid(), toBeRemoved.getLocalID(), toBeRemoved.getRemoteDpid(), toBeRemoved.getRemoteID());
				switchManager.destroyVirtualLink(dvli);
				lsi->removeVlink(tobeRemovedID);
			} catch (SwitchManagerException e)
			{
				logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "%s",e.what());
				throw GraphManagerException();
			}
		}
	}
#endif

	//Remove NFs, if they no longer appear in the graph
	for(list<string>::iterator nf = rri.nfs.begin(); nf != rri.nfs.end(); nf++)
	{
		if(!graph->stillExistNF(*nf))
		{
			logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "The NF '%s' is no longer part of the graph",(*nf).c_str());

			//Stop the NF
#ifdef RUN_NFS
			computeController->stopNF(*nf);
#else
			logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Flag RUN_NFS disabled. No NF to be stopped");
#endif

			set<string> nf_ports;
			map<unsigned int, string>lsi_nf_ports = lsi->getNetworkFunctionsPortsNameOnSwitchMap(*nf);
			for (map<unsigned int,string>::iterator lsi_nfp_it = lsi_nf_ports.begin(); lsi_nfp_it != lsi_nf_ports.end(); ++lsi_nfp_it) {
				nf_ports.insert(lsi_nfp_it->second);
			}

			try
			{
				DestroyNFportsIn dnpi(lsi->getDpid(), *nf, nf_ports);
				switchManager.destroyNFPorts(dnpi);
				lsi->removeNF(*nf);
			} catch (SwitchManagerException e)
			{
				logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "%s",e.what());
				throw GraphManagerException();
			}
		}
	}

	//Remove physical ports, if they no longer appear in the graph
	for(list<string>::iterator p = rri.ports.begin(); p != rri.ports.end(); p++)
	{
		if(!graph->stillExistPort(*p))
			logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "The port '%s' is no longer part of the graph",(*p).c_str());
	}

#ifndef UNIFY_NFFG
	//Remove the internal endpoint, if it no longer appear in the graph
	if((rri.endpointInternal != "") && (!graph->stillExistEndpoint(rri.endpointInternal)))
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "The internal endpoint '%s' is no longer part of the graph",rri.endpointInternal.c_str());
#endif

	//Remove the gre endpoint, if it no longer appear in the graph
	if((rri.endpointGre != "") && (!graph->stillExistEndpointGre(rri.endpointGre)))
	{
#ifdef VSWITCH_IMPLEMENTATION_OVSDB
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "The gre endpoint '%s' is no longer part of the graph",rri.endpointGre.c_str());

		try
		{
			DestroyEndpointIn depi(lsi->getDpid(),rri.endpointGre);
			switchManager.destroyEndpoint(depi);
			lsi->removeEndpoint(rri.endpointGre);
		} catch (SwitchManagerException e)
		{
			logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "%s",e.what());
			throw GraphManagerException();
		}
#else
		logger(ORCH_WARNING, MODULE_NAME, __FILE__, __LINE__, "GRE tunnel unavailable");
#endif
	}
}

#if 0
bool GraphManager::stopNetworkFunction(string graphID, string nf_name)
{
	//TODO: the NF must not have flows associated!!

	GraphInfo graphInfo = (tenantLSIs.find(graphID))->second;
	highlevel::Graph *graph = graphInfo.getGraph();
	ComputeController *computeController = graphInfo.getComputeController();
	LSI *lsi = graphInfo.getLSI();

	if(graph->stillExistNF(nf_name))
	{
		logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "The graph still contains flows associated with the NF: \"%s\"",nf_name.c_str());
		return false;
	}

#ifdef RUN_NFS
	computeController->stopNF(nf_name);
#else
	logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Flag RUN_NFS disabled. No NF to be stopped");
#endif
	try
	{
	//	list<struct nf_port_info> tmpListPorts = lsi->getNetworkFunctionsPortsInfo(nf_name);
		map<string,unsigned int>tmpListPorts= lsi->getNetworkFunctionsPorts(nf_name);

		set<string> nf_ports;
		for (map<string,unsigned int>::iterator lsi_nfp_it = tmpListPorts.begin(); lsi_nfp_it != tmpListPorts.end(); ++lsi_nfp_it)
				nf_ports.insert(lsi_nfp_it->first);


//		set<struct nf_port_info> portsToBeRemoved(tmpListPorts.begin(),tmpListPorts.end());

		DestroyNFportsIn dnpi(lsi->getDpid(),nf_name,/*portsToBeRemoved*/nf_ports);
		switchManager.destroyNFPorts(dnpi);
		lsi->removeNF(nf_name);
	} catch (SwitchManagerException e)
	{
		logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "%s",e.what());
		return false;
	}

	return true;
}
#endif

string GraphManager::findEndPointTowardsGRE(highlevel::Graph *graph, string ep)
{
	list<highlevel::Rule> rules = graph->getRules();
	for(list<highlevel::Rule>::iterator rule = rules.begin(); rule != rules.end(); rule++)
	{
		highlevel::Action *action = rule->getAction();
		highlevel::Match match = rule->getMatch();
		if(action->getType() == highlevel::ACTION_ON_ENDPOINT_GRE && match.matchOnEndPointInternal())
		{
			highlevel::ActionEndPointGre *action_ep = (highlevel::ActionEndPointGre*)action;
			stringstream ss;
			ss << action_ep->getPort();

			if(ep == ss.str())
			{
				stringstream ssm;
				ssm << match.getEndPoint();
				return ssm.str();
			}
		}
	}

	assert(0);

	return ""; //just for the compiler
}

string GraphManager::findEndPointTowardsNF(highlevel::Graph *graph, string nf)
{
	list<highlevel::Rule> rules = graph->getRules();
	for(list<highlevel::Rule>::iterator rule = rules.begin(); rule != rules.end(); rule++)
	{
		highlevel::Action *action = rule->getAction();
		highlevel::Match match = rule->getMatch();
		if(action->getType() == highlevel::ACTION_ON_NETWORK_FUNCTION && match.matchOnEndPointInternal())
		{
			highlevel::ActionNetworkFunction *action_nf = (highlevel::ActionNetworkFunction*)action;
			stringstream ss;
			ss << action->getInfo() << "_" << action_nf->getPort();

			if(nf == ss.str())
			{
				stringstream ssm;
				ssm << match.getEndPoint();
				return ssm.str();
			}
		}
	}

	assert(0);

	return ""; //just for the compiler
}

#if 0
bool GraphManager::canDeleteFlow(highlevel::Graph *graph, string flowID)
{
	highlevel::Rule r = graph->getRuleFromID(flowID);
	highlevel::Match m = r.getMatch();
	highlevel::Action *a = r.getAction();

	map<string, vector<string> > endpoints = graph->getEndPoints();
	for(map<string, vector<string> >::iterator mep = endpoints.begin(); mep != endpoints.end(); mep++)
	{
		string ep = mep->first;

		if( (a->getType() == highlevel::ACTION_ON_ENDPOINT) && (a->toString() == ep) )
		{
			if(availableEndPoints.find(ep)->second !=0)
			{
				logger(ORCH_INFO, MODULE_NAME, __FILE__, __LINE__, "The flow cannot be deleted. It defines (in the action) the endpoint \"%s\" that is used %d times in other graphs; first remove the rules in those graphs.",ep.c_str(),availableEndPoints.find(ep)->second);
				return false;
			}
		}
		if(m.matchOnEndPoint())
		{
			stringstream ss;
			ss << m.getGraphID() << ":" << m.getEndPoint();
			if(ss.str() == ep && availableEndPoints.find(ep)->second !=0)
			{
				logger(ORCH_INFO, MODULE_NAME, __FILE__, __LINE__, "The flow cannot be deleted. It defines (in the match) the endpoint \"%s\" that is used %d times in other graphs; first remove the rules in those graphs.",ep.c_str(),availableEndPoints.find(ep)->second);
				return false;
			}
		}
	}

	return true;
}
#endif

void GraphManager::printInfo(lowlevel::Graph graphLSI0, LSI *lsi0)
{
	logger(ORCH_INFO, MODULE_NAME, __FILE__, __LINE__, "");
	logger(ORCH_INFO, MODULE_NAME, __FILE__, __LINE__, "");
	logger(ORCH_INFO, MODULE_NAME, __FILE__, __LINE__, "Graphs deployed:");
	logger(ORCH_INFO, MODULE_NAME, __FILE__, __LINE__, "");

	logger(ORCH_INFO, MODULE_NAME, __FILE__, __LINE__, "\tID: 'LSI-0'");
	logger(ORCH_INFO, MODULE_NAME, __FILE__, __LINE__, "\tRules: ");

	map<string,LSI *> lsis;
	for(map<string,GraphInfo>::iterator graphs = tenantLSIs.begin(); graphs != tenantLSIs.end(); graphs++)
		lsis[graphs->first] = graphs->second.getLSI();

	graphLSI0.prettyPrint(lsi0,lsis);
	logger(ORCH_INFO, MODULE_NAME, __FILE__, __LINE__, "");

	printInfo(false);
}

void GraphManager::printInfo(bool completed)
{
	if(completed)
	{
		logger(ORCH_INFO, MODULE_NAME, __FILE__, __LINE__, "");
		logger(ORCH_INFO, MODULE_NAME, __FILE__, __LINE__, "");
		logger(ORCH_INFO, MODULE_NAME, __FILE__, __LINE__, "Graphs deployed:");
		logger(ORCH_INFO, MODULE_NAME, __FILE__, __LINE__, "");
	}

	map<string,GraphInfo>::iterator it;
	for(it = tenantLSIs.begin(); it != tenantLSIs.end(); it++)
	{
		int id;
		sscanf(it->first.c_str(),"%d",&id);

		if(id == 2)
		{
			coloredLogger(ANSI_COLOR_BLUE,ORCH_INFO, MODULE_NAME, __FILE__, __LINE__, "\tGraph ID: '%s'",it->first.c_str());
			coloredLogger(ANSI_COLOR_BLUE,ORCH_INFO, MODULE_NAME, __FILE__, __LINE__, "\tVNF installed:");
		}
		else if(id == 3)
		{
			coloredLogger(ANSI_COLOR_RED,ORCH_INFO, MODULE_NAME, __FILE__, __LINE__, "\tID: '%s'",it->first.c_str());
			coloredLogger(ANSI_COLOR_RED,ORCH_INFO, MODULE_NAME, __FILE__, __LINE__, "\tVNF installed:");
		}
		else
		{
			logger(ORCH_INFO, MODULE_NAME, __FILE__, __LINE__, "\tID: '%s'",it->first.c_str());
			logger(ORCH_INFO, MODULE_NAME, __FILE__, __LINE__, "\tVNF installed:");
		}

		ComputeController *computeController = it->second.getComputeController();
		computeController->printInfo(id);
		logger(ORCH_INFO, MODULE_NAME, __FILE__, __LINE__, "");
	}

	logger(ORCH_INFO, MODULE_NAME, __FILE__, __LINE__, "");
}

