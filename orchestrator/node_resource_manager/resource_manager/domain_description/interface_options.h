#ifndef NODE_ORCHESTRATOR_INTERFACE_OPTIONS_H
#define NODE_ORCHESTRATOR_INTERFACE_OPTIONS_H

#include <string>
#include <map>

using namespace std;

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

    };

}



#endif //NODE_ORCHESTRATOR_INTERFACE_OPTIONS_H
