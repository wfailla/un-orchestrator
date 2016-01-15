#include "high_level_graph_endpoint_interface.h"

namespace highlevel
{

EndPointInterface::EndPointInterface(string id, string name, string node_id, string sw_id, string interface) :
	id(id), name(name), node_id(node_id), sw_id(sw_id), interface(interface)
{

}

EndPointInterface::~EndPointInterface()
{

}

bool EndPointInterface::operator==(const EndPointInterface &other) const
{
	if(id == other.id && name == other.name)
		return true;
		
	return false;
}

string EndPointInterface::getId()
{
	return id;
}

string EndPointInterface::getName()
{
	return name;
}

string EndPointInterface::getNodeId()
{
	return node_id;
}

string EndPointInterface::getSwId()
{
	return sw_id;
}

string EndPointInterface::getInterface()
{
	return interface;
}

void EndPointInterface::print()
{
	if(LOGGING_LEVEL <= ORCH_DEBUG_INFO)
	{
		cout << "\t\tid:" << id << endl;
		cout << "\t\t\tname: " << name << endl;
		cout << "\t\ttype:" << IFACE << endl;
		cout << "\t\t\tinterface: " << endl << "\t\t{" << endl;
		cout << "\t\t\tnode-id: " << node_id << endl;
		cout << "\t\t\tsw-id: " << sw_id << endl;
		cout << "\t\tinterface:" << interface << endl;
		cout << "\t\t}" << endl;
	}
}

Object EndPointInterface::toJSON()
{
	Object EndPointInterface, iface;
	
	EndPointInterface[EP_ID] = id.c_str();
	EndPointInterface[EP_NAME] = name.c_str();
	EndPointInterface[EP_TYPE] = IFACE;
	
	iface[NODE_ID] = node_id.c_str();
	if(strcmp(sw_id.c_str(), "") != 0)
		iface[SW_ID] = sw_id.c_str();
	iface[IFACE] = interface.c_str();
	
	EndPointInterface[IFACE] = iface;
	
	return EndPointInterface;
}

}
