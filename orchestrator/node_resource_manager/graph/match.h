#ifndef MATCH_H_
#define MATCH_H_ 1

#pragma once

#include <inttypes.h>

#include <rofl/platform/unix/cunixenv.h>
#include <rofl/platform/unix/cdaemon.h>
#include <rofl/common/cparams.h>

#include <rofl/common/ciosrv.h>
#include <rofl/common/crofbase.h>
#include <rofl/common/crofdpt.h>
#include <rofl/common/logging.h>

#include "../../utils/logger.h"
#include "../../utils/constants.h"

#include <iostream>

#include <json_spirit/json_spirit.h>
#include <json_spirit/value.h>
#include <json_spirit/writer.h>

using namespace json_spirit;
using namespace rofl;
using namespace std;

namespace graph
{

class Match
{

protected:
	
	/*
	*	Ethernet
	*/
	char *eth_src;
	char *eth_dst;
	bool isEthType;
	uint16_t ethType;
	
	/*
	*	VLAN
	*/
	bool isVlanID;
	uint16_t vlanID;
	bool isNoVlan;
	bool isAnyVlan;
	bool isEndpointVlanID;
	
	/*
	*	IPv4
	*/
	char *ipv4_src;
	char *ipv4_dst;
	
	/*
	*	TCP
	*/
	bool isTcpSrc;
	uint16_t tcp_src;
	bool isTcpDst;
	uint16_t tcp_dst;

	/*
	*	UDP
	*/
	bool isUdpSrc;
	uint16_t udp_src;
	bool isUdpDst;
	uint16_t udp_dst;
	
	/*
	*	IPv6
	*/
	char *ipv6_src;
	char *ipv6_dst;
	
	/*
	*	GRE
	*/
	char *gre_key;
	
	Match();

	bool isEqual(const Match &other) const;

public:

	void setEthSrc(char *eth_src);
	void setEthDst(char *eth_dst);
	void setEthType(uint16_t ethType);
	void setVlanID(uint16_t vlanID);
	void setVlanIDNoVlan();
	void setVlanIDAnyVlan();
	void setEndpointVlanID(uint16_t vlanID);
	void setIpv4Src(char *ipv4_src);
	void setIpv4Dst(char *ipv4_dst);
	void setTcpSrc(uint16_t tcp_src);
	void setTcpDst(uint16_t tcp_dst);
	void setUdpSrc(uint16_t udp_src);
	void setUdpDst(uint16_t udp_dst);
	void setIpv6Src(char *ipv6_src);
	void setIpv6Dst(char *ipv6_dst);
	void setGreKey(char *gre_key);
	
	virtual void setAllCommonFields(Match match);
	
	virtual void print();
	virtual string prettyPrint();
	virtual void toJSON(Object &match);
};

}

#endif //MATCH_H_
