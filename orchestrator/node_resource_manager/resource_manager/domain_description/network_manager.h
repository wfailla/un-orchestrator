#ifndef NODE_ORCHESTRATOR_NETWORK_MANAGER_H
#define NODE_ORCHESTRATOR_NETWORK_MANAGER_H

#include <list>
#include <json_spirit/json_spirit.h>
#include <json_spirit/value.h>
#include <json_spirit/writer.h>
#include "../keywords.h"
#include "interface.h"

using namespace std;
using namespace json_spirit;

namespace domainInformations
{

    class NetworkManager
    {
    private:
        /**
        *	@brief: list of interfaces
        */
        list<Interface> interfaces;
    public:
        NetworkManager();
        void addInterface(Interface interface);
        Object toJSON();
    };

}


#endif //NODE_ORCHESTRATOR_NETWORK_MANAGER_H
