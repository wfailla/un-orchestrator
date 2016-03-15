#include "security_manager.h"

SecurityManager::SecurityManager() : token(NULL), method(NULL), url(NULL), connection(NULL), dbmanager(NULL) { }

SecurityManager::~SecurityManager() {
	// I do not pretend to be owner of these data, so I will leave deletion to others...
	token = NULL;
	method = NULL;
	url = NULL;
	connection = NULL;
	dbmanager = NULL;
}

bool SecurityManager::isLoginRequest() {
	/*
	 * Checking method name and url is enough because the REST server
	 * already verifies that the request is well-formed.
	 */
	return (strcmp(method, POST) == 0
			&& url[0] == '/'
			&& strncmp(url + sizeof(char), BASE_URL_LOGIN, sizeof(char) * strlen(BASE_URL_LOGIN)) == 0);
}

bool SecurityManager::checkAuthentication() {
	if(token == NULL) {
		logger(ORCH_INFO, MODULE_NAME, __FILE__, __LINE__, "\"Token\" header not present in the request");
		return false;
	}

	dbmanager->selectToken((char *)token);
	if(strcmp(dbmanager->getToken(), token) == 0) {
		//User authenticated!
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "User authenticated");
		return true;
	} else {
		//User unauthenticated!
		logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "User unauthenticated");
		return false;
	}

	return false;
}

bool SecurityManager::checkAuthorization() {
	bool authorized = false;
	char *user = NULL;

	assert(token != NULL && method != NULL && url != NULL);
	if (token == NULL || method == NULL || url == NULL)
		return false;

	if(dbmanager->selectToken((char *) token)) {
		user = dbmanager->getUser();
		assert(user != NULL);

		authorized = dbmanager->hasPermission(user, method, url);

		if(authorized) {
			logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "User authorized");
			return true;
		} else {
			logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "User unauthorized");
			return false;
		}
	}

	return false;
}

bool SecurityManager::filterRequest(struct MHD_Connection *current_connection,
		SQLiteManager *current_dbmanager, const char *current_method,
		const char *current_url) {

	connection = current_connection;
	dbmanager = current_dbmanager;
	method = current_method;
	url = current_url;

	assert(connection != NULL && dbmanager != NULL && method != NULL && url != NULL);

	// Login requests do not need to be authenticated and authorized
	if(isLoginRequest())
		return true;

	token = MHD_lookup_connection_value(connection, MHD_HEADER_KIND, "X-Auth-Token");

	if (!checkAuthentication())
		return false;

	return checkAuthorization();
}
