#ifndef DESCRIPTION_H_
#define DESCRIPTION_H_ 1

#include "nf_type.h"

#pragma once

#include "../utils/logger.h"
#include "../utils/constants.h"

#include <string>
#include <assert.h>

using namespace std;

class NFsManager;

class Description
{
friend NFsManager;

private:
	nf_t type;
	string uri;
	
public:
	Description(nf_t type, string uri);
	Description(string type, string uri);
	virtual ~Description();
	
	string getURI();
	nf_t getType();
};

#endif //DESCRIPTION_H_
