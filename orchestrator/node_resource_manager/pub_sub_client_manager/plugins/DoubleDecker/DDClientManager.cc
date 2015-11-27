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
	
	ddclient_t *client;
	
	//Export boot information
	client = init(name, customer, PATH_KEYFILE, dealer);
	
	sleep(2);
	
	char *file_name = "node_resource_manager/pub_sub_client_manager/plugins/DoubleDecker/ExampleConfigurationJson.json", c, *mesg = "";
  
	FILE *fp = fopen(file_name, "r");
	if(fp == NULL)
	  	logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "ERROR reading file.");
  	
  	int i = 0, n = 0;
	while(fscanf(fp, "%c", &c) != EOF){
		i++;
	}
  
  	n = i;
  
	mesg = (char *)calloc(n, sizeof(char));
		
	fclose(fp);
  
	fp = fopen(file_name, "r");
	if(fp == NULL)
		logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "ERROR reading file.");
  	
	for(i=0;i<n;i++){
  		fscanf(fp, "%c", &c);
		mesg[i] = c;
  	}
  
	mesg[i-1] = '\0';
  
	logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Publish node configuration.");
  
	fclose(fp);
  
	//Publish NF-FG
	publish("NF-FG", mesg, strlen(mesg), client);
	
	return true;
}
