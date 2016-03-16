#include "SQLiteManager.h"
#include "SQLiteManager_constants.h"

sqlite3 *SQLiteManager::db = NULL;
char *SQLiteManager::db_name = NULL;

char usr[BUFFER_SIZE], pwd[BUFFER_SIZE], token[BUFFER_SIZE];

//Constructor
SQLiteManager::SQLiteManager(char *db_name){
	this->db_name = db_name;
	/*Open database*/
	if(connect(db_name)){
		logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "Can't open database: %s.", sqlite3_errmsg(this->db));
		throw SQLiteManagerException();
	} else{
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Opened database successfully.");
	}
}

//Destroyer
SQLiteManager::~SQLiteManager(){
	/*Close database*/
	disconnect();
}

//Connect
int SQLiteManager::connect(char *db_name){
	int rc = 0;

	/*Open database*/
	rc = sqlite3_open(db_name, &(this->db));

	return rc;
}

//Disconnect
void SQLiteManager::disconnect(){
	sqlite3_close(this->db);
}

//SELECT callback
int SQLiteManager::callback(void *NotUsed, int argc, char **argv, char **azColName){
	for(int i=0; i<argc; i++){
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "%s = %s", azColName[i], argv[i] ? argv[i] : "NULL");
		if(i==0)
			strcpy(usr, argv[i]);
		else if(i==1)
			strcpy(pwd, argv[i]);
		else if(i==2){
			if(argv[i] != NULL)
				strcpy(token, argv[i]);
			else
				strcpy(token, "");
		}
	}

	return 0;
}

//Create users table
bool SQLiteManager::createTable(){
	int rc = 0;
	char *zErrMsg = 0, *sql = 0;

	/* Create SQL statement for creating the following tables:
	 * - USERS
	 * - PERMISSIONS
	 * */
   	sql = "CREATE TABLE USERS("  \
    		"USER		TEXT PRIMARY KEY  NOT NULL," \
    	    	"PWD		TEXT    		  NOT NULL," \
        	"TOKEN		TEXT     		  NULL," \
 		"TIMESTAMP	TEXT     		  NULL);" \
 		"CREATE TABLE PERMISSIONS(" \
		"USER TEXT NOT NULL," \
		"HTTP_METHOD TEXT NOT NULL," \
		"URL TEXT NOT NULL," \
		"PRIMARY KEY (USER, HTTP_METHOD, URL)" \
		");";

	/*Execute SQL statement*/
	rc = sqlite3_exec(this->db, sql, callback, 0, &zErrMsg);
	if(rc != SQLITE_OK){
		logger(ORCH_WARNING, MODULE_NAME, __FILE__, __LINE__, "SQL error: %s.", zErrMsg);
		eraseAllToken();
		return false;
	} else{
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Table created successfully.");
	}

	return true;
}

//Insert username and password into the users table
bool SQLiteManager::insertUsrPwd(char *user, char *pwd){
	int rc = 0;
	char *zErrMsg = 0, sql[BUFFER_SIZE];

	/* Create SQL statement */
	sprintf(sql, "INSERT INTO USERS (USER, PWD) VALUES ('%s', '%s'); SELECT * FROM USERS;", user, pwd);

	/*Execute SQL statement*/
	rc = sqlite3_exec(this->db, sql, callback, 0, &zErrMsg);
	if(rc != SQLITE_OK){
		logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "SQL error: %s.", zErrMsg);
		return false;
	} else{
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Records created successfully.");
	}

	return true;
}


// Insert a sample permission for a user into the PERMISSIONS table
bool SQLiteManager::insertUsrPermission(char *user, char *http_method, char *url){
	int rc = 0;
	char *zErrMsg = 0, sql[BUFFER_SIZE];

	/* Create SQL statement */
	sprintf(sql, "INSERT INTO PERMISSIONS (USER, HTTP_METHOD, URL) VALUES ('%s', '%s', '%s'); SELECT * FROM PERMISSIONS;", user, http_method, url);

	/*Execute SQL statement*/
	rc = sqlite3_exec(this->db, sql, callback, 0, &zErrMsg);
	if(rc != SQLITE_OK){
		logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "SQL error: %s.", zErrMsg);
		return false;
	} else{
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Records created successfully.");
	}

	return true;
}

//Search username and password into the users table
bool SQLiteManager::selectUsrPwd(char *user, char *pwd){
	int rc = 0;
	char *zErrMsg = 0, sql[BUFFER_SIZE];

	/* Create SQL statement */
   	sprintf(sql, "SELECT * from USERS where USER = '%s' and PWD = '%s';", user, pwd);

	/*Execute SQL statement*/
	rc = sqlite3_exec(this->db, sql, callback, 0, &zErrMsg);
	if(rc != SQLITE_OK){
		logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "SQL error: %s.", zErrMsg);
		return false;
	} else{
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Operation done successfully.");
	}

	return true;
}

