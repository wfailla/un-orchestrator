/*
  Copyright (c) 2015 Pontus Sköldström, Bertrand Pechenot 
    
  This file is part of libdd, the DoubleDecker hierarchical
  messaging system DoubleDecker is free software; you can
  redistribute it and/or modify it under the terms of the GNU Lesser
  General Public License (LGPL) version 2.1 as published by the Free
  Software Foundation.  
  
  As a special exception, the Authors give you permission to link this
  library with independent modules to produce an executable,
  regardless of the license terms of these independent modules, and to
  copy and distribute the resulting executable under terms of your
  choice, provided that you also meet, for each linked independent
  module, the terms and conditions of the license of that module. An
  independent module is a module which is not derived from or based on
  this library.  If you modify this library, you must extend this
  exception to your version of the library.  DoubleDecker is
  distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
  License for more details.  You should have received a copy of the
  GNU Lesser General Public License along with this program.  If not,
  see <http://www.gnu.org/licenses/>.
*/
#include "DDClient.h"

ddclient_t *client;

// ///////////////////
// ///SUBLIST stuff //
// //////////////////
typedef struct {
  char *topic;
  char *scope;
  char active;
} ddtopic_t;

void sublist_print(ddclient_t *dd) {
	ddtopic_t *item;
	while ((item = zlistx_next(dd->sublist)))
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Topic: %s Scope: %s Active: %d", item->topic, item->scope, item->active);
}

cparser_result_t cparser_cmd_show_subscriptions (cparser_context_t * context)
{
	logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "List of subscriptions:");
	sublist_print (client);
	return CPARSER_OK;
}

cparser_result_t cparser_cmd_show_status (cparser_context_t * context)
{
	if (client->state == DD_STATE_UNREG)
    {
	    logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "DoubleDecker client: UNREGISTRED");
    }
	else if (client->state == DD_STATE_REGISTERED)
    {
    	logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "DoubleDecker client: REGISTRED");
    }
    else if (client->state == DD_STATE_CHALLENGED)
    {
    	logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "DoubleDecker client: AUTHENTICATING");
    }
    else
    {
    	logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "DoubleDecker client: UNKNOWN!");
    }
  return CPARSER_OK;
}

cparser_result_t cparser_cmd_subscribe_topic_scope (cparser_context_t * context, char **topic_ptr, char **scope_ptr)
{
	char *topic;
	char *scope;
	
	if (topic_ptr)
	    topic = *topic_ptr;
	else
    {
		logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "error: subscribe 'topic' 'ALL/REGION/CLUSTER/NODE/NOSCOPE, 1/2/3'");
    	return CPARSER_NOT_OK;
    }
	if (scope_ptr)
		scope = *scope_ptr;
	else
    {
		logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "error: subscribe 'topic' 'ALL/REGION/CLUSTER/NODE/NOSCOPE, 1/2/3'");
    	return CPARSER_NOT_OK;
    }
    
	client->subscribe (topic, scope, client);
	
	return CPARSER_OK;
}

cparser_result_t cparser_cmd_no_subscribe_topic_scope (cparser_context_t * context, char **topic_ptr, char **scope_ptr)
{
	char *topic;
	char *scope;
	
	if (topic_ptr)
	    topic = *topic_ptr;
	else
    {
		logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "error: no subscribe 'topic' 'ALL/REGION/CLUSTER/NODE/NOSCOPE, 1/2/3'");
    	return CPARSER_NOT_OK;
    }
  	if (scope_ptr)
    	scope = *scope_ptr;
  	else
    {
    	logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "error: no subscribe 'topic' 'ALL/REGION/CLUSTER/NODE/NOSCOPE, 1/2/3'");
    	return CPARSER_NOT_OK;
    }
    
  	client->unsubscribe (topic, scope, client);

  	return CPARSER_OK;
}

cparser_result_t cparser_cmd_publish_topic_message (cparser_context_t * context, char **topic_ptr, char **message_ptr)
{
	char *topic;
	char *message;
	if (topic_ptr)
	    topic = *topic_ptr;
	else
    {
	    logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "error: publish 'topic' 'message'");
	    return CPARSER_NOT_OK;
    }
	if (message_ptr)
		message = *message_ptr;
	else
    {
	    logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "error: publish 'topic' 'message'");
	    return CPARSER_NOT_OK;
    }
	// +1 for \0 in strlen
	client->publish (topic, message, strlen (message) + 1, client);
	return CPARSER_OK;
}

