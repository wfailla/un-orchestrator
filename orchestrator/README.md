# Universal Node Orchestrator

The Universal Node orchestrator (un-orchestrator) is the main component of the 
Universal Node (UN). It handles the orchestration of compute and network
resources within a UN, hence managing the complete lifecycle of computing
containers (e.g., VMs, Docker, DPDK processes) and networking primitives 
(e.g., OpenFlow rules, logical switching instances, etc).
It receives commands through a REST API according to the Network Functions 
Forwarding Graph (NF-FG) formalism, and takes care of implementing them on 
the physical node. 

More in detail, when it receives a command to deploy a new NF-FG, it does all
the operations required to actually implement the forwarding graph: 

  * retrieve the most appropriate image for the selected network function;
  * configure the virtual switch (vSwitch) to create a new LSI and the ports 
    required to connect it to the network functions to be deployed;
  * deploy and starts the network functions;
  * translate the rules to steer the traffic into OpelFlow flowmod messages 
    to be sent to the vSwitch (some flowmod are sent to the new LSI, others 
    to the LSI-0, i.e. an LSI that steers the traffic into the proper graph.).

Similarly, the un-orchestrator takes care of updating or destroying a graph,
when the proper messages are received.


### The network controller

The network controller is the sub-module that interacts with the vSwitch.
It consists of two parts:

  * the Openflow controller(s): a new Openflow controller is created for each
    new LSI, which is used to steer the traffic among the ports of the LSI
    itself;
  * the switch manager: it is used to create/destroy LSI, create/destroy 
    virtual ports, and so on. In practice, it allows the un-orchestrator to
    interact with the vSwitch in order to perform management operations. Each
    virtual switch implementation (e.g., xDPd, OvS) may require a different
    implementation for the switch manager, according to the commands
    supported by the vSwitch itself.

If you are interested to add the support for a new virtual switch, please 
check the file network\_controller/switch\_manager/README.


### The compute controller

The compute controller is the sub-module that interacts with the hypervisor
and handles the lifecycle of a virtual network function (i.e., creating,
updating, destroying a VNF), including the operations needed to attach
those ports to the running vSwitch.

Currently it supports network functions as (KVM) VMs, Docker and DPDK 
processes, although only a subset of them can be available depending on 
the chosen vSwitch.


### NF-FG

The un-orchestrator supports two NF-FG versions:

  * the initial JSON-based format defined in WP5 and used in the initial
    part of the project;
  * the new XML-based format defined in WP3 that includes both top-down
    communication (for the actual forwarding graph) and bottom-up primitives
    (for resources and capabilities).

The former format is supported natively, while the other requires setting
up an additional library as described in README_COMPILE.md#nf-fg-library.


### Compile and run

Some additional files are provided to compile and use the un-orchestrator:

  * README_COMPILE.md: to compile the un-orchestrator
  * README_RUN.md: to start the un-orchestrator
  * README_RESTAPI.md: some usage examples about the REST interface of
    the the un-orchestrator
