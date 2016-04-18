#include "high_level_output_action_endpoint_gre.h"

namespace highlevel
{

ActionEndPointGre::ActionEndPointGre(unsigned int endpoint, string input_endpoint) :
	Action(ACTION_ON_ENDPOINT_GRE), endpoint(endpoint), input_endpoint(input_endpoint)
{
}

bool ActionEndPointGre::operator==(const ActionEndPointGre &other) const
{
	if((endpoint == other.endpoint))
		return true;

	return false;
}

string ActionEndPointGre::getInfo()
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
			case 0:
				str = string(pnt);
				break;
		}

		pnt = strtok( NULL, delimiter );
		i++;
	}

	return str;
}

unsigned int ActionEndPointGre::getPort()
{
	return endpoint;
}

string ActionEndPointGre::getInputEndpoint()
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
				break;
		}

		pnt = strtok( NULL, delimiter );
		i++;
	}

	return str;
}

string ActionEndPointGre::toString()
{
	stringstream ss;
	ss << endpoint;

	return ss.str();
}

void ActionEndPointGre::print()
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

Object ActionEndPointGre::toJSON()
{
	Object action;
	action[OUTPUT] = input_endpoint.c_str();

	for(list<GenericAction*>::iterator ga = genericActions.begin(); ga != genericActions.end(); ga++)
		(*ga)->toJSON(action);

	return action;
}

}