cparser_result_t cparser_cmd_notify_destination_message (cparser_context_t * context, char **destination_ptr, char **message_ptr)
{
	char *destination;
	char *message;
	if (destination_ptr)
	    destination = *destination_ptr;
	else
    {
		logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "error: notify 'destination' 'message'");
	    return CPARSER_NOT_OK;
    }
	if (message_ptr)
    	message = *message_ptr;
  	else
    {
    	logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "error: notify 'destination' 'message'");
    	return CPARSER_NOT_OK;
    }
  	// +1 for \0 in strlen
  	client->notify (destination, message, strlen (message) + 1, client);

  	return CPARSER_OK;
}

cparser_result_t cparser_cmd_quit (cparser_context_t * context)
{
	client->shutdown(client);
	cparser_quit (context->parser);
	return CPARSER_OK;
}

cparser_result_t cparser_cmd_help (cparser_context_t * context)
{
	return cparser_help_cmd (context->parser, NULL);
}

// - compare two items, for sorting
// typedef int (czmq_comparator) (const void *item1, const void *item2);
int sublist_cmp(const void *item1, const void *item2) {
	ddtopic_t *i1, *i2;
	i1 = (ddtopic_t *)item1;
	i2 = (ddtopic_t *)item2;
	return strcmp(i1->topic, i2->topic);
}

// -- destroy an item
// typedef void (czmq_destructor) (void **item);
 void sublist_free(void **item) {
  ddtopic_t *i;
  i = *item;
  free(i->topic);
  free(i->scope);
  free(i);
}

// -- duplicate an item
// typedef void *(czmq_duplicator) (const void *item);
void *sublist_dup(const void *item) {
	ddtopic_t *new, *old;
	old = (ddtopic_t *)item;
	new = malloc(sizeof(ddtopic_t));
	new->topic = strdup(old->topic);
	new->scope = strdup(old->scope);
	new->active = old->active;
	return new;
}

// update or add topic/scope/active to list
void sublist_add(char *topic, char *scope, char active, ddclient_t *dd) {
	ddtopic_t *item; // = zlistx_first(dd->sublist);
	int found = 0;
	// Check if already there, if so update
	// dd_info("after _first item = %p\n",item);

	while ((item = zlistx_next(dd->sublist))) {
		if (strcmp(item->topic, topic) == 0 &&
    	    strcmp(item->scope, scope) == 0) {
    			item->active = active;
      			found = 1;
    	}
  	}

	// Otherwise, add new
	if (!found) {
	    ddtopic_t *new = malloc(sizeof(ddtopic_t));
	    new->topic = topic;
	    new->scope = scope;
	    new->active = active;
	    zlistx_add_start(dd->sublist, new);
	}
}

void sublist_delete(char *topic, char *scope, ddclient_t *dd) {
	ddtopic_t del;
	del.topic = topic;
	del.scope = scope;
	ddtopic_t *item = zlistx_find(dd->sublist, &del);
	if (item)
		zlistx_delete(dd->sublist, item);
}

void sublist_activate(char *topic, char *scope, ddclient_t *dd) {
	ddtopic_t *item; // = zlistx_first(dd->sublist);
	while ((item = zlistx_next(dd->sublist))) {
		if (strcmp(item->topic, topic) == 0 &&
	        strcmp(item->scope, scope) == 0) {
			item->active = 1;
    	}
  	}
}

void sublist_deactivate_all(ddclient_t *dd) {
	ddtopic_t *item;
	while ((item = zlistx_next(dd->sublist))) {
	    item->active = 0;
	}
}

void sublist_resubscribe(ddclient_t *dd) {
	ddtopic_t *item; // = zlistx_first(dd->sublist);
	while ((item = zlistx_next(dd->sublist))) {
    	zsock_send(dd->socket, "bbbss", &dd_version, 4, &dd_cmd_sub, 4,
               &dd->cookie, sizeof(dd->cookie), item->topic, item->scope);
  	}
}

