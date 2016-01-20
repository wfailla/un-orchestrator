## Docker-based example openvswitch

This file contains the instructions to build an example of Docker network function.

This is acontainer based on a ubuntu container with openvswitch installed

### Creating the Docker image

You can create the Docker image with the network function by launching the following command:

    sudo docker build --tag="ovs1" .

The name 'ovs1' must be used for alignment with the name-resolver and nffg file.
This will create the Docker image starting from the base image specified in `Dockerfile`; the new image is stored in the Docker default folder on your filesystem (localhost).

The script start.sh is executed at startup and sets an ip addres (10.0.10.1/24) to a control interface.
This ip is hard coded for now, it should eventually be imported from the nffg.

Currently no multiple instances of a VNF are allowed on the UN.
So if multiple ovs containers are needed, they should be buiuld with a different tag (eg. ovs2, ovs3, ...) and deployed as such via the name-resolver/nffg file.
