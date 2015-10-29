#ifndef REST_SERVER_H_
#define REST_SERVER_H_ 1

/**
*	@brief: the REST server is based on the microhttpd library:
*				www.gnu.org/software/libmicrohttpd/
*
*	Documentation on HTTP return values can be found at:
*		http://www.w3.org/Protocols/rfc2616/rfc2616-sec10.html
*
*	Documentation on HTTP headers can be found at:
*		http://www.w3.org/Protocols/rfc2616/rfc2616-sec14.html
*/

#pragma once

#define __STDC_FORMAT_MACROS

#include <microhttpd.h>
#include <string.h>
#include <assert.h>
#include <iostream>

#include <string>

#include <json_spirit/json_spirit.h>
#include <json_spirit/value.h>
#include <json_spirit/writer.h>

#include "ivshmem_cmdline_generator.h"
#include "logger.h"

using namespace json_spirit;
using namespace std;

class RestServer
{
	public:
		static int answer_to_connection (	void *cls,
									struct MHD_Connection *connection,
									const char *url,
									const char *method,
									const char *version,
									const char *upload_data,
									size_t *upload_data_size,
									void **con_cls);

		static void request_completed (void *cls,
								struct MHD_Connection *connection,
								void **con_cls,
								enum MHD_RequestTerminationCode toe);
	private:
		struct connection_info_struct {};

		static int doGet(struct MHD_Connection *connection, const char *url);
};

#endif //REST_SERVER_H_