//Seach token into the users table
bool SQLiteManager::selectToken(char *token){
	int rc = 0;
	char *zErrMsg = 0, sql[BUFFER_SIZE];

	/* Create SQL statement */
   	sprintf(sql, "SELECT * from USERS where TOKEN = '%s';", token);

	/*Execute SQL statement*/
	rc = sqlite3_exec(this->db, sql, callback, 0, &zErrMsg);
	if(rc != SQLITE_OK){
		logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "SQL error: %s.", zErrMsg);
		return false;
	} else{
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Operation done successfully.");
	}

	return true;
}

//Dump users table
bool SQLiteManager::selectAllTable(){
	int rc = 0;
	char *zErrMsg = 0, *sql = 0;

	/* Create SQL statement */
   	sql = "SELECT * from USERS;";

	/*Execute SQL statement*/
	rc = sqlite3_exec(this->db, sql, callback, 0, &zErrMsg);
	if(rc != SQLITE_OK){
		logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "SQL error: %s.", zErrMsg);
		return false;
	} else{
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Operation done successfully.");
	}

	return true;
}

//Update token and timestamp values
bool SQLiteManager::updateTokenAndTimestamp(char *user, char *token, char *timestamp){
	int rc = 0;
	char *zErrMsg = 0, sql[BUFFER_SIZE];

	/* Create SQL statement */
   	sprintf(sql, "UPDATE USERS set TOKEN = '%s', TIMESTAMP = '%s' where USER = '%s'; SELECT * FROM USERS;", token, timestamp, user);

	/*Execute SQL statement*/
	rc = sqlite3_exec(this->db, sql, callback, 0, &zErrMsg);
	if(rc != SQLITE_OK){
		logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "SQL error: %s.", zErrMsg);
		return false;
	} else{
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Operation done successfully.");
	}

	return true;
}

//Update password
bool SQLiteManager::updatePwd(char *user, char *pwd){
	int rc = 0;
	char *zErrMsg = 0, sql[BUFFER_SIZE];

	/* Create SQL statement */
   	sprintf(sql, "UPDATE USERS set PWD = '%s' where USER = '%s'; SELECT * FROM USERS;", pwd, user);

	/*Execute SQL statement*/
	rc = sqlite3_exec(this->db, sql, callback, 0, &zErrMsg);
	if(rc != SQLITE_OK){
		logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "SQL error: %s.", zErrMsg);
		return false;
	} else{
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Operation done successfully.");
	}

	return true;
}

//Erase all token value
bool SQLiteManager::eraseAllToken(){
	int rc = 0;
	char *zErrMsg = 0, sql[BUFFER_SIZE];

	/* Create SQL statement */
   	sprintf(sql, "UPDATE USERS set TOKEN = '', TIMESTAMP = ''; SELECT * FROM USERS;");

	/*Execute SQL statement*/
	rc = sqlite3_exec(this->db, sql, callback, 0, &zErrMsg);
	if(rc != SQLITE_OK){
		logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "SQL error: %s.", zErrMsg);
		return false;
	} else{
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Operation done successfully.");
	}

	return true;
}

char *SQLiteManager::getUser(){
	return usr;
}

char *SQLiteManager::getPwd(){
	return pwd;
}

char *SQLiteManager::getToken(){
	return token;
}


//SELECT COUNT callback
int SQLiteManager::selectCountCallback(void *count, int argc, char **argv, char **azColName) {
	int *c = (int *) count;
	*c = atoi(argv[0]);
	return 0;
}

bool SQLiteManager::hasPermission(const char *user, const char *http_method, const char *url) {
	int count = -99, rc = 0;
	char *zErrMsg = 0, sql[BUFFER_SIZE];

	/* Create SQL statement */
	sprintf(sql, "SELECT COUNT(*) from PERMISSIONS where USER = '%s' and HTTP_METHOD = '%s' and URL = '%s';", user,
			http_method, url);

	printf("Eseguo query: %s", sql);

	/*Execute SQL statement*/
	rc = sqlite3_exec(this->db, sql, selectCountCallback, &count, &zErrMsg);
	if (rc != SQLITE_OK) {
		logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "SQL error: %s.",
				zErrMsg);
		return false;
	}

	printf("COUNT: %d\n", count);
	return (count > 0);
}

