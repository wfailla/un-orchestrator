This is a simple network function, written in C, to be run in a Docker container.

When a packet is received on the interface eth0, its destination MAC address is 
changed, and the packet is sent back on the interface eth1

===============================================================================

Create the Docker image which contains this network function:

docker build --tag="localhost:5000/example" .

===============================================================================

* If you are going to use this network function together with the un-orchestrator, 
you can skip this part *

Run the container:

* in interactive mode

	sudo docker run -i -t --lxc-conf="lxc.network.type=phys" --lxc-conf="lxc.network.link=vEth0" \
		--lxc-conf="lxc.network.name=eth0" --lxc-conf="lxc.network.flags=up" \
		--lxc-conf="lxc.network.type=phys" --lxc-conf="lxc.network.link=vEth1" \
		--lxc-conf="lxc.network.name=eth1" --lxc-conf="lxc.network.flags=up" --net="none" \  
		--privileged="true" localhost:5000/example /bin/bash

	./example

* in background mode

	sudo docker run -d --lxc-conf="lxc.network.type=phys" --lxc-conf="lxc.network.link=vEth0" \
		--lxc-conf="lxc.network.name=eth0" --lxc-conf="lxc.network.flags=up" \
		--lxc-conf="lxc.network.type=phys" --lxc-conf="lxc.network.link=vEth1" \
		--lxc-conf="lxc.network.name=eth1" --lxc-conf="lxc.network.flags=up" --net="none" \  
		--privileged="true" localhost:5000/example
		
In both the cases, vEth0 and vEth1 are vEth interfaces available in the host.