// ////////////////////////////////////////////////////
// // Commands for subscribe / publish / sendmessage //
// ////////////////////////////////////////////////////
int subscribe(char *topic, char *scope, ddclient_t *dd) {
	char *scopestr;
	if (strcmp(scope, "all") == 0) {
	    scopestr = "/";
	} else if (strcmp(scope, "region") == 0) {
    	scopestr = "/*/";
  	} else if (strcmp(scope, "cluster") == 0) {
    	scopestr = "/*/*/";
  	} else if (strcmp(scope, "node") == 0) {
    	scopestr = "/*/*/*/";
  	} else if (strcmp(scope, "noscope") == 0) {
    	scopestr = "noscope";
  	} else {
    	// TODO
    	// check that scope follows re.fullmatch("/((\d)+/)+", scope):
    	scopestr = scope;
  	}
  	sublist_add(topic, scopestr, 0, dd);
  	if (dd->state == DD_STATE_REGISTERED)
    	zsock_send(dd->socket, "bbbss", &dd_version, 4, &dd_cmd_sub, 4,
               &dd->cookie, sizeof(dd->cookie), topic, scopestr);
  	return 0;
}

int unsubscribe(char *topic, char *scope, struct ddclient *dd) {
	char *scopestr;
	if (strcmp(scope, "all") == 0) {
    	scopestr = "/";
  	} else if (strcmp(scope, "region") == 0) {
    	scopestr = "/*/";
  	} else if (strcmp(scope, "cluster") == 0) {
    	scopestr = "/*/*/";
  	} else if (strcmp(scope, "node") == 0) {
    	scopestr = "/*/*/*/";
  	} else if (strcmp(scope, "noscope") == 0) {
    	scopestr = "noscope";
  	} else {
    	// TODO
    	// check that scope follows re.fullmatch("/((\d)+/)+", scope):
    	scopestr = scope;
  	}
  	sublist_delete(topic, scopestr, dd);
  	if (dd->state == DD_STATE_REGISTERED)
    	zsock_send(dd->socket, "bbbss", &dd_version, 4, &dd_cmd_unsub, 4,
               &dd->cookie, sizeof(dd->cookie), topic, scopestr);
 	return 0;
}

int publish(char *topic, char *message, int mlen, ddclient_t *dd) {
	unsigned char *precalck = NULL;
	int srcpublic = 0;
	int dstpublic = 0;
	int retval;

    logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "dd->publish called t: %s m: %s l: %d\n", topic, message, mlen);
	// dd_info ("dd->publish called t: %s m: %s l: %d\n", topic, message,
	// mlen);

	if (strcmp(dd->customer, "public") == 0) {
		srcpublic = 1;
  	}
  	if (strncmp("public.", topic, strlen("public.")) == 0) {
    	dstpublic = 1;
  	}

  	char *dot = strchr(topic, '.');
  	if (dot && srcpublic) {
    	*dot = '\0';
    	precalck = zhash_lookup(dd->keys->clientkeys, topic);
    	if (precalck) {
      		//	dd_info("encrypting with tenant key: %s\n",topic);
      		// TODO: This is not allowed by the broker
      		// We should return an error if this is happening
      		logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "Public client cannot publish to tenants!\n");
     		return -1;
    	}
    	*dot = '.';
  	}
  	if (!precalck && !dstpublic) {
    	precalck = dd->keys->custboxk;
   	 	//      dd_info ("encrypting with my own key\n");
  	} else if (dstpublic) {
    	precalck = dd->keys->pubboxk;
    	//    dd_info("encrypting with public key\n");
  	}

  	int enclen = mlen + crypto_box_NONCEBYTES + crypto_box_MACBYTES;
  	//  dd_info ("encrypted message will be %d bytes\n", enclen);
  	// unsigned char ciphertext[enclen];
  	unsigned char *dest = calloc(1, enclen);
  	unsigned char *ciphertext = dest; // dest+crypto_box_NONCEBYTES;

  	// increment nonce
 	sodium_increment(dd->nonce, crypto_box_NONCEBYTES);
  	memcpy(dest, dd->nonce, crypto_box_NONCEBYTES);

  	dest += crypto_box_NONCEBYTES;
  	retval = crypto_box_easy_afternm(dest, message, mlen, dd->nonce, precalck);
  	//  char *hex = calloc (1, 1000);
  	//  sodium_bin2hex (hex, 1000, ciphertext, enclen);
  	//  dd_info ("ciphertext size %d: %s\n", enclen, hex);
  	//  free (hex);

	if (retval != 0) {
	    logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "DD: Unable to encrypt %d bytes!\n", mlen);
	    free(ciphertext);
	    return -1;
	}
	if (dd->state == DD_STATE_REGISTERED) {
    	zsock_send(dd->socket, "bbbszb", &dd_version, 4, &dd_cmd_pub, 4,
               &dd->cookie, sizeof(dd->cookie), topic, ciphertext, enclen);
  	}
  	free(ciphertext);
}

