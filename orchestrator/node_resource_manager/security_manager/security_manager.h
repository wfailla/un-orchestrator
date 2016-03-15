#ifndef SECURITY_MANAGER_H_
#define SECURITY_MANAGER_H_ 1

/**
*	Class that implements operations related to security for the REST server. The capabilities currently provided are:
*
*		- Authentication
*		- Authorization
*/


#pragma once

#define __STDC_FORMAT_MACROS

#include <microhttpd.h>

#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <inttypes.h>
#include <sstream>

#include <openssl/sha.h>
#include <openssl/rand.h>

#include "../../utils/constants.h"

#include "../database_manager/SQLite/SQLiteManager.h"

#include <json_spirit/json_spirit.h>
#include <json_spirit/value.h>
#include <json_spirit/writer.h>

#include <sstream>
#include <fstream>


class SecurityManager
{
private:
	const char *token = NULL, *method = NULL, *url = NULL;
	struct MHD_Connection *connection = NULL;
	SQLiteManager *dbmanager = NULL;

	bool isLoginRequest();
	bool checkAuthentication();
	bool checkAuthorization();

public:
	SecurityManager();
	~SecurityManager();

	bool filterRequest(struct MHD_Connection *, SQLiteManager *, const char *, const char *);
};

#endif
