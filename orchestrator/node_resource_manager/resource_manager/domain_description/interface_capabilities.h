#ifndef NODE_ORCHESTRATOR_INTERFACE_CAPABILITIES_H
#define NODE_ORCHESTRATOR_INTERFACE_CAPABILITIES_H

#include <json_spirit/json_spirit.h>
#include <json_spirit/value.h>
#include <json_spirit/writer.h>
#include "../keywords.h"
using namespace std;
using namespace json_spirit;

namespace domainInformations
{

    class InterfaceCapabilities
    {
    private:
        /**
        *	@brief: describes if an interface supports gre
        */
        bool greSupport;
		/**
        *	@brief: describes if greSupport variable makes sense
        */
		bool isGreSupportDefined;
    public:
        InterfaceCapabilities();
        void setGreSupport(bool greSupport);
        Object toJSON();
    };

}

#endif //NODE_ORCHESTRATOR_INTERFACE_CAPABILITIES_H
