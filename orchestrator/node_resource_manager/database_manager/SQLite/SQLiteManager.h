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

	SQLiteManager();
	
	~SQLiteManager();
	
	bool createTable(char *db_name);
	
	bool insertOperation(char *db_name, char *user, char *pwd);
	
	bool selectOperation(char *db_name, char *user, char *pwd);
	
	bool selectTokenOperation(char *db_name, char *token);
	
	bool selectAllOperation(char *db_name);
	
	bool updateOperation(char *db_name, char *user, char *token);
	
	bool deleteRow(char *db_name, pair<char *, char *> condition);
	
	bool eraseTable(char *db_name);
	
	bool dropTable(char *db_name);
	
	bool dropDB(char *db_name);
	
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
