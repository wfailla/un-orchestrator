#include "high_level_output_action_endpoint_internal.h"

namespace highlevel
{

ActionEndPointInternal::ActionEndPointInternal(string graphID, unsigned int endpoint, string input_endpoint) :
	Action(ACTION_ON_ENDPOINT_INTERNAL), graphID(graphID), endpoint(endpoint), input_endpoint(input_endpoint)
{
}	

bool ActionEndPointInternal::operator==(const ActionEndPointInternal &other) const
{
	if((graphID == other.graphID) && (endpoint == other.endpoint))
		return true;
		
	return false;
}

string ActionEndPointInternal::getInfo()
{
	return graphID;
}

unsigned int ActionEndPointInternal::getPort()
{
	return endpoint;
}

string ActionEndPointInternal::getInputEndpoint()
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

string ActionEndPointInternal::toString()
{
	stringstream ss;
	ss << graphID << ":" << endpoint;
	
	return ss.str();
}

void ActionEndPointInternal::print()
{
	if(LOGGING_LEVEL <= ORCH_DEBUG_INFO)
	{
		cout << "\t\tAction:" << endl << "\t\t{" << endl;
		cout << "\t\t\toutput_to_port: " << graphID << ":" << endpoint << endl;
		for(list<GenericAction*>::iterator ga = genericActions.begin(); ga != genericActions.end(); ga++)
			(*ga)->print();
		cout << "\t\t}" << endl;
	}
}

Object ActionEndPointInternal::toJSON()
{
	Object action;
	stringstream ss;
	ss << graphID << ":" << input_endpoint;

	action[OUTPUT] = ss.str().c_str();
	
	for(list<GenericAction*>::iterator ga = genericActions.begin(); ga != genericActions.end(); ga++)
		(*ga)->toJSON(action);
	
	return action;
}

}
