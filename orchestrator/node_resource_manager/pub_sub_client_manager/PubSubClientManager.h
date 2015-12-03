#ifndef PubSubClientManager_H_
#define PubSubClientManager_H_ 1

#pragma once

#include <stdio.h>
#include <exception>

using namespace std;

class PubSubClientManager
{
public:
	
	virtual bool publishDomainInformation() = 0;
	
	virtual void terminateClient() = 0;
};

class PubSubClientManagerException: public exception
{
public:
	virtual const char* what() const throw()
	{
		return "PubSubClientManagerException";
	}
};

#endif
