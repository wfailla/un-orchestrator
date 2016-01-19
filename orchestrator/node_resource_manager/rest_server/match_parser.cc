#include "match_parser.h"

string MatchParser::graphID(string name_port)
{
	return nfName(name_port);
}

string MatchParser::nfName(string name_port)
{
	char delimiter[] = ":";
	char tmp[BUFFER_SIZE];
	strcpy(tmp,name_port.c_str());
	char *pnt=strtok(tmp, delimiter);
	while( pnt!= NULL )
	{
		return string(pnt);
	}
	
	return "";
}

unsigned int MatchParser::nfPort(string name_port)
{
	char delimiter[] = ":";
	char tmp[BUFFER_SIZE];
	strcpy(tmp,name_port.c_str());
	char *pnt=strtok((char*)name_port.c_str(), delimiter);
	unsigned int port = 0;

	int i = 0;
	while( pnt!= NULL )
	{
		switch(i)
		{
			case 1:
				sscanf(pnt,"%u",&port);
				return port;
			break;
		}
		
		pnt = strtok( NULL, delimiter );
		i++;
	}
	
	return port;
}

bool MatchParser::nfIsPort(string name_port)
{
	char delimiter[] = ":";
	char tmp[BUFFER_SIZE];
	strcpy(tmp,name_port.c_str());
	char *pnt=strtok((char*)name_port.c_str(), delimiter);
	unsigned int port = 0;

	int i = 0;
	while( pnt!= NULL )
	{
		switch(i)
		{
			case 1:
				sscanf(pnt,"%u",&port);
				return true;
			break;
		}
		
		pnt = strtok( NULL, delimiter );
		i++;
	}
	
	return false;
}

string MatchParser::epName(string name_port)
{
	char delimiter[] = ":";
	char tmp[BUFFER_SIZE];
	strcpy(tmp,name_port.c_str());
	char *pnt=strtok((char*)name_port.c_str(), delimiter);

	int i = 0;
	while( pnt!= NULL )
	{
		switch(i)
		{
			case 1:
				return pnt;
			break;
		}
		
		pnt = strtok( NULL, delimiter );
		i++;
	}
	
	return "";
}

unsigned int MatchParser::epPort(string name_port)
{
	return nfPort(name_port);
}

/**
*	http://stackoverflow.com/questions/4792035/how-do-you-validate-that-a-string-is-a-valid-mac-address-in-c
*/
bool MatchParser::validateMac(const char* mac)
{
	int i = 0;
	int s = 0;

	while (*mac)
	{
		if (isxdigit(*mac))
			i++;
		else if (*mac == ':' || *mac == '-')
		{
			if (i == 0 || i / 2 - 1 != s)
			break;

			++s;
		}
		else
			s = -1;

		++mac;
	}

    return (i == 12 && (s == 5 || s == 0));
}

/**
*	http://stackoverflow.com/questions/318236/how-do-you-validate-that-a-string-is-a-valid-ipv4-address-in-c
*/
bool MatchParser::validateIpv4(const string &ipAddress)
{
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ipAddress.c_str(), &(sa.sin_addr));
    return result != 0;
}

bool MatchParser::validateIpv6(const string &ipAddress)
{
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET6, ipAddress.c_str(), &(sa.sin_addr));
    return result != 0;
}

bool MatchParser::validateIpv4Netmask(const string &netmask)
{
	if(!validateIpv4(netmask))
		return false;
		
	bool zero = true;
	unsigned int mask;
	
	int first, second, third, fourth;
	sscanf(netmask.c_str(),"%d.%d.%d.%d",&first,&second,&third,&fourth);
	mask = (first << 24) + (second << 16) + (third << 8) + fourth;
	
	for(int i = 0; i < 32; i++)
	{
		if(((mask & 0x1) == 0) && !zero)
			return false;
		if(((mask & 0x1) == 1) && zero)
			zero = false;
			
		mask = mask >> 1;
	}
	
	return true;
}

