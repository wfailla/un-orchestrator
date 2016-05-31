#ifndef NODE_ORCHESTRATOR_NEIGH_H
#define NODE_ORCHESTRATOR_NEIGH_H

#include <string>
#include <json_spirit/json_spirit.h>
#include <json_spirit/value.h>
#include <json_spirit/writer.h>
#include "../keywords.h"

using namespace std;
using namespace json_spirit;

namespace domainInformations
{

    class NeighborInfo
    {
    private:
        /**
        *	@brief: name of neighbor domain (e.g. "UN")
        */
        string domainName;
        /**
        *	@brief: interface of neighbor
        */
        string remoteInterface;
        /**
        *	@brief: name of neighbor type
        */
        string domainType;
    public:

        NeighborInfo();
        void setDomainName(string name);
        void setRemoteInterface(string remoteInterface);
        void setType(string domainType);
		Object toJSON();

	};

}


#endif //NODE_ORCHESTRATOR_NEIGH_H
