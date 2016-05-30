#ifndef NODE_ORCHESTRATOR_INTERFACE_CAPABILITIES_H
#define NODE_ORCHESTRATOR_INTERFACE_CAPABILITIES_H

using namespace std;

namespace domainInformations
{

    class InterfaceCapabilities
    {
    private:
        /**
        *	@brief: describes if an interface supports gre
        */
        bool greSupport;
    public:
        InterfaceCapabilities();
        void setGreSupport(bool greSupport);
    };

}

#endif //NODE_ORCHESTRATOR_INTERFACE_CAPABILITIES_H