// - Publish between public and other customers not working
int notify(char *target, char *message, int mlen, ddclient_t *dd) {
	unsigned char *precalck = NULL;
	int srcpublic = 0;
	int dstpublic = 0;

	if (strcmp(dd->customer, "public") == 0) {
	    srcpublic = 1;
	}
  	if (strncmp("public.", target, strlen("public.")) == 0) {
    	dstpublic = 1;
  	}

  	/* dd_info ("dd->sendmsg called t: %s m: %s l: %d\n", target, message,
   	* mlen);
   	*/
  	char *dot = strchr(target, '.');

  	int retval;
  	if (dot && srcpublic) {
    	*dot = '\0';
    	precalck = zhash_lookup(dd->keys->clientkeys, target);
    	if (precalck) {
      		/* dd_info("encrypting with tenant key: %s\n",target); */
    	}
    	*dot = '.';
  	}
  	if (!precalck && !dstpublic) {
    	precalck = dd->keys->custboxk;
    	/* dd_info ("encrypting with my own key\n"); */
  	} else if (dstpublic) {
    precalck = dd->keys->pubboxk;
    /* dd_info("encrypting with public key\n"); */
  }

	int enclen = mlen + crypto_box_NONCEBYTES + crypto_box_MACBYTES;
	/* dd_info ("encrypted message will be %d bytes\n", enclen); */
	// unsigned char ciphertext[enclen];
	unsigned char *dest = calloc(1, enclen);
	unsigned char *ciphertext = dest; // dest+crypto_box_NONCEBYTES;

	// increment nonce
	sodium_increment(dd->nonce, crypto_box_NONCEBYTES);
	memcpy(dest, dd->nonce, crypto_box_NONCEBYTES);

	dest += crypto_box_NONCEBYTES;
	retval = crypto_box_easy_afternm(dest, message, mlen, dd->nonce, precalck);
	/* char *hex = calloc (1, 1000); */
	/* sodium_bin2hex (hex, 1000, ciphertext, enclen); */
	/* dd_info ("ciphertext size %d: %s\n", enclen, hex); */
	/* free (hex); */

	if (retval == 0) {
	} else {
	    logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "DD: Unable to encrypt %d bytes!\n", mlen);
	    free(ciphertext);
	    return -1;
	}
	if (dd->state == DD_STATE_REGISTERED) {
	    zsock_send(dd->socket, "bbbsb", &dd_version, 4, &dd_cmd_send, 4,
               &dd->cookie, sizeof(dd->cookie), target, ciphertext,
               enclen);
	}
	
	free(ciphertext);
}

int ddclient_shutdown(ddclient_t *dd) {
  logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "DD: Shutting down client..");
  if (dd->state == DD_STATE_REGISTERED) {
    zsock_send(dd->socket, "bbb", &dd_version, 4, &dd_cmd_unreg, 4,
               &dd->cookie, sizeof(dd->cookie));
  }
  zlistx_destroy(&dd->sublist);
  //zloop_destroy(&dd->loop);
  zsock_destroy((zsock_t **)&dd->socket);
  dd->state = DD_STATE_EXIT;
}

// ////////////////////////
// callbacks from zloop //
// ////////////////////////

 int s_ping(zloop_t *loop, int timerid, void *args) {
  ddclient_t *dd = (ddclient_t *)args;
  if (dd->state == DD_STATE_REGISTERED)
    zsock_send(dd->socket, "bbb", &dd_version, 4, &dd_cmd_ping, 4,
               &dd->cookie, sizeof(dd->cookie));
  logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "PING.");
}

 int s_heartbeat(zloop_t *loop, int timerid, void *args) {
  ddclient_t *dd = (ddclient_t *)args;
  dd->timeout++;
  if (dd->timeout > 3) {
    dd->state = DD_STATE_UNREG;
    dd->registration_loop =
        zloop_timer(loop, 1000, 0, s_ask_registration, dd);
    zloop_timer_end(loop, dd->heartbeat_loop);
    sublist_deactivate_all(dd);
    dd->on_discon(dd);
  }
}

