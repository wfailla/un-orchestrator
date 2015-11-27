#include "DDClientManager.h"
#include "DDClientManager_constants.h"

//Constructor
DDClientManager::DDClientManager(){
}

//Destroyer
DDClientManager::~DDClientManager(){
}

//ExportDomainInformation
bool DDClientManager::exportDomainInformation(char *customer, char *name, char *dealer){
	//int retVal = 0;
	//char *cmdLine = new char[256];
	
	//sprintf(cmdLine, PATH_SCRIPT_RUN_CLIENT, customer, keyfile, name, dealer);
	
	//retVal = system(cmdLine);
	//if(retVal < 0){
	//	logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "Client DD is not started!");
	//	return false;
	//}
	//else{
	//	logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Client DD is started!");
	//	return true;
	//}
	
	init(name, customer, PATH_KEYFILE, dealer);
	
	return true;
}
