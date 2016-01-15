#include "high_level_output_action_port.h"

namespace highlevel
{

ActionPort::ActionPort(string port, string input_port) :
	Action(ACTION_ON_PORT), port(port), input_port(input_port)
{

}

ActionPort::~ActionPort()
{

}

bool ActionPort::operator==(const ActionPort &other) const
{
	if(port == other.port && input_port == other.input_port)
		return true;
		
	return false;
}

string ActionPort::getInfo()
{
	return port;
}

string ActionPort::toString()
{
	return port;
}

void ActionPort::print()
{
	if(LOGGING_LEVEL <= ORCH_DEBUG_INFO)
	{
		cout << "\t\tAction:" << endl << "\t\t{" << endl;
		cout << "\t\t\toutput_to_port: " << port << endl;
		for(list<GenericAction*>::iterator ga = genericActions.begin(); ga != genericActions.end(); ga++)
			(*ga)->print();
		cout << "\t\t}" << endl;
	}
}

Object ActionPort::toJSON()
{
	Object action;
	action[OUTPUT] = input_port.c_str();
	
	for(list<GenericAction*>::iterator ga = genericActions.begin(); ga != genericActions.end(); ga++)
		(*ga)->toJSON(action);
	
	return action;
}

}
