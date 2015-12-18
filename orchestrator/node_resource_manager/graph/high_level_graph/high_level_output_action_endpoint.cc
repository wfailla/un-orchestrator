#include "high_level_output_action_endpoint.h"

namespace highlevel
{

ActionEndPoint::ActionEndPoint(unsigned int endpoint) :
	Action(ACTION_ON_ENDPOINT), endpoint(endpoint)
{
}	

bool ActionEndPoint::operator==(const ActionEndPoint &other) const
{
	if((endpoint == other.endpoint))
		return true;
		
	return false;
}

string ActionEndPoint::getInfo()
{
	return "";
}

unsigned int ActionEndPoint::getPort()
{
	return endpoint;
}

string ActionEndPoint::toString()
{
	stringstream ss;
	ss << endpoint;
	
	return ss.str();
}

void ActionEndPoint::print()
{
	if(LOGGING_LEVEL <= ORCH_DEBUG_INFO)
	{
		cout << "\t\tAction:" << endl << "\t\t{" << endl;
		cout << "\t\t\tendpoint: " << endpoint << endl;
		for(list<GenericAction*>::iterator ga = genericActions.begin(); ga != genericActions.end(); ga++)
			(*ga)->print();
		cout << "\t\t}" << endl;
	}
}

Object ActionEndPoint::toJSON()
{
	Object action;
	stringstream ep;
	ep << endpoint;
	action[ENDPOINT] = ep.str().c_str();
	
	for(list<GenericAction*>::iterator ga = genericActions.begin(); ga != genericActions.end(); ga++)
		(*ga)->toJSON(action);
	
	return action;
}

}
