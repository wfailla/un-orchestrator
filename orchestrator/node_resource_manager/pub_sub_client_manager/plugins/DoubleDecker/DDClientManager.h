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

#include "src/DDClient.h"

#ifdef __cplusplus
}
#endif

using namespace std;

class DDClientManager : public PubSubClientManager
{
private:

public:
	DDClientManager();
	
	~DDClientManager();
	
	bool exportDomainInformation(char *, char *, char *);
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
