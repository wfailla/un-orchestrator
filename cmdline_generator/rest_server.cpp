#include "rest_server.h"

#define MODULE_NAME "cmdline-generator"

void RestServer::request_completed (void *cls, struct MHD_Connection *connection,
						void **con_cls, enum MHD_RequestTerminationCode toe)
{
	struct connection_info_struct *con_info = (struct connection_info_struct *)(*con_cls);

	if (con_info == NULL)
		return;

	free (con_info);
	*con_cls = NULL;
}

int RestServer::answer_to_connection (	void *cls,
										struct MHD_Connection *connection,
										const char *url,
										const char *method,
										const char *version,
										const char *upload_data,
										size_t *upload_data_size,
										void **con_cls)
{
	if(*con_cls == NULL)
	{
		struct connection_info_struct *con_info;
		con_info = (struct connection_info_struct*)malloc (sizeof (struct connection_info_struct));

		if (NULL == con_info)
			return MHD_NO;

		if (strcmp (method, "GET") != 0)
		{
			LOG(ORCH_WARNING, MODULE_NAME, "Received different request to GET");
			struct MHD_Response *response = MHD_create_response_from_buffer (0,(void*) "", MHD_RESPMEM_PERSISTENT);
			int ret = MHD_queue_response (connection, MHD_HTTP_NOT_IMPLEMENTED, response);
			MHD_destroy_response (response);
			return ret;
		}

		*con_cls = (void*) con_info;
		return MHD_YES;
	}

	if (strcmp(method, "GET") == 0)
		return doGet(connection,url);

	//XXX: just for the compiler
	return MHD_YES;
}

int RestServer::doGet(struct MHD_Connection *connection, const char *url)
{

	struct MHD_Response *response;
	int ret;
	char cmdline[PATH_MAX];
	char * port = (char *) url + 1;

	/*XXX: Some control about the length to avoid attacks */
	ret = get_cmdline(port, cmdline, PATH_MAX);
	if(ret == 0)
	{
		Object json;
		json["command"] = cmdline;
		stringstream ssj;
 		write_formatted(json, ssj);
 		string sssj = ssj.str();
 		char * aux = (char *) malloc(sizeof(char) * (sssj.length()+1));
 		strcpy(aux,sssj.c_str());
		response = MHD_create_response_from_buffer (strlen(aux),(void*) aux, MHD_RESPMEM_MUST_FREE);
		MHD_add_response_header (response, "Content-Type", "application/json");
		MHD_add_response_header (response, "Cache-Control", "no-cache");
		ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
		MHD_destroy_response (response);
		return ret;
	}
	else
	{
		int ret;
		response = MHD_create_response_from_buffer (0,(void*) "", MHD_RESPMEM_PERSISTENT);
		ret = MHD_queue_response (connection, MHD_HTTP_NOT_FOUND, response);
		MHD_destroy_response (response);
		return ret;
	}

	return 0;
}
