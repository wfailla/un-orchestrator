#include "interface_state.h"

namespace domainInformations
{
    InterfaceState::InterfaceState()
    {

    }

    void InterfaceState::setAdminStatus(string status)
    {
        if(status==STATUS_UP)
            adminStatus = INTERFACE_STATE_UP;
        else
            adminStatus = INTERFACE_STATE_DOWN;
    }

    void InterfaceState::setOperStatus(string status)
    {
        if(status==STATUS_UP)
            operStatus = INTERFACE_STATE_UP;
        else
            operStatus = INTERFACE_STATE_DOWN;
    }


}
