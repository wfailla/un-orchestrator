#include "nf.h"


NF::NF(string name, int nports, string description) :
	name(name),nports(nports),description(description)
{

}

void NF::addImplementation(Description *description)
{
	descriptions.push_back(description);
}

string NF::getName()
{
	return name;
}

Object NF::toJSON()
{
	Object nf;	
	
	nf["name"]  = name;
	nf["nports"]  = nports;
	nf["summary"] = description;
	
	Array descr;
	for(list<Description*>::iterator i = descriptions.begin(); i != descriptions.end();i++)
	{
		descr.push_back((*i)->toJSON());
	}
	
	nf["descriptions"] = descr;
	
	return nf;
}
