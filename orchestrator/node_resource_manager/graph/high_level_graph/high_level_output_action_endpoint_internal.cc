#include "high_level_output_action_endpoint_internal.h"

namespace highlevel
{

ActionEndPointInternal::ActionEndPointInternal(string group, string input_endpoint) :
	Action(ACTION_ON_ENDPOINT_INTERNAL), group(group), input_endpoint(input_endpoint)
{
}	

bool ActionEndPointInternal::operator==(const ActionEndPointInternal &other) const
{
	if(/*(graphID == other.graphID) && */group == other.group)
		return true;
		
	return false;
}

string ActionEndPointInternal::getInfo()
{
	return group;
}

unsigned int ActionEndPointInternal::getGroup()
{
	unsigned int in_group;

	sscanf(group.c_str(), "%u", &in_group);

	return in_group;
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
	ss << group;
	
	return ss.str();
}

Object ActionEndPointInternal::toJSON()
{
	Object action;
	stringstream ss;
	ss << input_endpoint;

	action[OUTPUT] = ss.str().c_str();
	
	for(list<GenericAction*>::iterator ga = genericActions.begin(); ga != genericActions.end(); ga++)
		(*ga)->toJSON(action);
	
	return action;
}

}
