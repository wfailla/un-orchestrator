#ifndef NODE_ORCHESTRATOR_DESCRIPTION_PARSER_H
#define NODE_ORCHESTRATOR_DESCRIPTION_PARSER_H

#include <string>
#include <fstream>
#include <json_spirit/json_spirit.h>
#include <json_spirit/value.h>
#include <json_spirit/writer.h>

#include "../../utils/logger.h"
#include "resource_manager.h"
#include "domain_description/domain_description.h"
#include "domain_description/network_manager.h"
#include "keywords.h"

using namespace json_spirit;
using namespace std;

class DescriptionParser
{
private:
	static bool parseInterface(Object &openconfig_if_interface,domainInformations::Interface& interface);

	friend ResourceManager;
protected:
    static bool parseDescription(string description,domainInformations::DomainDescription *domainDescription);
};

#endif //NODE_ORCHESTRATOR_DESCRIPTION_PARSER_H
