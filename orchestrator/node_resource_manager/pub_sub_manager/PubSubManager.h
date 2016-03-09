#ifndef PubSubClientManager_H_
#define PubSubClientManager_H_ 1

#pragma once

#include <stdio.h>
#include <exception>

using namespace std;

class PubSubClientManager
{
public:

	/**
	*	@brief: Publish domain information at the boot time
	*/
	virtual bool publishBoot(char *, char *, char *) = 0;

	/**
	*	@brief: Publish updating domain information
	*/
	virtual bool publishUpdating() = 0;

	/**
	*	@brief: Terminate client
	*/
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
