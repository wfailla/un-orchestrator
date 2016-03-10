#ifndef DDClientManager_H_
#define DDClientManager_H_ 1

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../../../../utils/logger.h"
#include "../../../../utils/constants.h"

#include "../../PubSubManager.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "DDClient.h"

#ifdef __cplusplus
}
#endif

using namespace std;

class DDClientManager : public PubSubClientManager
{
private:

	/**
	*	@brief: Double decker client pointer
	*/
	ddclient_t *client;

public:

	DDClientManager();

	~DDClientManager();

	bool publishBoot(char *descr_file, char *client_name, char *dealer_name);

	bool publishUpdating();

	void terminateClient();

	ddclient_t *getClient();
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