int s_ask_registration(zloop_t *loop, int timerid, void *args) {
	logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "trying to register");

	ddclient_t *dd = (ddclient_t *)args;
	if (dd->state == DD_STATE_UNREG) {
    	zsock_set_linger(dd->socket, 0);
    	zsock_destroy((zsock_t **)&dd->socket);
    	dd->socket = zsock_new_dealer(NULL);
    	if (!dd->socket) {
    		logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "DD: Error in zsock_new_dealer: %s", zmq_strerror(errno));
      	free(dd);
      	return -1;
    }
    //      zsock_set_identity (dd->socket, dd->client_name);
    int rc = zsock_connect(dd->socket, dd->endpoint);
    if (rc != 0) {
    	logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "DD: Error in zmq_connect: %s",
              zmq_strerror(errno));
    	free(dd);
    	return -1;
    }
    
	zloop_reader(loop, dd->socket, s_on_dealer_msg, dd);
    struct ddkeystate *k = dd->keys;
    
    zsock_send(dd->socket, "bbs", &dd_version, 4, &dd_cmd_addlcl, 4, k->hash);
  }
  
  return 0;
}

// /////////////////////////////////////
// / callbacks for different messages //
// ////////////////////////////////////
void cmd_cb_regok(zmsg_t *msg, ddclient_t *dd, zloop_t *loop) {
	zframe_t *cookie_frame;
	cookie_frame = zmsg_pop(msg);
	if (cookie_frame == NULL) {
	    logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "DD: Misformed REGOK message, missing COOKIE!\n");
	    return;
	}
	unsigned long long int *cookie;
	cookie = (unsigned long long int *)zframe_data(cookie_frame);
	dd->cookie = (unsigned long long int)*cookie;
	zframe_destroy(&cookie_frame);
	dd->state = DD_STATE_REGISTERED;
	zsock_send(dd->socket, "bbb", &dd_version, 4, &dd_cmd_ping, 4,
             &dd->cookie, sizeof(dd->cookie));

	dd->heartbeat_loop = zloop_timer(loop, 1500, 0, s_heartbeat, dd);
	zloop_timer_end(loop, dd->registration_loop);
	// if this is re-registration, we should try to subscribe again
	sublist_resubscribe(dd);
	dd->on_reg(dd);

	logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "REGOK!");
  
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
	publish("NF-FG", mesg, strlen(mesg), dd);
    
	free(mesg);
    
	//dd->shutdown(dd);
    
	// TODO call library registered on_reg() callback here
}

void cmd_cb_pong(zmsg_t *msg, ddclient_t *dd, zloop_t *loop) {
	zloop_timer(loop, 1500, 1, s_ping, dd);
	//dd_info("PONG");
}

void cmd_cb_chall(zmsg_t *msg, ddclient_t *dd) {
	int retval = 0;
	//  logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__,"cmd_cb_chall called\n");

	// zmsg_print(msg);
	zframe_t *encrypted = zmsg_first(msg);
	unsigned char *data = zframe_data(encrypted);
	int enclen = zframe_size(encrypted);
	unsigned char *decrypted = calloc(1, enclen);

	retval = crypto_box_open_easy_afternm(
      decrypted, data + crypto_box_NONCEBYTES,
      enclen - crypto_box_NONCEBYTES, data, dd->keys->ddboxk);
	if (retval != 0) {
    	logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "Unable to decrypt CHALLENGE from broker\n");
    	return;
  	}

	zsock_send(dd->socket, "bbfss", &dd_version, 4, &dd_cmd_challok, 4,
             zframe_new(decrypted, enclen - crypto_box_NONCEBYTES -
                                       crypto_box_MACBYTES),
    dd->keys->hash, dd->client_name);         
}

