#include "pub_sub.h"

zactor_t *DoubleDeckerClient::client = NULL;

bool DoubleDeckerClient::init()
{

	logger(ORCH_INFO, DD_CLIENT_MODULE_NAME, __FILE__, __LINE__, "Inizializing the '%s'",DD_CLIENT_MODULE_NAME);
	
	//Start and register a DD client on the brocker.
	//TODO: read the parameters somewhere. Do not embed them in the code!
	client = start_ddactor((int)1, "universal-node", "a", "tcp://127.0.0.1:5555",
                  "/home/unify/a-keys.json");

	//Start a new thread that waits for events
	pthread_t thread[1];
	pthread_create(&thread[0],NULL,loop,NULL);

	logger(ORCH_INFO, DD_CLIENT_MODULE_NAME, __FILE__, __LINE__, "Module inizialized!");
	
	return true;
}

void *DoubleDeckerClient::loop(void *param)
{

	while(true)
	{
		//receive a message from the DD
		zmsg_t *msg = zmsg_recv (client);
		zmsg_print(msg);
		//retrieve the event
		char *event = zmsg_popstr(msg);

		if(streq("reg",event))
		{
			//When the registration is successful
			logger(ORCH_INFO, DD_CLIENT_MODULE_NAME, __FILE__, __LINE__, "Succcessfully registered on the Double Decker network!");
			free(event);
		}
		else if (streq("discon",event))
		{
			logger(ORCH_WARNING, DD_CLIENT_MODULE_NAME, __FILE__, __LINE__, "Connection with the Double Decker network has been lost!");
			free(event);
			//TODO: what to do in this case?
		} 
		else if (streq("pub",event))
		{
			logger(ORCH_WARNING, DD_CLIENT_MODULE_NAME, __FILE__, __LINE__, "Received a 'publication' event. This event is ignored");
			free(event);
			//TODO: add here a callback that handle the proper event
		}
		else if (streq("data",event))
		{
			logger(ORCH_WARNING, DD_CLIENT_MODULE_NAME, __FILE__, __LINE__, "Received a 'data' event. This event is ignored");
			free(event);
		}
	}
	
	return NULL;
}

void DoubleDeckerClient::publish(topic_t topic, char *message)
{
	assert(client != NULL);

	int len = strlen(message);
	zsock_send(client,"sssb", "publish", topicToString(topic), message,&len, sizeof(len));
}

char *DoubleDeckerClient::topicToString(topic_t topic)
{
	switch(topic)
	{
		case NFFG:
			return "NF-FG";
		default:
			assert(0 && "This is impossible!");
			return "";
	}
}

