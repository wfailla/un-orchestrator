#include "SQLiteManager.h"
#include "SQLiteManager_constants.h"

sqlite3 *SQLiteManager::db = NULL;

char usr[BUFFER_SIZE], pwd[BUFFER_SIZE], token[BUFFER_SIZE];

//Constructor
SQLiteManager::SQLiteManager(){
}

//Destroyer
SQLiteManager::~SQLiteManager(){
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

//Create Table
bool SQLiteManager::createTable(char *db_name){
	int rc = 0;
	char *zErrMsg = 0, *sql = 0;
	
	/*Open database*/
	rc = connect(db_name);
	if(rc){
		logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "Can't open database: %s.", sqlite3_errmsg(this->db));
		return false;
	} else{
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Opened database successfully.");
	}
	
	/* Create SQL statement */
   	sql = "CREATE TABLE USERS("  \
    		"USER		TEXT PRIMARY KEY  NOT NULL," \
    	    "PWD		TEXT    		  NOT NULL," \
        	"TOKEN		TEXT     		  NULL );";

	/*Execute SQL statement*/
	rc = sqlite3_exec(this->db, sql, callback, 0, &zErrMsg);
	if(rc != SQLITE_OK){
		logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "SQL error: %s.", zErrMsg);
		dropTable(DB_NAME);
		return false;
	} else{
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Table created successfully.");
	}
	
	/*Close database*/
	disconnect();
	
	return true;
}

//Insert Operation
bool SQLiteManager::insertOperation(char *db_name, char *user, char *pwd){
	int rc = 0;
	char *zErrMsg = 0, sql[BUFFER_SIZE];
	
	/*Open database*/
	rc = connect(db_name);
	if(rc){
		logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "Can't open database: %s.", sqlite3_errmsg(this->db));
		return false;
	} else{
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Opened database successfully.");
	}
	
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
	
	/*Close database*/
	disconnect();
	
	return true;
}

//Select Operation
bool SQLiteManager::selectOperation(char *db_name, char *user, char *pwd){
	int rc = 0;
	char *zErrMsg = 0, sql[BUFFER_SIZE];
	
	/*Open database*/
	rc = connect(db_name);
	if(rc){
		logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "Can't open database: %s.", sqlite3_errmsg(this->db));
		return false;
	} else{
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Opened database successfully.");
	}
	
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
	
	/*Close database*/
	disconnect();
	
	return true;
}

//Select Token Operation
bool SQLiteManager::selectTokenOperation(char *db_name, char *token){
	int rc = 0;
	char *zErrMsg = 0, sql[BUFFER_SIZE];
	
	/*Open database*/
	rc = connect(db_name);
	if(rc){
		logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "Can't open database: %s.", sqlite3_errmsg(this->db));
		return false;
	} else{
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Opened database successfully.");
	}
	
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
	
	/*Close database*/
	disconnect();
	
	return true;
}

//Select All Operation
bool SQLiteManager::selectAllOperation(char *db_name){
	int rc = 0;
	char *zErrMsg = 0, *sql = 0;
	
	/*Open database*/
	rc = connect(db_name);
	if(rc){
		logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "Can't open database: %s.", sqlite3_errmsg(this->db));
		return false;
	} else{
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Opened database successfully.");
	}
	
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
	
	/*Close database*/
	disconnect();
	
	return true;
}

//Update Operation
bool SQLiteManager::updateOperation(char *db_name, char *user, char *token){
	int rc = 0;
	char *zErrMsg = 0, sql[BUFFER_SIZE];
	
	/*Open database*/
	rc = connect(db_name);
	if(rc){
		logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "Can't open database: %s.", sqlite3_errmsg(this->db));
		return false;
	} else{
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Opened database successfully.");
	}
	
	/* Create SQL statement */
   	sprintf(sql, "UPDATE USERS set TOKEN = '%s' where USER = '%s'; SELECT * FROM USERS;", token, user);
   	
	/*Execute SQL statement*/
	rc = sqlite3_exec(this->db, sql, callback, 0, &zErrMsg);
	if(rc != SQLITE_OK){
		logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "SQL error: %s.", zErrMsg);
		return false;
	} else{
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Operation done successfully.");
	}
	
	/*Close database*/
	disconnect();
	
	return true;
}

//Delete Operation
bool SQLiteManager::deleteRow(char *db_name, pair<char *, char *> condition){
	int rc = 0;
	char *zErrMsg = 0, sql[BUFFER_SIZE];
	
	/*Open database*/
	rc = connect(db_name);
	if(rc){
		logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "Can't open database: %s.", sqlite3_errmsg(this->db));
		return false;
	} else{
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Opened database successfully.");
	}
	
	/* Create delete SQL statement */
	sprintf(sql, "DELETE from USERS where %s = '%s';", condition.first, condition.second);
	
	/*Execute SQL statement*/
	rc = sqlite3_exec(this->db, sql, callback, 0, &zErrMsg);
	if(rc != SQLITE_OK){
		logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "SQL error: %s.", zErrMsg);
		return false;
	} else{
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Operation done successfully.");
	}
	
	/*Close database*/
	disconnect();
	
	return true;
}

//Erase Table
bool SQLiteManager::eraseTable(char *db_name){
	int rc = 0;
	char *zErrMsg = 0, *sql = 0;
	
	/*Open database*/
	rc = connect(db_name);
	if(rc){
		logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "Can't open database: %s.", sqlite3_errmsg(this->db));
		return false;
	} else{
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Opened database successfully.");
	}
	
	/* Create delete SQL statement */
    sql = "DELETE from USERS;";
	
	/*Execute SQL statement*/
	rc = sqlite3_exec(this->db, sql, callback, 0, &zErrMsg);
	if(rc != SQLITE_OK){
		logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "SQL error: %s.", zErrMsg);
		return false;
	} else{
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Operation done successfully.");
	}
	
	/*Close database*/
	disconnect();
	
	return true;
}

//Drop Table
bool SQLiteManager::dropTable(char *db_name){
	int rc = 0;
	char *zErrMsg = 0, *sql = 0;
	
	/*Open database*/
	rc = connect(db_name);
	if(rc){
		logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "Can't open database: %s.", sqlite3_errmsg(this->db));
		return false;
	} else{
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Opened database successfully.");
	}
	
	/* Create dropped SQL statement */
    sql = "DROP TABLE USERS;";
	
	/*Execute SQL statement*/
	rc = sqlite3_exec(this->db, sql, callback, 0, &zErrMsg);
	if(rc != SQLITE_OK){
		logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "SQL error: %s.", zErrMsg);
		return false;
	} else{
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Operation done successfully.");
	}
	
	/*Close database*/
	disconnect();
	
	return true;
}

//Drop Database
bool SQLiteManager::dropDB(char *db_name){
	int rc = 0;
	char *zErrMsg = 0, *sql = 0;
	
	/*Open database*/
	rc = connect(db_name);
	if(rc){
		logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "Can't open database: %s.", sqlite3_errmsg(this->db));
		return false;
	} else{
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Opened database successfully.");
	}
	
	/* Create dropped SQL statement */
    sprintf(sql, "DROP DATABASE %s;", db_name);
	
	/*Execute SQL statement*/
	rc = sqlite3_exec(this->db, sql, callback, 0, &zErrMsg);
	if(rc != SQLITE_OK){
		logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "SQL error: %s.", zErrMsg);
		return false;
	} else{
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Operation done successfully.");
	}
	
	/*Close database*/
	disconnect();
	
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

