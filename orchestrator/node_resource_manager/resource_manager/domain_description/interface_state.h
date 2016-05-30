#ifndef NODE_ORCHESTRATOR_INTERFACE_STATE_H
#define NODE_ORCHESTRATOR_INTERFACE_STATE_H

#include <string>
#include "../keywords.h"
using namespace std;

namespace domainInformations
{

    class InterfaceState
    {
    private:
        typedef enum{INTERFACE_STATE_UP,INTERFACE_STATE_DOWN}status_t;
        /**
        *	@brief: interface status for admin (e.g. "UP")
        */
        status_t adminStatus;
        /**
        *	@brief: interface status for operator (?) (e.g. "DOWN")
        */
        status_t operStatus;

    public:

        InterfaceState();

        /**
        *	@brief: set the interface status for admin
        */
        void setAdminStatus(string status);

        /**
        *	@brief: set the interface status for operator (?)
        */
        void setOperStatus(string status);

    };

}



#endif //NODE_ORCHESTRATOR_INTERFACE_STATE_H
