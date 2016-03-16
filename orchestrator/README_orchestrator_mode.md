# Orchestrator in band and out of band

UN-orchestrator supports two operating modes that depend on how the UN connects with an external (upper) orchestrator.
In particular, the port that is used to send/receive commands (e.g., forwarding graphs) to the external orchestrator can be configured *out of band* or *in band*; this port is called *orchestration port* in the rest of this document.

![orchestrator_in_band_and_out_of_band](https://raw.githubusercontent.com/netgroup-polito/un-orchestrator/master/images/orchestrator_in_band_and_out_of_band.png)


### Out of band
In this case the orchestration port is not attached to the LSI-0, hence is not under the control of the UN local orchestrator, differently from other ports (e.g., the ones that connect to end users).
In this case all the IP traffic to/from the orchestrator is terminated on this port, without requiring the setup of any specific flowrule from the local orchestrator.

In this case the UN needs to be provided with the correct IP configuration, i.e., a valid IP address on the  orchestration port and the proper routing table. 


### In band
In this case the orchestration port is attached to the LSI-0 *and* the IP address is moved on the LSI-0.
Hence, all the IP traffic to/from the orchestrator has to be forwarded to the internal LSI-0 port by setting up the proper forwarding rules.
This allows the tenant traffic to cross the LSA-0 toward the right tenant LSA (e.g., LSI-N), while at the same time it supports the management of the UN through the same port.

In this case, the following additional rules are installed in the LSI-0: 


##### Note

Pay attention that physical ports attached to a virtual bridge should not have any IP address; the IP address should be moved to the internal port (often called "local") of the LSI-0 instead.
We experienced several mis-behaviours when physical ports with IP addresses are attached to a bridge, hence we definitely discourage this kind of configuration. 
