#include "monitoring_controller.h"

MonitoringController::MonitoringController()
{
}

void MonitoringController::startMonitoring(string measure_string, list< pair<string, string> > vnfsMapping, list<map<unsigned int, string> > portsMapping)
{
	logger(ORCH_INFO, MONITORING_CONTROLLER_MODULE_NAME, __FILE__, __LINE__, "Starting a monitoring session (through the monitoring plugin)");

	assert(vnfsMapping.size() == portsMapping.size());

	//Prepare the json to publish on the Double Decker bus
	list<map<unsigned int, string> >::iterator pm = portsMapping.begin();
	list<pair<string, string> >::iterator vm = vnfsMapping.begin();

	Array vnfs_array;
	for(; pm != portsMapping.end(); pm++, vm++)
	{
		Object vnf;
		vnf["id"] = "";
		
		Array ports;
	
		map<unsigned int, string> m = *pm;
		for(map<unsigned int, string>::iterator mm = m.begin(); mm != m.end(); mm++)
		{
			Object port;
			
			port["id"] = mm->first;
			port["name"] = mm->second;
		
			ports.push_back(port);
		}

		vnf["id"] = vm->first;
		vnf["name"] = vm->second;
		vnf["ports"] = ports;
	
		vnfs_array.push_back(vnf);
	}

	Object nffg;
	nffg["measure"] = measure_string.c_str();
	nffg["VNFs"] = vnfs_array;
	
	Object params;
	params["nffg"] = nffg;
	
	Object json;
	json["params"] = params;
	json["jsonrpc"] = "2.0";
	json["method"] = "startNFFG";

	stringstream message;
	write_formatted(json,message);

	logger(ORCH_DEBUG_INFO, MONITORING_CONTROLLER_MODULE_NAME, __FILE__, __LINE__, "Sending the following message to the plugin monitor:");
	logger(ORCH_DEBUG_INFO, MONITORING_CONTROLLER_MODULE_NAME, __FILE__, __LINE__, "%s",message.str().c_str());

	DoubleDeckerClient::publish(UNIFY_MPP,message.str().c_str());
}

void MonitoringController::stopMonitoring()
{
	logger(ORCH_INFO, MONITORING_CONTROLLER_MODULE_NAME, __FILE__, __LINE__, "Stopping a monitoring session (through the monitoring plugin)");

	Object json;
	json["params"] = "";
	json["jsonrpc"] = "2.0";
	json["method"] = "stopNFFG";

	stringstream message;
	write_formatted(json,message);

	logger(ORCH_DEBUG_INFO, MONITORING_CONTROLLER_MODULE_NAME, __FILE__, __LINE__, "Sending the following message to the plugin monitor:");
	logger(ORCH_DEBUG_INFO, MONITORING_CONTROLLER_MODULE_NAME, __FILE__, __LINE__, "%s",message.str().c_str());

	DoubleDeckerClient::publish(UNIFY_MPP,message.str().c_str());
}
