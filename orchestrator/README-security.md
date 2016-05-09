# UN Security Module

## Overview

Security Manager is the component that implements the security features available in the Universal Node. Currently, this module exposes the following features:
- User authentication
- User authorization (i.e. permissions)

## Authentication
- The user sends an authentication request to the server through the REST API
- It authentication is successful, the server provides the user a token, which must be attached to each request

## Authorization
Once a user requests a Read/Update/Delete operation on a resource, the UN follows the steps depicted below.
![create-permissions](https://raw.githubusercontent.com/netgroup-polito/un-orchestrator/new_master/images/RUD.PNG)
For some operations, the "Perform operation" step may not need to access the UN database (e.g. read a graph).

Authorization handling is more complex when the user asks to create a new resource.
![rud-permissions](https://raw.githubusercontent.com/netgroup-polito/un-orchestrator/new_master/images/Creation.PNG)
