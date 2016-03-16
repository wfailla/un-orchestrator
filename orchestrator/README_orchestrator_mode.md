# Orchestrator in band and out of band

UN-orchestrator supports two operative modes depending on the position of
the orchestration port:

* Out of band
* In band

![orchestrator_in_band_and_out_of_band](https://raw.githubusercontent.com/netgroup-polito/un-orchestrator/master/images/orchestrator_in_band_and_out_of_band.png)

### Out of band
User ports are attached to LSI-0, orchestration port is not seen by un-orchestrator
and is used to manage the universal node, for example receive the forwarding graph.

### In band:
User ports are attached to LSI-0, orchestration port is directly assigned to virtual
bridge as local port: such configuration allow the passage of users' traffic
and the management of the universal node through the same port.

##### Note:

Pay attention that physical ports with an ip address attached to virtual bridge
don't work well, so ports connected to LSI-0 must have no ip address.
