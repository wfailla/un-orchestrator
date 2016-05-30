#ifndef NODE_ORCHESTRATOR_NEIGH_H
#define NODE_ORCHESTRATOR_NEIGH_H

#include <string>

using namespace std;

namespace domainInformations
{

    class NeighborInfo
    {
    private:
        /**
        *	@brief: name of neighbor domain
        */
        string domainName;
        /**
        *	@brief: name of neighbor type
        */
        string remoteInterface;
    public:

        NeighborInfo();
        void setDomainName(string name);
        void setRemoteInterface(string remoteInterface);
    };

}


#endif //NODE_ORCHESTRATOR_NEIGH_H
