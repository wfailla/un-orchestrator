#ifdef UNIFY_NFFG
	#include <Python.h>
	#include "node_resource_manager/virtualizer/virtualizer.h"
#endif	

#include "utils/constants.h"
#include "utils/logger.h"
#include "node_resource_manager/rest_server/rest_server.h"

#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <openssl/sha.h>
#include "node_resource_manager/database_manager/SQLite/SQLiteManager.h"

#include "node_resource_manager/database_manager/SQLite/INIReader.h"	

/**
*	Global variables (defined in ../utils/constants.h)
*/
ofp_version_t OFP_VERSION;

/**
*	Private variables
*/
struct MHD_Daemon *http_daemon = NULL;

/*
*
* Pointer to database class
*
*/
SQLiteManager *dbm = NULL;

/**
*	Private prototypes
*/
bool parse_command_line(int argc, char *argv[],int *rest_port, bool *cli_auth, char **nffg_file_name,int *core_mask, char **ports_file_name);
bool usage(void);
bool doChecks(void);
void terminateRestServer(void);
#ifdef UNIFY_NFFG
	void terminateVirtualizer(void);
#endif

/**
*	Implementations
*/

void singint_handler(int sig)
{
    logger(ORCH_INFO, MODULE_NAME, __FILE__, __LINE__, "The '%s' is terminating...",MODULE_NAME);

	MHD_stop_daemon(http_daemon);
	terminateRestServer();
	
#ifdef UNIFY_NFFG
	terminateVirtualizer();
#endif
	
	logger(ORCH_INFO, MODULE_NAME, __FILE__, __LINE__, "Bye :D");
	exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
	//Check for root privileges
	if(geteuid() != 0)
	{	
		logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "Root permissions are required to run %s\n",argv[0]);
		exit(EXIT_FAILURE);	
	}
	
	if(!doChecks())
		exit(EXIT_FAILURE);	
	
	//XXX: change this line to use different versions of Openflow
	OFP_VERSION = OFP_12;	

	int core_mask;
	int rest_port, t_rest_port;
	bool cli_auth, t_cli_auth;
	char *ports_file_name = new char[BUFFER_SIZE], *t_ports_file_name = NULL;
	char *nffg_file_name = new char[BUFFER_SIZE], *t_nffg_file_name = NULL;

	if(!parse_command_line(argc,argv,&t_rest_port,&t_cli_auth,&t_nffg_file_name,&core_mask,&t_ports_file_name))
		exit(EXIT_FAILURE);
	
	rest_port = t_rest_port;
	cli_auth = t_cli_auth;
	strcpy(ports_file_name, t_ports_file_name);
	if(strcmp(t_nffg_file_name, "UNKNOWN") != 0)
		strcpy(nffg_file_name, t_nffg_file_name);
	else	
		nffg_file_name = NULL;

	//XXX: this code avoids that the program terminates when system() is executed
	sigset_t mask;
	sigfillset(&mask);
	sigprocmask(SIG_SETMASK, &mask, NULL);

#ifdef UNIFY_NFFG
	//Initialize the Python code
	setenv("PYTHONPATH",PYTHON_DIRECTORY ,1);
	Py_SetProgramName(argv[0]);  /* optional but recommended */
    Py_Initialize();

    if(!Virtualizer::init())
    {
    	logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "Cannot start the virtualizer. The %s cannot be run.",MODULE_NAME);
		return EXIT_FAILURE;
	}
#endif
	if(!RestServer::init(cli_auth,nffg_file_name,core_mask,ports_file_name))
	{
		logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "Cannot start the %s",MODULE_NAME);
#ifdef UNIFY_NFFG
		terminateVirtualizer();
#endif
		exit(EXIT_FAILURE);	
	}

	http_daemon = MHD_start_daemon (MHD_USE_SELECT_INTERNALLY, rest_port, NULL, NULL,&RestServer::answer_to_connection,
		NULL, MHD_OPTION_NOTIFY_COMPLETED, &RestServer::request_completed, NULL,MHD_OPTION_END);
	
	if (NULL == http_daemon)
	{
		logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "Cannot start the HTTP deamon. The %s cannot be run.",MODULE_NAME);
		logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "Please, check that the TCP port %d is not used (use the command \"netstat -a | grep %d\")",rest_port,rest_port);
		
		terminateRestServer();
#ifdef UNIFY_NFFG
		terminateVirtualizer();
#endif
		
		return EXIT_FAILURE;
	}
	
	signal(SIGINT,singint_handler);
	
	logger(ORCH_INFO, MODULE_NAME, __FILE__, __LINE__, "The '%s' is started!",MODULE_NAME);
	logger(ORCH_INFO, MODULE_NAME, __FILE__, __LINE__, "Waiting for commands on TCP port \"%d\"",rest_port);
	
	rofl::cioloop::get_loop().run();
	
	return 0;
}