void cmd_cb_data(zmsg_t *msg, ddclient_t *dd) {
	int retval;
	char *source = zmsg_popstr(msg);
	/* dd_info("cmd_cb_data: S: %s\n", source); */
	zframe_t *encrypted = zmsg_first(msg);
	unsigned char *data = zframe_data(encrypted);
	int enclen = zframe_size(encrypted);
	unsigned char *decrypted =
    calloc(1, enclen - crypto_box_NONCEBYTES - crypto_box_MACBYTES);
	unsigned char *precalck = NULL;
	char *dot = strchr(source, '.'), file_name[256];
	if (dot) {
    	*dot = '\0';
    	precalck = zhash_lookup(dd->keys->clientkeys, source);
    	if (precalck) {
      		// dd_info("decrypting with tenant key:%s\n", source);
    	}
    	*dot = '.';
  	}

	if (!precalck) {
	    if (strncmp("public.", source, strlen("public.")) == 0) {
    		precalck = dd->keys->pubboxk;
      		//	dd_info("decrypting with public tenant key\n");
    	} else {
      		precalck = dd->keys->custboxk;
      		//      dd_info("decrypting with my own key\n");
    	}
  	}
  
	retval = crypto_box_open_easy_afternm(
      decrypted, data + crypto_box_NONCEBYTES,
      enclen - crypto_box_NONCEBYTES, data, precalck);
	if (retval == 0) {
  		strcpy(file_name, decrypted);
    	dd->on_data(source, decrypted,
        	enclen - crypto_box_NONCEBYTES - crypto_box_MACBYTES, dd);
  	} else {
    	logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "DD: Unable to decrypt %d bytes from %s\n",
            enclen - crypto_box_NONCEBYTES - crypto_box_MACBYTES, source);
  	}
  
  	free(decrypted);
  
  	logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "DATA.");
}

// up to the user to free the memory!
void cmd_cb_pub(zmsg_t *msg, ddclient_t *dd) {
	int retval;
	char *source = zmsg_popstr(msg);
	char *topic = zmsg_popstr(msg);
	zframe_t *encrypted = zmsg_first(msg);
	unsigned char *data = zframe_data(encrypted);
	int enclen = zframe_size(encrypted);

	int mlen = enclen - crypto_box_NONCEBYTES - crypto_box_MACBYTES;
	unsigned char *decrypted = calloc(1, mlen);

	unsigned char *precalck = NULL;
	char *dot = strchr(source, '.');
	if (dot) {
    	*dot = '\0';
    	precalck = zhash_lookup(dd->keys->clientkeys, source);
    	if (precalck) {
      		//	dd_info("decrypting with tenant key:%s\n", source);
    	}
    	*dot = '.';
  	}
  	if (!precalck) {
    	if (strncmp("public.", source, strlen("public.")) == 0) {
    		precalck = dd->keys->pubboxk;
      		//	dd_info("decrypting with public tenant key\n");
    	} else {
      		precalck = dd->keys->custboxk;
      		//      dd_info("decrypting with my own key\n");
    	}
  	}

	retval = crypto_box_open_easy_afternm(
      decrypted, data + crypto_box_NONCEBYTES,
      enclen - crypto_box_NONCEBYTES, data, precalck);

	if (retval == 0) {
    	dd->on_pub(source, topic, decrypted, mlen, dd);
	} else {
    	logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "DD: Unable to decrypt %d bytes from %s, topic %s\n",
            mlen, source, topic);
  	}
  	free(decrypted);
  
  	logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "PUB!");
}

void cmd_cb_subok(zmsg_t *msg, ddclient_t *dd) {
	char *topic = zmsg_popstr(msg);
	char *scope = zmsg_popstr(msg);
	sublist_activate(topic, scope, dd);
  
	logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "SUBOK!");
}

void cmd_cb_nodst(zmsg_t *msg, ddclient_t *dd) {
	char *destination = zmsg_popstr(msg);
	dd->on_nodst(destination, dd);
}