bool MatchParser::parseMatch(Object object, highlevel::Match &match, map<string,set<unsigned int> > &nfs, map<string,string > &nfs_id, map<string,string > &iface_id, map<string,string > &iface_out_id, highlevel::Graph &graph)
{
	bool foundOne = false;
	bool foundEndPointID = false, foundProtocolField = false, definedInCurrentGraph = false;
	bool is_tcp = false;

	for(Object::const_iterator i = object.begin(); i != object.end(); i++)
	{
		const string& name  = i->first;
		const Value&  value = i->second;
	
		if(name == PORT_IN)
		{
			logger(ORCH_DEBUG, MODULE_NAME, __FILE__, __LINE__, "\"%s\"->\"%s\": \"%s\"",MATCH,PORT_IN,value.getString().c_str());
			if(foundOne)
			{
				logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Only one between keys \"%s\", \"%s\" and \"%s\" are allowed in \"%s\"",PORT_IN,VNF,ENDPOINT,MATCH);
				return false;
			}
		
			foundOne = true;
#ifdef UNIFY_NFFG
			//In this case, the virtualized port name must be translated into the real one.
			try
			{		
#else

			string port_in_name = value.getString();
			string realName;
			const char *port_in_name_tmp = port_in_name.c_str();
			char vnf_name_tmp[BUFFER_SIZE];

			//Check the name of port
			char delimiter[] = ":";
		 	char * pnt;

			int p_type = 0;

			char tmp[BUFFER_SIZE];
			strcpy(tmp,(char *)port_in_name_tmp);
			pnt=strtok(tmp, delimiter);
			int i = 0;

			while( pnt!= NULL )
			{
				switch(i)
				{
					case 0:
						//VNFs port type
						if(strcmp(pnt,VNF) == 0)
						{
							p_type = 0;
							match.setNFEndpointPort(port_in_name_tmp);
						}
						//end-points port type
						else if(strcmp(pnt,ENDPOINT) == 0){
							p_type = 1;
							match.setInputEndpoint(port_in_name_tmp);	
						}
						break;
					case 1:
						if(p_type == 0)
						{
							strcpy(vnf_name_tmp,nfs_id[pnt].c_str());
							strcat(vnf_name_tmp,":");
						}
						break;
					case 3:
						if(p_type == 0)
						{
							strcat(vnf_name_tmp,pnt);
						}
				}
		
				pnt = strtok( NULL, delimiter );
				i++;
			}
#endif			
			//VNFs port type
			if(p_type == 0)
			{
				//convert char *vnf_name_tmp to string vnf_name
				string vnf_name(vnf_name_tmp, strlen(vnf_name_tmp));

				string nf_name = nfName(vnf_name);
				char *tmp_vnf_name = new char[BUFFER_SIZE];
				strcpy(tmp_vnf_name, (char *)vnf_name.c_str());
				unsigned int port = nfPort(string(tmp_vnf_name));
				bool is_port = nfIsPort(string(tmp_vnf_name));
							
				if(nf_name == "" || !is_port)
				{
					logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Network function \"%s\" is not valid. It must be in the form \"name:port\"",vnf_name_tmp);
					return false;	
				}
				/*nf port starts from 0*/
				port++;

				match.setNFport(nf_name,port);
			
				set<unsigned int> ports;
				if(nfs.count(nf_name) != 0)
					ports = nfs[nf_name];
				ports.insert(port);
				nfs[nf_name] = ports;
			}
			//end-points port type
			else if(p_type == 1)
			{
				bool iface_found = false;
				char *s_value = new char[BUFFER_SIZE];
				strcpy(s_value, (char *)value.getString().c_str());
				string eP = epName(value.getString());
				if(eP != ""){
					map<string,string>::iterator it = iface_id.find(eP);
					map<string,string>::iterator it1 = iface_out_id.find(eP);
					if(it != iface_id.end()){
						//Physical port		
						realName.assign(iface_id[eP]);
						iface_found = true;		
					}
					else if(it1 != iface_out_id.end()){
						//Physical port		
						realName.assign(iface_out_id[eP]);	
						iface_found = true;	
					}
				}
				/*gre-tunnel endpoint*/
				if(!iface_found)
				{
					unsigned int endPoint = epPort(string(s_value));
					if(endPoint == 0)
					{
						logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Graph end point \"%s\" is not valid. It must be in the form \"endpoint:id\"",value.getString().c_str());
						return false;	
					}
					match.setEndPoint(endPoint);
			
					stringstream ss;
					ss << match.getEndPoint();
				}
				/*physical endpoint*/
				else
				{
#ifdef UNIFY_NFFG
					realName = Virtualizer::getRealName(port_in_name);		
#endif	
					match.setInputPort(realName);
					graph.addPort(realName);
				}
			} 
#ifdef UNIFY_NFFG
			}catch(exception e)
			{
				logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Error while translating the virtualized port '%s': %s",value.getString().c_str(),e.what());
				return false;
			}
