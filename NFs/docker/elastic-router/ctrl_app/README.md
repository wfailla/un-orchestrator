## Docker-based example ryu openflow controller

This file contains the instructions to build an example of Docker network function.

This is a container based on osrg/ryu docker container with additional python files for the elastic router control app.

### Creating the Docker image

You can create the Docker image with the network function by launching the following command:

    sudo docker build --tag="ctrl" .

The name 'ctrl1' must be used for alignment with the name-resolver and nffg file.
This will create the Docker image starting from the base image specified in `Dockerfile`; the new image is stored in the Docker default folder on your filesystem (localhost).

The script start.sh is executed at startup and starts:
- ssh server
- ryu application
- DoubleDecker Proxy

IP address, Cf-Or interface address, are imported from the nffg.

### SSH access
The deployed containers expose a SSH login port to the public ip of the UN where they are deployed.
Port 9000 is forwarded to port 22 inside the ctrl_app container.

SSH login can be done via `ssh root@localhost -p 9000` (to reach ctrl1 container via exposed port 9000)
SSH login credentials for the VNF containers:
* user: root
* password: root


To avoid password entering, a public key can be added to the `authorized_keys` file.
The fingerprint of the container changes on each build. SSH login complains about this because the fingerprint stored in  .ssh/known_hosts has changed after a new build. (This can be a man-in-the-middle attack).

To avoid this error, the  fingerprint of the container can be removed from .ssh/known_hosts or this command can be used to login via ssh: `ssh -o StrictHostKeyChecking=no root@localhost -p 9000`
