#ifndef MONITORING_CONTROLLER_H_
#define MONITORING_CONTROLLER_H_ 1

#include <json_spirit/json_spirit.h>
#include <json_spirit/value.h>
#include <json_spirit/writer.h>

#include "../node_resource_manager/pub_sub/pub_sub.h"
#include "../utils/logger.h"
#include "monitoring_controller_constants.h"

using namespace json_spirit;

class MonitoringController
{

public:
	MonitoringController();
	
	/**
	*	@brief: interact with the monitoring plugin
	*
	*	@param measure_string: string formatted according to the MEASURE language
	*/
	void setMonitoring(string measure_string);
};

#endif //MONITORING_CONTROLLER_H_