int s_on_dealer_msg(zloop_t *loop, zsock_t *handle, void *args) {
	ddclient_t *dd = (ddclient_t *)args;
	dd->timeout = 0;
	zmsg_t *msg = zmsg_recv(handle);
	// zmsg_print(msg);

	if (msg == NULL) {
		logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "DD: zmsg_recv returned NULL\n");
		return 0;
	}
	if (zmsg_size(msg) < 2) {
		logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "DD: Message length less than 2, error!\n");
	    zmsg_destroy(&msg);
	    return 0;
	}

	zframe_t *proto_frame = zmsg_pop(msg);

	if (*((uint32_t *)zframe_data(proto_frame)) != dd_version) {
	    logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "DD: Wrong version, expected 0x%x, got 0x%x\n",
            dd_version, *zframe_data(proto_frame));
	    zframe_destroy(&proto_frame);
	    return 0;
	}
	zframe_t *cmd_frame = zmsg_pop(msg);
	uint32_t cmd = *((uint32_t *)zframe_data(cmd_frame));
	zframe_destroy(&cmd_frame);
	switch (cmd) {
	case DD_CMD_SEND:
    	logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "DD: Got command DD_CMD_SEND");
    	break;
	case DD_CMD_FORWARD:
		logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "DD: Got command DD_CMD_FORWARD");
		break;
	case DD_CMD_PING:
		logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "DD: Got command DD_CMD_PING");
	    break;
	case DD_CMD_ADDLCL:
	    logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "DD: Got command DD_CMD_ADDLCL");
	    break;
	case DD_CMD_ADDDCL:
		logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "DD: Got command DD_CMD_ADDDCL");
	    break;
	case DD_CMD_ADDBR:
	    logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "DD: Got command DD_CMD_ADDBR");
	    break;
	case DD_CMD_UNREG:
	    logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "DD: Got command DD_CMD_UNREG");
	    break;
	case DD_CMD_UNREGDCLI:
	    logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "DD: Got command DD_CMD_UNREGDCLI");
	    break;
	case DD_CMD_UNREGBR:
	    logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "DD: Got command DD_CMD_UNREGBR");
	    break;
	case DD_CMD_DATA:
	    cmd_cb_data(msg, dd);
	    break;
	case DD_CMD_NODST:
	    logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "DD: Got command DD_CMD_NODST");
	    cmd_cb_nodst(msg, dd);
	    break;
	case DD_CMD_REGOK:
	    cmd_cb_regok(msg, dd, loop);
	    break;
	case DD_CMD_PONG:
	    cmd_cb_pong(msg, dd, loop);
	    break;
	case DD_CMD_CHALL:
	    logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "DD: Got command DD_CMD_CHALL");
	    cmd_cb_chall(msg, dd);
	    break;
	case DD_CMD_CHALLOK:
	    logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "DD: Got command DD_CMD_CHALLOK");
	    break;
	case DD_CMD_PUB:
	    cmd_cb_pub(msg, dd);
	    break;
	case DD_CMD_SUB:
	    logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "DD: Got command DD_CMD_SUB");
	    break;
	case DD_CMD_UNSUB:
	    logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "DD: Got command DD_CMD_UNSUB");
	    break;
	case DD_CMD_SENDPUBLIC:
	    logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "DD: Got command DD_CMD_SENDPUBLIC");
	    break;
	case DD_CMD_PUBPUBLIC:
	    logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "DD: Got command DD_CMD_PUBPUBLIC");
	    break;
	case DD_CMD_SENDPT:
	    logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "DD: Got command DD_CMD_SENDPT");
	    break;
	case DD_CMD_FORWARDPT:
	    logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "DD: Got command DD_CMD_FORWARDPT");
	    break;
	case DD_CMD_DATAPT:
	    logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "DD: Got command DD_CMD_DATAPT");
	    break;
	case DD_CMD_SUBOK:
	    cmd_cb_subok(msg, dd);
	    // dd_info("Got command DD_CMD_SUBOK\n");
	    break;
	default:
	    logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "DD: Unknown command, value: 0x%x", cmd);
	    break;
	}
	zmsg_destroy(&msg);
	return 0;
}

// callback functions
void on_reg (void *args)
{
	ddclient_t *dd = (ddclient_t*) args;
	logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Registered with broker %s!", dd->endpoint);
}

void on_discon (void *args)
{
	ddclient_t *dd = (ddclient_t*) args;
	logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Got disconnected from broker %s!", dd->endpoint);
}

void on_pub (char *source, char *topic, unsigned char *data,
	int length, void *args)
{
//	ddclient_t *dd = (ddclient_t*) args;
	logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "PUB S: %s T: %s L: %d D: '%s'", source, topic, length, data);
}

