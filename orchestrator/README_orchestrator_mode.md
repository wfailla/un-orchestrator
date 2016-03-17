# Orchestrator in band and out of band

UN-orchestrator supports two operating modes that depend on how the UN connects with an external (upper) orchestrator.
In particular, the port that is used to send/receive commands (e.g., NF-FG) to/from the external orchestrator can be configured *out of band* or *in band*; this port is called *orchestration port* in the rest of this document.

![orchestrator_in_band_and_out_of_band](https://raw.githubusercontent.com/netgroup-polito/un-orchestrator/master/images/orchestrator_in_band_and_out_of_band.png)


## Out of band
In this case the orchestration port is not attached to the LSI-0 then, unlike the other ports (e.g., the ones that connect to end users), it is not under the control of the un-orchestrator.
In this case all the IP traffic to/from the external orchestrator is terminated on this port, without requiring the setup of any specific flowrule from the un-orchestrator.

In this case the UN needs to be provided with the correct IP configuration, i.e., a valid IP address on the orchestration port and the proper routing table. 


## In band
In this case the orchestration port is attached to the LSI-0 *and* the IP address is moved on the LSI-0.
Hence, all the IP traffic to/from the external orchestrator has to be forwarded to the internal LSI-0 port by setting up the proper forwarding rules.
This allows the tenant traffic to cross the LSI-0 toward the right tenant LSI (e.g., LSI-N), while at the same time it supports the management of the UN through the same port.

In this case, the following additional rules are installed in the LSI-0:

* match: arp, arp_tpa=10.0.0.1, in_port=1 --- action: local 
* match: ip, in_port=1, nw_dst=10.0.0.1 --- action: local
* match: in_port=0 --- action: out_port=1

where users are attached to port 0 (eth0 in the figure above)  and Internet is reachable through port 1 (eth1).

First rule allow local orchestrator to receive ARP query packets addressed to him, and then, after the reply, become reachable from the extern. By means of the second, incoming traffic directed to local orchestrator is captured, hence he is able to receive control commands, while last rule make local orchestrator possible to response. 

#### Note

Pay attention that physical ports attached to the LSI-0 (and hence under the control of the un-orchestrator) should not have any IP address.
In case of *in-band* orchestration, the IP address should be moved to the internal port (often called "local") of the LSI-0 instead.

We experienced several mis-behaviours when physical ports with IP addresses are attached to a bridge, hence we definitely discourage this kind of configuration. 
