#include "match.h"

namespace graph
{

Match::Match() :
	eth_src(NULL), eth_dst(NULL), isEthType(false),
	isVlanID(false),isNoVlan(false),isAnyVlan(false),isEndpointVlanID(false),
	ipv4_src(NULL),
	ipv4_dst(NULL),
	isTcpSrc(false), isTcpDst(false),
	isUdpSrc(false), isUdpDst(false),
	ipv6_src(NULL), ipv6_dst(NULL)
{

}

bool Match::isEqual(const Match &other) const
{
	/*
	*	Ethernet
	*/
	if((eth_src == NULL && other.eth_src != NULL) ||
		(eth_src != NULL && other.eth_src == NULL))
		return false;
		
	if(eth_src != NULL && other.eth_src != NULL)
	{
		if(strcmp(eth_src,other.eth_src) != 0)
			return false;
	}
	
	if((eth_dst == NULL && other.eth_dst != NULL) ||
		(eth_dst != NULL && other.eth_dst == NULL))
		return false;
		
	if(eth_dst != NULL && other.eth_dst != NULL)
	{
		if(strcmp(eth_dst,other.eth_dst) != 0)
			return false;
	}
	
	if((isEthType && !other.isEthType) || (!isEthType && other.isEthType))
		return false;
	if(isEthType && ethType != other.ethType)
		return false;
	
	/*
	*	VLAN
	*/
	if((isVlanID && !other.isVlanID) || (!isVlanID && other.isVlanID))
		return false;
	if(isVlanID && vlanID != other.vlanID)
		return false;
		
	if((isNoVlan && !other.isNoVlan) || (!isNoVlan && other.isNoVlan))
		return false;
		
	if((isAnyVlan && !other.isAnyVlan) || (!isAnyVlan && other.isAnyVlan))
		return false;
		
	/*
	*	IPv4
	*/
	if((ipv4_src == NULL && other.ipv4_src != NULL) ||
		(ipv4_src != NULL && other.ipv4_src == NULL))
		return false;
		
	if(ipv4_src != NULL && other.ipv4_src != NULL)
	{
		if(strcmp(ipv4_src,other.ipv4_src) != 0)
			return false;
	}
	
	if((ipv4_dst == NULL && other.ipv4_dst != NULL) ||
		(ipv4_dst != NULL && other.ipv4_dst == NULL))
		return false;
		
	if(ipv4_dst != NULL && other.ipv4_dst != NULL)
	{
		if(strcmp(ipv4_dst,other.ipv4_dst) != 0)
			return false;
	}
	
	/*
	*	TCP
	*/
	if((isTcpSrc && !other.isTcpSrc) || (!isTcpSrc && other.isTcpSrc))
		return false;
	if(isTcpSrc && tcp_src != other.tcp_src)
		return false;
	
	if((isTcpDst && !other.isTcpDst) || (!isTcpDst && other.isTcpDst))
		return false;
	if(isTcpDst && tcp_dst != other.tcp_dst)
		return false;

	/*
	*	UDP
	*/
	if((isUdpSrc && !other.isUdpSrc) || (!isUdpSrc && other.isUdpSrc))
		return false;
	if(isUdpSrc && udp_src != other.udp_src)
		return false;
	
	if((isUdpDst && !other.isUdpDst) || (!isUdpDst && other.isUdpDst))
		return false;
	if(isUdpDst && udp_dst != other.udp_dst)
		return false;

	/*
	*	IPv6
	*/
	if((ipv6_src == NULL && other.ipv6_src != NULL) ||
		(ipv6_src != NULL && other.ipv6_src == NULL))
		return false;
		
	if(ipv6_src != NULL && other.ipv6_src != NULL)
	{
		if(strcmp(ipv6_src,other.ipv6_src) != 0)
			return false;
	}
	
	if((ipv6_dst == NULL && other.ipv6_dst != NULL) ||
		(ipv6_dst != NULL && other.ipv6_dst == NULL))
		return false;
		
	if(ipv6_dst != NULL && other.ipv6_dst != NULL)
	{
		if(strcmp(ipv6_dst,other.ipv6_dst) != 0)
			return false;
	}

	return true;
}


void Match::setAllCommonFields(Match match)
{
	/*
	*	Ethernet
	*/
	if(match.eth_src != NULL)
		setEthSrc(match.eth_src);
	if(match.eth_dst != NULL)
		setEthDst(match.eth_dst);
	if(match.isEthType)
		setEthType(match.ethType);
	
	/*
	*	VLAN
	*/
	if(match.isVlanID || match.isEndpointVlanID)
		setVlanID(match.vlanID);
	else if(match.isAnyVlan)
		setVlanIDAnyVlan();
	else if(match.isNoVlan)
		setVlanIDNoVlan();	

	/*
	*	IPv4
	*/
	if(match.ipv4_src)
		setIpv4Src(match.ipv4_src);
	if(match.ipv4_dst)
		setIpv4Dst(match.ipv4_dst);
	
	/*
	*	TCP
	*/
	if(match.isTcpSrc)
		setTcpSrc(match.tcp_src);
	if(match.isTcpDst)
		setTcpDst(match.tcp_dst);

	/*
	*	UDP
	*/
	if(match.isUdpSrc)
		setUdpSrc(match.udp_src);
	if(match.isUdpDst)
		setUdpDst(match.udp_dst);
	
	/*
	*	IPv6
	*/
	if(match.ipv6_src)
		setIpv6Src(match.ipv6_src);
	if(match.ipv6_dst)
		setIpv6Dst(match.ipv6_dst);
}

void Match::setEthSrc(char *eth_src)
{
	this->eth_src = (char*)malloc(sizeof(char)*(strlen(eth_src)+1));
	strcpy(this->eth_src,eth_src);
}

void Match::setEthDst(char *eth_dst)
{
	this->eth_dst = (char*)malloc(sizeof(char)*(strlen(eth_dst)+1));
	strcpy(this->eth_dst,eth_dst);
}

void Match::setEthType(uint16_t ethType)
{
	this->ethType = ethType;
	isEthType = true;
}

void Match::setVlanID(uint16_t vlanID)
{
	this->vlanID = vlanID;
	isVlanID = true;
}

void Match::setVlanIDNoVlan()
{
	isVlanID = false;
	isAnyVlan = false;
	
	isNoVlan = true;
}
	
void Match::setVlanIDAnyVlan()
{
	isVlanID = false;	
	isNoVlan = false;
	
	isAnyVlan = true;
}

void Match::setEndpointVlanID(uint16_t vlanID)
{
	this->vlanID = vlanID;
	isEndpointVlanID = true;
}

void Match::setIpv4Src(char *ipv4_src)
{
	this->ipv4_src = (char*)malloc(sizeof(char)*(strlen(ipv4_src)+1));
	strcpy(this->ipv4_src,ipv4_src);
}

void Match::setIpv4Dst(char *ipv4_dst)
{
	this->ipv4_dst = (char*)malloc(sizeof(char)*(strlen(ipv4_dst)+1));
	strcpy(this->ipv4_dst,ipv4_dst);
}

void Match::setTcpSrc(uint16_t tcp_src)
{
	this->tcp_src = tcp_src;
	isTcpSrc = true;
}

void Match::setTcpDst(uint16_t tcp_dst)
{
	this->tcp_dst = tcp_dst;
	isTcpDst = true;
}

void Match::setUdpSrc(uint16_t udp_src)
{
	this->udp_src = udp_src;
	isUdpSrc = true;
}

void Match::setUdpDst(uint16_t udp_dst)
{
	this->udp_dst = udp_dst;
	isUdpDst = true;
}

void Match::setIpv6Src(char *ipv6_src)
{
	this->ipv6_src = (char*)malloc(sizeof(char)*(strlen(ipv6_src)+1));
	strcpy(this->ipv6_src,ipv6_src);
}

void Match::setIpv6Dst(char *ipv6_dst)
{
	this->ipv6_dst = (char*)malloc(sizeof(char)*(strlen(ipv6_dst)+1));
	strcpy(this->ipv6_dst,ipv6_dst);
}

void Match::print()
{
	if(LOGGING_LEVEL <= ORCH_DEBUG_INFO)
	{
		/*
		*	Ethernet
		*/
		if(eth_src != NULL)
			cout << "\t\t\tethernet src: " << eth_src << endl;
		if(eth_dst != NULL)
			cout << "\t\t\tethernet dst: " << eth_dst << endl;
		if(isEthType)
			cout << "\t\t\tethertype: " <<  "0x" << hex << ethType << endl;
	
		/*
		*	VLAN
		*/
		if(isVlanID || isEndpointVlanID)
			cout << "\t\t\tVLAN ID: " << hex << "0x" << vlanID << endl;
		else if(isAnyVlan)
			cout << "\t\t\tVLAN ID: ANY" << endl;
		else if(isNoVlan)
			cout << "\t\t\tNO VLAN" << endl;
	
		/*
		*	IPv4
		*/
		if(ipv4_src)
			cout << "\t\t\tIPv4 src: " << ipv4_src << endl;
		if(ipv4_dst)
			cout << "\t\t\tIPv4 dst: " << ipv4_dst << endl;

		/*
		*	TCP
		*/
		if(isTcpSrc)
			cout << "\t\t\tTCP src port: " << tcp_src << endl;
		if(isTcpDst)
			cout << "\t\t\tTCP dst port: " << tcp_dst << endl;

		/*
		*	UDP
		*/
		if(isUdpSrc)
			cout << "\t\t\tUDP src port: " << udp_src << endl;
		if(isUdpDst)
			cout << "\t\t\tUDP dst port: " << udp_dst << endl;
	
		/*
		*	IPv6
		*/
		if(ipv6_src)
			cout << "\t\t\tIPv6 src: " << ipv6_src << endl;
		if(ipv6_dst)
			cout << "\t\t\tIPv6 dst: " << ipv6_dst << endl;
	}
}

void Match::toJSON(Object &match)
{
		/*
		*	Ethernet
		*/
		if(eth_src != NULL)
			match[ETH_SRC] = eth_src;
		if(eth_dst)
			match[ETH_DST] = eth_dst;
		if(isEthType)
		{
			stringstream ethtype;
			ethtype << hex << ethType;
			match[ETH_TYPE] = ethtype.str().c_str();
		}
		
		/*
		*	VLAN
		*/
		if(isVlanID)
		{
			stringstream vlanid;
			vlanid << dec << vlanID;
			match[VLAN_ID] = vlanid.str().c_str();
		}
		else if(isAnyVlan)
			match[VLAN_ID] = ANY_VLAN;
		else if(isNoVlan)
			match[VLAN_ID] = NO_VLAN;
	
		/*
		*	IPv4
		*/
		if(ipv4_src)
			match[IP_SRC] =  ipv4_src;
		if(ipv4_dst)
			match[IP_DST] =  ipv4_dst;

		/*
		*	TCP
		*/
		if(isTcpSrc)
		{
			stringstream tcpsrc;
			tcpsrc << tcp_src;
			match[PORT_SRC] = tcpsrc.str().c_str();
		}
		if(isTcpDst)
		{
			stringstream tcpdst;
			tcpdst << tcp_dst;
			match[PORT_DST] = tcpdst.str().c_str();
		}

		/*
		*	UDP
		*/
		if(isUdpSrc)
		{
			stringstream udpsrc;
			udpsrc << udp_src;
			match[PORT_SRC] = udpsrc.str().c_str();
		}
		if(isUdpDst)
		{
			stringstream udpdst;
			udpdst << udp_dst;
			match[PORT_DST] = udpdst.str().c_str();
		}
	
		/*
		*	IPv6
		*/
		if(ipv6_src)
			match[IP_SRC] = ipv6_src;
		if(ipv6_dst)
			match[IP_DST] = ipv6_dst;
}


string Match::prettyPrint()
{
	stringstream ss;
	ss << "";

	/*
	*	Ethernet
	*/	
	if(eth_src != NULL)
		ss << " # ethernet src: " << eth_src;
	if(eth_dst != NULL)
		ss << " # ethernet dst: " << eth_dst;
	if(isEthType)
		ss << " # ethertype: " <<  "0x" << hex << ethType;

	/*
	*	VLAN
	*/
	if(isVlanID)
		ss << " # VLAN ID: " << hex << "0x" << vlanID;
	else if(isAnyVlan)
		ss << " # VLAN ID: ANY";
	else if(isNoVlan)
		ss << " # NO VLAN";

	/*
	*	IPv4
	*/
	if(ipv4_src)
		ss << " # IPv4 src: " << ipv4_src;
	if(ipv4_dst)
		ss << " # IPv4 dst: " << ipv4_dst;

	/*
	*	TCP
	*/
	if(isTcpSrc)
		ss << " # TCP src port: " << tcp_src;
	if(isTcpDst)
		ss << " # TCP dst port: " << tcp_dst;

	/*
	*	UDP
	*/
	if(isUdpSrc)
		ss << " # UDP src port: " << udp_src;
	if(isUdpDst)
		ss << " # UDP dst port: " << udp_dst;

	/*
	*	IPv6
	*/
	if(ipv6_src)
		ss << " # IPv6 src: " << ipv6_src;
	if(ipv6_dst)
		ss << " # IPv6 dst: " << ipv6_dst;

	return ss.str();
}

}
