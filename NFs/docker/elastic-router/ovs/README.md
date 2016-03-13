## Docker-based example openvswitch

This file contains the instructions to build an example of Docker network function.

This is acontainer based on a ubuntu container with openvswitch installed

### Creating the Docker image

You can create the Docker image with the network function by launching the following command:

    sudo docker build --tag="ovs" .

In the name-resolver, different VNFs of this image are defines: `ovs1`, `ovs2`, `ovs3`, `ovs4`and `ovs5`.
These names are referenced in the nffg file.
This will create the Docker image starting from the base image specified in `Dockerfile`; the new image is stored in the Docker default folder on your filesystem (localhost).

The script start.sh is executed at startup of the container.
IP, mac, controller address configuration is imported from the nffg.

DP_ID is also specified and configured from the NFFG. All ovs containers have a datapath id starting with 99.
eg. `ovs1` has datapath id=9900000000000001


The control interface is out-of-band via a dedicated port in the NFFG.

Currently no multiple instances of a VNF are allowed on the UN.
So if multiple ovs containers are needed, they should be build with a different tag (eg. ovs2, ovs3, ...) and deployed as such via the name-resolver/nffg file.


### SSH access
The deployed containers expose a SSH login port to the public ip of the UN where they are deployed.
Port 900# is forwarded to port 22 inside the ovs container. This port mapping is defined in the nffg.
eg. `ovs1` container has ssh login via port 9001 on the UN public ip address.

SSH login can be done via `ssh root@localhost -p 9001` (to reach ovs1 container via exposed port 9001)
SSH login credentials for the VNF containers:
* user: root
* password: root


To avoid password entering, a public key can be added to the `authorized_keys` file.
The fingerprint of the container changes on each build. SSH login complains about this because the fingerprint stored in  .ssh/known_hosts has changed after a new build. (This can be a man-in-the-middle attack).

To avoid this error, the  fingerprint of the container can be removed from .ssh/known_hosts or this command can be used to login via ssh: `ssh -o StrictHostKeyChecking=no root@localhost -p 9001`