bool parse_command_line(int argc, char *argv[],int *rest_port, bool *cli_auth, char **nffg_file_name,int *core_mask, char **ports_file_name)
{
	int opt;
	char **argvopt;
	int option_index;
	
static struct option lgopts[] = {
		{"c", 1, 0, 0},
		{"i", 1, 0, 0},
		{"w", 1, 0, 0},
		{"h", 0, 0, 0},
		{NULL, 0, 0, 0}
	};

	argvopt = argv;
	uint32_t arg_c = 0;

	*core_mask = CORE_MASK;
	ports_file_name[0] = '\0';
	nffg_file_name[0] = '\0';
	*rest_port = REST_PORT;

	/*
	*	
	* Parsing universal node configuration file
	*
	*/
	INIReader reader(DEFAULT_FILE);

	if (reader.ParseError() < 0) {
		logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "Can't load a universal-node-config.ini file");
		return false;
	}
	
	/* physical ports file name */
	char *temp_config_file = (char *)reader.Get("rest server", "configuration_file", "UNKNOWN").c_str();
	if(strcmp(temp_config_file, "UNKNOWN") != 0 && strcmp(temp_config_file, "") != 0)
		*ports_file_name = temp_config_file;
	else
	{
		logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "Physical ports file must be present");
		return false;
	}

	/* REST port */
	int temp_rest_port = (int)reader.GetInteger("rest server", "server_port", -1);
	if(temp_rest_port != -1)
		*rest_port = temp_rest_port;

	/* client authentication */
	*cli_auth = reader.GetBoolean("user authentication", "user_authentication", true);		

	/* first nf-fg file name */
	char *temp_nf_fg = (char *)reader.Get("rest server", "nf-fg", "UNKNOWN").c_str();
	*nffg_file_name = temp_nf_fg;

	while ((opt = getopt_long(argc, argvopt, "", lgopts, &option_index)) != EOF)
    	{
		switch (opt)
		{
			/* long options */
			case 0:
	   			if (!strcmp(lgopts[option_index].name, "c"))/* core mask for network functions */
	   			{
	   				if(arg_c > 0)
	   				{
		   				logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "Argument \"--c\" can appear only once in the command line");
	   					return usage();
	   				}
	   				char *port = (char*)malloc(sizeof(char)*(strlen(optarg)+1));
	   				strcpy(port,optarg);
	   				
	   				sscanf(port,"%x",&(*core_mask));
	   				
	   				arg_c++;
	   			}
				else if (!strcmp(lgopts[option_index].name, "i"))/* inserting admin password */
	   			{
					if(*cli_auth)
						dbm = new SQLiteManager(DB_NAME);
					else
					{
						logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "Argument \"--i\" can appear only if authentication is required");
						return false;
					}
	   				/*
					*
					* Test user database
					*	
					*/
					unsigned char *hash_token = new unsigned char[HASH_SIZE];
					char *hash_pwd = new char[BUFFER_SIZE];
					char *tmp = new char[HASH_SIZE];
					char *pwd = new char[HASH_SIZE];

					if(dbm->createTable()){
						strcpy(pwd, optarg);
	
						SHA512((const unsigned char*)pwd, sizeof(pwd) - 1, hash_token);
	
						strcpy(tmp, "");
						strcpy(hash_pwd, "");

						    for (int i = 0; i < HASH_SIZE; i++) {
							sprintf(tmp, "%x", hash_token[i]);
							strcat(hash_pwd, tmp);
						    }
					    
						dbm->insertUsrPwd("admin", (char *)hash_pwd);
					}
	   				
	   				arg_c++;
	   			}
				else if (!strcmp(lgopts[option_index].name, "h"))/* help */
	   			{
	   				return usage();
	   			}
	   			else
	   			{
	   				logger(ORCH_ERROR, MODULE_NAME, __FILE__, __LINE__, "Invalid command line parameter '%s'\n",lgopts[option_index].name);
	   				return usage();
	   			}
				break;
			default:
				return usage();
		}
	}

	return true;
}

bool usage(void)
{
	char message[]=	\
	"Usage:                                                                                   \n" \
	"  sudo ./name-orchestrator 		                                                  \n" \
	"                                                                                         \n" \
	"Options:                                                                                 \n" \
	"  --c core_mask                                                                          \n" \
	"        Mask that specifies which cores must be used for DPDK network functions. These   \n" \
	"        cores will be allocated to the DPDK network functions in a round robin fashion   \n" \
	"        (default is 0x2)                                                                 \n" \
	"  --i admin password                                                                     \n" \
	"        Initialize local database and set the password for the default 'admin' user  	  \n" \
	"  --h                                                                                    \n" \
	"        Print this help.                                                                 \n" \
	"                                                                                         \n" \
	"Example:                                                                                 \n" \
	"  sudo ./node-orchestrator 								  \n";

	logger(ORCH_INFO, MODULE_NAME, __FILE__, __LINE__, "\n\n%s",message);
	
	return false;
}

/**
*	This function checks if the UN is properly configured.
*/
bool doChecks(void)
{

#ifdef VSWITCH_IMPLEMENTATION_OFCONFIG
	logger(ORCH_WARNING, MODULE_NAME, __FILE__, __LINE__, "The support to OFCONFIG is deprecated.");
#endif

	return true;
}

#ifdef UNIFY_NFFG
void terminateVirtualizer()
{
	//Terminate the Python code
	Virtualizer::terminate();
	Py_Finalize();
}
#endif

void terminateRestServer()
{
	try
	{
		RestServer::terminate();
	}catch(...)
	{
		//Do nothing, since the program is terminating
	}
}
