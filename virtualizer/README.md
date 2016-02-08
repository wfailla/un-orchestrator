# Virtualizer

The virtualizer is an intermediate module sit between the un-orchestrator and 
the upper layer orchestrator. It operates as follows:
  * receives commands from the upper layer orchestrator based on the virtualizer 
    library defined by WP3;
  * converts those command in the formalism natively supported by the un-orchestrator
    (described in [../orchestrator/README_NF-FG.md](../orchestrator/README_NF-FG.md));
  * sends the command to the un-orchestrator through the API described in 

The name-resolver is a module that returns the list of all existing implementations [../orchestrator/README_RESTAPI.md](../orchestrator/README_RESTAPI.md)
of a network function.

## Required libraries

In addition to the libraries already listed in the main [../README_COMPILE.md](../README_COMPILE.md),
some more components are required to compile the un-orchestrator.

	; Retrieve the virtualizer library.
	$ cd [un-orchestrator]
	$ git submodule update --init --recursive

	; Install other required libraries 
	$ sudo apt-get install python-pip
	$ sudo pip install gunicorn falcon cython enum

# How to configure the virtualizer

The virtualizer reads its configuration from the file [./config/configuration.in](./config/configuration.in), 
which must be properly edited before starting the virtualizer itself.

# How to run the virtualizer

	$ gunicorn -b ip:port example:api
	
Please, note that the virtualizer requires the un-orchestrator and the 
name-resolver running in the server.
