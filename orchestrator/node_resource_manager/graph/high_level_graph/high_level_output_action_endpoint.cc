#include "high_level_output_action_endpoint.h"

namespace highlevel
{

ActionEndPoint::ActionEndPoint(unsigned int endpoint, string input_endpoint) :
	Action(ACTION_ON_ENDPOINT), endpoint(endpoint), input_endpoint(input_endpoint)
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

string ActionEndPoint::getInputEndpoint()
{
	//Check the name of port
	char delimiter[] = ":";
	char * pnt;

	string str;

	char tmp[BUFFER_SIZE];
	strcpy(tmp,(char *)input_endpoint.c_str());
	pnt=strtok(tmp, delimiter);
	int i = 0;

	while( pnt!= NULL )
	{
		switch(i)
		{
			case 1:
				str = string(pnt);
		}

		pnt = strtok( NULL, delimiter );
		i++;
	}

	return str;
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
		cout << "\t\t\toutput_to_port: " << input_endpoint << endl;
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
	action[OUTPUT] = input_endpoint.c_str();

	for(list<GenericAction*>::iterator ga = genericActions.begin(); ga != genericActions.end(); ga++)
		(*ga)->toJSON(action);

	return action;
}

}
