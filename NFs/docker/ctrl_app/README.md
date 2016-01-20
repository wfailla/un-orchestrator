## Docker-based example ryu openflow controller

This file contains the instructions to build an example of Docker network function.

This is a container based on osrg/ryu docker container with additional python files for the elastic router control app.

### Creating the Docker image

You can create the Docker image with the network function by launching the following command:

    sudo docker build --tag="ctrl1" .

The name 'ctrl1' must be used for alignment with the name-resolver and nffg file.
This will create the Docker image starting from the base image specified in `Dockerfile`; the new image is stored in the Docker default folder on your filesystem (localhost).

The script start.sh is executed at startup and sets:
- ip addres (10.0.10.100/24) to a control interface eth0
- the cf-or interface is also made via this interface (connecting to 172.17.0.1/16 on docker0)
- start ryu application

This ip is hard coded for now, it should eventually be imported from the nffg.

