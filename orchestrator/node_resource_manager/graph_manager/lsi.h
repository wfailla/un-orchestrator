#ifndef LSI_H_
#define LSI_H_ 1

#pragma once

#include "virtual_link.h"
#include "../../compute_controller/description.h"

#include <map>
#include <set>
#include <list>
#include <string>
#include <vector>
#include <sstream>

using namespace std;

class VLink;

class LSI
{
//XXX: this class is a mess!
friend class GraphManager;

private:

	/**
	*	@brief: this is the address of the OF controller for this LSI
	*/
	string controllerAddress;
	
	/**
	*	@brief: this is the port used by the OF controller for the LSI
	*/
	string controllerPort;
	
	/**
	*	@brief: data plane identifier
	*/
	uint64_t dpid;
	
	/**
	*	@brief: the pair is <port name, port id>. It contains physical ports
	*/
	map<string,unsigned int> physical_ports;
	
	/**
	*	@brief: the pair is <port name, port type>
	*/
	map<string,string> ports_type;

	/**
	*	@brief: Data related to a specific NF
	*/
	struct nfData {
		/**
		*	@brief: Types of the NF ports.
		*		The map is
		*			<nf port name, port_type>
		*/
		map<string, PortType> ports_type;

		/**
		*	@brief: Names of the ports connected to the LSI and related to the network function
		*/
	    list<string> portsNameOnSwitch;

		/**
		 * 	@brief: port ids assigned to the ports by switch
		 *		The map is
		 *			<nf port name, switch_id>
		 */
		map<string, unsigned int> ports_switch_id;

		/**
		 * 	@brief: port ids as they are designated in the NF-FG and NF description
		 */
		list<unsigned int> nf_ports_id;
	};
	
	/**
	*	@brief: endpoints connected to the LSI 
	*		The map is
	*  			<endpoint name, list params>
	*				list params: gre key, local ip, remote ip
	*/
	map<string,list<string> > endpoints_ports;
	
	/**
	*	@brief: the pair is <endpoint name, endpoint id>
	*/
	map<string,unsigned int > endpoints_ports_id;
	
	/**
	*	@brief: NFs connected to the LSI.
	*		The map is
	*			<nf name, nfData >
	*/
	map<string, struct nfData>  network_functions;
	
	/**
	*	@brief: virtual links attached to the LSI
	*	FIXME although supported in this class VLink, the code does not support vlinks connected to multiple LSIs
	*/
	vector<VLink> virtual_links;

	/**
	*	@brief: the map is <nf name, vlink id>
	*		A NF port generates a vlink if it is defined in the action part of a rule
	*/
	map<string, uint64_t> nfs_vlinks;

	/**
	*	@brief: the map is <port name, vlink id>
	*		A physical port generates a vlink if it is defined in the action part of a rule
	*/
	map<string, uint64_t> ports_vlinks;

	/**
	*	@brief: the map is <endpoint name, vlink id>
	*		An endpoint generates a vlink if it is defined in the action part of a rule
	*/
	map<string, uint64_t> endpoints_vlinks;

public:

	LSI(string controllerAddress, string controllerPort, map<string,string> ports, map<string, list <unsigned int> > network_functions, map<string,list<string> > endpoints_ports, vector<VLink> virtual_links,map<string,nf_t>  nf_types);

	string getControllerAddress();
	string getControllerPort();

	list<string> getPhysicalPortsName();

	set<string> getNetworkFunctionsName();
	list<string> getNetworkFunctionsPortNames(string nf);
	
	map<string,list<string> > getEndpointsPorts();

	map<string,unsigned int > getEndpointsPortsId();

	list<uint64_t> getVirtualLinksRemoteLSI();

	uint64_t getDpid();

	list<string> getPhysicalPortsName();
	map<string,string> getPhysicalPortsType();
	map<string,unsigned int> getPhysicalPorts();

	set<string> getNetworkFunctionsName();
	map<string,unsigned int> getNetworkFunctionsPorts(string nf);
	list<string> getNetworkFunctionsPortNames(string nf);
	PortType getNetworkFunctionPortType(string nf, string port);
	map<string, list< struct nf_port_info> > getNetworkFunctionsPortsInfo();
	list<string> getNetworkFunctionsPortsNameOnSwitch(string nf);
	map<unsigned int, string> getNetworkFunctionsPortsNameOnSwitchMap(string nf);

	list<uint64_t> getVirtualLinksRemoteLSI();
	vector<VLink> getVirtualLinks();
	VLink getVirtualLink(uint64_t ID);
	map<string, uint64_t> getNFsVlinks();
	map<string, uint64_t> getPortsVlinks();
	map<string, uint64_t> getEndPointsVlinks();

	//FIXME: public is not a good choice
	void setNFsVLinks(map<string, uint64_t> nfs_vlinks);
	void addNFvlink(string NF, uint64_t vlinkID);
	void removeNFvlink(string nf_port);

	void setPortsVLinks(map<string, uint64_t> ports_vlinks);
	void addPortvlink(string port, uint64_t vlinkID);
	void removePortvlink(string port);

	void setEndPointsVLinks(map<string, uint64_t> endpoints_vlinks);
	void addEndpointvlink(string endpoint, uint64_t vlinkID);
	void removeEndPointvlink(string endpoint);

protected:
	void setDpid(uint64_t dpid);
	bool setPhysicalPortID(string port, uint64_t id);
	bool setNfSwitchPortsID(string nf, map<string, unsigned int>);
	void setVLinkIDs(unsigned int position, unsigned int localID, unsigned int remoteID);
	bool setEndpointPortID(string ep, uint64_t id);
	
	void setNetworkFunctionsPortsNameOnSwitch(string nf, list<string> names);

	int addVlink(VLink vlink);
	void removeVlink(uint64_t ID);

	bool addNF(string name, list< unsigned int> ports, const map<unsigned int, PortType>& nf_ports_type);
	void removeNF(string nf);
	
	void addEndpoint(string name, list<string> param);
	void removeEndpoint(string ep);
};

#endif //LSI_H_
