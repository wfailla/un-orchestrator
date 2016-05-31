#ifndef NODE_ORCHESTRATOR_INTERFACE_STATE_H
#define NODE_ORCHESTRATOR_INTERFACE_STATE_H

#include <string>
#include <json_spirit/json_spirit.h>
#include <json_spirit/value.h>
#include <json_spirit/writer.h>
#include "../keywords.h"

using namespace std;
using namespace json_spirit;

namespace domainInformations
{

    class InterfaceState
    {
    private:
        /**
        *	@brief: interface status for admin (e.g. "UP")
        */
        string adminStatus;
        /**
        *	@brief: interface status for operator (?) (e.g. "DOWN")
        */
		string operStatus;

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

		Object toJSON();

	};

}



#endif //NODE_ORCHESTRATOR_INTERFACE_STATE_H
