#include "interface_state.h"

namespace domainInformations
{
    InterfaceState::InterfaceState()
    {

    }

    void InterfaceState::setAdminStatus(string status)
    {
		adminStatus = status;
    }

    void InterfaceState::setOperStatus(string status)
    {
		operStatus = status;
    }

	Object InterfaceState::toJSON()
	{
		Object state;
		if(adminStatus!="")
			state[ADMIN_STATUS]=adminStatus;
		if(operStatus!="")
			state[OPER_STATUS]=operStatus;
		return state;
	}

}
