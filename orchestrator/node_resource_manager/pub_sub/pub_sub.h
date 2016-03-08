#ifndef PUB_SUB_H_
#define PUB_SUB_H_ 1

#include <czmq.h>

extern "C" {
	#include <dd.h>
	}

#include "../../utils/logger.h"
#include "pub_sub_constants.h"

typedef enum {
	TOPIC
	//TODO: add here the topics
}topic_t;

class DoubleDeckerClient
{
private:
	/**
	*	@brief: this is the client that interacts with the Double
	*		Decker bus
	*/
	static zactor_t *client;

	static void *loop(void *param);
	
public:
	DoubleDeckerClient() {}
	
	/**
	*	@brief: Inizialize the client and connect it to the Double
	*		Decker network
	*/
	static bool init();
	
	static void publish(topic_t topic, char *message);
};

#endif //PUB_SUB_H_ 1
