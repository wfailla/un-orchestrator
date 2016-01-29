# How to compile the Universal Node

## Required libraries

Several libraries are required to compile the universal node.
In the following we list the steps required on an **Ubuntu 14.04**.

	; Install required libraries
	; - build-essential: it includes GCC, basic libraries, etc
	; - cmake: to create cross-platform makefiles
	; - cmake-curses-gui: nice 'gui' to edit cmake files
	; - libboost-all-dev: nice c++ library with tons of useful functions
	; - libmicrohttpd-dev: embedded micro http server
	; - libxml2-dev: nice library to parse and create xml
	$ sudo apt-get install build-essential cmake cmake-curses-gui libboost-all-dev libmicrohttpd-dev libxml2-dev
	
	; Install JSON Spirit (nice library to parse JSON files)
	$ git clone https://github.com/sirikata/json-spirit
	; alternatively, a copy of JSON Spirit is provided in [un-orchestrator]/contrib/json-spirit.zip
	$ cd json-spirit/


## Getting the code

The UN source code can be downloaded in two ways, depending on whether
you would like to be in sync with the UN main GIT repository or just 
download the sources, without bein syncronized with the repository.

In the first case if would be easy to get new updates from the repository
and possibly commit your changes back in the main branch; in the second
case you will have to do that manually.

Getting the code through GIT:

	; Checkout the main GIT repository
	$ git checkout https://github.com/netgroup-polito/un-orchestrator.git
	; Your code is now in the 'un-orchestrator' folder.

Simply downloading source code:

	; Download source code in the 'master' branch
	$ wget https://github.com/netgroup-polito/un-orchestrator/archive/master.zip
	; your code is now in the 'un-orchestrator-master' folder.

In order to compile and setup the different components of the universal node,
you have to follow the instruction that are provided in each subfolder.


