#ifndef NODE_ORCHESTRATOR_INTERFACE_OPTIONS_H
#define NODE_ORCHESTRATOR_INTERFACE_OPTIONS_H

#include <string>
#include <map>
#include <json_spirit/json_spirit.h>
#include <json_spirit/value.h>
#include <json_spirit/writer.h>
#include "../keywords.h"

using namespace std;
using namespace json_spirit;

namespace domainInformations
{

    class InterfaceOptions
    {
    private:
        /**
        *	@brief: local ip of gre tunnel
        */
        string localIP;
        /**
        *	@brief: remote ip of gre tunnel
        */
        string remoteIP;
        /**
        *	@brief: key of gre tunnel
        */
        string key;

    public:

        InterfaceOptions();
        void setGreLocalIP(string localIP);
        void setGreRemoteIP(string remoteIP);
        void setGreKey(string key);
		Object toJSON();

    };

}



#endif //NODE_ORCHESTRATOR_INTERFACE_OPTIONS_H
