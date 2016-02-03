#include "low_level_match.h"

namespace lowlevel
{

Match::Match() :
	graph::Match(),isInput_port(false),is_local_port(false)
{

}

Match::Match(bool is_local_port) :
	graph::Match(),isInput_port(false), is_local_port(is_local_port)
{

}

bool Match::operator==(const Match &other) const
{
	logger(ORCH_DEBUG, MODULE_NAME, __FILE__, __LINE__, "Matches to be compared:");
	logger(ORCH_DEBUG, MODULE_NAME, __FILE__, __LINE__, "\tisInputPort %s vs %s ",(isInput_port)?"yes":"no",(other.isInput_port)?"yes":"no");
	logger(ORCH_DEBUG, MODULE_NAME, __FILE__, __LINE__, "\tinput port: %d vs %d",input_port,other.input_port);

	if((isInput_port && !other.isInput_port) ||
		(!isInput_port && other.isInput_port) )
		return false;
		
	if(input_port != other.input_port)
		return false;

	logger(ORCH_DEBUG, MODULE_NAME, __FILE__, __LINE__, "Comparing other fields...");

	return this->isEqual(other);
}

void Match::fillFlowmodMessage(rofl::openflow::cofflowmod &message)
{
	if(isInput_port)
		message.set_match().set_in_port(input_port);
	if(is_local_port)
		message.set_match().set_in_port(rofl::openflow::OFPP_LOCAL);
		
	if(eth_src != NULL)
	{
		message.set_match().set_eth_src(cmacaddr(eth_src));
	}
		
	if(eth_dst != NULL)
	{
		message.set_match().set_eth_dst(cmacaddr(eth_dst));
	}
	
	if(isEthType)
		message.set_match().set_eth_type(ethType);
	if(isVlanID || isEndpointVlanID)
	{
		message.set_match().set_vlan_vid(vlanID);
	}
	else if(isAnyVlan)
		message.set_match().set_vlan_present();
	else if(isNoVlan)
		message.set_match().set_vlan_untagged();
	if(ipv4_src)
	{
		message.set_match().set_ipv4_src(caddress_in4(ipv4_src));
	}
	if(ipv4_dst)
	{
		message.set_match().set_ipv4_dst(caddress_in4(ipv4_dst));
	}
	if(isTcpSrc)
		message.set_match().set_tcp_src(tcp_src);
	if(isTcpDst)
		message.set_match().set_tcp_dst(tcp_dst);
	if(isUdpSrc)
		message.set_match().set_udp_src(udp_src);
	if(isUdpDst)
		message.set_match().set_udp_dst(udp_dst);	
	if(ipv6_src)
	{
		message.set_match().set_ipv6_src(caddress_in6(ipv6_src));
	}
	if(ipv6_dst)
	{
		message.set_match().set_ipv6_dst(caddress_in6(ipv6_dst));
	}
	
	if(gre_key)
	{
		unsigned int key = 0;
		
		sscanf(gre_key, "%u", &key);
		message.set_match().set_tunnel_id(key);
	}
}

void Match::setInputPort(unsigned int input_port)
{
	this->input_port = input_port;
	isInput_port = true;
}

void Match::setAllCommonFields(graph::Match match)
{
	graph::Match::setAllCommonFields(match);
}

void Match::print()
{
	if(LOGGING_LEVEL <= ORCH_DEBUG_INFO)
	{
		cout << "\t\tmatch:" << endl << "\t\t{" << endl;
		if(is_local_port)
			cout << "\t\t\tport: "<< "LOCAL" << endl;
		else
			cout << "\t\t\tport: "<< input_port << endl;
		graph::Match::print();
		cout << "\t\t}" << endl;
	}
}

string Match::prettyPrint(LSI *lsi0,map<string,LSI *> lsis)
{
	stringstream ss;

	ss << "port: ";

	map<string,unsigned int> pysicalPorts = lsi0->getPhysicalPorts();
	for(map<string,unsigned int>::iterator it = pysicalPorts.begin(); it != pysicalPorts.end(); it++)
	{
		if(it->second == input_port)
		{
			ss << it->first << graph::Match::prettyPrint();
			return ss.str();
		}		
	}
	
	//The port corresponds to a virtual link... we search the corresponding graph
	
	for(map<string,LSI *>::iterator it = lsis.begin(); it != lsis.end(); it++)
	{
		vector<VLink> vlinks = it->second->getVirtualLinks();
		for(vector<VLink>::iterator vl = vlinks.begin(); vl != vlinks.end(); vl++)
		{
			if(vl->getRemoteID() == input_port)
			{
				ss << input_port << " (graph: " << it->first << ")";
				return ss.str();	
			}
		}
	}
	
	if(is_local_port)
		ss << "LOCAL" << " (LOCAL graph)";
	else
	{	
		//The code could be here only when a SIGINT is received and all the graph are going to be removed
		ss << input_port << " (unknown graph)";
	}
	
	return ss.str();
}

}