#endif			
			
		}
		else if(name == HARD_TIMEOUT)
		{
			logger(ORCH_WARNING, MODULE_NAME, __FILE__, __LINE__, "\"%s\" \"%s\" not available",MATCH,HARD_TIMEOUT);

			logger(ORCH_DEBUG, MODULE_NAME, __FILE__, __LINE__, "\"%s\"->\"%s\": \"%s\"",MATCH,HARD_TIMEOUT,value.getString().c_str());

			//XXX: currently, this information is ignored	
		}
		else if(name == ETH_TYPE)
		{
			logger(ORCH_DEBUG, MODULE_NAME, __FILE__, __LINE__, "\"%s\"->\"%s\": \"%s\"",MATCH,ETH_TYPE,value.getString().c_str());
			uint32_t ethType;
			if((sscanf(value.getString().c_str(),"%x",&ethType) != 1) || (ethType > 65535))
			{
				logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Key \"%s\" with wrong value \"%s\"",ETH_TYPE,value.getString().c_str());
				return false;
			}
			match.setEthType(ethType & 0xFFFF);
			logger(ORCH_DEBUG, MODULE_NAME, __FILE__, __LINE__, "\"%s\"->\"%s\": \"%s\"",MATCH,ETH_TYPE,value.getString().c_str(),ethType);
			foundProtocolField = true;
		}
		else if(name == VLAN_ID)
		{
			logger(ORCH_DEBUG, MODULE_NAME, __FILE__, __LINE__, "\"%s\"->\"%s\": \"%s\"",MATCH,VLAN_ID,value.getString().c_str());
			if(value.getString() == ANY_VLAN)
				match.setVlanIDAnyVlan();
			else if(value.getString() == NO_VLAN)
				match.setVlanIDNoVlan();
			else
			{
				uint32_t vlanID;
				if((sscanf(value.getString().c_str(),"%"SCNd32,&vlanID) != 1) && (vlanID > 4094))
				{
					logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Key \"%s\" with wrong value \"%s\"",VLAN_ID,value.getString().c_str());
					return false;
				}
				match.setVlanID(vlanID & 0xFFFF);
			}
			foundProtocolField = true;
		}
		else if(name == VLAN_PRIORITY)
		{
			logger(ORCH_WARNING, MODULE_NAME, __FILE__, __LINE__, "\"%s\" \"%s\" not available",MATCH,VLAN_PRIORITY);

			logger(ORCH_DEBUG, MODULE_NAME, __FILE__, __LINE__, "\"%s\"->\"%s\": \"%s\"",MATCH,VLAN_PRIORITY,value.getString().c_str());

			//XXX: currently, this information is ignored	
		}
		else if(name == ETH_SRC)
		{
			logger(ORCH_DEBUG, MODULE_NAME, __FILE__, __LINE__, "\"%s\"->\"%s\": \"%s\"",MATCH,ETH_SRC,value.getString().c_str());
			if(!validateMac(value.getString().c_str()))
			{
				logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Key \"%s\" with wrong value \"%s\"",ETH_SRC,value.getString().c_str());
				return false;
			}
			match.setEthSrc((char*)value.getString().c_str());
			foundProtocolField = true;
		}
		else if(name == ETH_DST)
		{
			logger(ORCH_DEBUG, MODULE_NAME, __FILE__, __LINE__, "\"%s\"->\"%s\": \"%s\"",MATCH,ETH_DST,value.getString().c_str());
			if(!validateMac(value.getString().c_str()))
			{
				logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Key \"%s\" with wrong value \"%s\"",ETH_DST,value.getString().c_str());
				return false;
			}
			match.setEthDst((char*)value.getString().c_str());
			foundProtocolField = true;
		}
		else if(name == IP_SRC)
		{
			size_t found = value.getString().find(':');
			//IPv6
			if(found!=string::npos)
			{
				logger(ORCH_DEBUG, MODULE_NAME, __FILE__, __LINE__, "\"%s\"->\"%s\": \"%s\"",MATCH,IP_SRC,value.getString().c_str());
				if(!validateIpv6(value.getString()))
				{
					logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Key \"%s\" with wrong value \"%s\"",IP_SRC,value.getString().c_str());
					return false;
				}
				match.setIpv6Src((char*)value.getString().c_str());
				foundProtocolField = true;	
			//IPv4
			} else {
				logger(ORCH_DEBUG, MODULE_NAME, __FILE__, __LINE__, "\"%s\"->\"%s\": \"%s\"",MATCH,IP_SRC,value.getString().c_str());
				if(!validateIpv4(value.getString()))
				{
					logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Key \"%s\" with wrong value \"%s\"",IP_SRC,value.getString().c_str());
					return false;
				}
				match.setIpv4Src((char*)value.getString().c_str());		
				foundProtocolField = true;
			}
		}
		else if(name == IP_DST)
		{
			size_t found = value.getString().find(':');
			//IPv6
			if(found!=string::npos)
			{
				logger(ORCH_DEBUG, MODULE_NAME, __FILE__, __LINE__, "\"%s\"->\"%s\": \"%s\"",MATCH,IP_DST,value.getString().c_str());
				if(!validateIpv6(value.getString()))
				{
					logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Key \"%s\" with wrong value \"%s\"",IP_DST,value.getString().c_str());
					return false;
				}
				match.setIpv6Dst((char*)value.getString().c_str());
				foundProtocolField = true;
			} else {
				logger(ORCH_DEBUG, MODULE_NAME, __FILE__, __LINE__, "\"%s\"->\"%s\": \"%s\"",MATCH,IP_SRC,value.getString().c_str());
				if(!validateIpv4(value.getString()))
				{
					logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Key \"%s\" with wrong value \"%s\"",IP_SRC,value.getString().c_str());
					return false;
				}
				match.setIpv4Src((char*)value.getString().c_str());		
				foundProtocolField = true;
			}		
		}
		else if(name == TOS_BITS)
		{
			logger(ORCH_WARNING, MODULE_NAME, __FILE__, __LINE__, "\"%s\" \"%s\" not available",MATCH,TOS_BITS);

			logger(ORCH_DEBUG, MODULE_NAME, __FILE__, __LINE__, "\"%s\"->\"%s\": \"%s\"",MATCH,TOS_BITS,value.getString().c_str());

			//XXX: currently, this information is ignored	
		}
		else if(name == PORT_SRC)
		{
			//TCP
			if(is_tcp)
			{
				logger(ORCH_DEBUG, MODULE_NAME, __FILE__, __LINE__, "\"%s\"->\"%s\": \"%s\"",MATCH,PORT_SRC,value.getString().c_str());
				uint32_t tcpSrc;
				if((sscanf(value.getString().c_str(),"%"SCNd32,&tcpSrc) != 1) || (tcpSrc > 65535))
				{
					logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Key \"%s\" with wrong value \"%s\"",PORT_SRC,value.getString().c_str());
					return false;
				}
				match.setTcpSrc(tcpSrc & 0xFFFF);
				foundProtocolField = true;
			//UDP
			} else {
				logger(ORCH_DEBUG, MODULE_NAME, __FILE__, __LINE__, "\"%s\"->\"%s\": \"%s\"",MATCH,PORT_SRC,value.getString().c_str());
				uint32_t udpSrc;
				if((sscanf(value.getString().c_str(),"%"SCNd32,&udpSrc) != 1)  || (udpSrc > 65535))
				{
					logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Key \"%s\" with wrong value \"%s\"",PORT_SRC,value.getString().c_str());
					return false;
				}
				match.setUdpSrc(udpSrc & 0xFFFF);
				foundProtocolField = true;
			}
		}
		else if(name == PORT_DST)
		{
			//TCP
			if(is_tcp)
			{
				logger(ORCH_DEBUG, MODULE_NAME, __FILE__, __LINE__, "\"%s\"->\"%s\": \"%s\"",MATCH,PORT_DST,value.getString().c_str());
				uint32_t tcpDst;
				if((sscanf(value.getString().c_str(),"%"SCNd32,&tcpDst) != 1)  || (tcpDst > 65535))
				{
					logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Key \"%s\" with wrong value \"%s\"",PORT_DST,value.getString().c_str());
					return false;
				}
				match.setTcpDst(tcpDst & 0xFFFF);
				foundProtocolField = true;
			//UDP
			} else {
				logger(ORCH_DEBUG, MODULE_NAME, __FILE__, __LINE__, "\"%s\"->\"%s\": \"%s\"",MATCH,PORT_DST,value.getString().c_str());
				uint32_t udpDst;
				if((sscanf(value.getString().c_str(),"%"SCNd32,&udpDst) != 1)  || (udpDst > 65535))
				{
					logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Key \"%s\" with wrong value \"%s\"",PORT_DST,value.getString().c_str());
					return false;
				}
				match.setUdpDst(udpDst & 0xFFFF);
				foundProtocolField = true;
			}
		}
		else if(name == PROTOCOL)
		{
			if(value.getString().compare(TCP) == 0)
				is_tcp = true;
			else
				is_tcp = false;
		}
		else
		{
			logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Invalid key: %s",name.c_str());
			return false;
		}
	}
	
	if(!foundOne)
	{
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "Neither Key \"%s\", nor key \"%s\" found in \"%s\"",PORT,_ID,MATCH);
		return false;
	}
	
	if(foundProtocolField && foundEndPointID && definedInCurrentGraph)
	{
		logger(ORCH_DEBUG_INFO, MODULE_NAME, __FILE__, __LINE__, "A \"%s\" specifying an \"%s\" (defined in the current graph) and at least a protocol field was found. This is not supported.",MATCH,ENDPOINT);
		return false;
	}
	
	return true;
}


