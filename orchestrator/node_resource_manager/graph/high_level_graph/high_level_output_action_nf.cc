#include "high_level_output_action_nf.h"

namespace highlevel
{

ActionNetworkFunction::ActionNetworkFunction(string nfName, string endpointPortName, unsigned int port) :
	Action(ACTION_ON_NETWORK_FUNCTION), nfName(nfName), endpointPortName(endpointPortName), port(port)
{

}

bool ActionNetworkFunction::operator==(const ActionNetworkFunction &other) const
{
	if((nfName == other.nfName) && (port == other.port) && (endpointPortName == other.endpointPortName))
		return true;

	return false;
}

string ActionNetworkFunction::getInfo()
{
	return nfName;
}

unsigned int ActionNetworkFunction::getPort()
{
	return port;
}

string ActionNetworkFunction::toString()
{
	stringstream ss;
	ss << nfName << ":" << port;

	return ss.str();
}

Object ActionNetworkFunction::toJSON()
{
	Object action;
	stringstream network_function;
	network_function << nfName << ":" << port;
	action[OUTPUT] = endpointPortName.c_str();

	for(list<GenericAction*>::iterator ga = genericActions.begin(); ga != genericActions.end(); ga++)
		(*ga)->toJSON(action);

	return action;
}

}
