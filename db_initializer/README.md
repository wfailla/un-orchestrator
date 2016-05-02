#Database Initalizer


##Overview

Database Initializer is the module responsible for creating and populating the database of the Universal Node orchestrator. The tables involved are briefly described below:
- Users: specifies information about users (e.g. password, group, ...)
- Login: includes the users currently authenticated with login information
- User creation permissions: for each user, it defines the permission to create a resource belonging to a particular class
- Current resources permissions: includes the resources provided by the system plus permissions information
- Default usage permissions: default permissions for each class of resoruces

##How to use it

Compile:

	$ cmake .
	$ make
	
Run: 

	$ sudo ./db_initializer <default-user-password>
