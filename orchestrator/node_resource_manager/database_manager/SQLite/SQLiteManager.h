#ifndef SQLiteManager_H_
#define SQLiteManager_H_ 1

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <exception>
#include <sqlite3.h>
#include <list>
#include <string.h>

#include "../../../utils/logger.h"
#include "../../../utils/constants.h"

using namespace std;

class SQLiteManager
{
private:
	
	/**
	*	@brief: Database pointer
	*/
	static sqlite3 *db;

	/**
	*	@brief: Name of database
	*/
	static char *db_name;

	/**
	*	@brief:	Open a connection with given database db_name
	*/
	int connect(char *db_name);
	
	/**
	*	@brief: Disconnect from given database
	*/
	void disconnect();
	
	/**
	*	@brief: SELECT callback
	*/
	static int callback(void *NotUsed, int argc, char **argv, char **azColName);
	
public:

	SQLiteManager(char *db_name);
	
	~SQLiteManager();
	
	bool createTable();
	
	bool insertUsrPwd(char *user, char *pwd);
	
	bool selectUsrPwd(char *user, char *pwd);
	
	bool selectToken(char *token);
	
	bool selectAllTable();
	
	bool updateToken(char *user, char *token);

	bool eraseAllToken();
	
	char *getUser();
	
	char *getPwd();
	
	char *getToken();
};

class SQLiteManagerException : public exception
{
public:
	virtual const char* what() const throw()
	{
		return "SQLiteManagerException";
	}
};

#endif
