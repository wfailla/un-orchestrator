#include "native.h"

bool Native::isSupported() {
/*
	TODO:
	Sergio: read from the configuration file the available programs and functionalities
		insert them in a dynamic structure (use a static pointer as private pointer in this class)
		this structure will be used from the function selectImplementation(native) 
	IMPROVEMENTS:
		call a script that checks the available functions in the system (e.g. iptables) and fills the structure
*/
	
	//clear previous capabilities
	capabilities.clear();
	
	logger(ORCH_DEBUG, MODULE_NAME, __FILE__, __LINE__, "Reading capabilities...");
	
	//read capabilities from capabilities_file
	std::ifstream cap_stream(CAPABILITIES_FILE);
	std::string str, name, path;
	
	while(std::getline(cap_stream, str)) {
		//2 string per line expected
		std::stringstream ss(str);
		ss >> name;
		if(ss)
			ss >> path;
		else{
			logger(ORCH_WARNING, MODULE_NAME, __FILE__, __LINE__, "Wrong line in capabilities file: %s\n",str);
			break;
		}
		if(ss){
			logger(ORCH_WARNING, MODULE_NAME, __FILE__, __LINE__, "Wrong line in capabilities file: %s\n",str);
			break;
		}
		capabilities.insert(std::pair<std::string, std::string>(name, path));
	}
	
	if(capabilities.size() > 0) {
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Native functions are supported.");
		return true;
	}
	
	/*
	 *	Sergio: eventuale chiamata a script
	int retVal;
	
	retVal = system(CHECK_NATIVE);
	retVal = retVal >> 8;

	logger(ORCH_DEBUG, MODULE_NAME, __FILE__, __LINE__, "Script returned: %d\n",retVal);

	if(retVal > 0)
	{
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Native functions are supported.");
		return true;
	}
	 */

	logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Native functions are not supported.");
	return false;
}

bool Native::startNF(StartNFIn sni) {

	uint64_t lsiID = sni.getLsiID();
	std::string nf_name = sni.getNfName();
	unsigned int n_ports = sni.getNumberOfPorts();
	std::map<unsigned int,pair<std::string, std::string> > ipv4PortsRequirements = sni.getIpv4PortsRequirements();
	std::map<unsigned int, std::string> ethPortsRequirements = sni.getEthPortsRequirements();
	
	std::string uri_image = description->getURI();
	
	std::stringstream command;
	command << PULL_AND_RUN_NATIVE_NF << " " << lsiID << " " << nf_name << " " << uri_image << " " << n_ports;
	
	//create the names of the ports
	for(unsigned int i = 1; i <= n_ports; i++)
		command << " " << lsiID << "_" << nf_name << "_" << i;
		
	//specify the IPv4 requirements for the ports
	for(unsigned int i = 1; i <= n_ports; i++)
	{
		if(ipv4PortsRequirements.count(i) == 0)
			command << " " << 0;
		else
		{
			pair<string, string> req = (ipv4PortsRequirements.find(i))->second;
			command << " " << req.first <<"/" << convertNetmask(req.second);
		}
	}
	//specify the ethernet requirements for the ports
	for(unsigned int i = 1; i <= n_ports; i++)
	{
		if(ethPortsRequirements.count(i) == 0)
			command << " " << 0;
		else
			command << " " << (ethPortsRequirements.find(i))->second;
	}

	logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Executing command \"%s\"",command.str().c_str());

	int retVal = system(command.str().c_str());
	retVal = retVal >> 8;
	
	if(retVal == 0)
		return false;
		
	return true;
}

bool Native::stopNF(StopNFIn sni) {

	uint64_t lsiID = sni.getLsiID();
	std::string nf_name = sni.getNfName();

	std::stringstream command;
	command << STOP_NATIVE_NF << " " << lsiID << " " << nf_name;
	
	logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Executing command \"%s\"",command.str().c_str());
	int retVal = system(command.str().c_str());
	retVal = retVal >> 8;

	if(retVal == 0)
		return false;

	return true;
}

unsigned int Native::convertNetmask(string netmask) {

	unsigned int slash = 0;
	unsigned int mask;
	
	int first, second, third, fourth;
	sscanf(netmask.c_str(),"%d.%d.%d.%d",&first,&second,&third,&fourth);
	mask = (first << 24) + (second << 16) + (third << 8) + fourth;
	
	for(int i = 0; i < 32; i++)
	{
		if((mask & 0x1) == 1)
			slash++;
		mask = mask >> 1;
	}
	
	return slash;
}

