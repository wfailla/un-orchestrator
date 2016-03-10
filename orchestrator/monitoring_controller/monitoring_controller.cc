#include "monitoring_controller.h"

MonitoringController::MonitoringController()
{

}

void MonitoringController::setMonitoring(string measure_string)
{
	logger(ORCH_INFO, MONITORING_CONTROLLER_MODULE_NAME, __FILE__, __LINE__, "Starting interaction with the monitoring plugin");

	//Prepare the json to publish on the Double Decker bus

	Object nffg;
	nffg["measure"] = measure_string.c_str();
	
	

	Object params;
	params["nffg"] = nffg;
	
	Object json;
	json["params"] = params;
	json["jsonrpc"] = "2.0";
	json["method"] = "updateNFFG";

 	stringstream message;
 	write_formatted(json,message);

	logger(ORCH_DEBUG_INFO, MONITORING_CONTROLLER_MODULE_NAME, __FILE__, __LINE__, "Sending the following message to the plugin monitor:");
	logger(ORCH_DEBUG_INFO, MONITORING_CONTROLLER_MODULE_NAME, __FILE__, __LINE__, "%s",message.str().c_str());

	DoubleDeckerClient::publish(UNIFY_MPP,message.str().c_str());
}
