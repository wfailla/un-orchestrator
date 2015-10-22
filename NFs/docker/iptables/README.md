This network function is a firewall based on iptables.

To change the configuration of the firewall, edit the file start.sh

===============================================================================

Create the Docker image which contains this network function:

docker build --tag="localhost:5000/iptables" .

===============================================================================

* If you are going to use this network function together with the un-orchestrator, 
you can skip this part *

* in interactive mode

	sudo docker run -i -t --lxc-conf="lxc.network.type=phys" --lxc-conf="lxc.network.link=vEth0" \
		--lxc-conf="lxc.network.name=eth0" --lxc-conf="lxc.network.flags=up" \
		--lxc-conf="lxc.network.type=phys" --lxc-conf="lxc.network.link=vEth1" \
		--lxc-conf="lxc.network.name=eth1" --lxc-conf="lxc.network.flags=up" --net="none" \
		--privileged="true" localhost:5000/iptables /bin/bash

	./start.sh

* in background mode

	sudo docker run -d --lxc-conf="lxc.network.type=phys" --lxc-conf="lxc.network.link=vEth0" \
		--lxc-conf="lxc.network.name=eth0" --lxc-conf="lxc.network.flags=up" \
		--lxc-conf="lxc.network.type=phys" --lxc-conf="lxc.network.link=vEth1" \
		--lxc-conf="lxc.network.name=eth1" --lxc-conf="lxc.network.flags=up" --net="none" \  
		--privileged="true" localhost:5000/iptables
		
In both the cases, vEth0 and vEth1 are vEth interfaces available in the host.

