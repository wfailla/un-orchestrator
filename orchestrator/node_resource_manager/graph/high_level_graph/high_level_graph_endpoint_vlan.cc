#include "high_level_graph_endpoint_vlan.h"

namespace highlevel
{

EndPointVlan::EndPointVlan(string id, string name, string vlan_id, string node_id, string sw_id, string interface) :
	id(id), name(name), vlan_id(vlan_id), node_id(node_id), sw_id(sw_id), interface(interface)
{

}

EndPointVlan::~EndPointVlan()
{

}

bool EndPointVlan::operator==(const EndPointVlan &other) const
{
	if(id == other.id && name == other.name)
		return true;
		
	return false;
}

string EndPointVlan::getId()
{
	return id;
}

string EndPointVlan::getName()
{
	return name;
}

string EndPointVlan::getVlanId()
{
	return vlan_id;
}

string EndPointVlan::getNodeId()
{
	return node_id;
}

string EndPointVlan::getSwId()
{
	return sw_id;
}

string EndPointVlan::getInterface()
{
	return interface;
}

void EndPointVlan::print()
{
	if(LOGGING_LEVEL <= ORCH_DEBUG_INFO)
	{
		cout << "\t\tid:" << id << endl;
		cout << "\t\t\tname: " << name << endl;
		cout << "\t\ttype:" << VLAN << endl;
		cout << "\t\t\tvlan: " << endl << "\t\t{" << endl;
		cout << "\t\t\tvlan-id: " << vlan_id << endl;
		cout << "\t\t\tnode-id: " << node_id << endl;
		cout << "\t\tswitch-id:" << sw_id << endl;
		cout << "\t\t\tinterface: " << interface << endl;
		cout << "\t\t}" << endl;
	}
}

Object EndPointVlan::toJSON()
{
	Object EndPointVlan, vlan;
	
	EndPointVlan[_ID] = id.c_str();
	EndPointVlan[_NAME] = name.c_str();
	EndPointVlan[EP_TYPE] = VLAN;
	
	vlan[VLAN_ID] = vlan_id.c_str();
	vlan[NODE_ID] = node_id.c_str();
	if(strcmp(sw_id.c_str(), "") != 0)
		vlan[SW_ID] = sw_id.c_str();
	vlan[IFACE] = interface.c_str();
	
	EndPointVlan[VLAN] = vlan;
	
	return EndPointVlan;
}

}
