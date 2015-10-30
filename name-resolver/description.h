#ifndef DESCRIPTION_H_
#define DESCRIPTION_H_ 1

#include <string>
#include <assert.h>

#include <json_spirit/json_spirit.h>
#include <json_spirit/value.h>
#include <json_spirit/writer.h>

#include "constants.h"
//#include "nf.h"

using namespace std;
using namespace json_spirit;

typedef enum{DPDK,DOCKER,KVM,NATIVE}nf_t;

class Description
{
protected:
	nf_t type;
	string uri;
	
public:
	Description(nf_t type, string uri);
	
	virtual ~Description(){};

	virtual Object toJSON();

};

#endif //DESCRIPTION_H_
