#include "high_level_output_action_nf.h"

namespace highlevel
{

ActionNetworkFunction::ActionNetworkFunction(string nf, string nf_endpoint_port, unsigned int port) :
	Action(ACTION_ON_NETWORK_FUNCTION), nf(nf), nf_endpoint_port(nf_endpoint_port), port(port)
{

}

bool ActionNetworkFunction::operator==(const ActionNetworkFunction &other) const
{
	if((nf == other.nf) && (port == other.port) && (nf_endpoint_port == other.nf_endpoint_port))
		return true;

	return false;
}

string ActionNetworkFunction::getInfo()
{
	return nf;
}

unsigned int ActionNetworkFunction::getPort()
{
	return port;
}

string ActionNetworkFunction::toString()
{
	stringstream ss;
	ss << nf << ":" << port;

	return ss.str();
}

Object ActionNetworkFunction::toJSON()
{
	Object action;
	stringstream network_function;
	network_function << nf << ":" << port;
	action[OUTPUT] = nf_endpoint_port.c_str();

	for(list<GenericAction*>::iterator ga = genericActions.begin(); ga != genericActions.end(); ga++)
		(*ga)->toJSON(action);

	return action;
}

}
