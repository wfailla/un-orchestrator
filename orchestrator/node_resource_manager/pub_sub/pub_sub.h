#ifndef PUB_SUB_H_
#define PUB_SUB_H_ 1

#include <czmq.h>

extern "C" {
	#include <dd.h>
	}

#include "../../utils/logger.h"
#include "pub_sub_constants.h"

typedef enum {
	NFFG,
	//[+] add here other topics
}topic_t;

class DoubleDeckerClient
{
private:
	/**
	*	@brief: this is the client that interacts with the Double
	*		Decker bus
	*/
	static zactor_t *client;

	/**
	*	@brief: wait for messages coming from the DoubleDecker network
	*/
	static void *loop(void *param);
	
	/**
	*	@brief: given a topic, returns a string to be used on the
	*		DoubleDecker network
	*/
	static char *topicToString(topic_t topic);

	DoubleDeckerClient() {}
	
public:
	/**
	*	@brief: Inizialize the client and connect it to the Double
	*		Decker network
	*
	*	@param	clientName: name of the client in the Double Decker
	*		network
	*	@param	brokerAddress: address:port of the brocker to be
	*		connected to
	*/
	static bool init(char *clientName, char *brokerAddress);
	
	static void publish(topic_t topic, char *message);
};

#endif //PUB_SUB_H_ 1
