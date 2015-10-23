This is a simple network function, written in C, to be run in a Docker container.

When a packet is received on the interface eth0, its destination MAC address is 
changed, and the packet is sent back on the interface eth1

===============================================================================

Create the Docker image which contains this network function:

docker build --tag="localhost:5000/example" .
