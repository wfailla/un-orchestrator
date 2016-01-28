#include "DDClientManager.h"
#include "DDClientManager_constants.h"

//Constructor
DDClientManager::DDClientManager(){
}

//Destroyer
DDClientManager::~DDClientManager(){
}

//Export Domain Information
bool DDClientManager::publishBoot(char *descr_file, char *client_name, char *dealer_name){
	try{
		char c, *mesg = "";
		
		if(descr_file == NULL)
			descr_file = FILE_NAME;
		else if(client_name == NULL)
			client_name = DD_NAME;
		else if(dealer_name == NULL)
			dealer_name = DD_DEALER;
		
		//initialization of client
		client = init(client_name, DD_CUSTOMER, PATH_KEYFILE, dealer_name);
		
		//waiting registration of client
		sleep(2);
		
		FILE *fp = fopen(descr_file, "r");
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

//Export Domain Information
bool DDClientManager::publishUpdating(){
	try{
		char c, *mesg = "";
		
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

//Terminate Client
void DDClientManager::terminateClient(){
	client->shutdown(client);
}

//Get Client
ddclient_t *DDClientManager::getClient(){
	return client;
}

