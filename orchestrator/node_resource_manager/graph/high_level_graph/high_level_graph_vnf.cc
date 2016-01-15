#include "high_level_graph_vnf.h"

namespace highlevel
{

VNFs::VNFs(string id, string name, string groups, string vnf_template, list<pair<string, string> > ports) :
	id(id), name(name), groups(groups), vnf_template(vnf_template)
{
	for(list<pair<string, string> >::iterator p = ports.begin(); p != ports.end(); p++)
	{
		this->ports.push_back((*p));
	}
}

VNFs::~VNFs()
{

}

bool VNFs::operator==(const VNFs &other) const
{
	if(id == other.id && name == other.name)
		return true;
		
	return false;
}

string VNFs::getId()
{
	return id;
}

string VNFs::getName()
{
	return name;
}

string VNFs::getGroups()
{
	return groups;
}

string VNFs::getVnfTemplate()
{
	return vnf_template;
}

list <pair<string, string> > VNFs::getPorts()
{
	return ports;
}

void VNFs::print()
{
	if(LOGGING_LEVEL <= ORCH_DEBUG_INFO)
	{
		cout << "\t\tid:" << id << endl;
		cout << "\t\t\tname: " << name << endl;
		cout << "\t\t\tvnf_template: " << vnf_template << endl;
		cout << "\t\t\tgroups: " << groups << endl;
		cout << "\t\t\tports: " << endl << "\t\t{" << endl;
		for(list<pair<string, string> >::iterator p = ports.begin(); p != ports.end(); p++)
		{
			cout << "\t\t\tid: " << p->first << endl;
			cout << "\t\t\tname: " << p->second << endl;
		}
		cout << "\t\t}" << endl;
	}
}

Object VNFs::toJSON()
{
	Object vnf;
	Array portS;
	
	vnf[_ID] = id.c_str();
	vnf[VNF_NAME] = name.c_str();
	vnf[VNF_TEMPLATE] = vnf_template.c_str();
	if(strcmp(groups.c_str(), "") != 0)
		vnf[VNF_GROUPS] = groups;
	
	for(list<pair<string, string> >::iterator p = ports.begin(); p != ports.end(); p++)
	{
		Object pp;
		
		pp[PORT_ID] = p->first.c_str();
		pp[PORT_NAME] = p->second.c_str();
		
		portS.push_back(pp);
	}
	
	vnf[VNF_PORTS] = portS;
	
	return vnf;
}

}
