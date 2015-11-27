#ifndef PubSubClientManager_H_
#define PubSubClientManager_H_ 1

#pragma once

#include <stdio.h>
#include <exception>

using namespace std;

class PubSubClientManager
{
public:
	
	virtual bool exportDomainInformation(char *, char *, char *) = 0;
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
