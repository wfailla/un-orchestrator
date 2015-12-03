#include "DDClientManager.h"
#include "DDClientManager_constants.h"

//Constructor
DDClientManager::DDClientManager(){
}

//Destroyer
DDClientManager::~DDClientManager(){
}

//ExportDomainInformation
bool DDClientManager::publishDomainInformation(){
	try{
		char c, *mesg = "";
		
		//initialization of client
		client = init(DD_NAME, DD_CUSTOMER, PATH_KEYFILE, DD_DEALER);
		
		//waiting registration of client
		sleep(2);
		
		FILE *fp = fopen(FILE_NAME, "r");
		if(fp == NULL)
			logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "ERROR reading file.");
	  	
  		int i = 0, n = 0;
		while(fscanf(fp, "%c", &c) != EOF){
			i++;
		}
	  
  		n = i;
  
		mesg = (char *)calloc(n, sizeof(char));
		
		fclose(fp);
	  
		fp = fopen(FILE_NAME, "r");
		if(fp == NULL)
			logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "ERROR reading file.");
	  	
		for(i=0;i<n;i++){
  			fscanf(fp, "%c", &c);
			mesg[i] = c;
  		}
	  
		mesg[i-1] = '\0';
  
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Publishing node configuration.");
	  
		fclose(fp);
	  
		//publish NF-FG
		client->publish("NF-FG", mesg, strlen(mesg), client);
	
		return true;
	} catch(...){
		new DDClientManagerException();
	}
}

void DDClientManager::terminateClient(){
	client->shutdown(client);
}

