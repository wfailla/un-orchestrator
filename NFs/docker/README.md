This folder contains examples of network functions implemented in Docker 
containers.

===============================================================================

To install the Docker environment for network functions, first follow the 
instructions provided here:

	http://docs.docker.com/installation/  

Then executes the following commands to properly configure the Docker environment:

	$ sudo apt-get install lxc -y  
	$ sudo su
	$ echo 'DOCKER_OPTS="-e lxc"' >> /etc/default/docker  
	$ exit
	$ sudo service docker restart

To check that everything was correct, run:

	$ sudo docker run hello-world

If you get an error, execute the following commands:

	$ sudo apt-get update
	$ sudo apt-get upgrade
	
===============================================================================

Please check individual README's in each sub-package.