void on_data (char *source, unsigned char *data, int length, void *args)
{
//	ddclient_t *dd = (ddclient_t*) args;
	logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "DATA S: %s L: %d D: '%s'", source, length, data);
}

void on_nodst (char *source, void *args)
{
//	ddclient_t *dd = (ddclient_t*) args;
	logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "NODST T: %s", source);
}

// Threads

void *ddclient(void *args) {
	ddclient_t *dd = (ddclient_t *)args;
	int rc;

	dd->socket = zsock_new_dealer(NULL);
	if (!dd->socket) {
    	logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "DD: Error in zsock_new_dealer: %s\n",
            zmq_strerror(errno));
		free(dd);
	    return;
	} 
  
	//  zsock_set_identity (dd->socket, dd->client_name);
	rc = zsock_connect(dd->socket, dd->endpoint);
	if (rc != 0) {
	    logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "DD: Error in zmq_connect: %s\n", zmq_strerror(errno));
	    free(dd);
	    return;
	}

	dd->keys = read_ddkeys(dd->keyfile, dd->customer);
	if (dd->keys == NULL) {
	    logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "DD: Error reading keyfile!%s\n", dd->keyfile);
	    return;
	}

	dd->sublist = zlistx_new();
	zlistx_set_destructor(dd->sublist, (czmq_destructor *)sublist_free);
	zlistx_set_duplicator(dd->sublist, (czmq_duplicator *)sublist_dup);
	zlistx_set_comparator(dd->sublist, (czmq_comparator *)sublist_cmp);

	dd->loop = zloop_new();
	assert(dd->loop);
	dd->registration_loop =
      zloop_timer(dd->loop, 1000, 0, s_ask_registration, dd);
	rc = zloop_reader(dd->loop, dd->socket, s_on_dealer_msg, dd);
	zloop_start(dd->loop);
	zloop_destroy(&dd->loop);
}

ddclient_t *start(int verbose, char *client_name, char *customer,
                           char *endpoint, char *keyfile, dd_con con,
                           dd_discon discon, dd_data data, dd_pub pub,
                           dd_nodst nodst) {
                          
	ddclient_t *dd = malloc(sizeof(ddclient_t));
	dd->verbose = verbose;
	dd->client_name = strdup(client_name);
	dd->customer = strdup(customer);
	dd->endpoint = strdup(endpoint);
	dd->keyfile = strdup(keyfile);
	dd->timeout = 0;
	dd->state = DD_STATE_UNREG;
	randombytes_buf(dd->nonce, crypto_box_NONCEBYTES);
	dd->on_reg = con;
	dd->on_discon = discon;
	dd->on_data = data;
	dd->on_pub = pub;
	dd->on_nodst = nodst;
	dd->subscribe = (void *)subscribe;
	dd->unsubscribe = (void *)unsubscribe;
	dd->publish = (void *)publish;
	dd->notify = (void *)notify;
	dd->shutdown = (void *)ddclient_shutdown;
  
	//FIXED
	zthread_new(ddclient, dd);
	//ddclient(dd);
	return dd;
}

void print_ddkeystate(ddkeystate_t *keys) {
	char *hex = malloc(100);
	logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Hash value: \t%s", keys->hash);
	logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Private key: \t%s", sodium_bin2hex(hex, 100, keys->privkey, 32));
	logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Public key: \t%s", sodium_bin2hex(hex, 100, keys->pubkey, 32));
	logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "DDPublic key: \t%s", sodium_bin2hex(hex, 100, keys->ddpubkey, 32));
	logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "PublicPub key: \t%s", sodium_bin2hex(hex, 100, keys->publicpubkey, 32));
	free(hex);
}

ddclient_t *init (char *client_name, char *customer, char *keyfile, char *connect_to)
{
	if (client_name == NULL || customer == NULL || keyfile == NULL || connect_to == NULL)
   	{
    	logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "usage: client -c <customer> -k <keyfile> -n <name> -d <tcp/ipc url>\n");
    	return NULL;
    }

	logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Starting DoubleDecker client");
    
	client = start(1, client_name, customer,
                           connect_to, keyfile, on_reg,
                           on_discon, on_data, on_pub,
                           on_nodst);
    
    return client;
}


