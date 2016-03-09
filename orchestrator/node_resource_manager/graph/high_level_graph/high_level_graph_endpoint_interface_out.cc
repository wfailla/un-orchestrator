#include "high_level_graph_endpoint_interface_out.h"

namespace highlevel
{

EndPointInterfaceOut::EndPointInterfaceOut(string id, string name, string node_id, string sw_id, string interface) :
	id(id), name(name), node_id(node_id), sw_id(sw_id), interface(interface)
{

}

EndPointInterfaceOut::~EndPointInterfaceOut()
{

}

bool EndPointInterfaceOut::operator==(const EndPointInterfaceOut &other) const
{
	if(id == other.id && name == other.name)
		return true;

	return false;
}

string EndPointInterfaceOut::getId()
{
	return id;
}

string EndPointInterfaceOut::getName()
{
	return name;
}

string EndPointInterfaceOut::getNodeId()
{
	return node_id;
}

string EndPointInterfaceOut::getSwId()
{
	return sw_id;
}

string EndPointInterfaceOut::getInterface()
{
	return interface;
}

void EndPointInterfaceOut::print()
{
	if(LOGGING_LEVEL <= ORCH_DEBUG_INFO)
	{
		cout << "\t\tid:" << id << endl;
		cout << "\t\t\tname: " << name << endl;
		cout << "\t\ttype:" << IFACE << endl;
		cout << "\t\t\tinterface-out: " << endl << "\t\t{" << endl;
		cout << "\t\t\tnode-id: " << node_id << endl;
		cout << "\t\t\tsw-id: " << sw_id << endl;
		cout << "\t\tinterface:" << interface << endl;
		cout << "\t\t}" << endl;
	}
}

Object EndPointInterfaceOut::toJSON()
{
	Object EndPointInterfaceOut, iface;

	EndPointInterfaceOut[_ID] = id.c_str();
	EndPointInterfaceOut[_NAME] = name.c_str();
	EndPointInterfaceOut[EP_TYPE] = EP_IFACE_OUT;

	iface[NODE_ID] = node_id.c_str();
	if(strcmp(sw_id.c_str(), "") != 0)
		iface[SW_ID] = sw_id.c_str();
	iface[IFACE] = interface.c_str();

	EndPointInterfaceOut[EP_IFACE_OUT] = iface;

	return EndPointInterfaceOut;
}

}
