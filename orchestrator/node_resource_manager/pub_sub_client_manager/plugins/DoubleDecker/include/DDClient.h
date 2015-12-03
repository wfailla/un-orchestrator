#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <czmq.h>
#include "../include/dd.h"
#include "../include/ddkeys.h"

#include "cli_parser/cparser.h"
#include "cli_parser/cparser_priv.h"
#include "cli_parser/cparser_token.h"

#include "../../../../../utils/logger.h"
#include "../../../../../utils/constants.h"

#include "../DDClientManager_constants.h"

void sublist_print(ddclient_t *);
cparser_result_t cparser_cmd_show_subscriptions (cparser_context_t *);
cparser_result_t cparser_cmd_show_status (cparser_context_t *);
cparser_result_t cparser_cmd_subscribe_topic_scope (cparser_context_t *, char **, char **);
cparser_result_t cparser_cmd_no_subscribe_topic_scope (cparser_context_t *, char **, char **);
cparser_result_t cparser_cmd_publish_topic_message (cparser_context_t *, char **, char **);
cparser_result_t cparser_cmd_notify_destination_message (cparser_context_t *, char **, char **);
cparser_result_t cparser_cmd_quit (cparser_context_t *);
cparser_result_t cparser_cmd_help (cparser_context_t *);
int sublist_cmp(const void *, const void *);
void sublist_free(void **);
void *sublist_dup(const void *);
void sublist_add(char *, char *, char, ddclient_t *);
void sublist_delete(char *, char *, ddclient_t *);
void sublist_activate(char *, char *, ddclient_t *);
void sublist_deactivate_all(ddclient_t *);
void sublist_resubscribe(ddclient_t *);
void on_reg (void *);
void on_discon (void *);
void on_pub (char *, char *, unsigned char *, int, void *);
void on_pub (char *, char *, unsigned char *, int, void *);
void on_data (char *, unsigned char *, int, void *);
void on_nodst (char *, void *);
void *ddclient(void *);
ddclient_t *start(int, char *, char *, char *, char *, dd_con, dd_discon, dd_data, dd_pub, dd_nodst);
void print_ddkeystate(ddkeystate_t *);
ddclient_t *init (char *, char *, char *, char *);
