#ifndef DDClientManager_H_
#define DDClientManager_H_ 1

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../../../../utils/logger.h"
#include "../../../../utils/constants.h"

#include "../../PubSubClientManager.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "include/DDClient.h"

#ifdef __cplusplus
}
#endif

using namespace std;

class DDClientManager : public PubSubClientManager
{
private:
	ddclient_t *client;
	
public:
	DDClientManager();
	
	~DDClientManager();
	
	bool publishDomainInformation();
	
	void terminateClient();
};

class DDClientManagerException : public PubSubClientManagerException
{
public:
	virtual const char* what() const throw()
	{
		return "DDClientManagerException";
	}
};

#endif
