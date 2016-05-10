# UN Security Module

## Overview

Security Manager is the component that implements the security features available in the Universal Node. Currently, this module exposes the following features:
- User authentication
- User authorization (i.e. permissions)

## Security and permissions: database structure

Security and permissions with respect tot he basic CRUD operations are managed through a local database, whose structure is depicted below:

![database-structure](https://raw.githubusercontent.com/netgroup-polito/un-orchestrator/permissions/images/Database.JPG)

As shown in the diagram above, the local database is composed by the following tables:
- Users: stores general information about the subscribed users in the node orchestrator
- Login: keeps track of the users that have been authenticated in the system
- Generic resources: lists the available classes of resources
- Current resources permissions: lists the available resources with the related permissions
- User creation permissions: for each user, specify the permission for creating an instance of a generic resource
- Default usage permissions: specifies default permissions to be assigned to the resources belonging to a particular class (i.e. generic resource)

## Authentication
- The user sends an authentication request to the server through the REST API
- It authentication is successful, the server provides the user a token, which must be attached to each request

## Authorization
Once a user requests a Read/Update/Delete operation on a resource, the UN follows the steps depicted below.
![create-permissions](https://raw.githubusercontent.com/netgroup-polito/un-orchestrator/new_master/images/RUD.PNG)
For some operations, the "Perform operation" step may not need to access the UN database (e.g. read a graph).

Authorization handling is more complex when the user asks to create a new resource.
![rud-permissions](https://raw.githubusercontent.com/netgroup-polito/un-orchestrator/new_master/images/Creation.PNG)
