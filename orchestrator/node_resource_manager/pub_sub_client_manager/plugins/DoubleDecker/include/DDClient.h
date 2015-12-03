#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <czmq.h>
#include <sodium.h>

#include "../../../../../utils/logger.h"
#include "../../../../../utils/constants.h"

#include "../DDClientManager_constants.h"

typedef struct ddkeystate {
  unsigned char *publicpubkey;
  unsigned char *ddpubkey;
  unsigned char *privkey;
  unsigned char *pubkey;
  unsigned char *ddboxk;
  unsigned char *custboxk;
  unsigned char *pubboxk;
  char *hash;
  zhash_t *clientkeys;
} ddkeystate_t;

typedef struct ddbrokerkeys {
  // list of tenant names
  zlist_t *tenants;
  // broker private and public keys
  unsigned char *privkey;
  unsigned char *pubkey;
  // Broker pre-calc key
  unsigned char *ddboxk;
  // Precalculated tenant keys
  zhash_t *tenantkeys;
  // hash string
  char *hash;
  uint64_t cookie;
} ddbrokerkeys_t;

typedef struct tenantsinfo {
  char *name;
  uint64_t cookie;
  char *boxk;
} ddtenant_t;

ddkeystate_t *read_ddkeys(char *filename, char *customer);
ddbrokerkeys_t *read_ddbrokerkeys(char *filename);
void print_ddkeystate(ddkeystate_t *keys);
void free_ddkeystate(ddkeystate_t *keys);

// state definitions
#define DD_STATE_UNREG       1
#define DD_STATE_ROOT        2
#define DD_STATE_EXIT        3
#define DD_STATE_CHALLENGED  4
#define DD_STATE_REGISTERED  5

// Commands and version
#define DD_VERSION 0x0d0d0001
#define DD_CMD_SEND         0
#define DD_CMD_FORWARD      1
#define DD_CMD_PING         2
#define DD_CMD_ADDLCL       3
#define DD_CMD_ADDDCL       4
#define DD_CMD_ADDBR        5
#define DD_CMD_UNREG        6
#define DD_CMD_UNREGDCLI    7
#define DD_CMD_UNREGBR      8
#define DD_CMD_DATA         9
#define DD_CMD_NODST       10
#define DD_CMD_REGOK       11
#define DD_CMD_PONG        12
#define DD_CMD_CHALL       13
#define DD_CMD_CHALLOK     14
#define DD_CMD_PUB         15
#define DD_CMD_SUB         16
#define DD_CMD_UNSUB       17
#define DD_CMD_SENDPUBLIC  18
#define DD_CMD_PUBPUBLIC   19
#define DD_CMD_SENDPT      20
#define DD_CMD_FORWARDPT   21
#define DD_CMD_DATAPT      22
#define DD_CMD_SUBOK       23

extern const uint32_t dd_cmd_send;
extern const uint32_t dd_cmd_forward;
extern const uint32_t dd_cmd_ping;
extern const uint32_t dd_cmd_addlcl;
extern const uint32_t dd_cmd_adddcl;
extern const uint32_t dd_cmd_addbr;
extern const uint32_t dd_cmd_unreg;
extern const uint32_t dd_cmd_unregdcli;
extern const uint32_t dd_cmd_unregbr;
extern const uint32_t dd_cmd_data;
extern const uint32_t dd_cmd_nodst;
extern const uint32_t dd_cmd_regok;
extern const uint32_t dd_cmd_pong;
extern const uint32_t dd_cmd_chall;
extern const uint32_t dd_cmd_challok;
extern const uint32_t dd_cmd_pub;
extern const uint32_t dd_cmd_sub;
extern const uint32_t dd_cmd_unsub;
extern const uint32_t dd_cmd_sendpublic;
extern const uint32_t dd_cmd_pubpublic;
extern const uint32_t dd_cmd_sendpt;
extern const uint32_t dd_cmd_forwardpt;
extern const uint32_t dd_cmd_datapt;
extern const uint32_t dd_cmd_subok;
extern const uint32_t dd_version;


// On connection
typedef void (dd_con) (void *);
// On disconnection
typedef void (dd_discon) (void *);
// On recieve DATA
typedef void (dd_data) (char *, unsigned char *, int, void *);
// On recieve PUB
typedef void (dd_pub) (char *, char *, unsigned char *, int, void *);
// On no destination
typedef void (dd_nodst) (char *, void *);

typedef struct ddclient
{
  void *socket;			//  Socket for clients & workers
  int verbose;			//  Print activity to stdout
  unsigned char *endpoint;	//  Broker binds to this endpoint
  unsigned char *customer;	//  Our customer id
  unsigned char *keyfile;	// JSON file with pub/priv keys
  unsigned char *client_name;	// This client name
  int timeout;			// Incremental timeout (trigger > 3)
  int state;			// Internal state
  int registration_loop;	// Timer ID for registration loop
  int heartbeat_loop;		// Timer ID for heartbeat loop
  uint64_t cookie; // Cookie from authentication 
  struct ddkeystate *keys;	// Encryption keys loaded from JSON file
  zlistx_t *sublist;		// List of subscriptions, and if they're active
  zloop_t *loop; 
  unsigned char nonce[crypto_box_NONCEBYTES];
  dd_con (*on_reg);
  dd_discon (*on_discon);
  dd_data (*on_data);
  dd_pub (*on_pub);
  dd_nodst (*on_nodst);
  void (*subscribe) (char *, char *,  struct ddclient *);
  void (*unsubscribe) (char *, char *,struct ddclient  *);
  void (*publish) (char *, char *, int, struct ddclient *);
  void (*notify) (char *, char *, int, struct ddclient *);
  void (*shutdown) (void *); 
} ddclient_t;

void sublist_print(ddclient_t *);
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
